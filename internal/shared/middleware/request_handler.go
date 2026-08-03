package middleware

import (
	"context"
	"fmt"
	"net/http"
	"time"

	"im/internal/shared/logger"

	"github.com/redis/go-redis/v9"
)

// RequestHandler 请求处理器（集成限流、超时、CORS）
type RequestHandler struct {
	rateLimiter *RateLimiter
	logger      *logger.Logger
}

// RateLimiter 限流器
type RateLimiter struct {
	redis  *redis.Client
	limit  int
	window time.Duration
}

// NewRateLimiter 创建限流器
func NewRateLimiter(redis *redis.Client, limit int, window time.Duration) *RateLimiter {
	return &RateLimiter{
		redis:  redis,
		limit:  limit,
		window: window,
	}
}

// Allow 检查是否允许请求（使用Redis分布式限流）
func (rl *RateLimiter) Allow(ip string) bool {
	ctx := context.Background()
	key := fmt.Sprintf("rate_limit:%s", ip)

	// 使用Redis的滑动窗口限流算法
	now := time.Now().Unix()
	windowStart := now - int64(rl.window.Seconds())

	// 清理过期的请求记录
	rl.redis.ZRemRangeByScore(ctx, key, "0", fmt.Sprintf("%d", windowStart))

	// 获取当前窗口内的请求数量
	count, err := rl.redis.ZCard(ctx, key).Result()
	if err != nil {
		// Redis错误时允许请求通过
		return true
	}

	// 检查是否超过限制
	if count >= int64(rl.limit) {
		return false
	}

	// 添加当前请求到滑动窗口
	member := fmt.Sprintf("%d:%d", now, time.Now().UnixNano())
	rl.redis.ZAdd(ctx, key, redis.Z{
		Score:  float64(now),
		Member: member,
	})

	// 设置key的过期时间
	rl.redis.Expire(ctx, key, rl.window)

	return true
}

// NewRequestHandler 创建请求处理器
func NewRequestHandler(logger *logger.Logger, redis *redis.Client) *RequestHandler {
	return &RequestHandler{
		rateLimiter: NewRateLimiter(redis, 100, time.Minute), // 100 req/min
		logger:      logger,
	}
}

// HandleRequest 处理请求（集成限流、超时、CORS）
func (rh *RequestHandler) HandleRequest(handler http.HandlerFunc) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		// 1. 限流检查
		ip := r.RemoteAddr
		if !rh.rateLimiter.Allow(ip) {
			rh.logger.Warnf("请求被限流: %s", ip)
			http.Error(w, "Too Many Requests", http.StatusTooManyRequests)
			return
		}

		// 2. CORS处理
		w.Header().Set("Access-Control-Allow-Origin", "*")
		w.Header().Set("Access-Control-Allow-Headers", "Content-Type")
		w.Header().Set("Access-Control-Allow-Methods", "POST, OPTIONS")
		w.Header().Set("Access-Control-Max-Age", "86400")

		// 预检请求优化
		if r.Method == "OPTIONS" {
			w.WriteHeader(http.StatusOK)
			return
		}

		// 3. 超时控制
		ctx, cancel := context.WithTimeout(r.Context(), 30*time.Second)
		defer cancel()

		r = r.WithContext(ctx)

		// 4. 执行处理器
		handler(w, r)
	}
}

// SetRateLimit 设置限流参数
func (rh *RequestHandler) SetRateLimit(redis *redis.Client, limit int, window time.Duration) {
	rh.rateLimiter = NewRateLimiter(redis, limit, window)
}
