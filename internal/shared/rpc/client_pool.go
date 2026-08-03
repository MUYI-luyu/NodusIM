package rpc

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"net/http"
	"strings"
	"sync"
	"time"

	"google.golang.org/protobuf/proto"
)

// ServiceClient 服务客户端
type ServiceClient struct {
	baseURL    string
	httpClient *http.Client
	timeout    time.Duration
}

// ClientPool 客户端连接池
type ClientPool struct {
	clients map[string]*ServiceClient
	mu      sync.RWMutex
}

// NewClientPool 创建客户端池
func NewClientPool() *ClientPool {
	return &ClientPool{
		clients: make(map[string]*ServiceClient),
	}
}

// GetClient 获取服务客户端
func (p *ClientPool) GetClient(serviceName, baseURL string) *ServiceClient {
	p.mu.Lock()
	defer p.mu.Unlock()

	// 规范化地址，缺少协议时默认 http
	if !strings.HasPrefix(baseURL, "http://") && !strings.HasPrefix(baseURL, "https://") {
		baseURL = "http://" + baseURL
	}

	key := serviceName + "|" + baseURL
	if client, exists := p.clients[key]; exists {
		return client
	}

	client := &ServiceClient{
		baseURL: baseURL,
		httpClient: &http.Client{
			Transport: &http.Transport{
				MaxIdleConns:        100,
				MaxIdleConnsPerHost: 10,
				IdleConnTimeout:     90 * time.Second,
				DisableKeepAlives:   false,
			},
			Timeout: 30 * time.Second,
		},
		timeout: 30 * time.Second,
	}

	p.clients[key] = client
	return client
}

// Call 调用服务
func (c *ServiceClient) Call(ctx context.Context, endpoint string, req proto.Message) ([]byte, error) {
	// 序列化请求
	reqData, err := proto.Marshal(req)
	if err != nil {
		return nil, fmt.Errorf("序列化请求失败: %v", err)
	}

	// 创建HTTP请求
	url := c.baseURL + endpoint
	httpReq, err := http.NewRequestWithContext(ctx, "POST", url, bytes.NewReader(reqData))
	if err != nil {
		return nil, fmt.Errorf("创建请求失败: %v", err)
	}

	httpReq.Header.Set("Content-Type", "application/x-protobuf")
	httpReq.Header.Set("Accept", "application/x-protobuf")

	// 发送请求
	resp, err := c.httpClient.Do(httpReq)
	if err != nil {
		return nil, fmt.Errorf("发送请求失败: %v", err)
	}
	defer resp.Body.Close()

	// 读取响应
	respData, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("读取响应失败: %v", err)
	}

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("服务返回错误状态: %d", resp.StatusCode)
	}

	return respData, nil
}

// CallWithRetry 带重试的调用
func (c *ServiceClient) CallWithRetry(ctx context.Context, endpoint string, req proto.Message, maxRetries int) ([]byte, error) {
	var lastErr error

	for i := 0; i <= maxRetries; i++ {
		if i > 0 {
			// 指数退避
			backoff := time.Duration(i*i) * 100 * time.Millisecond
			select {
			case <-ctx.Done():
				return nil, ctx.Err()
			case <-time.After(backoff):
			}
		}

		resp, err := c.Call(ctx, endpoint, req)
		if err == nil {
			return resp, nil
		}

		lastErr = err
	}

	return nil, fmt.Errorf("重试 %d 次后仍然失败: %v", maxRetries, lastErr)
}

// CircuitBreaker 熔断器
type CircuitBreaker struct {
	failureCount    int
	lastFailureTime time.Time
	state           State
	mu              sync.RWMutex
	threshold       int
	timeout         time.Duration
}

// State 熔断器状态
type State int

const (
	StateClosed State = iota
	StateOpen
	StateHalfOpen
)

// NewCircuitBreaker 创建熔断器
func NewCircuitBreaker(threshold int, timeout time.Duration) *CircuitBreaker {
	return &CircuitBreaker{
		threshold: threshold,
		timeout:   timeout,
		state:     StateClosed,
	}
}

// Call 熔断器调用
func (cb *CircuitBreaker) Call(fn func() error) error {
	cb.mu.Lock()
	defer cb.mu.Unlock()

	// 检查熔断器状态
	if cb.state == StateOpen {
		if time.Since(cb.lastFailureTime) > cb.timeout {
			cb.state = StateHalfOpen
		} else {
			return fmt.Errorf("熔断器开启")
		}
	}

	// 执行函数
	err := fn()

	if err != nil {
		cb.failureCount++
		cb.lastFailureTime = time.Now()

		if cb.failureCount >= cb.threshold {
			cb.state = StateOpen
		}
		return err
	}

	// 成功时重置计数器
	cb.failureCount = 0
	cb.state = StateClosed
	return nil
}

// ServiceRegistry 服务注册表
type ServiceRegistry struct {
	services map[string]string
	mu       sync.RWMutex
}

// NewServiceRegistry 创建服务注册表
func NewServiceRegistry() *ServiceRegistry {
	return &ServiceRegistry{
		services: make(map[string]string),
	}
}

// Register 注册服务
func (sr *ServiceRegistry) Register(serviceName, address string) {
	sr.mu.Lock()
	defer sr.mu.Unlock()
	sr.services[serviceName] = address
}

// Get 获取服务地址
func (sr *ServiceRegistry) Get(serviceName string) (string, bool) {
	sr.mu.RLock()
	defer sr.mu.RUnlock()
	address, exists := sr.services[serviceName]
	return address, exists
}

// List 列出所有服务
func (sr *ServiceRegistry) List() map[string]string {
	sr.mu.RLock()
	defer sr.mu.RUnlock()

	result := make(map[string]string)
	for k, v := range sr.services {
		result[k] = v
	}
	return result
}
