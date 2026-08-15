package model

// Friend 好友关系模型
type Friend struct {
	ID        int    `json:"id" db:"id"`
	UserUID   string `json:"user_uid" db:"user_uid"`
	FriendUID string `json:"friend_uid" db:"friend_uid"`
	Remark    string `json:"remark" db:"remark"`
	DND       bool   `json:"dnd" db:"dnd"` // 免打扰
	CreatedAt int64  `json:"created_at" db:"created_at"`
	UpdatedAt int64  `json:"updated_at" db:"updated_at"`
}

// FriendRequest 好友请求模型
type FriendRequest struct {
	ID        int    `json:"id" db:"id"`
	FromUID   string `json:"from_uid" db:"from_uid"`
	ToUID     string `json:"to_uid" db:"to_uid"`
	Message   string `json:"message" db:"message"`
	Status    string `json:"status" db:"status"` // pending, accepted, rejected
	CreatedAt int64  `json:"created_at" db:"created_at"`
	UpdatedAt int64  `json:"updated_at" db:"updated_at"`
}
