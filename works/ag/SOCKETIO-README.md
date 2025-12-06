# 🔌 Socket.IO 실시간 채팅 가이드

## 📋 목차
- [개요](#개요)
- [설치 및 실행](#설치-및-실행)
- [주요 기능](#주요-기능)
- [API 문서](#api-문서)
- [클라이언트 예제](#클라이언트-예제)

---

## 🎯 개요

Socket.IO를 사용한 실시간 양방향 통신 채팅 서버입니다.

### 주요 특징
- ✅ **실시간 메시지** - 즉시 전송/수신
- ✅ **방(Room) 기능** - 여러 채팅방 지원
- ✅ **타이핑 표시** - 상대방이 입력 중일 때 표시
- ✅ **개인 메시지** - 1:1 DM 기능
- ✅ **사용자 관리** - 온라인 사용자 추적
- ✅ **메시지 히스토리** - 최근 메시지 저장

---

## 🚀 설치 및 실행

### 1. 의존성 설치
```bash
npm install
```

### 2. 서버 실행
```bash
# 개발 모드
npm run dev:socketio

# 또는 직접 실행
node server-socketio.js
```

### 3. 채팅 UI 접속
브라우저에서 `http://localhost:3000/chat.html` 열기

---

## ✨ 주요 기능

### 1. 방(Room) 시스템
```javascript
// 클라이언트에서 방 입장
socket.emit('join', {
  username: '홍길동',
  room: 'general'  // 방 이름
});
```

### 2. 메시지 전송
```javascript
// 메시지 보내기
socket.emit('send_message', {
  message: '안녕하세요!'
});

// 메시지 받기
socket.on('receive_message', (data) => {
  console.log(`${data.username}: ${data.message}`);
});
```

### 3. 타이핑 표시
```javascript
// 타이핑 시작
socket.emit('typing');

// 타이핑 중지
socket.emit('stop_typing');

// 타이핑 표시 받기
socket.on('user_typing', (data) => {
  console.log(`${data.username}님이 입력 중...`);
});
```

### 4. 개인 메시지 (DM)
```javascript
// DM 보내기
socket.emit('private_message', {
  to: '김철수',
  message: '비밀 메시지'
});

// DM 받기
socket.on('receive_private_message', (data) => {
  console.log(`${data.from}님의 DM: ${data.message}`);
});
```

---

## 📚 Socket.IO 이벤트

### 📤 클라이언트 → 서버

| 이벤트 | 데이터 | 설명 |
|--------|--------|------|
| `join` | `{ username, room }` | 방 입장 |
| `send_message` | `{ message }` | 메시지 전송 |
| `typing` | - | 타이핑 시작 |
| `stop_typing` | - | 타이핑 중지 |
| `private_message` | `{ to, message }` | 개인 메시지 |

### 📥 서버 → 클라이언트

| 이벤트 | 데이터 | 설명 |
|--------|--------|------|
| `receive_message` | `{ username, message, timestamp }` | 메시지 수신 |
| `user_joined` | `{ username, message }` | 사용자 입장 |
| `user_left` | `{ username, message }` | 사용자 퇴장 |
| `user_typing` | `{ username }` | 타이핑 중 |
| `user_stop_typing` | `{ username }` | 타이핑 중지 |
| `room_users` | `[usernames]` | 방 사용자 목록 |
| `user_count` | `number` | 전체 접속자 수 |
| `message_history` | `[messages]` | 메시지 히스토리 |

---

## 🌐 REST API

### 메시지 히스토리 조회
```http
GET /api/messages?room=general&limit=50
```

**응답:**
```json
{
  "success": true,
  "count": 10,
  "room": "general",
  "data": [
    {
      "id": 1234567890,
      "username": "홍길동",
      "message": "안녕하세요!",
      "room": "general",
      "timestamp": "2024-01-01T00:00:00.000Z"
    }
  ]
}
```

### 온라인 사용자 조회
```http
GET /api/online-users?room=general
```

**응답:**
```json
{
  "success": true,
  "count": 3,
  "data": [
    {
      "username": "홍길동",
      "room": "general"
    }
  ]
}
```

---

## 💻 클라이언트 예제

### HTML + JavaScript

```html
<!DOCTYPE html>
<html>
<head>
    <title>채팅</title>
</head>
<body>
    <div id="messages"></div>
    <input type="text" id="messageInput">
    <button onclick="sendMessage()">전송</button>

    <script src="/socket.io/socket.io.js"></script>
    <script>
        // Socket.IO 연결
        const socket = io('http://localhost:3000');

        // 방 입장
        socket.emit('join', {
            username: '홍길동',
            room: 'general'
        });

        // 메시지 수신
        socket.on('receive_message', (data) => {
            const div = document.createElement('div');
            div.textContent = `${data.username}: ${data.message}`;
            document.getElementById('messages').appendChild(div);
        });

        // 메시지 전송
        function sendMessage() {
            const input = document.getElementById('messageInput');
            socket.emit('send_message', {
                message: input.value
            });
            input.value = '';
        }
    </script>
</body>
</html>
```

---

### React 예제

```javascript
import { useEffect, useState } from 'react';
import io from 'socket.io-client';

function Chat() {
  const [socket, setSocket] = useState(null);
  const [messages, setMessages] = useState([]);
  const [input, setInput] = useState('');

  useEffect(() => {
    // Socket.IO 연결
    const newSocket = io('http://localhost:3000');
    setSocket(newSocket);

    // 방 입장
    newSocket.emit('join', {
      username: '홍길동',
      room: 'general'
    });

    // 메시지 수신
    newSocket.on('receive_message', (data) => {
      setMessages(prev => [...prev, data]);
    });

    return () => newSocket.close();
  }, []);

  const sendMessage = () => {
    if (socket && input) {
      socket.emit('send_message', { message: input });
      setInput('');
    }
  };

  return (
    <div>
      <div>
        {messages.map((msg, i) => (
          <div key={i}>
            <strong>{msg.username}:</strong> {msg.message}
          </div>
        ))}
      </div>
      <input
        value={input}
        onChange={(e) => setInput(e.target.value)}
        onKeyPress={(e) => e.key === 'Enter' && sendMessage()}
      />
      <button onClick={sendMessage}>전송</button>
    </div>
  );
}
```

---

### Vue.js 예제

```vue
<template>
  <div>
    <div v-for="msg in messages" :key="msg.id">
      <strong>{{ msg.username }}:</strong> {{ msg.message }}
    </div>
    <input v-model="input" @keyup.enter="sendMessage">
    <button @click="sendMessage">전송</button>
  </div>
</template>

<script>
import io from 'socket.io-client';

export default {
  data() {
    return {
      socket: null,
      messages: [],
      input: ''
    };
  },
  mounted() {
    this.socket = io('http://localhost:3000');
    
    this.socket.emit('join', {
      username: '홍길동',
      room: 'general'
    });
    
    this.socket.on('receive_message', (data) => {
      this.messages.push(data);
    });
  },
  methods: {
    sendMessage() {
      if (this.input) {
        this.socket.emit('send_message', {
          message: this.input
        });
        this.input = '';
      }
    }
  },
  beforeUnmount() {
    this.socket?.close();
  }
};
</script>
```

---

## 🔧 고급 기능

### 1. 네임스페이스 사용
```javascript
// 서버
const chatNamespace = io.of('/chat');
chatNamespace.on('connection', (socket) => {
  // 채팅 전용 로직
});

// 클라이언트
const socket = io('http://localhost:3000/chat');
```

### 2. 미들웨어
```javascript
io.use((socket, next) => {
  const token = socket.handshake.auth.token;
  if (isValidToken(token)) {
    next();
  } else {
    next(new Error('인증 실패'));
  }
});
```

### 3. 브로드캐스트
```javascript
// 자신을 제외한 모든 클라이언트에게
socket.broadcast.emit('message', data);

// 특정 방의 모든 클라이언트에게
io.to('room1').emit('message', data);

// 여러 방에 동시에
io.to('room1').to('room2').emit('message', data);
```

---

## 🧪 테스트

### 1. 여러 브라우저 탭으로 테스트
- 탭 1: `http://localhost:3000/chat.html` (홍길동)
- 탭 2: `http://localhost:3000/chat.html` (김철수)
- 서로 메시지 주고받기

### 2. 다른 방 테스트
- 탭 1: room = "general"
- 탭 2: room = "tech"
- 같은 방만 메시지 공유 확인

### 3. 타이핑 표시 테스트
- 한 탭에서 입력 시작
- 다른 탭에서 "입력 중..." 표시 확인

---

## 📊 성능 최적화

### 1. 메시지 히스토리 제한
```javascript
const MAX_HISTORY = 100;
if (messageHistory.length > MAX_HISTORY) {
  messageHistory.shift();
}
```

### 2. 타이핑 표시 디바운스
```javascript
let typingTimeout;
socket.emit('typing');
clearTimeout(typingTimeout);
typingTimeout = setTimeout(() => {
  socket.emit('stop_typing');
}, 1000);
```

### 3. 연결 재시도
```javascript
const socket = io({
  reconnection: true,
  reconnectionDelay: 1000,
  reconnectionAttempts: 5
});
```

---

## 🔐 보안 고려사항

### 1. XSS 방지
```javascript
// HTML 이스케이프
function escapeHtml(text) {
  const div = document.createElement('div');
  div.textContent = text;
  return div.innerHTML;
}
```

### 2. Rate Limiting
```javascript
// 메시지 전송 제한
const messageRateLimit = new Map();
socket.on('send_message', (data) => {
  const lastMessage = messageRateLimit.get(socket.id);
  if (lastMessage && Date.now() - lastMessage < 1000) {
    return; // 1초에 1개만 허용
  }
  messageRateLimit.set(socket.id, Date.now());
  // 메시지 처리
});
```

### 3. 메시지 길이 제한
```javascript
if (data.message.length > 500) {
  socket.emit('error', { message: '메시지가 너무 깁니다.' });
  return;
}
```

---

## 📖 참고 자료

- [Socket.IO 공식 문서](https://socket.io/docs/)
- [Socket.IO Client API](https://socket.io/docs/v4/client-api/)
- [Socket.IO Server API](https://socket.io/docs/v4/server-api/)

---

## 🎓 다음 단계

1. **데이터베이스 연동** - 메시지를 SQLite에 영구 저장
2. **파일 전송** - 이미지/파일 공유 기능
3. **음성/영상 채팅** - WebRTC 통합
4. **푸시 알림** - 새 메시지 알림
5. **읽음 표시** - 메시지 읽음/안읽음 상태
