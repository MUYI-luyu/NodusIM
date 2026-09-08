package model

import "time"

// IMMessage 即时消息模型
type IMMessage struct {
	ID        string    `json:"id" bson:"_id"`
	From      string    `json:"from" bson:"from"`
	To        string    `json:"to" bson:"to"`
	Type      string    `json:"type" bson:"type"` // text, image, file, system
	Content   string    `json:"content" bson:"content"`
	Extra     string    `json:"extra" bson:"extra"`
	Timestamp int64     `json:"timestamp" bson:"timestamp"`
	CreatedAt time.Time `json:"created_at" bson:"created_at"`
}

// PrivateMessage 私聊消息
type PrivateMessage struct {
	IMMessage
	SessionKey string `json:"session_key" bson:"session_key"`
}

// GroupMessage 群聊消息
type GroupMessage struct {
	IMMessage
	GroupID string `json:"group_id" bson:"group_id"`
}
