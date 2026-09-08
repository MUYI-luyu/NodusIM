package utils

import (
	"crypto/rand"
	"encoding/hex"
	"fmt"

	"go.mongodb.org/mongo-driver/bson"
)

// GenerateMessageID 生成消息ID
func GenerateMessageID() string {
	bytes := make([]byte, 16)
	rand.Read(bytes)
	return "msg_" + hex.EncodeToString(bytes)
}

// GenerateSessionKey 生成私聊会话键
func GenerateSessionKey(from, to string) string {
	// 确保会话键的一致性，按字母顺序排序
	if from < to {
		return fmt.Sprintf("chat:%s:%s", from, to)
	}
	return fmt.Sprintf("chat:%s:%s", to, from)
}

// GenerateGroupSessionKey 生成群聊会话键
func GenerateGroupSessionKey(groupID string) string {
	return fmt.Sprintf("group:%s", groupID)
}

// MarshalMessage 序列化消息
func MarshalMessage(msg interface{}) ([]byte, error) {
	return bson.Marshal(msg)
}

// UnmarshalMessage 反序列化消息
func UnmarshalMessage(data []byte, msg interface{}) error {
	return bson.Unmarshal(data, msg)
}
