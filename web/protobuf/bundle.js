/*eslint-disable block-scoped-var, id-length, no-control-regex, no-magic-numbers, no-prototype-builtins, no-redeclare, no-shadow, no-var, sort-vars*/
"use strict";

var $protobuf = require("protobufjs/light");

var $root = ($protobuf.roots["default"] || ($protobuf.roots["default"] = new $protobuf.Root()))
.addJSON({
  protocol: {
    options: {
      go_package: "im/core/protocol/pb;pb"
    },
    nested: {
      Group: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          },
          name: {
            type: "string",
            id: 2
          },
          description: {
            type: "string",
            id: 3
          },
          ownerUid: {
            type: "string",
            id: 4
          },
          memberUids: {
            rule: "repeated",
            type: "string",
            id: 5
          },
          createdAt: {
            type: "int64",
            id: 6
          },
          updatedAt: {
            type: "int64",
            id: 7
          },
          remark: {
            type: "string",
            id: 8
          }
        }
      },
      GroupMember: {
        fields: {
          uid: {
            type: "string",
            id: 1
          },
          username: {
            type: "string",
            id: 2
          },
          nickname: {
            type: "string",
            id: 3
          },
          role: {
            type: "string",
            id: 4
          },
          joinTime: {
            type: "int64",
            id: 5
          },
          muted: {
            type: "bool",
            id: 6
          }
        }
      },
      GroupRequestItem: {
        fields: {
          id: {
            type: "string",
            id: 1
          },
          groupId: {
            type: "string",
            id: 2
          },
          groupName: {
            type: "string",
            id: 3
          },
          inviterUid: {
            type: "string",
            id: 4
          },
          inviteeUid: {
            type: "string",
            id: 5
          },
          status: {
            type: "string",
            id: 6
          },
          createdAt: {
            type: "string",
            id: 7
          },
          inviterUsername: {
            type: "string",
            id: 8
          },
          inviteeUsername: {
            type: "string",
            id: 9
          }
        }
      },
      CreateGroupReq: {
        fields: {
          name: {
            type: "string",
            id: 1
          },
          description: {
            type: "string",
            id: 2
          },
          ownerUid: {
            type: "string",
            id: 3
          },
          memberUids: {
            rule: "repeated",
            type: "string",
            id: 4
          }
        }
      },
      CreateGroupResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          },
          groupId: {
            type: "string",
            id: 3
          }
        }
      },
      JoinGroupReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          },
          uid: {
            type: "string",
            id: 2
          }
        }
      },
      JoinGroupResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          }
        }
      },
      GroupListReq: {
        fields: {
          uid: {
            type: "string",
            id: 1
          }
        }
      },
      GroupListResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          },
          groups: {
            rule: "repeated",
            type: "Group",
            id: 3
          }
        }
      },
      GroupInfoReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          }
        }
      },
      GroupInfoResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          },
          group: {
            type: "Group",
            id: 3
          }
        }
      },
      GroupMembersReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          }
        }
      },
      GroupMembersResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          },
          members: {
            rule: "repeated",
            type: "GroupMember",
            id: 3
          }
        }
      },
      LeaveGroupReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          },
          uid: {
            type: "string",
            id: 2
          }
        }
      },
      LeaveGroupResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          }
        }
      },
      GroupMemberRoleReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          },
          uid: {
            type: "string",
            id: 2
          }
        }
      },
      GroupMemberRoleResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          },
          role: {
            type: "string",
            id: 3
          }
        }
      },
      GroupMemberInfoReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          },
          uid: {
            type: "string",
            id: 2
          }
        }
      },
      GroupMemberInfoResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          },
          uid: {
            type: "string",
            id: 3
          },
          username: {
            type: "string",
            id: 4
          },
          nickname: {
            type: "string",
            id: 5
          },
          role: {
            type: "string",
            id: 6
          },
          joinTime: {
            type: "int64",
            id: 7
          }
        }
      },
      InviteToGroupReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          },
          inviterUid: {
            type: "string",
            id: 2
          },
          inviteeUids: {
            rule: "repeated",
            type: "string",
            id: 3
          }
        }
      },
      InviteToGroupResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          }
        }
      },
      SetGroupNicknameReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          },
          uid: {
            type: "string",
            id: 2
          },
          nickname: {
            type: "string",
            id: 3
          }
        }
      },
      SetGroupNicknameResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          }
        }
      },
      SetGroupRemarkReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          },
          uid: {
            type: "string",
            id: 2
          },
          remark: {
            type: "string",
            id: 3
          }
        }
      },
      SetGroupRemarkResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          }
        }
      },
      SetGroupDNDReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          },
          uid: {
            type: "string",
            id: 2
          },
          dnd: {
            type: "bool",
            id: 3
          }
        }
      },
      SetGroupDNDResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          },
          dnd: {
            type: "bool",
            id: 3
          }
        }
      },
      SetGroupMuteReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          },
          operatorUid: {
            type: "string",
            id: 2
          },
          targetUid: {
            type: "string",
            id: 3
          },
          mute: {
            type: "bool",
            id: 4
          }
        }
      },
      SetGroupMuteResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          }
        }
      },
      KickFromGroupReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          },
          operatorUid: {
            type: "string",
            id: 2
          },
          targetUid: {
            type: "string",
            id: 3
          }
        }
      },
      KickFromGroupResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          }
        }
      },
      UpdateGroupNameReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          },
          operatorUid: {
            type: "string",
            id: 2
          },
          newName: {
            type: "string",
            id: 3
          }
        }
      },
      UpdateGroupNameResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          }
        }
      },
      SetGroupAdminReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          },
          operatorUid: {
            type: "string",
            id: 2
          },
          targetUid: {
            type: "string",
            id: 3
          },
          setAdmin: {
            type: "bool",
            id: 4
          }
        }
      },
      SetGroupAdminResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          }
        }
      },
      DismissGroupReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          },
          operatorUid: {
            type: "string",
            id: 2
          }
        }
      },
      DismissGroupResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          }
        }
      },
      GroupRequestListReq: {
        fields: {
          uid: {
            type: "string",
            id: 1
          },
          token: {
            type: "string",
            id: 2
          }
        }
      },
      GroupRequestListResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          },
          items: {
            rule: "repeated",
            type: "GroupRequestItem",
            id: 3
          }
        }
      },
      HandleGroupRequestReq: {
        fields: {
          id: {
            type: "string",
            id: 1
          },
          token: {
            type: "string",
            id: 2
          },
          approve: {
            type: "bool",
            id: 3
          },
          groupId: {
            type: "string",
            id: 4
          },
          inviteeUid: {
            type: "string",
            id: 5
          }
        }
      },
      RegisterReq: {
        fields: {
          username: {
            type: "string",
            id: 1
          },
          password: {
            type: "string",
            id: 2
          },
          email: {
            type: "string",
            id: 3
          },
          emailCode: {
            type: "string",
            id: 4
          }
        }
      },
      LoginReq: {
        fields: {
          uid: {
            type: "string",
            id: 1
          },
          password: {
            type: "string",
            id: 2
          }
        }
      },
      LogoutReq: {
        fields: {
          token: {
            type: "string",
            id: 1
          }
        }
      },
      UserInfoReq: {
        fields: {
          token: {
            type: "string",
            id: 1
          }
        }
      },
      UserInfoResp: {
        fields: {
          uid: {
            type: "string",
            id: 1
          },
          username: {
            type: "string",
            id: 2
          },
          email: {
            type: "string",
            id: 3
          },
          code: {
            type: "int32",
            id: 4
          },
          msg: {
            type: "string",
            id: 5
          }
        }
      },
      UpdateUsernameReq: {
        fields: {
          uid: {
            type: "string",
            id: 1
          },
          newUsername: {
            type: "string",
            id: 2
          }
        }
      },
      UpdatePwdReq: {
        fields: {
          uid: {
            type: "string",
            id: 1
          },
          oldPwd: {
            type: "string",
            id: 2
          },
          newPwd: {
            type: "string",
            id: 3
          }
        }
      },
      DeleteAccountReq: {
        fields: {
          uid: {
            type: "string",
            id: 1
          }
        }
      },
      TokenCheckReq: {
        fields: {
          token: {
            type: "string",
            id: 1
          }
        }
      },
      SendEmailCodeReq: {
        fields: {
          email: {
            type: "string",
            id: 1
          },
          purpose: {
            type: "string",
            id: 2
          }
        }
      },
      ResetPasswordReq: {
        fields: {
          email: {
            type: "string",
            id: 1
          },
          emailCode: {
            type: "string",
            id: 2
          },
          newPassword: {
            type: "string",
            id: 3
          }
        }
      },
      AddFriendReq: {
        fields: {
          fromUid: {
            type: "string",
            id: 1
          },
          toUid: {
            type: "string",
            id: 2
          },
          verifyMsg: {
            type: "string",
            id: 3
          },
          token: {
            type: "string",
            id: 4
          }
        }
      },
      AddFriendResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          }
        }
      },
      FriendListReq: {
        fields: {
          uid: {
            type: "string",
            id: 1
          },
          token: {
            type: "string",
            id: 2
          }
        }
      },
      FriendListResp: {
        fields: {
          friendUids: {
            rule: "repeated",
            type: "string",
            id: 1
          },
          friendUsernames: {
            rule: "repeated",
            type: "string",
            id: 2
          },
          code: {
            type: "int32",
            id: 3
          },
          msg: {
            type: "string",
            id: 4
          },
          remarks: {
            rule: "repeated",
            type: "string",
            id: 5
          }
        }
      },
      FriendInfoReq: {
        fields: {
          uid: {
            type: "string",
            id: 1
          },
          friendUid: {
            type: "string",
            id: 2
          },
          token: {
            type: "string",
            id: 3
          }
        }
      },
      FriendInfoResp: {
        fields: {
          uid: {
            type: "string",
            id: 1
          },
          username: {
            type: "string",
            id: 2
          },
          email: {
            type: "string",
            id: 3
          },
          remark: {
            type: "string",
            id: 4
          },
          dnd: {
            type: "bool",
            id: 5
          },
          code: {
            type: "int32",
            id: 6
          },
          msg: {
            type: "string",
            id: 7
          }
        }
      },
      UpdateFriendRemarkReq: {
        fields: {
          uid: {
            type: "string",
            id: 1
          },
          friendUid: {
            type: "string",
            id: 2
          },
          remark: {
            type: "string",
            id: 3
          },
          token: {
            type: "string",
            id: 4
          }
        }
      },
      UpdateFriendRemarkResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          }
        }
      },
      SetFriendDNDReq: {
        fields: {
          uid: {
            type: "string",
            id: 1
          },
          friendUid: {
            type: "string",
            id: 2
          },
          dnd: {
            type: "bool",
            id: 3
          },
          token: {
            type: "string",
            id: 4
          }
        }
      },
      SetFriendDNDResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          }
        }
      },
      DeleteFriendReq: {
        fields: {
          uid: {
            type: "string",
            id: 1
          },
          friendUid: {
            type: "string",
            id: 2
          },
          token: {
            type: "string",
            id: 3
          }
        }
      },
      DeleteFriendResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          }
        }
      },
      FriendRequestListResp: {
        fields: {
          fromUids: {
            rule: "repeated",
            type: "string",
            id: 1
          },
          fromUsernames: {
            rule: "repeated",
            type: "string",
            id: 2
          },
          verifyMsgs: {
            rule: "repeated",
            type: "string",
            id: 3
          },
          code: {
            type: "int32",
            id: 4
          },
          msg: {
            type: "string",
            id: 5
          }
        }
      },
      HandleFriendRequestReq: {
        fields: {
          fromUid: {
            type: "string",
            id: 1
          },
          toUid: {
            type: "string",
            id: 2
          },
          accept: {
            type: "bool",
            id: 3
          },
          token: {
            type: "string",
            id: 4
          }
        }
      },
      HandleFriendRequestResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          }
        }
      },
      IMMessage: {
        fields: {
          type: {
            type: "string",
            id: 1
          },
          from: {
            type: "string",
            id: 2
          },
          fromUsername: {
            type: "string",
            id: 3
          },
          to: {
            type: "string",
            id: 4
          },
          content: {
            type: "string",
            id: 5
          },
          timestamp: {
            type: "int64",
            id: 6
          },
          extra: {
            type: "string",
            id: 7
          },
          token: {
            type: "string",
            id: 8
          },
          data: {
            type: "bytes",
            id: 9
          },
          filename: {
            type: "string",
            id: 10
          },
          filesize: {
            type: "int64",
            id: 11
          },
          mimeType: {
            type: "string",
            id: 12
          },
          groupId: {
            type: "string",
            id: 13
          }
        }
      },
      Notification: {
        fields: {
          type: {
            type: "string",
            id: 1
          },
          from: {
            type: "string",
            id: 2
          },
          fromUsername: {
            type: "string",
            id: 3
          },
          to: {
            type: "string",
            id: 4
          },
          groupId: {
            type: "string",
            id: 5
          },
          groupName: {
            type: "string",
            id: 6
          },
          content: {
            type: "string",
            id: 7
          },
          timestamp: {
            type: "int64",
            id: 8
          },
          extra: {
            type: "string",
            id: 9
          }
        }
      },
      FileInfo: {
        fields: {
          filename: {
            type: "string",
            id: 1
          },
          originalName: {
            type: "string",
            id: 2
          },
          size: {
            type: "int64",
            id: 3
          },
          type: {
            type: "string",
            id: 4
          },
          url: {
            type: "string",
            id: 5
          }
        }
      },
      IMMessageList: {
        fields: {
          messages: {
            rule: "repeated",
            type: "IMMessage",
            id: 1
          }
        }
      },
      APIResp: {
        fields: {
          code: {
            type: "int32",
            id: 1
          },
          msg: {
            type: "string",
            id: 2
          },
          data: {
            type: "bytes",
            id: 3
          }
        }
      },
      GetRecentPrivateMessagesReq: {
        fields: {
          from: {
            type: "string",
            id: 1
          },
          to: {
            type: "string",
            id: 2
          },
          count: {
            type: "int64",
            id: 3
          }
        }
      },
      GetRecentGroupMessagesReq: {
        fields: {
          groupId: {
            type: "string",
            id: 1
          },
          count: {
            type: "int64",
            id: 2
          }
        }
      }
    }
  }
});

module.exports = $root;
