# NodusIM

## 部署与运行指南

### 1. 环境准备

* Linux 或 macOS 开发环境，建议使用 Linux。
* Go 1.19 及以上版本。
* MySQL 8.0、Redis 6.2、MongoDB 4.4 与 etcd 3.5 及以上版本。
* 已安装 Traefik，且 `traefik` 命令可从终端直接调用。
* Python 3，用于启动本地静态 Web 页面。

后端依赖用途如下：

* **MySQL**：保存用户、好友关系、群组、成员与文件元数据。
* **Redis**：保存验证码、消息 Stream、近期消息缓存和离线提示，并通过 Pub/Sub 转发跨实例实时消息。
* **MongoDB**：保存私聊与群聊历史消息。
* **etcd**：保存服务实例注册信息，供服务间 RPC 发现实例。

---

### 2. 配置本地环境

复制配置模板并填写实际连接信息：

```bash
cp config.env.example config.env
```

至少配置 MySQL、Redis、MongoDB、JWT 与上传目录。若需要注册和密码重置流程，通知服务还需要配置邮件参数：

```text
EMAIL_HOST
EMAIL_PORT
EMAIL_USERNAME
EMAIL_PASSWORD
EMAIL_FROM
```

QQ 邮箱应填写 SMTP 授权码，而不是 QQ 登录密码。`config.env` 已被 Git 忽略，不会提交本地账号或密钥。

---

### 3. 启动后端服务

先确认 etcd 正在运行：

```bash
make check-etcd
```

构建并启动网关与六个服务：

```bash
make build
make start
```

启动脚本会运行 Traefik，并分别启动三实例的用户、好友、群组、消息、文件和通知服务。Traefik 网关监听 `8087` 端口。

---

### 4. 启动 Web 客户端

后端启动脚本不会托管静态页面。在另一个终端执行：

```bash
python3 -m http.server 8089 --directory /home/jianger/chatroom/web
```

浏览器访问 [http://127.0.0.1:8089/](http://127.0.0.1:8089/)。同一浏览器测试两个账号时，可在另一个端口启动同一目录，例如 `8086`，避免两个页面共享登录状态。

---

## 项目架构

```text
Web Client
    |
    | HTTP / WebSocket + Protobuf
    v
Traefik Gateway (:8087)
    |-- user-service          (:8090-8092)
    |-- friend-service        (:8100-8102)
    |-- group-service         (:8110-8112)
    |-- message-service       (:8120-8122)
    |-- file-service          (:8130-8132)
    `-- notification-service (:8140-8142)

etcd: 服务注册与 RPC 实例发现
MySQL: 关系型业务数据
MongoDB: 聊天历史
Redis: 验证码、Streams、近期消息缓存、离线提示、跨实例 Pub/Sub
```

---

## 1. 系统环境与架构

* **运行形态**：六个 Go 服务以独立进程启动，每个服务在本地运行三个实例。
* **接入层**：Traefik 根据请求前缀路由到对应服务，并在静态配置的实例间轮询分发。
* **服务发现**：服务启动后注册到 etcd；共享 RPC 管理器监听实例变化并复用 HTTP 客户端。
* **通信协议**：浏览器与后端、服务间调用均采用 Protobuf 编码；实时会话使用 WebSocket 二进制帧。
* **认证方式**：登录签发 JWT，受保护接口由共享中间件校验请求。

### 消息投递链路

普通私聊和群聊先生成稳定的消息 ID 并写入 Redis Streams。消费者组将事件持久化到 MongoDB，只有 MongoDB 写入成功才确认 Stream 消息；遗留 Pending 消息由存活消费者认领重试。MongoDB 是消息历史事实来源，Redis List 仅缓存近期会话消息。持久化完成后，在线收件人由连接管理器直接推送，本实例没有连接时通过 Redis Pub/Sub 转发到持有该连接的消息服务实例；实时投递失败的消息可通过历史接口在重连后补拉。

---

## 2. 主要功能概览

### 2.1 用户与通知

* 邮箱验证码注册、登录、用户信息查询、密码修改与重置。
* JWT 认证与通知服务邮件发送。

### 2.2 好友关系

* 好友申请、同意或拒绝、好友列表与删除好友。
* 好友备注与免打扰设置。

### 2.3 群组管理

* 创建群组、申请入群、成员列表与成员管理。
* 群主、管理员、普通成员角色，以及禁言和群消息免打扰设置。

### 2.4 实时消息

* WebSocket 私聊与群聊。
* Redis Streams 消费者组异步持久化，MongoDB `_id` 去重与 Pending 认领重试。
* Redis Pub/Sub 跨实例转发。
* MongoDB 历史消息、Redis 近期消息缓存与离线提示。

### 2.5 文件服务

* 文件上传、下载与网关转发访问。

---

## 3. 用户使用说明

1. 使用两个不同前端端口注册并登录两个账号。
2. 由账号 A 向账号 B 发送好友申请，在账号 B 的好友请求页面处理该申请。
3. 选择好友后发送私聊消息，验证对方实时收到消息。
4. 关闭账号 B 页面后继续发送消息，再次登录账号 B，验证离线投递与历史加载。
5. 创建群组、邀请或审批成员后发送群消息。
6. 通过文件按钮上传文件，使用返回地址下载文件。

---

## 4. 关键实现要点

* **服务拆分**：业务 Handler、Service、Storage 与 Model 按服务组织；共享配置、认证、数据库、发现、RPC 与 Protobuf 放在 `internal/shared`。
* **连接管理**：消息服务维护本地 WebSocket 连接，使用用户 ID 定位连接；跨实例投递通过 Redis 模式订阅完成。
* **存储分工**：MySQL 承担关系型业务状态，MongoDB 承担聊天历史，Redis 承担短生命周期状态和在线投递协作。
* **离线处理**：消息持久化成功后，若收件人不在线则写入离线列表；上线完成 WebSocket 登录后读取并清理该列表。
* **网关路由**：`/api/user`、`/api/friend`、`/api/group`、`/api/message`、`/api/file` 与 `/api/notification` 由 Traefik 去除前缀后转发。

---

## 5. 维护与注意事项

* 启动前确认 MySQL、Redis、MongoDB、etcd 与 Traefik 均可用。
* `scripts/start.sh` 只启动后端组件；Web 静态页面需单独启动。
* 当前 `make stop` 会按进程名清理本机 `etcd` 与 `traefik`。若机器上运行其他项目的同名进程，请不要直接执行该命令。
* `make clean` 会删除 `bin/`、`scripts/pids/` 与 `scripts/logs/`，不会删除业务数据库数据。
