import express from 'express';
import cors from 'cors';
import { eq, desc, like, and } from 'drizzle-orm';
import { db, initDatabase } from './db/index.js';
import { posts, comments, categories } from './db/schema.js';

const app = express();
const PORT = process.env.PORT || 3000;

// ==================== CORS 설정 ====================

// 방법 1: 기본 CORS (개발용 - 모든 출처 허용)
// app.use(cors());

// 방법 2: 환경별 CORS 설정 (권장)
const corsOptions = {
    origin: function (origin, callback) {
        // 개발 환경에서는 모든 출처 허용
        if (process.env.NODE_ENV === 'development') {
            callback(null, true);
            return;
        }

        // 프로덕션 환경에서는 특정 출처만 허용
        const allowedOrigins = [
            'https://myapp.com',
            'https://www.myapp.com',
            'https://admin.myapp.com',
            'http://localhost:3000',
            'http://localhost:3001'
        ];

        // origin이 undefined인 경우는 같은 출처 요청 (Postman, cURL 등)
        if (!origin || allowedOrigins.includes(origin)) {
            callback(null, true);
        } else {
            callback(new Error(`출처 ${origin}는 CORS 정책에 의해 차단되었습니다.`));
        }
    },

    // 허용할 HTTP 메서드
    methods: ['GET', 'POST', 'PUT', 'DELETE', 'PATCH', 'OPTIONS'],

    // 허용할 요청 헤더
    allowedHeaders: [
        'Content-Type',
        'Authorization',
        'X-API-Key',
        'X-Request-ID'
    ],

    // 클라이언트가 접근 가능한 응답 헤더
    exposedHeaders: [
        'X-Total-Count',
        'X-Page-Number',
        'X-Rate-Limit-Remaining'
    ],

    // 인증 정보 포함 허용 (쿠키, Authorization 헤더 등)
    credentials: true,

    // Preflight 요청 캐시 시간 (24시간)
    maxAge: 86400,

    // OPTIONS 요청 성공 상태 코드
    optionsSuccessStatus: 204
};

// CORS 미들웨어 적용
app.use(cors(corsOptions));

// ==================== 기타 미들웨어 ====================

app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// 요청 로깅 미들웨어
app.use((req, res, next) => {
    console.log(`[${new Date().toISOString()}] ${req.method} ${req.path} - Origin: ${req.get('origin') || 'same-origin'}`);
    next();
});

// 데이터베이스 초기화
initDatabase();

// ==================== 기본 라우트 ====================

app.get('/', (req, res) => {
    res.json({
        message: '🚀 Drizzle ORM + SQLite API 서버 (CORS 적용)',
        version: '1.0.0',
        database: 'SQLite',
        orm: 'Drizzle ORM',
        cors: {
            enabled: true,
            allowedOrigins: process.env.NODE_ENV === 'development'
                ? '모든 출처 (개발 모드)'
                : '제한된 출처 (프로덕션 모드)'
        },
        endpoints: {
            posts: '/api/posts',
            comments: '/api/comments',
            categories: '/api/categories',
            public: '/api/public',
            admin: '/api/admin',
            health: '/api/health'
        }
    });
});

app.get('/api/health', (req, res) => {
    res.json({
        status: 'OK',
        database: 'SQLite',
        timestamp: new Date(),
        cors: 'enabled'
    });
});

// ==================== 공개 API (모든 출처 허용) ====================

app.get('/api/public', cors(), (req, res) => {
    res.json({
        message: '이 엔드포인트는 모든 출처에서 접근 가능합니다.',
        data: {
            tip: 'CORS 제한 없음'
        }
    });
});

// ==================== 제한된 API (특정 출처만 허용) ====================

const adminCorsOptions = {
    origin: ['https://admin.myapp.com', 'http://localhost:3001'],
    credentials: true
};

app.get('/api/admin', cors(adminCorsOptions), (req, res) => {
    res.json({
        message: '관리자 전용 엔드포인트',
        data: {
            tip: '특정 출처만 접근 가능'
        }
    });
});

// ==================== POSTS API ====================

// 모든 게시글 조회
app.get('/api/posts', async (req, res) => {
    try {
        const { published, search, page = 1, limit = 10 } = req.query;

        let query = db.select().from(posts);

        // 필터링
        const conditions = [];
        if (published !== undefined) {
            conditions.push(eq(posts.published, published === 'true'));
        }
        if (search) {
            conditions.push(like(posts.title, `%${search}%`));
        }

        if (conditions.length > 0) {
            query = query.where(and(...conditions));
        }

        const result = await query.orderBy(desc(posts.createdAt));

        // 페이지네이션
        const offset = (parseInt(page) - 1) * parseInt(limit);
        const paginatedResult = result.slice(offset, offset + parseInt(limit));

        // 커스텀 헤더 추가 (CORS exposedHeaders에 포함됨)
        res.set('X-Total-Count', result.length.toString());
        res.set('X-Page-Number', page);
        res.set('X-Rate-Limit-Remaining', '100');

        res.json({
            success: true,
            count: paginatedResult.length,
            total: result.length,
            page: parseInt(page),
            data: paginatedResult
        });
    } catch (error) {
        res.status(500).json({
            success: false,
            message: '게시글 조회 실패',
            error: error.message
        });
    }
});

// 특정 게시글 조회 (댓글 포함)
app.get('/api/posts/:id', async (req, res) => {
    try {
        const postId = parseInt(req.params.id);

        const post = await db.select()
            .from(posts)
            .where(eq(posts.id, postId))
            .limit(1);

        if (post.length === 0) {
            return res.status(404).json({
                success: false,
                message: '게시글을 찾을 수 없습니다.'
            });
        }

        const postComments = await db.select()
            .from(comments)
            .where(eq(comments.postId, postId))
            .orderBy(desc(comments.createdAt));

        res.json({
            success: true,
            data: {
                ...post[0],
                comments: postComments
            }
        });
    } catch (error) {
        res.status(500).json({
            success: false,
            message: '게시글 조회 실패',
            error: error.message
        });
    }
});

// 게시글 생성
app.post('/api/posts', async (req, res) => {
    try {
        const { title, content, author, published } = req.body;

        if (!title || !content || !author) {
            return res.status(400).json({
                success: false,
                message: 'title, content, author는 필수입니다.'
            });
        }

        const result = await db.insert(posts).values({
            title,
            content,
            author,
            published: published || false
        }).returning();

        res.status(201).json({
            success: true,
            message: '게시글이 생성되었습니다.',
            data: result[0]
        });
    } catch (error) {
        res.status(500).json({
            success: false,
            message: '게시글 생성 실패',
            error: error.message
        });
    }
});

// 게시글 수정
app.put('/api/posts/:id', async (req, res) => {
    try {
        const postId = parseInt(req.params.id);
        const { title, content, published } = req.body;

        const updateData = {};
        if (title !== undefined) updateData.title = title;
        if (content !== undefined) updateData.content = content;
        if (published !== undefined) updateData.published = published;
        updateData.updatedAt = new Date().toISOString();

        const result = await db.update(posts)
            .set(updateData)
            .where(eq(posts.id, postId))
            .returning();

        if (result.length === 0) {
            return res.status(404).json({
                success: false,
                message: '게시글을 찾을 수 없습니다.'
            });
        }

        res.json({
            success: true,
            message: '게시글이 수정되었습니다.',
            data: result[0]
        });
    } catch (error) {
        res.status(500).json({
            success: false,
            message: '게시글 수정 실패',
            error: error.message
        });
    }
});

// 게시글 삭제
app.delete('/api/posts/:id', async (req, res) => {
    try {
        const postId = parseInt(req.params.id);

        const result = await db.delete(posts)
            .where(eq(posts.id, postId))
            .returning();

        if (result.length === 0) {
            return res.status(404).json({
                success: false,
                message: '게시글을 찾을 수 없습니다.'
            });
        }

        res.json({
            success: true,
            message: '게시글이 삭제되었습니다.',
            data: result[0]
        });
    } catch (error) {
        res.status(500).json({
            success: false,
            message: '게시글 삭제 실패',
            error: error.message
        });
    }
});

// ==================== COMMENTS API ====================

app.post('/api/comments', async (req, res) => {
    try {
        const { postId, author, content } = req.body;

        if (!postId || !author || !content) {
            return res.status(400).json({
                success: false,
                message: 'postId, author, content는 필수입니다.'
            });
        }

        const post = await db.select()
            .from(posts)
            .where(eq(posts.id, postId))
            .limit(1);

        if (post.length === 0) {
            return res.status(404).json({
                success: false,
                message: '게시글을 찾을 수 없습니다.'
            });
        }

        const result = await db.insert(comments).values({
            postId,
            author,
            content
        }).returning();

        res.status(201).json({
            success: true,
            message: '댓글이 생성되었습니다.',
            data: result[0]
        });
    } catch (error) {
        res.status(500).json({
            success: false,
            message: '댓글 생성 실패',
            error: error.message
        });
    }
});

app.get('/api/posts/:id/comments', async (req, res) => {
    try {
        const postId = parseInt(req.params.id);

        const result = await db.select()
            .from(comments)
            .where(eq(comments.postId, postId))
            .orderBy(desc(comments.createdAt));

        res.json({
            success: true,
            count: result.length,
            data: result
        });
    } catch (error) {
        res.status(500).json({
            success: false,
            message: '댓글 조회 실패',
            error: error.message
        });
    }
});

app.delete('/api/comments/:id', async (req, res) => {
    try {
        const commentId = parseInt(req.params.id);

        const result = await db.delete(comments)
            .where(eq(comments.id, commentId))
            .returning();

        if (result.length === 0) {
            return res.status(404).json({
                success: false,
                message: '댓글을 찾을 수 없습니다.'
            });
        }

        res.json({
            success: true,
            message: '댓글이 삭제되었습니다.',
            data: result[0]
        });
    } catch (error) {
        res.status(500).json({
            success: false,
            message: '댓글 삭제 실패',
            error: error.message
        });
    }
});

// ==================== CATEGORIES API ====================

app.get('/api/categories', async (req, res) => {
    try {
        const result = await db.select()
            .from(categories)
            .orderBy(categories.name);

        res.json({
            success: true,
            count: result.length,
            data: result
        });
    } catch (error) {
        res.status(500).json({
            success: false,
            message: '카테고리 조회 실패',
            error: error.message
        });
    }
});

app.post('/api/categories', async (req, res) => {
    try {
        const { name, description } = req.body;

        if (!name) {
            return res.status(400).json({
                success: false,
                message: 'name은 필수입니다.'
            });
        }

        const result = await db.insert(categories).values({
            name,
            description
        }).returning();

        res.status(201).json({
            success: true,
            message: '카테고리가 생성되었습니다.',
            data: result[0]
        });
    } catch (error) {
        if (error.message.includes('UNIQUE')) {
            return res.status(400).json({
                success: false,
                message: '이미 존재하는 카테고리 이름입니다.'
            });
        }

        res.status(500).json({
            success: false,
            message: '카테고리 생성 실패',
            error: error.message
        });
    }
});

app.delete('/api/categories/:id', async (req, res) => {
    try {
        const categoryId = parseInt(req.params.id);

        const result = await db.delete(categories)
            .where(eq(categories.id, categoryId))
            .returning();

        if (result.length === 0) {
            return res.status(404).json({
                success: false,
                message: '카테고리를 찾을 수 없습니다.'
            });
        }

        res.json({
            success: true,
            message: '카테고리가 삭제되었습니다.',
            data: result[0]
        });
    } catch (error) {
        res.status(500).json({
            success: false,
            message: '카테고리 삭제 실패',
            error: error.message
        });
    }
});

// ==================== 통계 API ====================

app.get('/api/stats', async (req, res) => {
    try {
        const allPosts = await db.select().from(posts);
        const allComments = await db.select().from(comments);
        const allCategories = await db.select().from(categories);

        const publishedPosts = allPosts.filter(p => p.published);

        res.json({
            success: true,
            data: {
                posts: {
                    total: allPosts.length,
                    published: publishedPosts.length,
                    draft: allPosts.length - publishedPosts.length
                },
                comments: {
                    total: allComments.length
                },
                categories: {
                    total: allCategories.length
                }
            }
        });
    } catch (error) {
        res.status(500).json({
            success: false,
            message: '통계 조회 실패',
            error: error.message
        });
    }
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

// CORS 에러 핸들러
app.use((err, req, res, next) => {
    if (err.message.includes('CORS')) {
        return res.status(403).json({
            success: false,
            message: 'CORS 정책 위반',
            error: err.message,
            origin: req.get('origin')
        });
    }

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
  🚀 Drizzle ORM + SQLite 서버가 시작되었습니다!
  
  📍 URL: http://localhost:${PORT}
  💾 데이터베이스: SQLite (sqlite.db)
  🔧 ORM: Drizzle ORM
  🔒 CORS: ${process.env.NODE_ENV === 'development' ? '개발 모드 (모든 출처 허용)' : '프로덕션 모드 (제한된 출처)'}
  ⏰ 시작 시간: ${new Date().toLocaleString('ko-KR')}
  
  📚 API 엔드포인트:
  
  📝 Posts (게시글)
  - GET    /api/posts              - 모든 게시글 조회 (페이지네이션, 검색)
  - GET    /api/posts/:id          - 특정 게시글 조회 (댓글 포함)
  - POST   /api/posts              - 게시글 생성
  - PUT    /api/posts/:id          - 게시글 수정
  - DELETE /api/posts/:id          - 게시글 삭제
  
  💬 Comments (댓글)
  - GET    /api/posts/:id/comments - 특정 게시글의 댓글 조회
  - POST   /api/comments           - 댓글 생성
  - DELETE /api/comments/:id       - 댓글 삭제
  
  🏷️  Categories (카테고리)
  - GET    /api/categories         - 모든 카테고리 조회
  - POST   /api/categories         - 카테고리 생성
  - DELETE /api/categories/:id     - 카테고리 삭제
  
  � 공개 API
  - GET    /api/public             - 모든 출처 허용
  
  🔐 제한 API
  - GET    /api/admin              - 관리자 출처만 허용
  
  �📊 기타
  - GET    /api/stats              - 통계 조회
  - GET    /api/health             - 헬스 체크
  
  💡 CORS 테스트:
  - 프론트엔드에서 fetch() 사용
  - credentials: 'include' 옵션으로 쿠키 포함
  - 커스텀 헤더 확인: X-Total-Count, X-Page-Number
  `);
});
