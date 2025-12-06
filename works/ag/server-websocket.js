import express from 'express';
import { createServer } from 'http';
import { WebSocketServer } from 'ws';
import cors from 'cors';
import { eq, desc } from 'drizzle-orm';
import { db, initDatabase } from './db/index.js';
import { posts, comments, categories } from './db/schema.js';

const app = express();
const httpServer = createServer(app);
const PORT = process.env.PORT || 3000;

// ==================== WebSocket 설정 ====================

const wss = new WebSocketServer({
    server: httpServer,
    path: '/ws'
});

// 메시지 저장소
const messageHistory = [];
const MAX_HISTORY = 100;

// 연결된 클라이언트 관리
const clients = new Map();

// 방별 클라이언트 관리
const rooms = new Map();

// ==================== WebSocket 헬퍼 함수 ====================

// 메시지 전송 헬퍼
function sendMessage(ws, type, data) {
    if (ws.readyState === ws.OPEN) {
        ws.send(JSON.stringify({ type, data }));
    }
}

// 방의 모든 클라이언트에게 메시지 전송
function broadcastToRoom(room, type, data, excludeWs = null) {
    const roomClients = rooms.get(room) || new Set();
    roomClients.forEach(clientWs => {
        if (clientWs !== excludeWs && clientWs.readyState === clientWs.OPEN) {
            sendMessage(clientWs, type, data);
        }
    });
}

// 모든 클라이언트에게 메시지 전송
function broadcastToAll(type, data) {
    clients.forEach((client, ws) => {
        if (ws.readyState === ws.OPEN) {
            sendMessage(ws, type, data);
        }
    });
}

// 방의 사용자 목록 업데이트
function updateRoomUsers(room) {
    const roomClients = rooms.get(room) || new Set();
    const usernames = [];

    roomClients.forEach(clientWs => {
        const client = clients.get(clientWs);
        if (client) {
            usernames.push(client.username);
        }
    });

    broadcastToRoom(room, 'room_users', usernames);
}

// ==================== WebSocket 이벤트 핸들러 ====================

wss.on('connection', (ws) => {
    console.log('✅ 새로운 WebSocket 연결');

    // 연결된 클라이언트 수 전송
    broadcastToAll('user_count', wss.clients.size);

    // 메시지 수신 핸들러
    ws.on('message', (message) => {
        try {
            const { type, data } = JSON.parse(message.toString());

            switch (type) {
                case 'join':
                    handleJoin(ws, data);
                    break;

                case 'send_message':
                    handleSendMessage(ws, data);
                    break;

                case 'typing':
                    handleTyping(ws);
                    break;

                case 'stop_typing':
                    handleStopTyping(ws);
                    break;

                case 'private_message':
                    handlePrivateMessage(ws, data);
                    break;

                default:
                    sendMessage(ws, 'error', { message: '알 수 없는 이벤트 타입입니다.' });
            }
        } catch (error) {
            console.error('메시지 처리 오류:', error);
            sendMessage(ws, 'error', { message: '메시지 처리 중 오류가 발생했습니다.' });
        }
    });

    // 연결 종료 핸들러
    ws.on('close', () => {
        handleDisconnect(ws);
    });

    // 에러 핸들러
    ws.on('error', (error) => {
        console.error('WebSocket 오류:', error);
    });
});

// ==================== 이벤트 핸들러 함수 ====================

// 방 입장
function handleJoin(ws, data) {
    const { username, room = 'general' } = data;

    if (!username) {
        sendMessage(ws, 'error', { message: '닉네임이 필요합니다.' });
        return;
    }

    // 클라이언트 정보 저장
    clients.set(ws, { username, room });

    // 방에 추가
    if (!rooms.has(room)) {
        rooms.set(room, new Set());
    }
    rooms.get(room).add(ws);

    console.log(`👤 ${username}님이 ${room} 방에 입장했습니다.`);

    // 입장 알림 (다른 사용자들에게)
    broadcastToRoom(room, 'user_joined', {
        username,
        message: `${username}님이 입장했습니다.`,
        timestamp: new Date().toISOString()
    }, ws);

    // 메시지 히스토리 전송
    const roomHistory = messageHistory
        .filter(msg => msg.room === room)
        .slice(-50);
    sendMessage(ws, 'message_history', roomHistory);

    // 방 사용자 목록 업데이트
    updateRoomUsers(room);

    // 연결 성공 응답
    sendMessage(ws, 'joined', {
        username,
        room,
        message: '방에 입장했습니다.'
    });
}

// 메시지 전송
function handleSendMessage(ws, data) {
    const client = clients.get(ws);

    if (!client) {
        sendMessage(ws, 'error', { message: '먼저 방에 입장해주세요.' });
        return;
    }

    const { message } = data;

    if (!message || message.trim() === '') {
        return;
    }

    // 메시지 길이 제한
    if (message.length > 500) {
        sendMessage(ws, 'error', { message: '메시지가 너무 깁니다. (최대 500자)' });
        return;
    }

    const messageData = {
        id: Date.now(),
        username: client.username,
        message: message.trim(),
        room: client.room,
        timestamp: new Date().toISOString()
    };

    // 메시지 히스토리에 저장
    messageHistory.push(messageData);
    if (messageHistory.length > MAX_HISTORY) {
        messageHistory.shift();
    }

    // 같은 방의 모든 사용자에게 메시지 전송
    broadcastToRoom(client.room, 'receive_message', messageData);

    console.log(`💬 [${client.room}] ${client.username}: ${message}`);
}

// 타이핑 시작
function handleTyping(ws) {
    const client = clients.get(ws);

    if (!client) return;

    broadcastToRoom(client.room, 'user_typing', {
        username: client.username
    }, ws);
}

// 타이핑 중지
function handleStopTyping(ws) {
    const client = clients.get(ws);

    if (!client) return;

    broadcastToRoom(client.room, 'user_stop_typing', {
        username: client.username
    }, ws);
}

// 개인 메시지
function handlePrivateMessage(ws, data) {
    const sender = clients.get(ws);

    if (!sender) {
        sendMessage(ws, 'error', { message: '먼저 방에 입장해주세요.' });
        return;
    }

    const { to, message } = data;

    if (!to || !message) {
        sendMessage(ws, 'error', { message: '수신자와 메시지가 필요합니다.' });
        return;
    }

    // 수신자 찾기
    let recipientWs = null;
    for (const [clientWs, client] of clients.entries()) {
        if (client.username === to) {
            recipientWs = clientWs;
            break;
        }
    }

    if (!recipientWs) {
        sendMessage(ws, 'error', { message: '사용자를 찾을 수 없습니다.' });
        return;
    }

    const dmData = {
        from: sender.username,
        message,
        timestamp: new Date().toISOString()
    };

    // 수신자에게 전송
    sendMessage(recipientWs, 'receive_private_message', dmData);

    // 발신자에게 확인 전송
    sendMessage(ws, 'private_message_sent', dmData);

    console.log(`📨 DM: ${sender.username} → ${to}: ${message}`);
}

// 연결 종료
function handleDisconnect(ws) {
    const client = clients.get(ws);

    if (client) {
        console.log(`❌ ${client.username}님이 연결 해제되었습니다.`);

        // 퇴장 알림
        broadcastToRoom(client.room, 'user_left', {
            username: client.username,
            message: `${client.username}님이 퇴장했습니다.`,
            timestamp: new Date().toISOString()
        });

        // 방에서 제거
        const roomClients = rooms.get(client.room);
        if (roomClients) {
            roomClients.delete(ws);
            if (roomClients.size === 0) {
                rooms.delete(client.room);
            } else {
                updateRoomUsers(client.room);
            }
        }

        // 클라이언트 목록에서 제거
        clients.delete(ws);
    }

    // 전체 접속자 수 업데이트
    broadcastToAll('user_count', wss.clients.size);
}

// ==================== Express 설정 ====================

const corsOptions = {
    origin: process.env.NODE_ENV === 'development' ? '*' : ['https://myapp.com'],
    credentials: true
};

app.use(cors(corsOptions));
app.use(express.json());
app.use(express.static('public'));

initDatabase();

// ==================== REST API ====================

app.get('/', (req, res) => {
    res.json({
        message: '🚀 WebSocket + Drizzle ORM API 서버',
        version: '1.0.0',
        features: {
            realtime: 'Native WebSocket',
            database: 'SQLite',
            orm: 'Drizzle ORM'
        },
        endpoints: {
            rest: {
                posts: '/api/posts',
                messages: '/api/messages',
                onlineUsers: '/api/online-users',
                health: '/api/health'
            },
            websocket: {
                endpoint: 'ws://localhost:' + PORT + '/ws',
                events: [
                    'join',
                    'send_message',
                    'typing',
                    'private_message'
                ]
            }
        }
    });
});

app.get('/api/health', (req, res) => {
    res.json({
        status: 'OK',
        websocket: {
            connected: wss.clients.size,
            rooms: Array.from(rooms.keys())
        },
        timestamp: new Date()
    });
});

// 메시지 히스토리 조회
app.get('/api/messages', (req, res) => {
    const { room = 'general', limit = 50 } = req.query;

    const roomMessages = messageHistory
        .filter(msg => msg.room === room)
        .slice(-parseInt(limit));

    res.json({
        success: true,
        count: roomMessages.length,
        room,
        data: roomMessages
    });
});

// 온라인 사용자 조회
app.get('/api/online-users', (req, res) => {
    const { room } = req.query;

    const users = [];
    for (const [ws, client] of clients.entries()) {
        if (!room || client.room === room) {
            users.push({
                username: client.username,
                room: client.room
            });
        }
    }

    res.json({
        success: true,
        count: users.length,
        data: users
    });
});

// Posts API
app.get('/api/posts', async (req, res) => {
    try {
        const result = await db.select().from(posts).orderBy(desc(posts.createdAt));
        res.json({
            success: true,
            count: result.length,
            data: result
        });
    } catch (error) {
        res.status(500).json({
            success: false,
            message: '게시글 조회 실패',
            error: error.message
        });
    }
});

// ==================== 에러 핸들링 ====================

app.use((req, res) => {
    res.status(404).json({
        success: false,
        message: '요청한 리소스를 찾을 수 없습니다.'
    });
});

app.use((err, req, res, next) => {
    console.error(err.stack);
    res.status(500).json({
        success: false,
        message: '서버 오류가 발생했습니다.',
        error: process.env.NODE_ENV === 'development' ? err.message : undefined
    });
});

// ==================== 서버 시작 ====================

httpServer.listen(PORT, () => {
    console.log(`
  🚀 WebSocket 서버가 시작되었습니다!
  
  📍 HTTP: http://localhost:${PORT}
  🔌 WebSocket: ws://localhost:${PORT}/ws
  💾 데이터베이스: SQLite
  🔧 ORM: Drizzle ORM
  ⚡ 실시간: Native WebSocket
  ⏰ 시작 시간: ${new Date().toLocaleString('ko-KR')}
  
  📚 REST API:
  - GET  /api/health           - 서버 상태
  - GET  /api/messages         - 메시지 히스토리
  - GET  /api/online-users     - 온라인 사용자
  - GET  /api/posts            - 게시글 목록
  
  🔌 WebSocket 메시지 타입:
  
  📤 클라이언트 → 서버:
  - join              - 방 입장 { username, room }
  - send_message      - 메시지 전송 { message }
  - typing            - 타이핑 시작
  - stop_typing       - 타이핑 중지
  - private_message   - 개인 메시지 { to, message }
  
  📥 서버 → 클라이언트:
  - joined            - 입장 완료
  - receive_message   - 메시지 수신
  - user_joined       - 사용자 입장
  - user_left         - 사용자 퇴장
  - user_typing       - 타이핑 중
  - room_users        - 방 사용자 목록
  - user_count        - 전체 접속자 수
  - message_history   - 메시지 히스토리
  - error             - 에러 메시지
  
  💡 테스트:
  - public/chat-ws.html 파일을 브라우저에서 열어보세요!
  `);
});
