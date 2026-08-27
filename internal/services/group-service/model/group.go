package model

// Group 群组模型
type Group struct {
	GroupID   string `json:"group_id" db:"group_id"`
	Name      string `json:"name" db:"name"`
	OwnerUID  string `json:"owner_uid" db:"owner_uid"`
	Remark    string `json:"remark" db:"remark"` // 用户对该群组的备注
	CreatedAt int64  `json:"created_at" db:"created_at"`
	UpdatedAt int64  `json:"updated_at" db:"updated_at"`
}

// GroupMember 群组成员模型
type GroupMember struct {
	GroupID  string `json:"group_id" db:"group_id"`
	UID      string `json:"uid" db:"uid"`
	Role     string `json:"role" db:"role"` // owner, admin, member
	Nickname string `json:"nickname" db:"nickname"`
	Remark   string `json:"remark" db:"remark"`
	DND      bool   `json:"dnd" db:"dnd"`
	Muted    bool   `json:"muted" db:"muted"`
	JoinTime int64  `json:"join_time" db:"join_time"`
}

// GroupInviteRequest 群组邀请请求模型
type GroupInviteRequest struct {
	ID         int    `json:"id" db:"id"`
	GroupID    string `json:"group_id" db:"group_id"`
	InviterUID string `json:"inviter_uid" db:"inviter_uid"`
	InviteeUID string `json:"invitee_uid" db:"invitee_uid"`
	Status     string `json:"status" db:"status"` // pending, approved, rejected
	CreatedAt  int64  `json:"created_at" db:"created_at"`
	UpdatedAt  int64  `json:"updated_at" db:"updated_at"`
}
