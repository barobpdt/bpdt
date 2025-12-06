import express from 'express';
import { createServer } from 'http';
import { Server } from 'socket.io';
import cors from 'cors';
import { eq, desc } from 'drizzle-orm';
import { db, initDatabase } from './db/index.js';
import { posts, comments, categories } from './db/schema.js';
import Database from 'better-sqlite3';
import { drizzle } from 'drizzle-orm/better-sqlite3';

const app = express();
const httpServer = createServer(app);
const PORT = process.env.PORT || 3000;

// ==================== Socket.IO 설정 ====================

const io = new Server(httpServer, {
    cors: {
        origin: process.env.NODE_ENV === 'development'
            ? '*'
            : ['https://myapp.com', 'http://localhost:3001'],
        methods: ['GET', 'POST'],
        credentials: true
    }
});

// 메시지 저장을 위한 간단한 메모리 저장소
const messageHistory = [];
const MAX_HISTORY = 100;

// 온라인 사용자 추적
const onlineUsers = new Map();

// ==================== Socket.IO 이벤트 핸들러 ====================

io.on('connection', (socket) => {
    console.log(`✅ 사용자 연결: ${socket.id}`);

    // 연결된 사용자 수 전송
    io.emit('user_count', io.engine.clientsCount);

    // 사용자 입장
    socket.on('join', (data) => {
        const { username, room = 'general' } = data;

        // 사용자 정보 저장
        onlineUsers.set(socket.id, { username, room });

        // 방 입장
        socket.join(room);

        console.log(`👤 ${username}님이 ${room} 방에 입장했습니다.`);

        // 방의 다른 사용자들에게 알림
        socket.to(room).emit('user_joined', {
            username,
            message: `${username}님이 입장했습니다.`,
            timestamp: new Date().toISOString()
        });

        // 최근 메시지 히스토리 전송
        const roomHistory = messageHistory
            .filter(msg => msg.room === room)
            .slice(-50);
        socket.emit('message_history', roomHistory);

        // 방의 사용자 목록 업데이트
        updateRoomUsers(room);
    });

    // 메시지 수신
    socket.on('send_message', (data) => {
        const user = onlineUsers.get(socket.id);

        if (!user) {
            socket.emit('error', { message: '먼저 입장해주세요.' });
            return;
        }

        const messageData = {
            id: Date.now(),
            username: user.username,
            message: data.message,
            room: user.room,
            timestamp: new Date().toISOString()
        };

        // 메시지 히스토리에 저장
        messageHistory.push(messageData);
        if (messageHistory.length > MAX_HISTORY) {
            messageHistory.shift();
        }

        // 같은 방의 모든 사용자에게 메시지 전송
        io.to(user.room).emit('receive_message', messageData);

        console.log(`💬 [${user.room}] ${user.username}: ${data.message}`);
    });

    // 타이핑 중 표시
    socket.on('typing', () => {
        const user = onlineUsers.get(socket.id);
        if (user) {
            socket.to(user.room).emit('user_typing', {
                username: user.username
            });
        }
    });

    // 타이핑 중지
    socket.on('stop_typing', () => {
        const user = onlineUsers.get(socket.id);
        if (user) {
            socket.to(user.room).emit('user_stop_typing', {
                username: user.username
            });
        }
    });

    // 개인 메시지 (DM)
    socket.on('private_message', (data) => {
        const { to, message } = data;
        const sender = onlineUsers.get(socket.id);

        if (!sender) return;

        // 수신자의 socket ID 찾기
        let recipientSocketId = null;
        for (const [socketId, user] of onlineUsers.entries()) {
            if (user.username === to) {
                recipientSocketId = socketId;
                break;
            }
        }

        if (recipientSocketId) {
            const dmData = {
                from: sender.username,
                message,
                timestamp: new Date().toISOString()
            };

            // 수신자에게 전송
            io.to(recipientSocketId).emit('receive_private_message', dmData);

            // 발신자에게도 확인 전송
            socket.emit('private_message_sent', dmData);

            console.log(`📨 DM: ${sender.username} → ${to}: ${message}`);
        } else {
            socket.emit('error', { message: '사용자를 찾을 수 없습니다.' });
        }
    });

    // 연결 해제
    socket.on('disconnect', () => {
        const user = onlineUsers.get(socket.id);

        if (user) {
            console.log(`❌ ${user.username}님이 연결 해제되었습니다.`);

            // 방의 다른 사용자들에게 알림
            socket.to(user.room).emit('user_left', {
                username: user.username,
                message: `${user.username}님이 퇴장했습니다.`,
                timestamp: new Date().toISOString()
            });

            onlineUsers.delete(socket.id);
            updateRoomUsers(user.room);
        }

        // 연결된 사용자 수 업데이트
        io.emit('user_count', io.engine.clientsCount);
    });
});

// 방의 사용자 목록 업데이트
function updateRoomUsers(room) {
    const roomUsers = [];
    for (const [socketId, user] of onlineUsers.entries()) {
        if (user.room === room) {
            roomUsers.push(user.username);
        }
    }
    io.to(room).emit('room_users', roomUsers);
}

// ==================== CORS 설정 ====================

const corsOptions = {
    origin: function (origin, callback) {
        if (process.env.NODE_ENV === 'development') {
            callback(null, true);
            return;
        }

        const allowedOrigins = [
            'https://myapp.com',
            'http://localhost:3000',
            'http://localhost:3001'
        ];

        if (!origin || allowedOrigins.includes(origin)) {
            callback(null, true);
        } else {
            callback(new Error(`출처 ${origin}는 CORS 정책에 의해 차단되었습니다.`));
        }
    },
    credentials: true,
    methods: ['GET', 'POST', 'PUT', 'DELETE'],
    allowedHeaders: ['Content-Type', 'Authorization']
};

app.use(cors(corsOptions));
app.use(express.json());
app.use(express.static('public'));

// 데이터베이스 초기화
initDatabase();

// ==================== REST API 라우트 ====================

app.get('/', (req, res) => {
    res.json({
        message: '🚀 Socket.IO + Drizzle ORM API 서버',
        version: '1.0.0',
        features: {
            realtime: 'Socket.IO',
            database: 'SQLite',
            orm: 'Drizzle ORM'
        },
        endpoints: {
            rest: {
                posts: '/api/posts',
                health: '/api/health'
            },
            socketio: {
                endpoint: 'ws://localhost:' + PORT,
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
        socketio: {
            connected: io.engine.clientsCount,
            rooms: Array.from(io.sockets.adapter.rooms.keys())
        },
        timestamp: new Date()
    });
});

// 메시지 히스토리 조회 API
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

// 온라인 사용자 조회 API
app.get('/api/online-users', (req, res) => {
    const { room } = req.query;

    let users = [];
    for (const [socketId, user] of onlineUsers.entries()) {
        if (!room || user.room === room) {
            users.push({
                username: user.username,
                room: user.room
            });
        }
    }

    res.json({
        success: true,
        count: users.length,
        data: users
    });
});

// 기본 Posts API (기존 코드 유지)
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
  🚀 Socket.IO + Drizzle ORM 서버가 시작되었습니다!
  
  📍 HTTP: http://localhost:${PORT}
  🔌 WebSocket: ws://localhost:${PORT}
  💾 데이터베이스: SQLite
  🔧 ORM: Drizzle ORM
  ⚡ 실시간: Socket.IO
  ⏰ 시작 시간: ${new Date().toLocaleString('ko-KR')}
  
  📚 REST API:
  - GET  /api/health           - 서버 상태 확인
  - GET  /api/messages         - 메시지 히스토리
  - GET  /api/online-users     - 온라인 사용자 목록
  - GET  /api/posts            - 게시글 목록
  
  🔌 Socket.IO 이벤트:
  
  📤 클라이언트 → 서버:
  - join              - 방 입장 { username, room }
  - send_message      - 메시지 전송 { message }
  - typing            - 타이핑 중
  - stop_typing       - 타이핑 중지
  - private_message   - 개인 메시지 { to, message }
  
  📥 서버 → 클라이언트:
  - receive_message   - 메시지 수신
  - user_joined       - 사용자 입장 알림
  - user_left         - 사용자 퇴장 알림
  - user_typing       - 타이핑 중 표시
  - room_users        - 방 사용자 목록
  - user_count        - 전체 접속자 수
  - message_history   - 메시지 히스토리
  
  💡 테스트:
  - public/chat.html 파일을 브라우저에서 열어보세요!
  `);
});
