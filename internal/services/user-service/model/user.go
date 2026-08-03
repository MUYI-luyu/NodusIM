package model

// User 用户模型
type User struct {
	UID       string `json:"uid" db:"uid"`
	Username  string `json:"username" db:"username"`
	Password  string `json:"-" db:"password"` // 密码不序列化
	Email     string `json:"email" db:"email"`
	CreatedAt int64  `json:"created_at" db:"created_at"`
	UpdatedAt int64  `json:"updated_at" db:"updated_at"`
}
