package rpc

import (
	"context"
	"fmt"
	"sync"

	"im/internal/shared/discovery"
	"im/internal/shared/logger"

	"google.golang.org/protobuf/proto"
)

// Manager RPC客户端池管理器
type Manager struct {
	clientPool *ClientPool
	logger     *logger.Logger
	mu         sync.RWMutex

	// etcd discovery
	discClient *discovery.Client
	etcdPrefix string

	// dynamic instances
	serviceInstances map[string][]string // service -> addresses
	rrIndex          map[string]int      // round-robin index per service
}

// NewManager 创建RPC管理器
func NewManager(logger *logger.Logger) *Manager {
	return &Manager{
		clientPool:       NewClientPool(),
		logger:           logger,
		serviceInstances: make(map[string][]string),
		rrIndex:          make(map[string]int),
	}
}

// UseEtcd 启用基于etcd的服务发现
func (m *Manager) UseEtcd(disc *discovery.Client, prefix string) {
	m.mu.Lock()
	defer m.mu.Unlock()
	m.discClient = disc
	m.etcdPrefix = prefix
}

// WatchService 订阅某服务的实例列表
func (m *Manager) WatchService(serviceName string) error {
	m.mu.RLock()
	disc := m.discClient
	prefix := m.etcdPrefix
	m.mu.RUnlock()
	if disc == nil {
		return fmt.Errorf("etcd discovery not enabled")
	}
	ctx := context.Background()
	return disc.WatchService(ctx, prefix, serviceName, func(instances []discovery.Instance) {
		addrs := make([]string, 0, len(instances))
		for _, ins := range instances {
			addrs = append(addrs, ins.Address)
		}
		m.mu.Lock()
		m.serviceInstances[serviceName] = addrs
		if _, ok := m.rrIndex[serviceName]; !ok {
			m.rrIndex[serviceName] = 0
		}
		m.mu.Unlock()
		m.logger.Infof("[discovery] %s 实例更新: %v", serviceName, addrs)
	})
}

// pickAddress 选择一个实例地址（来自etcd）
func (m *Manager) pickAddress(serviceName string) (string, error) {
	m.mu.Lock()
	defer m.mu.Unlock()

	// etcd addresses
	if addrs, ok := m.serviceInstances[serviceName]; ok && len(addrs) > 0 {
		idx := m.rrIndex[serviceName] % len(addrs)
		addr := addrs[idx]
		m.rrIndex[serviceName] = (m.rrIndex[serviceName] + 1) % len(addrs)
		return addr, nil
	}

	return "", fmt.Errorf("服务 %s 没有可用实例 (etcd)", serviceName)
}

// GetClient 获取服务客户端
func (m *Manager) GetClient(serviceName string) (*ServiceClient, error) {
	address, err := m.pickAddress(serviceName)
	if err != nil {
		return nil, err
	}
	return m.clientPool.GetClient(serviceName, address), nil
}

// Call 调用服务
func (m *Manager) Call(ctx context.Context, serviceName, endpoint string, req interface{}) ([]byte, error) {
	client, err := m.GetClient(serviceName)
	if err != nil {
		return nil, err
	}

	// 如果请求是proto.Message类型，直接调用
	if protoReq, ok := req.(proto.Message); ok {
		return client.Call(ctx, endpoint, protoReq)
	}

	return nil, fmt.Errorf("不支持的请求类型")
}

// CallWithRetry 带重试的调用
func (m *Manager) CallWithRetry(ctx context.Context, serviceName, endpoint string, req interface{}, maxRetries int) ([]byte, error) {
	client, err := m.GetClient(serviceName)
	if err != nil {
		return nil, err
	}

	if protoReq, ok := req.(proto.Message); ok {
		return client.CallWithRetry(ctx, endpoint, protoReq, maxRetries)
	}

	return nil, fmt.Errorf("不支持的请求类型")
}

// Close 关闭管理器
func (m *Manager) Close() error {
	// 客户端池会自动管理连接，这里不需要特殊处理
	return nil
}
