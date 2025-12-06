# 🗄️ Drizzle ORM + SQLite API 서버

Drizzle ORM을 사용한 완전한 CRUD REST API 서버입니다.

## 📋 목차
- [특징](#특징)
- [설치](#설치)
- [실행](#실행)
- [API 문서](#api-문서)
- [데이터베이스 스키마](#데이터베이스-스키마)

## ✨ 특징

- ✅ **Drizzle ORM** - 타입 안전한 SQL 쿼리
- ✅ **SQLite** - 파일 기반 경량 데이터베이스
- ✅ **완전한 CRUD** - 생성, 조회, 수정, 삭제
- ✅ **관계형 데이터** - Posts, Comments, Categories
- ✅ **자동 타임스탬프** - 생성/수정 시간 자동 기록
- ✅ **외래 키** - CASCADE 삭제 지원

## 🔧 설치

```bash
# 의존성 설치
npm install

# 또는 yarn
yarn install
```

## 🚀 실행

### Drizzle ORM 서버 실행
```bash
# 개발 모드 (자동 재시작)
npm run dev:drizzle

# 프로덕션 모드
npm run start:drizzle
```

### Drizzle Studio (데이터베이스 GUI)
```bash
npm run db:studio
```
브라우저에서 `https://local.drizzle.studio`로 접속하여 데이터베이스를 시각적으로 관리할 수 있습니다.

## 📊 데이터베이스 스키마

### Posts (게시글)
```javascript
{
  id: integer (PK, Auto Increment),
  title: text (NOT NULL),
  content: text (NOT NULL),
  author: text (NOT NULL),
  published: boolean (DEFAULT false),
  createdAt: timestamp,
  updatedAt: timestamp
}
```

### Comments (댓글)
```javascript
{
  id: integer (PK, Auto Increment),
  postId: integer (FK -> posts.id, CASCADE),
  author: text (NOT NULL),
  content: text (NOT NULL),
  createdAt: timestamp
}
```

### Categories (카테고리)
```javascript
{
  id: integer (PK, Auto Increment),
  name: text (UNIQUE, NOT NULL),
  description: text,
  createdAt: timestamp
}
```

## 📚 API 문서

### Base URL
```
http://localhost:3000
```

---

## 📝 Posts API

### 1. 모든 게시글 조회
```http
GET /api/posts
GET /api/posts?published=true
GET /api/posts?search=제목검색
```

**응답 예시:**
```json
{
  "success": true,
  "count": 2,
  "data": [
    {
      "id": 1,
      "title": "첫 번째 게시글",
      "content": "게시글 내용입니다.",
      "author": "홍길동",
      "published": true,
      "createdAt": "2024-01-01T00:00:00.000Z",
      "updatedAt": "2024-01-01T00:00:00.000Z"
    }
  ]
}
```

### 2. 특정 게시글 조회 (댓글 포함)
```http
GET /api/posts/:id
```

**응답 예시:**
```json
{
  "success": true,
  "data": {
    "id": 1,
    "title": "첫 번째 게시글",
    "content": "게시글 내용입니다.",
    "author": "홍길동",
    "published": true,
    "createdAt": "2024-01-01T00:00:00.000Z",
    "updatedAt": "2024-01-01T00:00:00.000Z",
    "comments": [
      {
        "id": 1,
        "postId": 1,
        "author": "김철수",
        "content": "좋은 글이네요!",
        "createdAt": "2024-01-01T01:00:00.000Z"
      }
    ]
  }
}
```

### 3. 게시글 생성
```http
POST /api/posts
Content-Type: application/json

{
  "title": "새로운 게시글",
  "content": "게시글 내용입니다.",
  "author": "홍길동",
  "published": false
}
```

### 4. 게시글 수정
```http
PUT /api/posts/:id
Content-Type: application/json

{
  "title": "수정된 제목",
  "content": "수정된 내용",
  "published": true
}
```

### 5. 게시글 삭제
```http
DELETE /api/posts/:id
```

---

## 💬 Comments API

### 1. 댓글 생성
```http
POST /api/comments
Content-Type: application/json

{
  "postId": 1,
  "author": "김철수",
  "content": "좋은 글이네요!"
}
```

### 2. 특정 게시글의 댓글 조회
```http
GET /api/posts/:id/comments
```

### 3. 댓글 삭제
```http
DELETE /api/comments/:id
```

---

## 🏷️ Categories API

### 1. 모든 카테고리 조회
```http
GET /api/categories
```

### 2. 카테고리 생성
```http
POST /api/categories
Content-Type: application/json

{
  "name": "기술",
  "description": "기술 관련 게시글"
}
```

### 3. 카테고리 삭제
```http
DELETE /api/categories/:id
```

---

## 📊 통계 API

```http
GET /api/stats
```

**응답 예시:**
```json
{
  "success": true,
  "data": {
    "posts": {
      "total": 10,
      "published": 7,
      "draft": 3
    },
    "comments": {
      "total": 25
    },
    "categories": {
      "total": 5
    }
  }
}
```

---

## 🧪 테스트 예시

### cURL로 테스트

```bash
# 게시글 생성
curl -X POST http://localhost:3000/api/posts \
  -H "Content-Type: application/json" \
  -d '{
    "title": "Drizzle ORM 사용기",
    "content": "Drizzle ORM은 정말 훌륭합니다!",
    "author": "개발자",
    "published": true
  }'

# 게시글 조회
curl http://localhost:3000/api/posts

# 댓글 생성
curl -X POST http://localhost:3000/api/comments \
  -H "Content-Type: application/json" \
  -d '{
    "postId": 1,
    "author": "독자",
    "content": "유익한 정보 감사합니다!"
  }'

# 특정 게시글과 댓글 조회
curl http://localhost:3000/api/posts/1

# 통계 조회
curl http://localhost:3000/api/stats
```

---

## 🗂️ 프로젝트 구조

```
.
├── db/
│   ├── schema.js          # Drizzle ORM 스키마 정의
│   └── index.js           # 데이터베이스 연결 및 초기화
├── server-drizzle.js      # Drizzle ORM 서버
├── drizzle.config.js      # Drizzle Kit 설정
├── package.json           # 의존성 관리
├── sqlite.db              # SQLite 데이터베이스 파일 (자동 생성)
└── DRIZZLE-README.md      # 이 문서
```

---

## 🎯 Drizzle ORM 주요 기능

### 1. 타입 안전한 쿼리
```javascript
// 자동 완성과 타입 체크 지원
const posts = await db.select().from(posts);
```

### 2. 관계형 쿼리
```javascript
// 외래 키 관계 자동 처리
const comments = await db.select()
  .from(comments)
  .where(eq(comments.postId, 1));
```

### 3. 필터링과 정렬
```javascript
// 복잡한 쿼리도 간단하게
const result = await db.select()
  .from(posts)
  .where(and(
    eq(posts.published, true),
    like(posts.title, '%검색어%')
  ))
  .orderBy(desc(posts.createdAt));
```

### 4. CASCADE 삭제
```javascript
// 게시글 삭제 시 댓글도 자동 삭제
await db.delete(posts).where(eq(posts.id, 1));
// 연결된 모든 댓글도 자동으로 삭제됨
```

---

## 🔍 Drizzle Studio

Drizzle Studio는 데이터베이스를 시각적으로 관리할 수 있는 웹 기반 GUI입니다.

```bash
npm run db:studio
```

**기능:**
- 📊 테이블 데이터 조회 및 편집
- ➕ 레코드 추가/수정/삭제
- 🔍 데이터 검색 및 필터링
- 📈 스키마 시각화

---

## 💡 팁

### 데이터베이스 초기화
서버를 처음 실행하면 `sqlite.db` 파일이 자동으로 생성되고 테이블이 초기화됩니다.

### 데이터베이스 리셋
```bash
# sqlite.db 파일 삭제 후 서버 재시작
rm sqlite.db
npm run start:drizzle
```

### 마이그레이션
```bash
# 스키마 변경사항을 데이터베이스에 적용
npm run db:push
```

---

## 🚀 배포

Drizzle ORM + SQLite는 다음 플랫폼에 배포 가능합니다:

- **Railway** ✅ (추천)
- **Render** ✅
- **Fly.io** ✅
- **Vercel** ⚠️ (서버리스 함수로 제한적)

---

## 📖 참고 자료

- [Drizzle ORM 공식 문서](https://orm.drizzle.team/)
- [SQLite 문서](https://www.sqlite.org/docs.html)
- [Better SQLite3](https://github.com/WiseLibs/better-sqlite3)

---

## 🤝 기여

이슈와 PR은 언제나 환영합니다!
