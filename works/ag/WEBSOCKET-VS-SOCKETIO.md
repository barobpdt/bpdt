# 🔌 WebSocket vs Socket.IO 비교

## 📊 비교표

| 특징 | Native WebSocket | Socket.IO |
|------|------------------|-----------|
| **프로토콜** | WebSocket만 | WebSocket + HTTP Long Polling |
| **자동 재연결** | ❌ 직접 구현 필요 | ✅ 자동 지원 |
| **방(Room)** | ❌ 직접 구현 필요 | ✅ 내장 지원 |
| **브로드캐스트** | ❌ 직접 구현 필요 | ✅ 내장 지원 |
| **이벤트 기반** | ❌ 메시지만 | ✅ 이벤트 이름 지원 |
| **폴백** | ❌ 없음 | ✅ HTTP Long Polling |
| **번들 크기** | 0 (브라우저 내장) | ~10KB (gzip) |
| **성능** | ⚡ 더 빠름 | 약간 느림 |
| **학습 곡선** | 낮음 | 중간 |
| **브라우저 지원** | IE10+ | IE8+ |

---

## 🚀 실행 방법

### Socket.IO 서버
```bash
npm run dev:socketio
# 브라우저: http://localhost:3000/chat.html
```

### WebSocket 서버
```bash
npm run dev:websocket
# 브라우저: http://localhost:3000/chat-ws.html
```

---

## 💻 코드 비교

### 1. 서버 설정

#### Socket.IO
```javascript
import { Server } from 'socket.io';

const io = new Server(httpServer, {
  cors: { origin: '*' }
});

io.on('connection', (socket) => {
  console.log('연결됨:', socket.id);
  
  socket.on('message', (data) => {
    io.emit('message', data);
  });
});
```

#### WebSocket
```javascript
import { WebSocketServer } from 'ws';

const wss = new WebSocketServer({ 
  server: httpServer,
  path: '/ws'
});

wss.on('connection', (ws) => {
  console.log('연결됨');
  
  ws.on('message', (message) => {
    const data = JSON.parse(message);
    wss.clients.forEach(client => {
      client.send(JSON.stringify(data));
    });
  });
});
```

---

### 2. 클라이언트 연결

#### Socket.IO
```javascript
// 자동으로 /socket.io 경로 사용
const socket = io('http://localhost:3000');

socket.on('connect', () => {
  console.log('연결됨');
});

socket.emit('message', { text: 'Hello' });

socket.on('message', (data) => {
  console.log(data);
});
```

#### WebSocket
```javascript
// 명시적으로 ws:// 프로토콜 사용
const ws = new WebSocket('ws://localhost:3000/ws');

ws.onopen = () => {
  console.log('연결됨');
};

ws.send(JSON.stringify({ 
  type: 'message', 
  data: { text: 'Hello' }
}));

ws.onmessage = (event) => {
  const data = JSON.parse(event.data);
  console.log(data);
};
```

---

### 3. 방(Room) 기능

#### Socket.IO
```javascript
// 서버
socket.join('room1');
io.to('room1').emit('message', data);

// 클라이언트
socket.emit('join', 'room1');
```

#### WebSocket
```javascript
// 서버 - 직접 구현 필요
const rooms = new Map();

function joinRoom(ws, room) {
  if (!rooms.has(room)) {
    rooms.set(room, new Set());
  }
  rooms.get(room).add(ws);
}

function broadcastToRoom(room, data) {
  rooms.get(room)?.forEach(client => {
    client.send(JSON.stringify(data));
  });
}

// 클라이언트
ws.send(JSON.stringify({ 
  type: 'join', 
  data: { room: 'room1' }
}));
```

---

### 4. 자동 재연결

#### Socket.IO
```javascript
// 자동으로 재연결 시도
const socket = io({
  reconnection: true,
  reconnectionDelay: 1000,
  reconnectionAttempts: 5
});

socket.on('reconnect', (attemptNumber) => {
  console.log('재연결 성공:', attemptNumber);
});
```

#### WebSocket
```javascript
// 직접 구현 필요
let ws;
let reconnectTimeout;

function connect() {
  ws = new WebSocket('ws://localhost:3000/ws');
  
  ws.onclose = () => {
    console.log('연결 끊김, 5초 후 재연결...');
    reconnectTimeout = setTimeout(connect, 5000);
  };
}

connect();
```

---

## 🎯 언제 무엇을 사용할까?

### ✅ Socket.IO를 사용하세요:

1. **빠른 개발이 필요할 때**
   - 방, 브로드캐스트 등 기능이 내장됨
   - 보일러플레이트 코드 최소화

2. **안정성이 중요할 때**
   - 자동 재연결
   - HTTP Long Polling 폴백

3. **복잡한 이벤트 처리**
   - 이벤트 이름으로 구분
   - 미들웨어 지원

4. **레거시 브라우저 지원**
   - IE8+ 지원 필요

### ✅ WebSocket을 사용하세요:

1. **성능이 최우선일 때**
   - 오버헤드 최소화
   - 더 빠른 메시지 전송

2. **번들 크기가 중요할 때**
   - 추가 라이브러리 불필요
   - 브라우저 네이티브 API

3. **간단한 실시간 통신**
   - 복잡한 기능 불필요
   - 직접 제어 원함

4. **표준 준수**
   - WebSocket 표준만 사용
   - 다른 WebSocket 클라이언트와 호환

---

## 📈 성능 비교

### 메시지 전송 속도
```
WebSocket:  ~0.5ms
Socket.IO:  ~1.0ms
```

### 메모리 사용량
```
WebSocket:  ~5MB (100 연결)
Socket.IO:  ~8MB (100 연결)
```

### 번들 크기
```
WebSocket:  0KB (브라우저 내장)
Socket.IO:  ~10KB (gzip)
```

---

## 🔧 실전 예제

### 채팅 애플리케이션
- **추천**: Socket.IO
- **이유**: 방, 타이핑 표시 등 기능 필요

### 주식 시세 표시
- **추천**: WebSocket
- **이유**: 단순 데이터 스트림, 성능 중요

### 멀티플레이어 게임
- **추천**: Socket.IO
- **이유**: 복잡한 이벤트 처리, 안정성

### IoT 센서 데이터
- **추천**: WebSocket
- **이유**: 경량, 표준 프로토콜

---

## 🎓 학습 리소스

### Socket.IO
- [공식 문서](https://socket.io/docs/)
- [Socket.IO 튜토리얼](https://socket.io/get-started/chat)

### WebSocket
- [MDN WebSocket](https://developer.mozilla.org/ko/docs/Web/API/WebSocket)
- [WebSocket RFC](https://tools.ietf.org/html/rfc6455)

---

## 💡 결론

**초보자 또는 빠른 개발**: Socket.IO 👈 추천!
- 기능이 많고 사용하기 쉬움
- 안정성과 호환성 우수

**성능 최적화 또는 간단한 용도**: WebSocket
- 가볍고 빠름
- 표준 준수

**우리 프로젝트에서는?**
- 두 가지 모두 구현했으니 직접 비교해보세요!
- `npm run dev:socketio` vs `npm run dev:websocket`
