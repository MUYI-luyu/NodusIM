package utils

import (
	"crypto/rand"
	"encoding/hex"
)

// GenerateGroupID 生成群组ID
func GenerateGroupID() string {
	bytes := make([]byte, 16)
	rand.Read(bytes)
	return "group_" + hex.EncodeToString(bytes)
}
