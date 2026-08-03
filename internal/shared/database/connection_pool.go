package database

import (
	"context"
	"database/sql"
	"fmt"
	"time"

	_ "github.com/go-sql-driver/mysql"
	"github.com/redis/go-redis/v9"
	"go.mongodb.org/mongo-driver/mongo"
	"go.mongodb.org/mongo-driver/mongo/options"
)

// ConnectionPool 数据库连接池
type ConnectionPool struct {
	MySQL   *sql.DB
	MongoDB *mongo.Client
	Redis   *redis.Client
}

// PoolConfig 连接池配置
type PoolConfig struct {
	MySQL MySQLConfig
	Mongo MongoConfig
	Redis RedisConfig
}

// MySQLConfig MySQL配置
type MySQLConfig struct {
	Host            string
	Port            int
	Username        string
	Password        string
	Database        string
	MaxOpenConns    int
	MaxIdleConns    int
	ConnMaxLifetime time.Duration
	ConnMaxIdleTime time.Duration
}

// MongoConfig MongoDB配置
type MongoConfig struct {
	URI             string
	Database        string
	MaxPoolSize     uint64
	MinPoolSize     uint64
	MaxConnIdleTime time.Duration
}

// RedisConfig Redis配置
type RedisConfig struct {
	Host         string
	Port         int
	Password     string
	DB           int
	PoolSize     int
	MinIdleConns int
	MaxRetries   int
}

// NewConnectionPool 创建连接池
func NewConnectionPool(config PoolConfig) (*ConnectionPool, error) {
	pool := &ConnectionPool{}

	// 初始化MySQL连接池
	if err := pool.initMySQL(config.MySQL); err != nil {
		return nil, fmt.Errorf("初始化MySQL连接池失败: %v", err)
	}

	// 初始化MongoDB连接池
	if err := pool.initMongoDB(config.Mongo); err != nil {
		return nil, fmt.Errorf("初始化MongoDB连接池失败: %v", err)
	}

	// 初始化Redis连接池
	if err := pool.initRedis(config.Redis); err != nil {
		return nil, fmt.Errorf("初始化Redis连接池失败: %v", err)
	}

	return pool, nil
}

// initMySQL 初始化MySQL连接池
func (p *ConnectionPool) initMySQL(config MySQLConfig) error {
	dsn := fmt.Sprintf("%s:%s@tcp(%s:%d)/%s?charset=utf8mb4&parseTime=True&loc=Local&timeout=10s&readTimeout=30s&writeTimeout=30s",
		config.Username, config.Password, config.Host, config.Port, config.Database)

	db, err := sql.Open("mysql", dsn)
	if err != nil {
		return err
	}

	// 设置连接池参数
	db.SetMaxOpenConns(config.MaxOpenConns)
	db.SetMaxIdleConns(config.MaxIdleConns)
	db.SetConnMaxLifetime(config.ConnMaxLifetime)
	db.SetConnMaxIdleTime(config.ConnMaxIdleTime)

	// 测试连接
	if err := db.Ping(); err != nil {
		return err
	}

	p.MySQL = db
	return nil
}

// initMongoDB 初始化MongoDB连接池
func (p *ConnectionPool) initMongoDB(config MongoConfig) error {
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()

	clientOptions := options.Client().ApplyURI(config.URI)
	clientOptions.SetMaxPoolSize(uint16(config.MaxPoolSize))
	clientOptions.SetMaxConnIdleTime(config.MaxConnIdleTime)

	client, err := mongo.Connect(ctx, clientOptions)
	if err != nil {
		return err
	}

	// 测试连接
	if err := client.Ping(ctx, nil); err != nil {
		return err
	}

	p.MongoDB = client
	return nil
}

// initRedis 初始化Redis连接池
func (p *ConnectionPool) initRedis(config RedisConfig) error {
	rdb := redis.NewClient(&redis.Options{
		Addr:         fmt.Sprintf("%s:%d", config.Host, config.Port),
		Password:     config.Password,
		DB:           config.DB,
		PoolSize:     config.PoolSize,
		MinIdleConns: config.MinIdleConns,
		MaxRetries:   config.MaxRetries,
		DialTimeout:  5 * time.Second,
		ReadTimeout:  3 * time.Second,
		WriteTimeout: 3 * time.Second,
		PoolTimeout:  4 * time.Second,
	})

	// 测试连接
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	if err := rdb.Ping(ctx).Err(); err != nil {
		return err
	}

	p.Redis = rdb
	return nil
}

// Close 关闭所有连接
func (p *ConnectionPool) Close() error {
	var errs []error

	if p.MySQL != nil {
		if err := p.MySQL.Close(); err != nil {
			errs = append(errs, err)
		}
	}

	if p.MongoDB != nil {
		ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
		defer cancel()
		if err := p.MongoDB.Disconnect(ctx); err != nil {
			errs = append(errs, err)
		}
	}

	if p.Redis != nil {
		if err := p.Redis.Close(); err != nil {
			errs = append(errs, err)
		}
	}

	if len(errs) > 0 {
		return fmt.Errorf("关闭连接时发生错误: %v", errs)
	}

	return nil
}

// GetDefaultConfig 获取默认配置
func GetDefaultConfig() PoolConfig {
	return PoolConfig{
		MySQL: MySQLConfig{
			MaxOpenConns:    100,
			MaxIdleConns:    10,
			ConnMaxLifetime: 1 * time.Hour,
			ConnMaxIdleTime: 30 * time.Minute,
		},
		Mongo: MongoConfig{
			MaxPoolSize:     100,
			MinPoolSize:     5,
			MaxConnIdleTime: 30 * time.Minute,
		},
		Redis: RedisConfig{
			PoolSize:     100,
			MinIdleConns: 10,
			MaxRetries:   3,
		},
	}
}
