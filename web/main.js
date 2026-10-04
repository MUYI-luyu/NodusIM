// ======= 统一API/WS地址配置 =======
// 微服务架构 - 前端直接访问，API通过网关访问
const API_BASE = 'http://127.0.0.1:8087';
const WS_BASE = 'ws://127.0.0.1:8087/ws';

// ======= 微服务API路径映射 =======
const API_PATHS = {
  // 用户服务
  user_info: '/api/user/user_info',
  login: '/api/user/login',
  register: '/api/user/register',
  update_username: '/api/user/update_username',
  update_pwd: '/api/user/update_password',
  delete_account: '/api/user/delete_account',
  logout: '/api/user/logout',
  send_email_code: '/api/notification/send_email_code',
  reset_password: '/api/user/reset_password',
  
  // 好友服务
  friend_list: '/api/friend/friend_list',
  friend_info: '/api/friend/friend_info',
  update_friend_remark: '/api/friend/update_friend_remark',
  set_friend_dnd: '/api/friend/set_friend_dnd',
  delete_friend: '/api/friend/delete_friend',
  add_friend: '/api/friend/add_friend',
  friend_request_list: '/api/friend/friend_request_list',
  handle_friend_request: '/api/friend/handle_friend_request',
  
  // 群组服务
  group_list: '/api/group/group_list',
  group_info: '/api/group/group_info',
  group_member_info: '/api/group/group_member_info',
  set_group_remark: '/api/group/set_group_remark',
  leave_group: '/api/group/leave_group',
  dismiss_group: '/api/group/dismiss_group',
  set_group_nickname: '/api/group/set_group_nickname',
  set_group_dnd: '/api/group/set_group_dnd',
  get_group_dnd: '/api/group/get_group_dnd',
  invite_to_group: '/api/group/invite_to_group',
  update_group_name: '/api/group/update_group_name',
  kick_from_group: '/api/group/kick_from_group',
  set_group_mute: '/api/group/set_group_mute',
  set_group_admin: '/api/group/set_group_admin',
  group_members: '/api/group/group_members',
  create_group: '/api/group/create_group',
  join_group: '/api/group/join_group',
  group_request_list: '/api/group/group_request_list',
  handle_group_request: '/api/group/handle_group_request',
  
  // 消息服务
  get_recent_private_messages: '/api/message/get_recent_private_messages',
  get_recent_group_messages: '/api/message/get_recent_group_messages',
  
  // 文件服务
  upload: '/upload'
};

// ========== 通用居中弹窗 ==========
function showModal({ title = '', content = '', inputs = [], okText = '确定', cancelText = '取消', onOk }) {
  const mask = document.getElementById('modal-mask');
  const dialog = document.getElementById('modal-dialog');
  const titleDiv = document.getElementById('modal-title');
  const contentDiv = document.getElementById('modal-content');
  const actionsDiv = document.getElementById('modal-actions');
  
  // 清空
  titleDiv.textContent = title;
  contentDiv.innerHTML = '';
  if (content) {
    const contentHtml = document.createElement('div');
    contentHtml.innerHTML = content;
    contentDiv.appendChild(contentHtml);
  }
  actionsDiv.innerHTML = '';
  
  // 输入框
  const inputEls = [];
  for (const inp of inputs) {
    const label = document.createElement('div');
    label.textContent = inp.label || '';
    label.className = 'modal-input-label';
    contentDiv.appendChild(label);
    
    const input = document.createElement('input');
    input.type = inp.type || 'text';
    input.value = inp.value || '';
    input.placeholder = inp.placeholder || '';
    input.className = 'modal-input';
    contentDiv.appendChild(input);
    inputEls.push(input);
  }
  
  // 按钮
  const okBtn = document.createElement('button');
  okBtn.textContent = okText;
  okBtn.className = 'modal-btn modal-btn-primary';
  okBtn.onclick = () => {
    mask.style.display = dialog.style.display = 'none';
    if (onOk) onOk(inputEls.map(i => i.value));
  };
  actionsDiv.appendChild(okBtn);
  
  if (cancelText) {
    const cancelBtn = document.createElement('button');
    cancelBtn.textContent = cancelText;
    cancelBtn.className = 'modal-btn modal-btn-secondary';
    cancelBtn.onclick = () => {
      mask.style.display = dialog.style.display = 'none';
    };
    actionsDiv.appendChild(cancelBtn);
  }
  
  mask.style.display = dialog.style.display = 'block';
  
  // ESC关闭
  function escListener(e) {
    if (e.key === 'Escape') {
      mask.style.display = dialog.style.display = 'none';
      document.removeEventListener('keydown', escListener);
    }
  }
  document.addEventListener('keydown', escListener);
  
  // 点击遮罩关闭
  mask.onclick = () => {
    mask.style.display = dialog.style.display = 'none';
    document.removeEventListener('keydown', escListener);
  };
  
  // 聚焦第一个输入
  if (inputEls.length) setTimeout(() => inputEls[0].focus(), 50);
}

document.addEventListener('DOMContentLoaded', function() {
  protobuf.load('protobuf/bundle.json', function(err, root) {
    if (err) {
      alert('protobuf加载失败: ' + err);
      return;
    }
    window.pbRoot = root;
    // ========== 页面切换 ==========
    const loginPanel = document.getElementById('login-panel');
    const registerPanel = document.getElementById('register-panel');
    const mainPanel = document.getElementById('main-panel');
    const userInfoSpan = document.getElementById('user-info');
    const friendListDiv = document.getElementById('friend-list');
    const chatTitleDiv = document.getElementById('chat-title');
    const chatHistoryDiv = document.getElementById('chat-history');
    const chatInput = document.getElementById('chat-input');
    const sendBtn = document.getElementById('send-btn');
    const groupListDiv = document.getElementById('group-list');
    const tabFriends = document.getElementById('tab-friends');
    const tabGroups = document.getElementById('tab-groups');
    const friendListHeader = document.getElementById('friend-list-header');
    const groupListHeader = document.getElementById('group-list-header');

    let currentFriend = null;
    let currentGroup = null;
    let ws = null;
    let heartbeatInterval = null;
    let myUid = null;
    let myUsername = null;
    let token = null;
    let unreadMap = {};
    let friendListCache = null;
    let groupListCache = null;
    let wsReady = false; // 等待服务端login_ack后再认为就绪
    
    window.currentFriend = currentFriend;
    window.currentGroup = currentGroup;
    window.myUid = myUid;

    document.getElementById('to-register').onclick = e => {
      e.preventDefault();
      loginPanel.style.display = 'none';
      registerPanel.style.display = '';
    };
    document.getElementById('to-login').onclick = e => {
      e.preventDefault();
      registerPanel.style.display = 'none';
      loginPanel.style.display = '';
    };

    // ========== Protobuf 类型 ==========
    const LoginReq = root.lookupType('protocol.LoginReq');
    const RegisterReq = root.lookupType('protocol.RegisterReq');
    const APIResp = root.lookupType('protocol.APIResp');
    const UserInfoReq = root.lookupType('protocol.UserInfoReq');
    const UserInfoResp = root.lookupType('protocol.UserInfoResp');
    const FriendListReq = root.lookupType('protocol.FriendListReq');
    const FriendListResp = root.lookupType('protocol.FriendListResp');
    const IMMessage = root.lookupType('protocol.IMMessage');
    const IMMessageList = root.lookupType('protocol.IMMessageList');
    const GetRecentPrivateMessagesReq = root.lookupType('protocol.GetRecentPrivateMessagesReq');
    const GetRecentGroupMessagesReq = root.lookupType('protocol.GetRecentGroupMessagesReq');
    const GroupListReq = root.lookupType('protocol.GroupListReq');
    const GroupListResp = root.lookupType('protocol.GroupListResp');
    const Notification = root.lookupType('protocol.Notification');

    // ========== 登录/注册逻辑 ==========
    const loginBtn = document.getElementById('login-btn');
    const loginError = document.getElementById('login-error');
    // 自动登录/会话恢复
    const savedToken = window.localStorage.getItem('token');
    if (savedToken) {
      // 调用 /user_info 校验token
      const UserInfoReq = root.lookupType('protocol.UserInfoReq');
      const APIResp = root.lookupType('protocol.APIResp');
      const UserInfoResp = root.lookupType('protocol.UserInfoResp');
      (async function() {
        try {
          const req = UserInfoReq.encode(UserInfoReq.create({ token: savedToken })).finish();
          const resp = await fetch(`${API_BASE}${API_PATHS.user_info}`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/x-protobuf' },
            body: req
          });
          if (!resp.ok) throw new Error('自动登录失败');
          const buf = await resp.arrayBuffer();
          const apiMsg = APIResp.decode(new Uint8Array(buf));
          if (apiMsg.code !== 0) throw new Error(apiMsg.msg);
          const userInfo = UserInfoResp.decode(apiMsg.data);
          myUid = userInfo.uid;
          myUsername = userInfo.username;
          token = savedToken;
          showMainPanel();
          await fetchUserInfoAndFriends(token);
          connectWebSocket();
        } catch (e) {
          window.localStorage.removeItem('token');
          window.localStorage.removeItem('uid');
          showLoginPanel();
        }
      })();
    }
    loginBtn.onclick = async () => {
      loginError.textContent = '';
      const uid = document.getElementById('login-uid').value.trim();
      const pwd = document.getElementById('login-pwd').value;
      if (!uid || !pwd) {
        loginError.textContent = '请输入UID和密码';
        return;
      }
      try {
        // protobuf编码请求体
        const payload = { uid, password: pwd };
        const body = LoginReq.encode(LoginReq.create(payload)).finish();
        const resp = await fetch(`${API_BASE}${API_PATHS.login}`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-protobuf' },
          body
        });
        if (!resp.ok) throw new Error('网络错误');
        const buf = await resp.arrayBuffer();
        const msg = APIResp.decode(new Uint8Array(buf));
        if (msg.code !== 0) throw new Error(msg.msg);
        // token在data字段
        token = new TextDecoder().decode(msg.data);
        window.localStorage.setItem('token', token);
        window.localStorage.setItem('uid', uid);
        showMainPanel();
        await fetchUserInfoAndFriends(token);
        connectWebSocket();
      } catch (e) {
        loginError.textContent = e.message || '登录失败';
      }
    };

    const registerBtn = document.getElementById('register-btn');
    const registerError = document.getElementById('register-error');
    registerBtn.onclick = async () => {
      registerError.textContent = '';
      const username = document.getElementById('reg-username').value.trim();
      const email = document.getElementById('reg-email').value.trim();
      const pwd = document.getElementById('reg-pwd').value;
      const emailCode = document.getElementById('reg-email-code').value.trim();
      
      if (!username || !email || !pwd || !emailCode) {
        registerError.textContent = '请填写所有信息';
        return;
      }
      
      try {
        // protobuf编码请求体
        const payload = { username, email, password: pwd, emailCode };
        const body = RegisterReq.encode(RegisterReq.create(payload)).finish();
        const resp = await fetch(`${API_BASE}${API_PATHS.register}`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-protobuf' },
          body
        });
        if (!resp.ok) throw new Error('网络错误');
        const buf = await resp.arrayBuffer();
        const msg = APIResp.decode(new Uint8Array(buf));
        if (msg.code !== 0) throw new Error(msg.msg);
        // 新增：注册成功后弹窗提示UID
        const newUid = new TextDecoder().decode(msg.data);
        showModal({
          title: '注册成功',
          content: `<div style='font-size:1.1em;'>您的账号UID为 <b>${newUid}</b><br>请妥善保存，用于登录</div>`,
          okText: '去登录'
        });
        showLoginPanel();
      } catch (e) {
        registerError.textContent = e.message || '注册失败';
      }
    };

    async function fetchUserInfoAndFriends(tokenVal) {
      // 获取用户信息
      try {
        const userInfoReq = UserInfoReq.encode(UserInfoReq.create({ token: tokenVal })).finish();
        const resp = await fetch(`${API_BASE}${API_PATHS.user_info}`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-protobuf' },
          body: userInfoReq
        });
        if (!resp.ok) throw new Error('用户信息获取失败');
        const buf = await resp.arrayBuffer();
        const apiMsg = APIResp.decode(new Uint8Array(buf));
        if (apiMsg.code !== 0) throw new Error(apiMsg.msg);
        const userInfo = UserInfoResp.decode(apiMsg.data);
        userInfoSpan.textContent = `${userInfo.username} (UID: ${userInfo.uid})`;
        myUid = userInfo.uid;
        myUsername = userInfo.username;
        window.myUid = myUid;
        // 获取好友列表
        await fetchFriendList(userInfo.uid, tokenVal);
      } catch (e) {
        userInfoSpan.textContent = '用户信息获取失败: ' + (e.message || e);
      }
    }

    async function fetchFriendList(uid, tokenVal) {
      try {
        const friendListReq = FriendListReq.encode(FriendListReq.create({ uid, token: tokenVal })).finish();
        const resp = await fetch(`${API_BASE}${API_PATHS.friend_list}`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-protobuf' },
          body: friendListReq
        });
        if (!resp.ok) throw new Error('好友列表获取失败');
        const buf = await resp.arrayBuffer();
        const apiMsg = APIResp.decode(new Uint8Array(buf));
        if (apiMsg.code !== 0) throw new Error(apiMsg.msg);
        const friendList = FriendListResp.decode(apiMsg.data);
        friendListCache = friendList; // 缓存
        renderFriendList(friendList);
      } catch (e) {
        friendListDiv.innerHTML = '<div class="error">好友列表获取失败: ' + (e.message || e) + '</div>';
      }
    }

    function renderFriendList(friendList) {
      if (!friendList.friendUids || friendList.friendUids.length === 0) {
        friendListDiv.innerHTML = '<div class="empty-state">暂无好友</div>';
        chatTitleDiv.textContent = '';
        chatHistoryDiv.innerHTML = '';
        currentFriend = null;
        window.currentFriend = currentFriend;
        return;
      }
      friendListDiv.innerHTML = '';
      for (let i = 0; i < friendList.friendUids.length; i++) {
        const uid = friendList.friendUids[i];
        const name = friendList.friendUsernames[i] || uid;
        const remark = friendList.remarks && friendList.remarks[i] ? friendList.remarks[i] : '';
        const div = document.createElement('div');
        div.className = 'list-item list-item-flex';
        // 左侧：名称
        const nameSpan = document.createElement('span');
        nameSpan.textContent = remark ? `${remark}(${name})` : name;
        nameSpan.className = 'list-item-name';
        // 右侧：...按钮
        const moreBtn = document.createElement('button');
        moreBtn.className = 'circle-more-btn';
        moreBtn.title = '好友详情';
        moreBtn.innerHTML = '&#x22EE;'; // 竖向省略号
        moreBtn.onclick = async (e) => {
          e.stopPropagation();
          // 获取好友详细信息
          const FriendInfoReq = root.lookupType('protocol.FriendInfoReq');
          const APIResp = root.lookupType('protocol.APIResp');
          const FriendInfoResp = root.lookupType('protocol.FriendInfoResp');
          const reqBuf = FriendInfoReq.encode(FriendInfoReq.create({ uid: myUid, friendUid: uid, token })).finish();
          try {
            const resp = await fetch(`${API_BASE}${API_PATHS.friend_info}`, {
              method: 'POST',
              headers: { 'Content-Type': 'application/x-protobuf' },
              body: reqBuf
            });
            const buf = await resp.arrayBuffer();
            const apiMsg = APIResp.decode(new Uint8Array(buf));
            if (apiMsg.code !== 0) throw new Error(apiMsg.msg);
            const info = FriendInfoResp.decode(apiMsg.data);
            // 弹窗内容
            let html = `<div style='margin-bottom:8px;'><b>UID:</b> ${info.uid}</div>`;
            html += `<div style='margin-bottom:8px;'><b>用户名:</b> ${info.username}</div>`;
            html += `<div style='margin-bottom:8px;'><b>邮箱:</b> ${info.email}</div>`;
            html += `<div style='margin-bottom:8px;'><b>备注:</b> ${info.remark || ''}</div>`;
            html += `<div style='margin-bottom:8px;'><b>免打扰:</b> ${info.dnd ? '已开启' : '未开启'}</div>`;
            showModal({
              title: '好友详情',
              content: html,
              okText: '关闭',
              cancelText: '',
              onOk: null
            });
            // 额外操作按钮
            const actionsDiv = document.getElementById('modal-actions');
            // 设置备注按钮
            const remarkBtn = document.createElement('button');
            remarkBtn.textContent = '设置备注';
            remarkBtn.className = 'btn-margin-right';
            remarkBtn.onclick = function() {
              showModal({
                title: '设置备注',
                inputs: [{ label: '备注', value: info.remark || '', placeholder: '输入备注' }],
                okText: '保存',
                onOk: async ([remark]) => {
                  const UpdateFriendRemarkReq = root.lookupType('protocol.UpdateFriendRemarkReq');
                  const reqBuf = UpdateFriendRemarkReq.encode(UpdateFriendRemarkReq.create({ uid: myUid, friendUid: uid, remark, token })).finish();
                  const resp = await fetch(`${API_BASE}${API_PATHS.update_friend_remark}`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-protobuf' },
                    body: reqBuf
                  });
                  const buf = await resp.arrayBuffer();
                  const apiMsg = APIResp.decode(new Uint8Array(buf));
                  alert(apiMsg.msg || (apiMsg.code === 0 ? '备注已更新' : '设置失败'));
                  fetchFriendList(myUid, token);
                }
              });
            };
            actionsDiv.insertBefore(remarkBtn, actionsDiv.firstChild);
            // 设置免打扰按钮
            const dndBtn = document.createElement('button');
            dndBtn.textContent = info.dnd ? '关闭免打扰' : '开启免打扰';
            dndBtn.className = 'btn-margin-right';
            dndBtn.onclick = async function() {
              const SetFriendDNDReq = root.lookupType('protocol.SetFriendDNDReq');
              const reqBuf = SetFriendDNDReq.encode(SetFriendDNDReq.create({ uid: myUid, friendUid: uid, dnd: !info.dnd, token })).finish();
              const resp = await fetch(`${API_BASE}${API_PATHS.set_friend_dnd}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-protobuf' },
                body: reqBuf
              });
              const buf = await resp.arrayBuffer();
              const apiMsg = APIResp.decode(new Uint8Array(buf));
              alert(apiMsg.msg || (apiMsg.code === 0 ? '设置成功' : '设置失败'));
              fetchFriendList(myUid, token);
            };
            actionsDiv.insertBefore(dndBtn, actionsDiv.firstChild);
            // 删除好友按钮
            const delBtn = document.createElement('button');
            delBtn.textContent = '删除好友';
            delBtn.className = 'btn-danger';
            delBtn.onclick = async function() {
              if (!confirm('确定要删除该好友吗？')) return;
              const DeleteFriendReq = root.lookupType('protocol.DeleteFriendReq');
              const reqBuf = DeleteFriendReq.encode(DeleteFriendReq.create({ uid: myUid, friendUid: uid, token })).finish();
              const resp = await fetch(`${API_BASE}${API_PATHS.delete_friend}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-protobuf' },
                body: reqBuf
              });
              const buf = await resp.arrayBuffer();
              const apiMsg = APIResp.decode(new Uint8Array(buf));
              alert(apiMsg.msg || (apiMsg.code === 0 ? '已删除' : '删除失败'));
              fetchFriendList(myUid, token);
            };
            actionsDiv.insertBefore(delBtn, actionsDiv.firstChild);
          } catch (e) {
            alert('获取好友信息失败: ' + (e.message || e));
          }
        };
        // 未读红点
        if (unreadMap['user:' + uid] > 0) {
          const badge = document.createElement('span');
          badge.textContent = unreadMap['user:' + uid] > 99 ? '99+' : unreadMap['user:' + uid];
          badge.className = 'unread-badge';
          nameSpan.appendChild(badge);
        }
        div.appendChild(nameSpan);
        div.appendChild(moreBtn);
        div.onclick = () => {
          selectFriend(uid, name, remark);
        };
        if (currentFriend && currentFriend.uid === uid) {
          div.classList.add('selected');
        }
        friendListDiv.appendChild(div);
      }
    }

    function selectFriend(uid, name, remark) {
              currentFriend = { uid, name, remark };
        window.currentFriend = currentFriend;
      currentGroup = null; // 选择好友时清空群组
      window.currentGroup = currentGroup;
      unreadMap['user:' + uid] = 0; // 清除未读
      // 修复：用缓存的好友列表数据刷新
      if (friendListCache) {
        renderFriendList(friendListCache);
      }
      // 高亮
      const items = friendListDiv.querySelectorAll('.list-item');
      items.forEach(item => {
        if (item.textContent === (remark ? `${remark}(${name})` : name)) {
          item.classList.add('selected');
        } else {
          item.classList.remove('selected');
        }
      });
      // 聊天区标题
      chatTitleDiv.textContent = `与 ${remark ? `${remark}(${name})` : name} 聊天`;
      // 拉取并展示历史消息
      chatHistoryDiv.innerHTML = '<div class="empty-state">加载中...</div>';
              fetchRecentPrivateMessages(myUid, uid, 50).then(msgs => {
          chatHistoryDiv.innerHTML = '';
          if (!msgs.length) {
            chatHistoryDiv.innerHTML = '<div class="empty-state">暂无消息</div>';
          } else {
          msgs.reverse().forEach(msg => {
            appendMessage({
              from: msg.from,
              content: msg.type === 'chat' ? replaceEmojis(msg.content) : msg.content,
              self: msg.from === myUid,
              timestamp: msg.timestamp,
              type: msg.type,
              extra: msg.extra,
              username: msg.fromUsername
            });
          });
        }
      }).catch(e => {
        chatHistoryDiv.innerHTML = '<div class="empty-tip" style="color:#e74c3c;padding:16px;">消息加载失败: '+(e.message||e)+'</div>';
      });
    }

    // 拉取群组列表
    async function fetchGroupList(tokenVal) {
      try {
        const req = GroupListReq.encode(GroupListReq.create({ uid: myUid, token: tokenVal })).finish();
        const resp = await fetch(`${API_BASE}${API_PATHS.group_list}`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-protobuf' },
          body: req
        });
        if (!resp.ok) throw new Error('群组列表获取失败');
        const buf = await resp.arrayBuffer();
        const apiMsg = APIResp.decode(new Uint8Array(buf));
        if (apiMsg.code !== 0) throw new Error(apiMsg.msg);
        const groupList = GroupListResp.decode(apiMsg.data);
        groupListCache = groupList.groups || [];
        renderGroupList(groupListCache);
      } catch (e) {
        groupListDiv.innerHTML = '<div class="error">群组列表获取失败: ' + (e.message || e) + '</div>';
      }
    }
    function renderGroupList(groups) {
      if (!groups || groups.length === 0) {
        groupListDiv.innerHTML = '<div style="color:#888;padding:16px;">暂无群组</div>';
        return;
      }
      groupListDiv.innerHTML = '';
      groups.forEach(group => {
        const groupName = group.name || group.groupName || group.group_id || group.groupId || '未知群组';
        const groupId = group.groupId || group.group_id || group.id || '';
        const div = document.createElement('div');
        div.className = 'list-item';
        div.className = 'list-item list-item-flex';
        // 左侧：群名（优先显示备注）
        const nameSpan = document.createElement('span');
        const remark = group.remark || '';
        nameSpan.textContent = remark ? `${remark}(${groupName})` : groupName;
        nameSpan.className = 'list-item-name';
        // 右侧：...按钮
        const moreBtn = document.createElement('button');
        moreBtn.className = 'circle-more-btn';
        moreBtn.title = '群组详情';
        moreBtn.innerHTML = '&#x22EE;';
        moreBtn.onclick = async (e) => {
          e.stopPropagation();
          // 获取群组详细信息
          const GroupInfoReq = root.lookupType('protocol.GroupInfoReq');
          const APIResp = root.lookupType('protocol.APIResp');
          const GroupInfoResp = root.lookupType('protocol.GroupInfoResp');
          const reqBuf = GroupInfoReq.encode(GroupInfoReq.create({ groupId })).finish();
          try {
            const resp = await fetch(`${API_BASE}${API_PATHS.group_info}`, {
              method: 'POST',
              headers: { 'Content-Type': 'application/x-protobuf' },
              body: reqBuf
            });
            const buf = await resp.arrayBuffer();
            const apiMsg = APIResp.decode(new Uint8Array(buf));
            if (apiMsg.code !== 0) throw new Error(apiMsg.msg);
            const info = GroupInfoResp.decode(apiMsg.data).group;
            // 获取角色
            const GroupMemberRoleReq = root.lookupType('protocol.GroupMemberRoleReq');
            const GroupMemberRoleResp = root.lookupType('protocol.GroupMemberRoleResp');
            const roleReqBuf = GroupMemberRoleReq.encode(GroupMemberRoleReq.create({ groupId, uid: myUid })).finish();
            let role = 'member';
            try {
              // 替换为 group_member_info
              const infoReq = root.lookupType('protocol.GroupMemberInfoReq');
              const infoResp = root.lookupType('protocol.GroupMemberInfoResp');
              const infoReqBuf = infoReq.encode(infoReq.create({ groupId, uid: myUid })).finish();
              const resp = await fetch(`${API_BASE}${API_PATHS.group_member_info}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-protobuf' },
                body: infoReqBuf
              });
              const buf = await resp.arrayBuffer();
              const apiMsg = APIResp.decode(new Uint8Array(buf));
              if (apiMsg.code === 0) {
                const info = infoResp.decode(apiMsg.data);
                role = info.role;
              }
            } catch {}
            // 获取备注、免打扰
            let remark = group.remark || '';
            let dnd = false;
            try {
              const GroupRemarkReq = root.lookupType('protocol.SetGroupRemarkReq');
              const GroupDNDReq = root.lookupType('protocol.SetGroupDNDReq');
              const GroupDNDResp = root.lookupType('protocol.SetGroupDNDResp');
              // 备注直接用group.remark
              // 免打扰需查接口
              const dndReq = GroupDNDReq.encode(GroupDNDReq.create({ groupId, uid: myUid })).finish();
              const dndResp = await fetch(`${API_BASE}${API_PATHS.get_group_dnd}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-protobuf' },
                body: dndReq
              });
              const dndBuf = await dndResp.arrayBuffer();
              const dndApiMsg = APIResp.decode(new Uint8Array(dndBuf));
              if (dndApiMsg.code === 0) {
                const dndData = GroupDNDResp.decode(dndApiMsg.data);
                dnd = dndData.dnd;
              }
            } catch {}
            // 弹窗内容
            let html = `<div style='margin-bottom:8px;'><b>群ID:</b> ${info.groupId}</div>`;
            html += `<div style='margin-bottom:8px;'><b>群名:</b> ${info.name}</div>`;
            html += `<div style='margin-bottom:8px;'><b>群主:</b> ${info.ownerUid}</div>`;
            html += `<div style='margin-bottom:8px;'><b>成员数:</b> ${info.memberUids.length}</div>`;
            html += `<div style='margin-bottom:8px;'><b>备注:</b> ${remark}</div>`;
            // html += `<div style='margin-bottom:8px;'><b>免打扰:</b> ${dnd ? '已开启' : '未开启'}</div>`;
            showModal({
              title: '群组详情',
              content: html,
              okText: '关闭',
              cancelText: '',
              onOk: null
            });
            // 额外操作按钮
            const actionsDiv = document.getElementById('modal-actions');
            // 设置备注按钮
            const remarkBtn = document.createElement('button');
            remarkBtn.textContent = '设置备注';
            remarkBtn.className = 'btn-margin-right';
            remarkBtn.onclick = function() {
              showModal({
                title: '设置群备注',
                inputs: [{ label: '备注', value: remark || '', placeholder: '输入备注' }],
                okText: '保存',
                onOk: async ([newRemark]) => {
                  const SetGroupRemarkReq = root.lookupType('protocol.SetGroupRemarkReq');
                  const reqBuf = SetGroupRemarkReq.encode(SetGroupRemarkReq.create({ groupId, uid: myUid, remark: newRemark })).finish();
                  const resp = await fetch(`${API_BASE}${API_PATHS.set_group_remark}`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-protobuf' },
                    body: reqBuf
                  });
                  const buf = await resp.arrayBuffer();
                  const apiMsg = APIResp.decode(new Uint8Array(buf));
                  alert(apiMsg.msg || (apiMsg.code === 0 ? '备注已更新' : '设置失败'));
                  fetchGroupList(token);
                }
              });
            };
            actionsDiv.insertBefore(remarkBtn, actionsDiv.firstChild);
            // 退出群组按钮（非owner）
            if (role !== 'owner') {
              const leaveBtn = document.createElement('button');
              leaveBtn.textContent = '退出群组';
                          leaveBtn.className = 'btn-danger';
              leaveBtn.onclick = async function() {
                if (!confirm('确定要退出该群组吗？')) return;
                const LeaveGroupReq = root.lookupType('protocol.LeaveGroupReq');
                const reqBuf = LeaveGroupReq.encode(LeaveGroupReq.create({ groupId, uid: myUid })).finish();
                const resp = await fetch(`${API_BASE}${API_PATHS.leave_group}`, {
                  method: 'POST',
                  headers: { 'Content-Type': 'application/x-protobuf' },
                  body: reqBuf
                });
                const buf = await resp.arrayBuffer();
                const apiMsg = APIResp.decode(new Uint8Array(buf));
                alert(apiMsg.msg || (apiMsg.code === 0 ? '已退出' : '操作失败'));
                fetchGroupList(token);
              };
              actionsDiv.insertBefore(leaveBtn, actionsDiv.firstChild);
            }
            // 解散群组按钮（owner）
            if (role === 'owner') {
              const dismissBtn = document.createElement('button');
              dismissBtn.textContent = '解散群组';
                          dismissBtn.className = 'btn-danger';
              dismissBtn.onclick = async function() {
                if (!confirm('确定要解散该群组吗？')) return;
                const DismissGroupReq = root.lookupType('protocol.DismissGroupReq');
                const reqBuf = DismissGroupReq.encode(DismissGroupReq.create({ groupId, operatorUid: myUid })).finish();
                const resp = await fetch(`${API_BASE}${API_PATHS.dismiss_group}`, {
                  method: 'POST',
                  headers: { 'Content-Type': 'application/x-protobuf' },
                  body: reqBuf
                });
                const buf = await resp.arrayBuffer();
                const apiMsg = APIResp.decode(new Uint8Array(buf));
                alert(apiMsg.msg || (apiMsg.code === 0 ? '群组已解散' : '操作失败'));
                fetchGroupList(token);
              };
              actionsDiv.insertBefore(dismissBtn, actionsDiv.firstChild);
            }
            // 设置群昵称按钮
            const nicknameBtn = document.createElement('button');
            nicknameBtn.textContent = '设置群昵称';
            nicknameBtn.className = 'btn-margin-right';
            nicknameBtn.onclick = function() {
              showModal({
                title: '设置群昵称',
                inputs: [{ label: '群昵称', value: '', placeholder: '输入群昵称' }],
                okText: '保存',
                onOk: async ([nickname]) => {
                  const SetGroupNicknameReq = root.lookupType('protocol.SetGroupNicknameReq');
                  const reqBuf = SetGroupNicknameReq.encode(SetGroupNicknameReq.create({ groupId, uid: myUid, nickname })).finish();
                  const resp = await fetch(`${API_BASE}${API_PATHS.set_group_nickname}`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-protobuf' },
                    body: reqBuf
                  });
                  const buf = await resp.arrayBuffer();
                  const apiMsg = APIResp.decode(new Uint8Array(buf));
                  alert(apiMsg.msg || (apiMsg.code === 0 ? '群昵称已更新' : '设置失败'));
                  fetchGroupList(token);
                }
              });
            };
            actionsDiv.insertBefore(nicknameBtn, actionsDiv.firstChild);
            // 设置群免打扰按钮
            const dndBtn = document.createElement('button');
            dndBtn.textContent = dnd ? '关闭群免打扰' : '开启群免打扰';
            dndBtn.className = 'btn-margin-right';
            dndBtn.onclick = async function() {
              const SetGroupDNDReq = root.lookupType('protocol.SetGroupDNDReq');
              const reqBuf = SetGroupDNDReq.encode(SetGroupDNDReq.create({ groupId, uid: myUid, dnd: !dnd })).finish();
              const resp = await fetch(`${API_BASE}${API_PATHS.set_group_dnd}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-protobuf' },
                body: reqBuf
              });
              const buf = await resp.arrayBuffer();
              const apiMsg = APIResp.decode(new Uint8Array(buf));
              if (apiMsg.code === 0) {
                // 更新本地状态
                dnd = !dnd;
                dndBtn.textContent = dnd ? '关闭群免打扰' : '开启群免打扰';
                alert('设置成功');
              } else {
                alert('设置失败: ' + apiMsg.msg);
              }
            };
            actionsDiv.insertBefore(dndBtn, actionsDiv.firstChild);
            // 邀请新成员按钮
            const inviteBtn = document.createElement('button');
            inviteBtn.textContent = '邀请新成员';
            inviteBtn.className = 'btn-margin-right';
            inviteBtn.onclick = function() {
              showModal({
                title: '邀请新成员',
                inputs: [{ label: '成员UID(多个用逗号分隔)', placeholder: '输入UID' }],
                okText: '邀请',
                onOk: async ([uids]) => {
                  const InviteToGroupReq = root.lookupType('protocol.InviteToGroupReq');
                  const reqBuf = InviteToGroupReq.encode(InviteToGroupReq.create({ groupId, inviterUid: myUid, inviteeUids: uids.split(',').map(s => s.trim()).filter(Boolean) })).finish();
                  const resp = await fetch(`${API_BASE}${API_PATHS.invite_to_group}`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-protobuf' },
                    body: reqBuf
                  });
                  const buf = await resp.arrayBuffer();
                  const apiMsg = APIResp.decode(new Uint8Array(buf));
                  alert(apiMsg.msg || (apiMsg.code === 0 ? '邀请已发送' : '邀请失败'));
                }
              });
            };
            actionsDiv.insertBefore(inviteBtn, actionsDiv.firstChild);
            // 管理员/群主操作
            if (role === 'admin' || role === 'owner') {
              // 修改群名
              const updateNameBtn = document.createElement('button');
              updateNameBtn.textContent = '修改群名';
              updateNameBtn.className = 'btn-margin-right';
              updateNameBtn.onclick = function() {
                showModal({
                  title: '修改群名',
                  inputs: [{ label: '新群名', value: info.name, placeholder: '输入新群名' }],
                  okText: '保存',
                  onOk: async ([newName]) => {
                    const UpdateGroupNameReq = root.lookupType('protocol.UpdateGroupNameReq');
                    const reqBuf = UpdateGroupNameReq.encode(UpdateGroupNameReq.create({ groupId, operatorUid: myUid, newName })).finish();
                    const resp = await fetch(`${API_BASE}${API_PATHS.update_group_name}`, {
                      method: 'POST',
                      headers: { 'Content-Type': 'application/x-protobuf' },
                      body: reqBuf
                    });
                    const buf = await resp.arrayBuffer();
                    const apiMsg = APIResp.decode(new Uint8Array(buf));
                    alert(apiMsg.msg || (apiMsg.code === 0 ? '群名已更新' : '设置失败'));
                    fetchGroupList(token);
                  }
                });
              };
              actionsDiv.insertBefore(updateNameBtn, actionsDiv.firstChild);
              // 移除成员
              const kickBtn = document.createElement('button');
              kickBtn.textContent = '移除成员';
              kickBtn.className = 'btn-margin-right';
              kickBtn.onclick = function() {
                showModal({
                  title: '移除成员',
                  inputs: [{ label: '成员UID', placeholder: '输入要移除的成员UID' }],
                  okText: '移除',
                  onOk: async ([targetUid]) => {
                    const KickFromGroupReq = root.lookupType('protocol.KickFromGroupReq');
                    const reqBuf = KickFromGroupReq.encode(KickFromGroupReq.create({ groupId, operatorUid: myUid, targetUid })).finish();
                    const resp = await fetch(`${API_BASE}${API_PATHS.kick_from_group}`, {
                      method: 'POST',
                      headers: { 'Content-Type': 'application/x-protobuf' },
                      body: reqBuf
                    });
                    const buf = await resp.arrayBuffer();
                    const apiMsg = APIResp.decode(new Uint8Array(buf));
                    alert(apiMsg.msg || (apiMsg.code === 0 ? '已移除' : '操作失败'));
                  }
                });
              };
              actionsDiv.insertBefore(kickBtn, actionsDiv.firstChild);
              // 设置禁言
              const muteBtn = document.createElement('button');
              muteBtn.textContent = '设置禁言';
              muteBtn.className = 'btn-margin-right';
              muteBtn.onclick = function() {
                showModal({
                  title: '设置禁言',
                  inputs: [
                    { label: '成员UID', placeholder: '输入要禁言的成员UID' },
                    { label: '禁言(true=禁言,false=取消)', placeholder: 'true/false' }
                  ],
                  okText: '设置',
                  onOk: async ([targetUid, mute]) => {
                    const SetGroupMuteReq = root.lookupType('protocol.SetGroupMuteReq');
                    const reqBuf = SetGroupMuteReq.encode(SetGroupMuteReq.create({ groupId, operatorUid: myUid, targetUid, mute: mute === 'true' })).finish();
                    const resp = await fetch(`${API_BASE}${API_PATHS.set_group_mute}`, {
                      method: 'POST',
                      headers: { 'Content-Type': 'application/x-protobuf' },
                      body: reqBuf
                    });
                    const buf = await resp.arrayBuffer();
                    const apiMsg = APIResp.decode(new Uint8Array(buf));
                    alert(apiMsg.msg || (apiMsg.code === 0 ? '设置成功' : '操作失败'));
                  }
                });
              };
              actionsDiv.insertBefore(muteBtn, actionsDiv.firstChild);
            }
            // 群主专属操作
            if (role === 'owner') {
              // 设置/取消管理员
              const adminBtn = document.createElement('button');
              adminBtn.textContent = '设置/取消管理员';
              adminBtn.className = 'btn-margin-right';
              adminBtn.onclick = function() {
                showModal({
                  title: '设置/取消管理员',
                  inputs: [
                    { label: '成员UID', placeholder: '输入成员UID' },
                    { label: '设置为管理员(true/false)', placeholder: 'true/false' }
                  ],
                  okText: '设置',
                  onOk: async ([targetUid, setAdmin]) => {
                    const SetGroupAdminReq = root.lookupType('protocol.SetGroupAdminReq');
                    const reqBuf = SetGroupAdminReq.encode(SetGroupAdminReq.create({ groupId, operatorUid: myUid, targetUid, setAdmin: setAdmin === 'true' })).finish();
                    const resp = await fetch(`${API_BASE}${API_PATHS.set_group_admin}`, {
                      method: 'POST',
                      headers: { 'Content-Type': 'application/x-protobuf' },
                      body: reqBuf
                    });
                    const buf = await resp.arrayBuffer();
                    const apiMsg = APIResp.decode(new Uint8Array(buf));
                    alert(apiMsg.msg || (apiMsg.code === 0 ? '设置成功' : '操作失败'));
                  }
                });
              };
              actionsDiv.insertBefore(adminBtn, actionsDiv.firstChild);
            }
            // 成员列表按钮
            const memberListBtn = document.createElement('button');
            memberListBtn.textContent = '成员列表';
            memberListBtn.className = 'btn-margin-right';
            memberListBtn.onclick = async function() {
              const GroupMembersReq = root.lookupType('protocol.GroupMembersReq');
              const APIResp = root.lookupType('protocol.APIResp');
              const GroupMembersResp = root.lookupType('protocol.GroupMembersResp');
              const reqBuf = GroupMembersReq.encode(GroupMembersReq.create({ groupId })).finish();
              try {
                const resp = await fetch(`${API_BASE}${API_PATHS.group_members}`, {
                  method: 'POST',
                  headers: { 'Content-Type': 'application/x-protobuf' },
                  body: reqBuf
                });
                const buf = await resp.arrayBuffer();
                const apiMsg = APIResp.decode(new Uint8Array(buf));
                if (apiMsg.code !== 0) throw new Error(apiMsg.msg);
                const members = GroupMembersResp.decode(apiMsg.data).members;
                let html = `<div style='max-height:320px;overflow:auto;'>`;
                html += `<table style='width:100%;font-size:0.98em;border-collapse:collapse;'>`;
                html += `<tr><th style='text-align:left;'>UID</th><th style='text-align:left;'>用户名</th><th style='text-align:left;'>昵称</th><th style='text-align:left;'>角色</th><th style='text-align:left;'>加入时间</th></tr>`;
                for (const m of members) {
                  html += `<tr>`;
                  html += `<td>${m.uid}</td>`;
                  html += `<td>${m.username || ''}</td>`;
                  html += `<td>${m.nickname || ''}</td>`;
                  html += `<td>${m.role || ''}</td>`;
                  html += `<td>${m.joinTime ? new Date(m.joinTime * 1000).toLocaleString() : ''}</td>`;
                  html += `</tr>`;
                }
                html += `</table></div>`;
                showModal({ title: '成员列表', content: html, okText: '关闭' });
              } catch (e) {
                alert('获取成员列表失败: ' + (e.message || e));
              }
            };
            actionsDiv.insertBefore(memberListBtn, actionsDiv.firstChild);
          } catch (e) {
            alert('获取群组信息失败: ' + (e.message || e));
          }
        };
        // 未读红点
        if (unreadMap['group:' + groupId] > 0) {
          const badge = document.createElement('span');
          badge.textContent = unreadMap['group:' + groupId] > 99 ? '99+' : unreadMap['group:' + groupId];
          badge.className = 'unread-badge';
          nameSpan.appendChild(badge);
        }
        div.appendChild(nameSpan);
        div.appendChild(moreBtn);
        div.className = 'list-item list-item-flex clickable';
        div.onclick = () => {
          selectGroup(groupId, groupName, remark);
        };
        if (currentGroup && currentGroup.groupId === groupId) {
          div.classList.add('selected');
        }
        groupListDiv.appendChild(div);
      });
    }
    function selectGroup(groupId, name, remark) {
              currentGroup = { groupId, name, remark };
        window.currentGroup = currentGroup;
      currentFriend = null; // 选择群组时清空好友
      window.currentFriend = currentFriend;
      unreadMap['group:' + groupId] = 0; // 清除未读
      // 用缓存的群组列表数据刷新
      if (groupListCache) {
        renderGroupList(groupListCache);
      }
      // 高亮
      const items = groupListDiv.querySelectorAll('.list-item');
      items.forEach(item => {
        const remark = currentGroup.remark || '';
        const displayName = remark ? `${remark}(${name})` : name;
        if (item.textContent === displayName) {
          item.classList.add('selected');
        } else {
          item.classList.remove('selected');
        }
      });
      // 聊天区标题
      const displayName = remark ? `${remark}(${name})` : (name || groupId || '未知群组');
      chatTitleDiv.textContent = `群聊：${displayName}`;
      // 拉取并展示历史消息
      chatHistoryDiv.innerHTML = '<div class="empty-tip" style="color:#888;padding:16px;">加载中...</div>';
      fetchRecentGroupMessages(groupId, 50).then(msgs => {
        chatHistoryDiv.innerHTML = '';
        if (!msgs.length) {
          chatHistoryDiv.innerHTML = '<div class="empty-tip" style="color:#888;padding:16px;">暂无消息</div>';
        } else {
          msgs.reverse().forEach(msg => {
            appendMessage({
              from: msg.from,
              content: msg.type === 'chat' ? replaceEmojis(msg.content) : msg.content,
              self: msg.from === myUid,
              timestamp: msg.timestamp,
              type: msg.type,
              extra: msg.extra,
              username: msg.fromUsername
            });
          });
        }
      }).catch(e => {
        chatHistoryDiv.innerHTML = '<div class="empty-tip" style="color:#e74c3c;padding:16px;">消息加载失败: '+(e.message||e)+'</div>';
      });
    }

    // ========== 消息类型验证 ==========
    function isValidNotificationType(type) {
      const validTypes = [
        'friend_request', 'private_chat_message', 'group_chat_message',
        'group_application_pending', 'group_invite', 'group_invite_approved',
        'group_kicked', 'group_admin_change', 'dismissed', 'offline_messages'
      ];
      return validTypes.includes(type);
    }
    
    function isValidMessageType(type) {
      const validTypes = [
        'chat', 'image', 'file', 'login', 'error', 'login_ack', 'ping'
      ];
      return validTypes.includes(type);
    }
    
    // 解析扩展字段
    function parseExtra(extra) {
      if (!extra) return {};
      const result = {};
      extra.split(',').forEach(item => {
        const [key, value] = item.split(':');
        if (key && value) {
          result[key.trim()] = value.trim();
        }
      });
      return result;
    }
    
    // ========== 表情映射 ==========
    const emojiMap = {
      ':smile:': '😊', ':laugh:': '😄', ':cry:': '😢', ':angry:': '😠', ':heart:': '❤️', ':thumbsup:': '👍', ':thumbsdown:': '👎', ':ok:': '👌', ':clap:': '👏', ':wave:': '👋', ':pray:': '🙏', ':fire:': '🔥', ':star:': '⭐', ':moon:': '🌙', ':sun:': '☀️', ':rainbow:': '🌈', ':coffee:': '☕', ':beer:': '🍺', ':pizza:': '🍕', ':cake:': '🎂'
    };
    function replaceEmojis(text) {
      for (const code in emojiMap) {
        text = text.replaceAll(code, emojiMap[code]);
      }
      return text;
    }
    // ========== 表情按钮 ==========
    const emojiBtn = document.getElementById('emoji-btn');
    emojiBtn.onclick = function() {
      let menu = document.getElementById('emoji-menu');
      if (menu) { menu.remove(); return; }
      menu = document.createElement('div');
      menu.id = 'emoji-menu';
      menu.className = 'emoji-menu';
      for (const code in emojiMap) {
        const btn = document.createElement('button');
        btn.type = 'button';
        btn.textContent = emojiMap[code];
        btn.title = code;
        btn.className = 'emoji-btn';
        btn.onclick = () => {
          chatInput.value += code;
          menu.remove();
          chatInput.focus();
        };
        menu.appendChild(btn);
      }
      document.body.appendChild(menu);
      document.addEventListener('click', function hideMenu(e) {
        if (!menu.contains(e.target) && e.target !== emojiBtn) {
          menu.remove();
          document.removeEventListener('click', hideMenu);
        }
      });
    };

    // ========== 图片/文件按钮 ==========
    const imgBtn = document.getElementById('img-btn');
    const fileBtn = document.getElementById('file-btn');
    const fileInput = document.getElementById('file-input');
    imgBtn.onclick = () => { fileInput.accept = 'image/*'; fileInput.click(); };
    fileBtn.onclick = () => { fileInput.accept = '*/*'; fileInput.click(); };
    fileInput.onchange = async function() {
      if (!fileInput.files || !fileInput.files[0]) return;
      const file = fileInput.files[0];
      if (file.type.startsWith('image/')) {
        await sendFileOrImage(file, 'image');
      } else {
        await sendFileOrImage(file, 'file');
      }
      fileInput.value = '';
    };
    async function sendFileOrImage(file, type) {
      if (!(currentFriend || currentGroup)) { alert('请先选择好友或群组'); return; }
      if (!ws || ws.readyState !== 1) { alert('WebSocket未连接'); return; }
      // 上传文件
      const form = new FormData();
      form.append('file', file);
      const resp = await fetch(`${API_BASE}${API_PATHS.upload}`, { method: 'POST', body: form });
      if (!resp.ok) { alert('上传失败'); return; }
      const buf = await resp.arrayBuffer();
      const APIResp = root.lookupType('protocol.APIResp');
      const FileInfo = root.lookupType('protocol.FileInfo');
      const apiMsg = APIResp.decode(new Uint8Array(buf));
      if (apiMsg.code !== 0) { alert('上传失败: ' + apiMsg.msg); return; }
      const fileInfo = FileInfo.decode(apiMsg.data);
      // 发送消息
      const msgObj = {
        from: myUid,
        type: type,
        content: fileInfo.url || '', // 只用url字段
        extra: fileInfo.original_name || '', // 用original_name字段
        filename: fileInfo.filename || '',
        filesize: fileInfo.size || file.size,
        mimeType: fileInfo.type || file.type,
        timestamp: Math.floor(Date.now()/1000),
      };
      if (currentFriend) {
        msgObj.to = currentFriend.uid;
      } else if (currentGroup) {
        msgObj.groupId = currentGroup.groupId;
      }
      const msgBuf = IMMessage.encode(IMMessage.create(msgObj)).finish();
      ws.send(msgBuf);
      
      // 立即显示回显
      appendMessage({ 
        from: myUid, 
        content: msgObj.content, 
        self: true, 
        timestamp: msgObj.timestamp, 
        type: type, 
        extra: msgObj.extra 
      });
    }

    // ========== 消息发送 ==========
    sendBtn.onclick = sendMessage;
    chatInput.addEventListener('keydown', function(e) {
      if (e.key === 'Enter') sendMessage();
    });

    async function sendMessage() {
      if (!(currentFriend || currentGroup)) {
        alert('请先选择好友或群组');
        return;
      }
      let content = chatInput.value.trim();
      if (!content) return;
      if (!ws || ws.readyState !== 1) {
        alert('WebSocket未连接');
        return;
      }
      
      // 表情替换（仅用于显示）
      const displayContent = replaceEmojis(chatInput.value.trim());
      
      // 构造IMMessage
      const msgObj = {
        from: myUid,
        type: 'chat',
        content: content,
        timestamp: Math.floor(Date.now()/1000),
      };
      if (currentFriend) {
        msgObj.to = currentFriend.uid;
      } else if (currentGroup) {
        msgObj.groupId = currentGroup.groupId;
      }
      const msgBuf = IMMessage.encode(IMMessage.create(msgObj)).finish();
      ws.send(msgBuf);
      
      // 立即显示回显
      appendMessage({ 
        from: myUid, 
        content: displayContent, 
        self: true, 
        timestamp: msgObj.timestamp, 
        type: msgObj.type 
      });
      
      chatInput.value = '';
    }

    function appendMessage({ from, content, self, timestamp, type, extra, username }) {
      // 移除暂无消息提示
      const emptyTip = chatHistoryDiv.querySelector('.empty-tip');
      if (emptyTip) emptyTip.remove();
      const div = document.createElement('div');
      div.className = `message-container ${self ? 'self' : 'other'}`;
      let html = '';
      // 群聊消息显示昵称
      let nickname = '';
      if (currentGroup && !self) {
        // 优先显示群昵称，格式: nickname:xxx
        if (extra && typeof extra === 'string') {
          const match = extra.match(/nickname:([^,]+)/);
          if (match) nickname = match[1];
        }
        // 如果没有群昵称，则显示用户名
        if (!nickname && username) nickname = username;
        // 最后兜底显示UID
        if (!nickname) nickname = from;
        if (nickname) {
          html += `<div style='font-size:0.98em;color:#409eff;margin-bottom:2px;'>${nickname}</div>`;
        }
      }
      if (type === 'image') {
        // 用API_BASE拼接图片URL
        const imgUrl = (content && content.startsWith('http')) ? content : (content ? API_BASE + content : '');
        // 用 <a href="图片URL" download> 包裹图片，实现点击下载
        html += imgUrl ? `<a href="${imgUrl}" download><img src="${imgUrl}" alt="图片" style="max-width:180px;max-height:120px;border-radius:8px;vertical-align:middle;cursor:pointer;"></a>` : '';
        if (extra) html += `<div style="font-size:0.9em;color:#888;">${extra}</div>`;
      } else if (type === 'file') {
        // 用API_BASE拼接文件URL
        const fileUrl = (content && content.startsWith('http')) ? content : (content ? API_BASE + content : '');
        html += fileUrl ? `<a href="${fileUrl}" target="_blank" style="color:#409eff;text-decoration:underline;">${extra || '文件'}</a>` : '';
      } else {
        html += `<span style="display:inline-block;padding:6px 14px;border-radius:16px;background:${self ? '#409eff' : '#eee'};color:${self ? '#fff' : '#222'};max-width:60%;word-break:break-all;">${content}</span>`;
      }
      // 格式化时间
      const timeStr = timestamp ? formatTime(timestamp) : '';
      if (timeStr) {
        html += `<div style="font-size:0.85em;color:#bbb;margin-top:2px;">${timeStr}</div>`;
      }
      div.innerHTML = html;
      chatHistoryDiv.appendChild(div);
      chatHistoryDiv.scrollTop = chatHistoryDiv.scrollHeight;
    }
    function formatTime(ts) {
      if (!ts) return '';
      const d = new Date(Number(ts) * 1000); // 将秒转换为毫秒
      const y = d.getFullYear();
      const m = (d.getMonth() + 1).toString().padStart(2, '0');
      const day = d.getDate().toString().padStart(2, '0');
      const h = d.getHours().toString().padStart(2, '0');
      const min = d.getMinutes().toString().padStart(2, '0');
      const s = d.getSeconds().toString().padStart(2, '0');
      return `${y}-${m}-${day} ${h}:${min}:${s}`;
    }

    function connectWebSocket() {
      // 只有在有token的情况下才连接WebSocket
      if (!token) {
        return;
      }
      
      if (ws) ws.close();
      ws = new WebSocket(WS_BASE);
      ws.binaryType = 'arraybuffer';
      ws.onopen = () => {
        // 登录认证
        const loginMsg = IMMessage.encode(IMMessage.create({ type: 'login', token })).finish();
        ws.send(loginMsg);
        wsReady = false;
        
        // 启动心跳
        if (heartbeatInterval) clearInterval(heartbeatInterval);
        heartbeatInterval = setInterval(() => {
          // 发送心跳包 - 使用 Protobuf 格式
          try {
            const pingMsg = IMMessage.encode(IMMessage.create({ type: 'ping' })).finish();
            ws.send(pingMsg);
          } catch (e) {}
        }, 30000); // 30秒
      };
      ws.onclose = () => {
        if (heartbeatInterval) clearInterval(heartbeatInterval);
        if (token) {
          setTimeout(connectWebSocket, 2000);
        }
      };
      ws.onerror = () => {
        ws.close();
      };
      ws.onmessage = async (event) => {
        // 仅处理二进制protobuf消息，忽略任何文本/非二进制控制信息
        if (!(event.data instanceof ArrayBuffer)) {
          return;
        }
        try {
          const buf = new Uint8Array(event.data);
          
          // 忽略心跳回复
          if (typeof event.data === 'string' && event.data === 'pong') return;
          
          // 首先尝试解码为 Notification（通知消息）
          let notif = null;
          try {
            notif = Notification.decode(buf);
            // 验证是否为有效的通知消息
            if (notif && notif.type && isValidNotificationType(notif.type)) {
              // 收到通知
              
              // 判断是否为当前聊天对象的消息，若是则不弹窗
              if (
                (notif.type === 'private_chat_message' && currentFriend && notif.from === currentFriend.uid) ||
                (notif.type === 'group_chat_message' && currentGroup && notif.groupId === currentGroup.groupId)
              ) {
                // 不弹窗
                return;
              }
              
              let display;
              switch (notif.type) {
                case 'friend_request':
                  display = `[好友请求] ${notif.fromUsername}(${notif.from}) 请求加你为好友`;
                  break;
                case 'private_chat_message':
                  display = `[私聊] ${notif.fromUsername}: ${notif.content}`;
                  // 通知仅用于弹窗提示；未读统一由 IMMessage 分支计算
                  break;
                case 'group_chat_message':
                  display = `[群聊][${notif.groupName || ''}] ${notif.fromUsername}: ${notif.content}`;
                  // 通知仅用于弹窗提示；未读统一由 IMMessage 分支计算
                  break;
                case 'group_application_pending':
                  display = `[群申请] ${notif.fromUsername}(${notif.from}) 申请加入群聊 [${notif.groupName || ''}]`;
                  break;
                case 'group_invite': {
                  const extra = parseExtra(notif.extra || '');
                  display = `[群审批] ${notif.fromUsername} 邀请 ${extra.invitee_username || ''} 加入群聊 [${notif.groupName || ''}]，请审批`;
                  break;
                }
                case 'group_invite_approved':
                  display = `[群通知] 你已成功加入群聊 [${notif.groupName || ''}]`;
                  break;
                case 'group_kicked':
                  display = `[群通知] 你已被 ${notif.fromUsername} 移出群聊 [${notif.groupName || ''}]`;
                  break;
                case 'group_admin_change': {
                  const extra = parseExtra(notif.extra || '');
                  display = extra.set_admin === 'true'
                    ? `[群通知] 你在群聊 [${notif.groupName || ''}] 被设为管理员`
                    : `[群通知] 你在群聊 [${notif.groupName || ''}] 被取消管理员`;
                  break;
                }
                case 'dismissed':
                  display = `[群系统][${notif.groupName || ''}] 群已被解散 by ${notif.fromUsername}`;
                  break;
                case 'offline_messages':
                  display = `[离线消息] ${notif.content}`;
                  break;
                default:
                  return;
              }
              notifyList.unshift(display);
              if (notifyList.length > 30) notifyList.length = 30;
              renderNotifyPopup();
              setTimeout(() => {
                notifyPopup.style.display = 'block';
                alert(display);
              }, 10);
              return;
            }
          } catch (e) {
            // Notification 解码失败，继续尝试 IMMessage（静默）
          }
          
          // 尝试解码为 IMMessage（聊天消息）
          let msg = null;
          try {
            msg = IMMessage.decode(buf);
            // 验证是否为有效的聊天消息
            if (!(msg && msg.type && isValidMessageType(msg.type))) {
              return;
            }
          } catch (e) {
            return;
          }
          
          // 连接就绪信号与心跳
          if (msg.type === 'ping') { return; }
          if (msg.type === 'login_ack') { wsReady = true; return; }
          if (!wsReady) { return; }
          
          if (!msg) return;
          // 私聊消息
          // 收到消息
          console.log('收到消息', msg);
          if (msg.type === 'chat' && currentFriend && msg.from === currentFriend.uid && msg.to === myUid) {
            appendMessage({ from: msg.from, content: replaceEmojis(msg.content), self: false, timestamp: msg.timestamp, type: msg.type });
          } else if (msg.type === 'image' && currentFriend && msg.from === currentFriend.uid && msg.to === myUid) {
            appendMessage({ from: msg.from, content: msg.content, self: false, timestamp: msg.timestamp, type: 'image', extra: msg.extra });
          } else if (msg.type === 'file' && currentFriend && msg.from === currentFriend.uid && msg.to === myUid) {
            appendMessage({ from: msg.from, content: msg.content, self: false, timestamp: msg.timestamp, type: 'file', extra: msg.extra });
          }
          // 私聊未读
          else if ((msg.type === 'chat' || msg.type === 'image' || msg.type === 'file') && msg.to === myUid && (!currentFriend || msg.from !== currentFriend.uid)) {
            unreadMap['user:' + msg.from] = (unreadMap['user:' + msg.from] || 0) + 1;
            if (friendListCache) {
              renderFriendList(friendListCache);
            }
          }
          // 群聊消息
          else if (msg.type === 'chat' && currentGroup && msg.groupId === currentGroup.groupId) {
            appendMessage({ from: msg.from, content: replaceEmojis(msg.content), self: msg.from === myUid, timestamp: msg.timestamp, type: msg.type, extra: msg.extra, username: msg.fromUsername });
          } else if (msg.type === 'image' && currentGroup && msg.groupId === currentGroup.groupId) {
            appendMessage({ from: msg.from, content: msg.content, self: msg.from === myUid, timestamp: msg.timestamp, type: 'image', extra: msg.extra });
          } else if (msg.type === 'file' && currentGroup && msg.groupId === currentGroup.groupId) {
            appendMessage({ from: msg.from, content: msg.content, self: msg.from === myUid, timestamp: msg.timestamp, type: 'file', extra: msg.extra });
          }
          // 群聊未读
          else if ((msg.type === 'chat' || msg.type === 'image' || msg.type === 'file') && msg.groupId && (!currentGroup || msg.groupId !== currentGroup.groupId)) {
            unreadMap['group:' + msg.groupId] = (unreadMap['group:' + msg.groupId] || 0) + 1;
            renderGroupList(groupListCache);
          }
          else if (msg.type === 'error') {
            if (msg.content && (msg.content.indexOf('请先登录') !== -1 || msg.content.indexOf('未登录') !== -1)) {
              // 未登录相关错误静默处理
              return;
            }
            console.log('收到error消息', msg);
            if (msg.content && msg.content.indexOf('禁言') !== -1) {
              alert('你已被群主或管理员禁言，无法发送消息');
            } else {
              alert('消息错误: ' + msg.content);
            }
          }
          // 处理自己发送的消息回显（包括私聊和群聊）
          else if ((msg.type === 'chat' || msg.type === 'image' || msg.type === 'file') && msg.from === myUid) {
            // 检查是否在正确的聊天界面
            const isInCorrectChat = 
              (currentFriend && msg.to === currentFriend.uid) || 
              (currentGroup && msg.groupId === currentGroup.groupId);
            
            if (isInCorrectChat) {
              const displayContent = msg.type === 'chat' ? replaceEmojis(msg.content) : msg.content;
              
              appendMessage({ 
                from: myUid, 
                content: displayContent, 
                self: true, 
                timestamp: msg.timestamp, 
                type: msg.type,
                extra: msg.extra,
                username: msg.fromUsername
              });
            }
          }
        } catch (e) {}
      };
    }

    function showMainPanel() {
      loginPanel.style.display = 'none';
      registerPanel.style.display = 'none';
      mainPanel.style.display = '';
      // 重新绑定个人中心按钮，确保元素已渲染
      const userCenterBtn = document.getElementById('user-center-btn');
      if (userCenterBtn) {
        userCenterBtn.onclick = function() {
          showModal({
            title: '个人中心',
            content: `<div style='margin-bottom:12px;'>
              <button id='uc-info' style='margin:4px 0;width:100%;padding:7px 0;'>查看个人信息</button><br>
              <button id='uc-nick' style='margin:4px 0;width:100%;padding:7px 0;'>修改昵称</button><br>
              <button id='uc-pwd' style='margin:4px 0;width:100%;padding:7px 0;'>修改密码</button><br>
              <button id='uc-del' style='margin:4px 0;width:100%;padding:7px 0;color:#e74c3c;'>注销账号</button><br>
              <button id='uc-logout' style='margin:4px 0;width:100%;padding:7px 0;'>登出</button>
            </div>`,
            okText: '关闭',
            cancelText: '',
            onOk: null
          });
          setTimeout(() => {
            const infoBtn = document.getElementById('uc-info');
            if (infoBtn) infoBtn.onclick = async function() {
              const UserInfoReq = root.lookupType('protocol.UserInfoReq');
              const APIResp = root.lookupType('protocol.APIResp');
              const UserInfoResp = root.lookupType('protocol.UserInfoResp');
              const reqBuf = UserInfoReq.encode(UserInfoReq.create({ token })).finish();
              try {
                const resp = await fetch(`${API_BASE}${API_PATHS.user_info}`, {
                  method: 'POST',
                  headers: { 'Content-Type': 'application/x-protobuf' },
                  body: reqBuf
                });
                const buf = await resp.arrayBuffer();
                const apiMsg = APIResp.decode(new Uint8Array(buf));
                if (apiMsg.code !== 0) throw new Error(apiMsg.msg);
                const info = UserInfoResp.decode(apiMsg.data);
                let html = `<div style='margin-bottom:8px;'><b>UID:</b> ${info.uid}</div>`;
                html += `<div style='margin-bottom:8px;'><b>昵称:</b> ${info.username}</div>`;
                html += `<div style='margin-bottom:8px;'><b>邮箱:</b> ${info.email}</div>`;
                showModal({ title: '个人信息', content: html, okText: '关闭' });
              } catch (e) { alert('获取失败: ' + (e.message || e)); }
            };
            const nickBtn = document.getElementById('uc-nick');
            if (nickBtn) nickBtn.onclick = function() {
              showModal({
                title: '修改昵称',
                inputs: [{ label: '新昵称', placeholder: '输入新昵称' }],
                okText: '保存',
                onOk: async ([newUsername]) => {
                  if (!newUsername) return;
                  const UpdateUsernameReq = root.lookupType('protocol.UpdateUsernameReq');
                  const APIResp = root.lookupType('protocol.APIResp');
                  const reqBuf = UpdateUsernameReq.encode(UpdateUsernameReq.create({ uid: myUid, newUsername })).finish();
                  const resp = await fetch(`${API_BASE}${API_PATHS.update_username}`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-protobuf' },
                    body: reqBuf
                  });
                  const buf = await resp.arrayBuffer();
                  const apiMsg = APIResp.decode(new Uint8Array(buf));
                  alert(apiMsg.msg || (apiMsg.code === 0 ? '昵称已更新' : '设置失败'));
                }
              });
            };
            const pwdBtn = document.getElementById('uc-pwd');
            if (pwdBtn) pwdBtn.onclick = function() {
              showModal({
                title: '修改密码',
                inputs: [
                  { label: '原密码', type: 'password', placeholder: '原密码' },
                  { label: '新密码', type: 'password', placeholder: '新密码' }
                ],
                okText: '保存',
                onOk: async ([oldPwd, newPwd]) => {
                  if (!oldPwd || !newPwd) return;
                  const UpdatePwdReq = root.lookupType('protocol.UpdatePwdReq');
                  const APIResp = root.lookupType('protocol.APIResp');
                  const reqBuf = UpdatePwdReq.encode(UpdatePwdReq.create({ uid: myUid, oldPwd, newPwd })).finish();
                  const resp = await fetch(`${API_BASE}${API_PATHS.update_pwd}`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-protobuf' },
                    body: reqBuf
                  });
                  const buf = await resp.arrayBuffer();
                  const apiMsg = APIResp.decode(new Uint8Array(buf));
                  alert(apiMsg.msg || (apiMsg.code === 0 ? '密码已更新' : '设置失败'));
                }
              });
            };
            const delBtn = document.getElementById('uc-del');
            if (delBtn) delBtn.onclick = function() {
              if (!confirm('确定要注销账号吗？')) return;
              const DeleteAccountReq = root.lookupType('protocol.DeleteAccountReq');
              const APIResp = root.lookupType('protocol.APIResp');
              const reqBuf = DeleteAccountReq.encode(DeleteAccountReq.create({ uid: myUid })).finish();
              fetch(`${API_BASE}${API_PATHS.delete_account}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-protobuf' },
                body: reqBuf
              }).then(async resp => {
                const buf = await resp.arrayBuffer();
                const apiMsg = APIResp.decode(new Uint8Array(buf));
                alert(apiMsg.msg || (apiMsg.code === 0 ? '账号已注销' : '注销失败'));
                if (apiMsg.code === 0) {
                  window.localStorage.removeItem('token');
                  window.localStorage.removeItem('uid');
                  token = null;
                  myUid = null;
                  myUsername = null;
                  window.myUid = myUid;
                  if (ws) { ws.close(); ws = null; }
                  showLoginPanel();
                }
              });
            };
            const logoutBtn = document.getElementById('uc-logout');
            if (logoutBtn) logoutBtn.onclick = async function() {
              if (!token) { showLoginPanel(); return; }
              
              // 先执行本地登出逻辑
              window.localStorage.removeItem('token');
              window.localStorage.removeItem('uid');
              token = null;
              myUid = null;
              myUsername = null;
              window.myUid = myUid;
              if (ws) { ws.close(); ws = null; }
              showLoginPanel();
              
              // 尝试通知服务器（不阻塞用户界面）
              try {
                const LogoutReq = root.lookupType('protocol.LogoutReq');
                const reqBuf = LogoutReq.encode(LogoutReq.create({ token: window.localStorage.getItem('token') })).finish();
                const resp = await fetch(`${API_BASE}${API_PATHS.logout}`, {
                  method: 'POST',
                  headers: { 'Content-Type': 'application/x-protobuf' },
                  body: reqBuf
                });
                if (!resp.ok) {
                  console.warn('登出API调用失败，但本地已登出');
                }
              } catch (e) {
                console.warn('登出API调用异常，但本地已登出:', e.message);
              }
            };
          }, 100);
        };
      }
    }
    function showLoginPanel() {
      loginPanel.style.display = '';
      registerPanel.style.display = 'none';
      mainPanel.style.display = 'none';
    }

    // ========== 主界面逻辑骨架 ==========
    // TODO: 填充群组列表，聊天窗口，WebSocket消息接收等
    tabGroups.onclick = () => {
      tabGroups.classList.add('active');
      tabFriends.classList.remove('active');
      friendListDiv.style.display = 'none';
      groupListDiv.style.display = '';
      chatTitleDiv.textContent = '';
      chatHistoryDiv.innerHTML = '';
      currentGroup = null;
      currentFriend = null;
      window.currentGroup = currentGroup;
      window.currentFriend = currentFriend;
      if (friendListHeader) friendListHeader.style.display = 'none';
      if (groupListHeader) groupListHeader.style.display = 'flex';
      fetchGroupList(token);
    };
    tabFriends.onclick = () => {
      tabFriends.classList.add('active');
      tabGroups.classList.remove('active');
      friendListDiv.style.display = '';
      groupListDiv.style.display = 'none';
      chatTitleDiv.textContent = '';
      chatHistoryDiv.innerHTML = '';
      currentGroup = null;
      currentFriend = null;
      window.currentGroup = currentGroup;
      window.currentFriend = currentFriend;
      if (friendListHeader) friendListHeader.style.display = 'flex';
      if (groupListHeader) groupListHeader.style.display = 'none';
      fetchFriendList(myUid, token);
    };

    // ========== 添加好友/群组按钮 ==========
    const addFriendBtn = document.getElementById('add-friend-btn');
    if (addFriendBtn) {
      addFriendBtn.onclick = async function() {
        showModal({
          title: '好友操作',
          content: `<div style='display:flex;flex-direction:column;gap:12px;padding:8px 0;'>
            <button id='btn-add-friend' style='padding:10px 0;font-size:1em;border-radius:4px;border:none;background:#409eff;color:#fff;cursor:pointer;'>添加好友</button>
            <button id='btn-handle-friend-req' style='padding:10px 0;font-size:1em;border-radius:4px;border:none;background:#eee;color:#222;cursor:pointer;'>处理好友请求</button>
          </div>`,
          okText: '关闭',
          cancelText: '',
          onOk: null
        });
        setTimeout(() => {
          const btnAdd = document.getElementById('btn-add-friend');
          if (btnAdd) btnAdd.onclick = function() {
            showModal({
              title: '添加好友',
              inputs: [
                { label: '对方UID', placeholder: '请输入对方UID' },
                { label: '验证消息', placeholder: '验证消息(可选)' }
              ],
              okText: '发送请求',
              onOk: async ([toUid, verifyMsg]) => {
                if (!toUid) return;
                if (toUid && token) {
                  const AddFriendReq = root.lookupType('protocol.AddFriendReq');
                  const APIResp = root.lookupType('protocol.APIResp');
                  const reqBuf = AddFriendReq.encode(AddFriendReq.create({ fromUid: myUid, toUid, verifyMsg, token })).finish();
                  try {
                    const resp = await fetch(`${API_BASE}${API_PATHS.add_friend}`, {
                      method: 'POST',
                      headers: { 'Content-Type': 'application/x-protobuf' },
                      body: reqBuf
                    });
                    const buf = await resp.arrayBuffer();
                    const msg = APIResp.decode(new Uint8Array(buf));
                    alert(msg.msg || (msg.code === 0 ? '好友请求已发送' : '添加失败'));
                  } catch (e) {
                    alert('添加失败: ' + (e.message || e));
                  }
                }
              }
            });
          };
          const btnHandle = document.getElementById('btn-handle-friend-req');
          if (btnHandle) btnHandle.onclick = async function() {
            // 拉取好友请求列表
            const FriendListReq = root.lookupType('protocol.FriendListReq');
            const APIResp = root.lookupType('protocol.APIResp');
            const FriendRequestListResp = root.lookupType('protocol.FriendRequestListResp');
            const reqBuf = FriendListReq.encode(FriendListReq.create({ uid: myUid, token })).finish();
            try {
              const resp = await fetch(`${API_BASE}${API_PATHS.friend_request_list}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-protobuf' },
                body: reqBuf
              });
              const buf = await resp.arrayBuffer();
              const msg = APIResp.decode(new Uint8Array(buf));
              if (msg.code !== 0) throw new Error(msg.msg);
              const reqList = FriendRequestListResp.decode(msg.data);
              let html = '';
              if (!reqList.fromUids || reqList.fromUids.length === 0) {
                html = `<div style='color:#888;padding:12px;'>暂无好友请求</div>`;
              } else {
                html = reqList.fromUids.map((uid, i) => {
                  const uname = reqList.fromUsernames[i] || uid;
                  const vmsg = reqList.verifyMsgs[i] || '';
                  return `<div style='margin-bottom:10px;padding:8px 0;border-bottom:1px solid #eee;'>
                    <b>${uname}</b> (UID: ${uid})<br>
                    <span style='color:#888;'>验证消息: ${vmsg}</span><br>
                    <button class='btn-accept-friend' data-uid='${uid}' style='margin:6px 8px 0 0;padding:4px 14px;background:#409eff;color:#fff;border:none;border-radius:4px;cursor:pointer;'>同意</button>
                    <button class='btn-reject-friend' data-uid='${uid}' style='margin:6px 0 0 0;padding:4px 14px;background:#eee;color:#222;border:none;border-radius:4px;cursor:pointer;'>拒绝</button>
                  </div>`;
                }).join('');
              }
              showModal({
                title: '处理好友请求',
                content: html,
                okText: '关闭',
                cancelText: '',
                onOk: null
              });
              setTimeout(() => {
                document.querySelectorAll('.btn-accept-friend').forEach(btn => {
                  btn.onclick = async function() {
                    const fromUid = btn.getAttribute('data-uid');
                    const HandleFriendRequestReq = root.lookupType('protocol.HandleFriendRequestReq');
                    const reqBuf = HandleFriendRequestReq.encode(HandleFriendRequestReq.create({ fromUid, toUid: myUid, accept: true, token })).finish();
                    const resp = await fetch(`${API_BASE}${API_PATHS.handle_friend_request}`, {
                      method: 'POST',
                      headers: { 'Content-Type': 'application/x-protobuf' },
                      body: reqBuf
                    });
                    const buf = await resp.arrayBuffer();
                    const msg = APIResp.decode(new Uint8Array(buf));
                    alert(msg.msg || (msg.code === 0 ? '已同意' : '操作失败'));
                  };
                });
                document.querySelectorAll('.btn-reject-friend').forEach(btn => {
                  btn.onclick = async function() {
                    const fromUid = btn.getAttribute('data-uid');
                    const HandleFriendRequestReq = root.lookupType('protocol.HandleFriendRequestReq');
                    const reqBuf = HandleFriendRequestReq.encode(HandleFriendRequestReq.create({ fromUid, toUid: myUid, accept: false, token })).finish();
                    const resp = await fetch(`${API_BASE}${API_PATHS.handle_friend_request}`, {
                      method: 'POST',
                      headers: { 'Content-Type': 'application/x-protobuf' },
                      body: reqBuf
                    });
                    const buf = await resp.arrayBuffer();
                    const msg = APIResp.decode(new Uint8Array(buf));
                    alert(msg.msg || (msg.code === 0 ? '已拒绝' : '操作失败'));
                  };
                });
              }, 50);
            } catch (e) {
              showModal({ title: '处理好友请求', content: `<div style='color:#e74c3c;padding:12px;'>加载失败: ${e.message || e}</div>`, okText: '关闭' });
            }
          };
        }, 50);
      };
    }
    const addGroupBtn = document.getElementById('add-group-btn');
    if (addGroupBtn) {
      addGroupBtn.onclick = async function() {
        showModal({
          title: '群组操作',
          content: `<div style='display:flex;flex-direction:column;gap:12px;padding:8px 0;'>
            <button id='btn-create-group' style='padding:10px 0;font-size:1em;border-radius:4px;border:none;background:#409eff;color:#fff;cursor:pointer;'>创建群组</button>
            <button id='btn-join-group' style='padding:10px 0;font-size:1em;border-radius:4px;border:none;background:#eee;color:#222;cursor:pointer;'>加入群组</button>
            <button id='btn-handle-group-req' style='padding:10px 0;font-size:1em;border-radius:4px;border:none;background:#eee;color:#222;cursor:pointer;'>处理群组请求</button>
          </div>`,
          okText: '关闭',
          cancelText: '',
          onOk: null
        });
        setTimeout(() => {
          const btnCreate = document.getElementById('btn-create-group');
          if (btnCreate) btnCreate.onclick = function() {
            showModal({
              title: '创建群组',
              inputs: [
                { label: '群组名称', placeholder: '输入群组名称' }
              ],
              okText: '创建',
              onOk: async ([groupName]) => {
                if (!groupName) return;
                const CreateGroupReq = root.lookupType('protocol.CreateGroupReq');
                const APIResp = root.lookupType('protocol.APIResp');
                const reqBuf = CreateGroupReq.encode(CreateGroupReq.create({ name: groupName, ownerUid: myUid, token })).finish();
                try {
                  const resp = await fetch(`${API_BASE}${API_PATHS.create_group}`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-protobuf' },
                    body: reqBuf
                  });
                  const buf = await resp.arrayBuffer();
                  const msg = APIResp.decode(new Uint8Array(buf));
                  alert(msg.msg || (msg.code === 0 ? '群组创建成功' : '创建失败'));
                  fetchGroupList(token);
                } catch (e) {
                  alert('创建失败: ' + (e.message || e));
                }
              }
            });
          };
          const btnJoin = document.getElementById('btn-join-group');
          if (btnJoin) btnJoin.onclick = function() {
            showModal({
              title: '加入群组',
              inputs: [
                { label: '群组ID', placeholder: '输入群组ID' }
              ],
              okText: '申请加入',
              onOk: async ([groupId]) => {
                if (!groupId) return;
                const JoinGroupReq = root.lookupType('protocol.JoinGroupReq');
                const APIResp = root.lookupType('protocol.APIResp');
                const reqBuf = JoinGroupReq.encode(JoinGroupReq.create({ groupId, uid: myUid, token })).finish();
                try {
                  const resp = await fetch(`${API_BASE}${API_PATHS.join_group}`, {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-protobuf' },
                    body: reqBuf
                  });
                  const buf = await resp.arrayBuffer();
                  const msg = APIResp.decode(new Uint8Array(buf));
                  alert(msg.msg || (msg.code === 0 ? '入群申请已发送' : '加入失败'));
                } catch (e) {
                  alert('加入失败: ' + (e.message || e));
                }
              }
            });
          };
          const btnHandle = document.getElementById('btn-handle-group-req');
          if (btnHandle) btnHandle.onclick = async function() {
            // 拉取待审批群组邀请请求
            const GroupRequestListReq = root.lookupType('protocol.GroupRequestListReq');
            const APIResp = root.lookupType('protocol.APIResp');
            const GroupRequestListResp = root.lookupType('protocol.GroupRequestListResp');
            const reqBuf = GroupRequestListReq.encode(GroupRequestListReq.create({ uid: myUid, token })).finish();
            try {
              const resp = await fetch(`${API_BASE}${API_PATHS.group_request_list}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-protobuf' },
                body: reqBuf
              });
              const buf = await resp.arrayBuffer();
              const msg = APIResp.decode(new Uint8Array(buf));
              if (msg.code !== 0) throw new Error(msg.msg);
              const reqList = GroupRequestListResp.decode(msg.data);
              let html = '';
              if (!reqList.items || reqList.items.length === 0) {
                html = `<div style='color:#888;padding:12px;'>暂无待审批的群组请求</div>`;
              } else {
                html = reqList.items.map(item => {
                  return `<div style='margin-bottom:10px;padding:8px 0;border-bottom:1px solid #eee;'>
                    <b>群名:</b> ${item.groupName} (ID: ${item.groupId})<br>
                    <b>邀请人:</b> ${item.inviterUsername} (UID: ${item.inviterUid})<br>
                    <b>被邀请:</b> ${item.inviteeUsername} (UID: ${item.inviteeUid})<br>
                    <b>状态:</b> ${item.status}<br>
                    <button class='btn-approve-group' data-id='${item.id}' data-group='${item.groupId}' data-invitee='${item.inviteeUid}' style='margin:6px 8px 0 0;padding:4px 14px;background:#409eff;color:#fff;border:none;border-radius:4px;cursor:pointer;'>同意</button>
                    <button class='btn-reject-group' data-id='${item.id}' data-group='${item.groupId}' data-invitee='${item.inviteeUid}' style='margin:6px 0 0 0;padding:4px 14px;background:#eee;color:#222;border:none;border-radius:4px;cursor:pointer;'>拒绝</button>
                  </div>`;
                }).join('');
              }
              showModal({
                title: '处理群组请求',
                content: html,
                okText: '关闭',
                cancelText: '',
                onOk: null
              });
              setTimeout(() => {
                document.querySelectorAll('.btn-approve-group').forEach(btn => {
                  btn.onclick = async function() {
                    const id = btn.getAttribute('data-id');
                    const groupId = btn.getAttribute('data-group');
                    const inviteeUid = btn.getAttribute('data-invitee');
                    const HandleGroupRequestReq = root.lookupType('protocol.HandleGroupRequestReq');
                    const reqBuf = HandleGroupRequestReq.encode(HandleGroupRequestReq.create({ id, groupId, inviteeUid, approve: true, token })).finish();
                    const resp = await fetch(`${API_BASE}${API_PATHS.handle_group_request}`, {
                      method: 'POST',
                      headers: { 'Content-Type': 'application/x-protobuf' },
                      body: reqBuf
                    });
                    const buf = await resp.arrayBuffer();
                    const msg = APIResp.decode(new Uint8Array(buf));
                    alert(msg.msg || (msg.code === 0 ? '已同意' : '操作失败'));
                  };
                });
                document.querySelectorAll('.btn-reject-group').forEach(btn => {
                  btn.onclick = async function() {
                    const id = btn.getAttribute('data-id');
                    const groupId = btn.getAttribute('data-group');
                    const inviteeUid = btn.getAttribute('data-invitee');
                    const HandleGroupRequestReq = root.lookupType('protocol.HandleGroupRequestReq');
                    const reqBuf = HandleGroupRequestReq.encode(HandleGroupRequestReq.create({ id, groupId, inviteeUid, approve: false, token })).finish();
                    const resp = await fetch(`${API_BASE}${API_PATHS.handle_group_request}`, {
                      method: 'POST',
                      headers: { 'Content-Type': 'application/x-protobuf' },
                      body: reqBuf
                    });
                    const buf = await resp.arrayBuffer();
                    const msg = APIResp.decode(new Uint8Array(buf));
                    alert(msg.msg || (msg.code === 0 ? '已拒绝' : '操作失败'));
                  };
                });
              }, 50);
            } catch (e) {
              showModal({ title: '处理群组请求', content: `<div style='color:#e74c3c;padding:12px;'>加载失败: ${e.message || e}</div>`, okText: '关闭' });
            }
          };
        }, 50);
      };
    }

    const notifyBtn = document.getElementById('notify-btn');
    const notifyPopup = document.getElementById('notify-popup');
    let notifyList = [];

    // 通知按钮点击展开/收起
    notifyBtn.onclick = function() {
      if (notifyPopup.style.display === 'none' || !notifyPopup.style.display) {
        renderNotifyPopup();
        notifyPopup.style.display = 'block';
      } else {
        notifyPopup.style.display = 'none';
      }
    };
    function renderNotifyPopup() {
      if (notifyList.length === 0) {
        notifyPopup.innerHTML = '<div style="color:#888;padding:16px;">暂无通知</div>';
        return;
      }
      notifyPopup.innerHTML = notifyList.map(n => `<div style="padding:10px 16px;border-bottom:1px solid #eee;word-break:break-all;">${n}</div>`).join('');
    }
    // 点击弹窗外关闭
    document.addEventListener('click', function(e) {
      if (notifyPopup.style.display === 'block' && !notifyPopup.contains(e.target) && e.target !== notifyBtn) {
        notifyPopup.style.display = 'none';
      }
    });

    // 拉取最近N条私聊消息
    async function fetchRecentPrivateMessages(from, to, count) {
      // 保证key顺序和后端一致
      let uid1 = from, uid2 = to;
      if (uid1 > uid2) { const tmp = uid1; uid1 = uid2; uid2 = tmp; }
      const reqBuf = GetRecentPrivateMessagesReq.encode(GetRecentPrivateMessagesReq.create({ from: uid1, to: uid2, count })).finish();
      const resp = await fetch(`${API_BASE}${API_PATHS.get_recent_private_messages}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-protobuf' },
        body: reqBuf
      });
      if (!resp.ok) throw new Error('消息获取失败');
      const buf = await resp.arrayBuffer();
      const apiMsg = APIResp.decode(new Uint8Array(buf));
      if (apiMsg.code !== 0) throw new Error(apiMsg.msg);
      const msgList = IMMessageList.decode(apiMsg.data);
      return msgList.messages || [];
    }
    // 拉取最近N条群聊消息
    async function fetchRecentGroupMessages(groupId, count) {
      const reqBuf = GetRecentGroupMessagesReq.encode(GetRecentGroupMessagesReq.create({ groupId: groupId, count })).finish();
      const resp = await fetch(`${API_BASE}${API_PATHS.get_recent_group_messages}`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/x-protobuf' },
        body: reqBuf
      });
      if (!resp.ok) throw new Error('消息获取失败');
      const buf = await resp.arrayBuffer();
      const apiMsg = APIResp.decode(new Uint8Array(buf));
      if (apiMsg.code !== 0) throw new Error(apiMsg.msg);
      const msgList = IMMessageList.decode(apiMsg.data);
      return msgList.messages || [];
    }

    // 发送验证码功能
    document.addEventListener('click', function(e) {
      if (e.target.id === 'send-email-code') {
        const email = document.getElementById('reg-email').value.trim();
        if (!email) {
          alert('请先输入邮箱');
          return;
        }
        
        const btn = e.target;
        btn.disabled = true;
        btn.textContent = '发送中...';
        
        // 发送验证码请求
        const SendEmailCodeReq = root.lookupType('protocol.SendEmailCodeReq');
        const APIResp = root.lookupType('protocol.APIResp');
        const reqBuf = SendEmailCodeReq.encode(SendEmailCodeReq.create({ 
          email: email, 
          purpose: 'register' 
        })).finish();
        
        fetch(`${API_BASE}${API_PATHS.send_email_code}`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-protobuf' },
          body: reqBuf
        })
        .then(resp => resp.arrayBuffer())
        .then(buf => {
          const apiMsg = APIResp.decode(new Uint8Array(buf));
          if (apiMsg.code === 0) {
            alert('验证码已发送到您的邮箱');
            // 开始倒计时
            let countdown = 60;
            const timer = setInterval(() => {
              btn.textContent = `${countdown}秒后重发`;
              countdown--;
              if (countdown < 0) {
                clearInterval(timer);
                btn.disabled = false;
                btn.textContent = '发送验证码';
              }
            }, 1000);
          } else {
            alert('发送失败: ' + apiMsg.msg);
            btn.disabled = false;
            btn.textContent = '发送验证码';
          }
        })
        .catch(e => {
          alert('发送失败: ' + e.message);
          btn.disabled = false;
          btn.textContent = '发送验证码';
        });
      }
      
      // 忘记密码功能
      if (e.target.id === 'forgot-password') {
        e.preventDefault();
        showModal({
          title: '重置密码',
          content: `
            <div style="margin-bottom:12px;">
              <input type="email" id="reset-email" placeholder="请输入注册邮箱" style="width:100%;padding:8px;border:1px solid #ddd;border-radius:4px;margin-bottom:8px;">
              <div style="display:flex;gap:8px;margin-bottom:8px;">
                <input type="text" id="reset-email-code" placeholder="邮箱验证码" style="flex:1;padding:8px;border:1px solid #ddd;border-radius:4px;">
                <button type="button" id="send-reset-code" style="padding:8px 12px;background:#409eff;color:#fff;border:none;border-radius:4px;cursor:pointer;">发送验证码</button>
              </div>
              <input type="password" id="reset-new-password" placeholder="新密码" style="width:100%;padding:8px;border:1px solid #ddd;border-radius:4px;">
            </div>
          `,
          okText: '重置密码',
          onOk: async () => {
            const email = document.getElementById('reset-email').value.trim();
            const emailCode = document.getElementById('reset-email-code').value.trim();
            const newPassword = document.getElementById('reset-new-password').value;
            
            if (!email || !emailCode || !newPassword) {
              alert('请填写所有信息');
              return;
            }
            
            try {
              const ResetPasswordReq = root.lookupType('protocol.ResetPasswordReq');
              const APIResp = root.lookupType('protocol.APIResp');
              const reqBuf = ResetPasswordReq.encode(ResetPasswordReq.create({ 
                email: email, 
                emailCode: emailCode, 
                newPassword: newPassword 
              })).finish();
              
              const resp = await fetch(`${API_BASE}${API_PATHS.reset_password}`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-protobuf' },
                body: reqBuf
              });
              
              const buf = await resp.arrayBuffer();
              const apiMsg = APIResp.decode(new Uint8Array(buf));
              if (apiMsg.code === 0) {
                alert('密码重置成功，请使用新密码登录');
                return true; // 关闭模态框
              } else {
                alert('重置失败: ' + apiMsg.msg);
                return false;
              }
            } catch (e) {
              alert('重置失败: ' + e.message);
              return false;
            }
          }
        });
      }
      
      // 发送重置密码验证码
      if (e.target.id === 'send-reset-code') {
        const email = document.getElementById('reset-email').value.trim();
        if (!email) {
          alert('请先输入邮箱');
          return;
        }
        
        const btn = e.target;
        btn.disabled = true;
        btn.textContent = '发送中...';
        
        const SendEmailCodeReq = root.lookupType('protocol.SendEmailCodeReq');
        const APIResp = root.lookupType('protocol.APIResp');
        const reqBuf = SendEmailCodeReq.encode(SendEmailCodeReq.create({ 
          email: email, 
          purpose: 'reset_password' 
        })).finish();
        
        fetch(`${API_BASE}${API_PATHS.send_email_code}`, {
          method: 'POST',
          headers: { 'Content-Type': 'application/x-protobuf' },
          body: reqBuf
        })
        .then(resp => resp.arrayBuffer())
        .then(buf => {
          const apiMsg = APIResp.decode(new Uint8Array(buf));
          if (apiMsg.code === 0) {
            alert('验证码已发送到您的邮箱');
            // 开始倒计时
            let countdown = 60;
            const timer = setInterval(() => {
              btn.textContent = `${countdown}秒后重发`;
              countdown--;
              if (countdown < 0) {
                clearInterval(timer);
                btn.disabled = false;
                btn.textContent = '发送验证码';
              }
            }, 1000);
          } else {
            alert('发送失败: ' + apiMsg.msg);
            btn.disabled = false;
            btn.textContent = '发送验证码';
          }
        })
        .catch(e => {
          alert('发送失败: ' + e.message);
          btn.disabled = false;
          btn.textContent = '发送验证码';
        });
      }
    });
  });
}); 
