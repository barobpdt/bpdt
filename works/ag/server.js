const express = require('express');
const cors = require('cors');
const path = require('path');

const app = express();
const PORT = process.env.PORT || 3000;

// 미들웨어
app.use(cors());
app.use(express.json());
app.use(express.static('public'));

// 메모리 데이터베이스 (간단한 예제용)
let todos = [
    { id: 1, title: '프로젝트 계획 수립', completed: false, createdAt: new Date() },
    { id: 2, title: 'API 개발', completed: false, createdAt: new Date() },
    { id: 3, title: '프론트엔드 구현', completed: false, createdAt: new Date() }
];

let users = [
    { id: 1, name: '홍길동', email: 'hong@example.com', role: 'admin' },
    { id: 2, name: '김철수', email: 'kim@example.com', role: 'user' }
];

// ==================== 기본 라우트 ====================

// 홈페이지
app.get('/', (req, res) => {
    res.json({
        message: '🚀 Express API 서버에 오신 것을 환영합니다!',
        version: '1.0.0',
        endpoints: {
            todos: '/api/todos',
            users: '/api/users',
            health: '/api/health'
        }
    });
});

// 헬스 체크
app.get('/api/health', (req, res) => {
    res.json({
        status: 'OK',
        timestamp: new Date(),
        uptime: process.uptime()
    });
});

// ==================== TODO API ====================

// 모든 TODO 조회
app.get('/api/todos', (req, res) => {
    const { completed } = req.query;

    let filteredTodos = todos;
    if (completed !== undefined) {
        filteredTodos = todos.filter(todo =>
            todo.completed === (completed === 'true')
        );
    }

    res.json({
        success: true,
        count: filteredTodos.length,
        data: filteredTodos
    });
});

// 특정 TODO 조회
app.get('/api/todos/:id', (req, res) => {
    const todo = todos.find(t => t.id === parseInt(req.params.id));

    if (!todo) {
        return res.status(404).json({
            success: false,
            message: 'TODO를 찾을 수 없습니다.'
        });
    }

    res.json({
        success: true,
        data: todo
    });
});

// TODO 생성
app.post('/api/todos', (req, res) => {
    const { title } = req.body;

    if (!title) {
        return res.status(400).json({
            success: false,
            message: 'title은 필수입니다.'
        });
    }

    const newTodo = {
        id: todos.length > 0 ? Math.max(...todos.map(t => t.id)) + 1 : 1,
        title,
        completed: false,
        createdAt: new Date()
    };

    todos.push(newTodo);

    res.status(201).json({
        success: true,
        message: 'TODO가 생성되었습니다.',
        data: newTodo
    });
});

// TODO 수정
app.put('/api/todos/:id', (req, res) => {
    const todo = todos.find(t => t.id === parseInt(req.params.id));

    if (!todo) {
        return res.status(404).json({
            success: false,
            message: 'TODO를 찾을 수 없습니다.'
        });
    }

    const { title, completed } = req.body;

    if (title !== undefined) todo.title = title;
    if (completed !== undefined) todo.completed = completed;
    todo.updatedAt = new Date();

    res.json({
        success: true,
        message: 'TODO가 수정되었습니다.',
        data: todo
    });
});

// TODO 삭제
app.delete('/api/todos/:id', (req, res) => {
    const index = todos.findIndex(t => t.id === parseInt(req.params.id));

    if (index === -1) {
        return res.status(404).json({
            success: false,
            message: 'TODO를 찾을 수 없습니다.'
        });
    }

    const deletedTodo = todos.splice(index, 1)[0];

    res.json({
        success: true,
        message: 'TODO가 삭제되었습니다.',
        data: deletedTodo
    });
});

// ==================== USER API ====================

// 모든 사용자 조회
app.get('/api/users', (req, res) => {
    res.json({
        success: true,
        count: users.length,
        data: users
    });
});

// 특정 사용자 조회
app.get('/api/users/:id', (req, res) => {
    const user = users.find(u => u.id === parseInt(req.params.id));

    if (!user) {
        return res.status(404).json({
            success: false,
            message: '사용자를 찾을 수 없습니다.'
        });
    }

    res.json({
        success: true,
        data: user
    });
});

// 사용자 생성
app.post('/api/users', (req, res) => {
    const { name, email, role } = req.body;

    if (!name || !email) {
        return res.status(400).json({
            success: false,
            message: 'name과 email은 필수입니다.'
        });
    }

    const newUser = {
        id: users.length > 0 ? Math.max(...users.map(u => u.id)) + 1 : 1,
        name,
        email,
        role: role || 'user',
        createdAt: new Date()
    };

    users.push(newUser);

    res.status(201).json({
        success: true,
        message: '사용자가 생성되었습니다.',
        data: newUser
    });
});

// ==================== 통계 API ====================

// 통계 조회
app.get('/api/stats', (req, res) => {
    const completedTodos = todos.filter(t => t.completed).length;
    const pendingTodos = todos.filter(t => !t.completed).length;

    res.json({
        success: true,
        data: {
            todos: {
                total: todos.length,
                completed: completedTodos,
                pending: pendingTodos,
                completionRate: todos.length > 0
                    ? ((completedTodos / todos.length) * 100).toFixed(2) + '%'
                    : '0%'
            },
            users: {
                total: users.length,
                admins: users.filter(u => u.role === 'admin').length,
                regularUsers: users.filter(u => u.role === 'user').length
            }
        }
    });
});

// ==================== 에러 핸들링 ====================

// 404 에러
app.use((req, res) => {
    res.status(404).json({
        success: false,
        message: '요청한 리소스를 찾을 수 없습니다.',
        path: req.path
    });
});

// 서버 에러
app.use((err, req, res, next) => {
    console.error(err.stack);
    res.status(500).json({
        success: false,
        message: '서버 오류가 발생했습니다.',
        error: process.env.NODE_ENV === 'development' ? err.message : undefined
    });
});

// ==================== 서버 시작 ====================

app.listen(PORT, () => {
    console.log(`
    🚀 서버가 시작되었습니다!
    
    📍 URL: http://localhost:${PORT}
    🌍 환경: ${process.env.NODE_ENV || 'development'}
    ⏰ 시작 시간: ${new Date().toLocaleString('ko-KR')}
    
    📚 API 엔드포인트:
    - GET    /api/todos          - 모든 TODO 조회
    - GET    /api/todos/:id      - 특정 TODO 조회
    - POST   /api/todos          - TODO 생성
    - PUT    /api/todos/:id      - TODO 수정
    - DELETE /api/todos/:id      - TODO 삭제
    
    - GET    /api/users          - 모든 사용자 조회
    - GET    /api/users/:id      - 특정 사용자 조회
    - POST   /api/users          - 사용자 생성
    
    - GET    /api/stats          - 통계 조회
    - GET    /api/health         - 헬스 체크
    `);
});
