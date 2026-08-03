package discovery

import (
	"context"
	"fmt"
	"net"
	"strings"
	"time"

	"go.etcd.io/etcd/api/v3/mvccpb"
	clientv3 "go.etcd.io/etcd/client/v3"
)

// Config 服务发现配置
type Config struct {
	Endpoints []string
}

// Client etcd客户端
type Client struct {
	client *clientv3.Client
}

// Registrar 服务注册器
type Registrar struct {
	client *clientv3.Client
}

// Instance 服务实例
type Instance struct {
	Address string
}

// New 创建新的etcd客户端
func New(cfg Config) (*Client, error) {
	client, err := clientv3.New(clientv3.Config{
		Endpoints:   cfg.Endpoints,
		DialTimeout: 5 * time.Second,
	})
	if err != nil {
		return nil, err
	}
	return &Client{client: client}, nil
}

// Register 注册服务
func (r *Registrar) Register(client *Client, prefix, serviceName, ip string, port int, ttl int) error {
	r.client = client.client
	key := fmt.Sprintf("%s/%s/%s:%d", prefix, serviceName, ip, port)
	value := fmt.Sprintf("%s:%d", ip, port)

	// 创建租约
	lease, err := r.client.Grant(context.Background(), int64(ttl))
	if err != nil {
		return err
	}

	// 注册服务
	_, err = r.client.Put(context.Background(), key, value, clientv3.WithLease(lease.ID))
	if err != nil {
		return err
	}

	// 保持租约活跃
	ch, kaerr := r.client.KeepAlive(context.Background(), lease.ID)
	if kaerr != nil {
		return kaerr
	}

	// 启动goroutine处理租约续期
	go func() {
		for ka := range ch {
			_ = ka // 忽略响应
		}
	}()

	return nil
}

// Deregister 注销服务
func (r *Registrar) Deregister() {
	if r.client != nil {
		r.client.Close()
	}
}

// ListInstances 列出某服务的所有实例
func (c *Client) ListInstances(ctx context.Context, prefix, serviceName string) ([]Instance, error) {
	keyPrefix := fmt.Sprintf("%s/%s/", prefix, serviceName)
	resp, err := c.client.Get(ctx, keyPrefix, clientv3.WithPrefix())
	if err != nil {
		return nil, err
	}
	var result []Instance
	for _, kv := range resp.Kvs {
		if kv == nil || kv.Value == nil {
			continue
		}
		addr := string(kv.Value)
		if strings.TrimSpace(addr) == "" {
			continue
		}
		result = append(result, Instance{Address: addr})
	}
	return result, nil
}

// WatchService 持续监听某服务的实例变化，并在每次变化时回调返回完整实例列表
func (c *Client) WatchService(ctx context.Context, prefix, serviceName string, onChange func([]Instance)) error {
	keyPrefix := fmt.Sprintf("%s/%s/", prefix, serviceName)

	// 初次加载
	instances, err := c.ListInstances(ctx, prefix, serviceName)
	if err == nil {
		onChange(instances)
	}

	watchChan := c.client.Watch(ctx, keyPrefix, clientv3.WithPrefix())
	go func() {
		for {
			select {
			case <-ctx.Done():
				return
			case wr, ok := <-watchChan:
				if !ok {
					return
				}
				// 仅当有PUT/DELETE时，重新列举
				changed := false
				for _, ev := range wr.Events {
					if ev.Type == mvccpb.PUT || ev.Type == mvccpb.DELETE {
						changed = true
						break
					}
				}
				if changed {
					list, err := c.ListInstances(context.Background(), prefix, serviceName)
					if err == nil {
						onChange(list)
					}
				}
			}
		}
	}()
	return nil
}

// Close 关闭客户端
func (c *Client) Close() error {
	if c.client != nil {
		return c.client.Close()
	}
	return nil
}

// GetOutboundIP 获取本机外网IP
func GetOutboundIP() string {
	conn, err := net.Dial("udp", "8.8.8.8:80")
	if err != nil {
		return "127.0.0.1"
	}
	defer conn.Close()

	localAddr := conn.LocalAddr().(*net.UDPAddr)
	return localAddr.IP.String()
}
