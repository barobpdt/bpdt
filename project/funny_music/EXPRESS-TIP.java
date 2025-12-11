##> neon 사용법
	const sql = neon(DATABASE_URL)
	const result = await sql`insert into tbl (a,b,c) values (a,b,c) RETURNING *`
	clog(result[0])

##> 게시글 CRUD cros 적용
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


##> ENV 활용
#config/env.js
	import "dotenv/config";
	export const ENV = {
	  PORT: process.env.PORT || 5001,
	  DATABASE_URL: process.env.DATABASE_URL,
	  NODE_ENV: process.env.NODE_ENV,
	};
	
#config/db
	import { drizzle } from "drizzle-orm/neon-http";
	import { neon } from "@neondatabase/serverless";
	import { ENV } from "./env.js";
	import * as schema from "../db/schema.js";
	const sql = neon(ENV.DATABASE_URL);
	export const db = drizzle(sql, { schema });
	
#config/cron
	import cron from "cron";
	import https from "https";
	const job = new cron.CronJob("*/14 * * * *", function () {
	  https
		.get(process.env.API_URL, (res) => {
		  if (res.statusCode === 200) console.log("GET request sent successfully");
		  else console.log("GET request failed", res.statusCode);
		})
		.on("error", (e) => console.error("Error while sending request", e));
	});
	export default job;

#db/schema
	import { pgTable, serial, text, timestamp, integer } from "drizzle-orm/pg-core";
	export const favoritesTable = pgTable("favorites", {
	  id: serial("id").primaryKey(),
	  userId: text("user_id").notNull(),
	  recipeId: integer("recipe_id").notNull(),
	  title: text("title").notNull(),
	  image: text("image"),
	  cookTime: text("cook_time"),
	  servings: text("servings"),
	  createdAt: timestamp("created_at").defaultNow(),
	});

import express from "express";
import { ENV } from "./config/env.js";
import { db } from "./config/db.js";
import { favoritesTable } from "./db/schema.js";
import { and, eq } from "drizzle-orm";
import job from "./config/cron.js";

const app = express();
const PORT = ENV.PORT || 5001;

if (ENV.NODE_ENV === "production") job.start();

app.use(express.json());

app.get("/api/health", (req, res) => {
  res.status(200).json({ success: true });
});

app.post("/api/favorites", async (req, res) => {
  try {
    const { userId, recipeId, title, image, cookTime, servings } = req.body;

    if (!userId || !recipeId || !title) {
      return res.status(400).json({ error: "Missing required fields" });
    }

    const newFavorite = await db
      .insert(favoritesTable)
      .values({
        userId,
        recipeId,
        title,
        image,
        cookTime,
        servings,
      })
      .returning();

    res.status(201).json(newFavorite[0]);
  } catch (error) {
    console.log("Error adding favorite", error);
    res.status(500).json({ error: "Something went wrong" });
  }
});

app.get("/api/favorites/:userId", async (req, res) => {
  try {
    const { userId } = req.params;
    const userFavorites = await db
      .select()
      .from(favoritesTable)
      .where(eq(favoritesTable.userId, userId));

    res.status(200).json(userFavorites);
  } catch (error) {
    console.log("Error fetching the favorites", error);
    res.status(500).json({ error: "Something went wrong" });
  }
});

app.delete("/api/favorites/:userId/:recipeId", async (req, res) => {
  try {
    const { userId, recipeId } = req.params;
    await db
      .delete(favoritesTable)
      .where(
        and(eq(favoritesTable.userId, userId), eq(favoritesTable.recipeId, parseInt(recipeId)))
      );

    res.status(200).json({ message: "Favorite removed successfully" });
  } catch (error) {
    console.log("Error removing a favorite", error);
    res.status(500).json({ error: "Something went wrong" });
  }
});

app.listen(PORT, () => {
  console.log("Server is running on PORT:", PORT);
});


##> 카카오 인증구현

[backend]
app.post('/auth', auth);
const auth = async (req, res) => {
  const { authToken } = req.body;
  try {
    const kakaoResponse = await axios
      .post(
        'https://kauth.kakao.com/oauth/token',
        {},
        {
          header: {
            'Content-type': 'application/x-www-form-urlencoded;charset=utf-8',
          },
          params: {
            grant_type: 'authorization_code',
            client_id: ENV.KAKAO_REST_API_KEY,
            code: authToken,
            redirect_uri: `http://localhost:3000/auth?social_provider=kakao`,
          },
        }
      )
      .then((res) => res.data.access_token);
      console.log('카카오응답', kakaoResponse);
	res.send(kakaoResponse);
	// kakaoResponse => 
	//	access_token, token_type:'bearer', expires_in, 
	//	scope:'profile_nickname', refresh_token, refresh_token_expires_in
  } catch (err) {
    console.log('에러', err);
  }
};

[frontend 호출]
const Auth = () => {
  const navigate = useNavigate();
  const authMutation = async (authToken: string) => {
    const code = await axios
      .post('http://localhost:4000/auth', {
        authToken,
      })
      .then((res) => res.data);
    return code;
  };
  useEffect(() => {
  	// url parameter의 code를 authToken으로 저장한다.
    const authToken = new URL(window.location.href).searchParams.get('code');
    try {
      if (!authToken) return;
      authMutation(authToken);
      // 토큰 전송후 '/'페이지로 리다이렉트
      navigate('/');
    } catch (err) {
      alert(err);
    }
  }, []);
  return <>auth loading..</>;
};
export default Auth;

##> 카카오 인증구현 (별도 라우트 만드는 경우)
[src/Router/kakaoRoutes.ts]
import { Request, Response, Router } from "express";
import { setUsers } from "../Firebase/user";
import dotenv from "dotenv";
import axios from "axios";
import qs from "qs";
export const kakaoRouter = Router();

/* login 이후 나타나는 callback page */
kakaoRouter.get("/oauth/callback/kakao", async (req: Request, res: Response) => {
  /* access token 발급 */
  let token: any;
  try {
    token = await axios({
      method: "POST",
      url: "https://kauth.kakao.com/oauth/token",
      headers: {
        "content-type": "application/x-www-form-urlencoded",
      },
      data: qs.stringify({
        grant_type: "authorization_code",
        client_id: ENV.CLIENT_ID,
        redirectUri: ENV.REDIRECT_URI,
        code: req.query.code as string,
      }),
    });
  } catch (e: any) {
    res.json(e.data);
    return;
  }

  /* access token 발급받은 뒤 사용자 정보 가져옴 */
  let user: any;
  try {
    user = await axios({
      method: "GET",
      url: "https://kapi.kakao.com/v2/user/me",
      headers: {
        Authorization: `Bearer ${token.data.access_token}`,
      },
    });
  } catch (e: any) {
    res.json(e.data);
    return;
  }

  /* 가지고 온 사용자 정보 DB */
  await setUsers(user.data); 
  /*
  const userData = user.data
  const userInfo = {
    _id: userData.id,
    email: userData.kakao_account.email,
    name: userData.kakao_account.profile.nickname,
  };
  */
  /* session 저장 */
  req.session.userData = { 
	_id: userData.id, 
	name: userData.kakao_account.profile.nickname };
  res.redirect("http://localhost:4000/main");
});

[서버시작]
import express from "express";
import { sessionConfig } from "./Config/sessionConfig";
import { kakaoRouter } from "./Router/kakaoRoutes";

const app = express();
const PORT = process.env.PORT;

/* kakao login */
app.use(sessionConfig);
app.use(kakaoRouter);

app.listen(PORT, () => {
  console.log(`Server listening on port ${PORT}`);
});

[frontend]
const auth = (e: React.FormEvent<HTMLFormElement>) => {
	e.preventDefault();
	const kakaoAuth = `https://kauth.kakao.com/oauth/authorize?client_id=${rest_api_key}&redirect_uri=${redirect_uri}&response_type=code`;
	window.location.assign(kakaoAuth);
};
return (
<>
  <Form onSubmit={auth}>
	<KakaoSignin>Kakao Signin</KakaoSignin>
  </Form>
</>
);