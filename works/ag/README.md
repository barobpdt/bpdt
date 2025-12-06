# 🚀 Express REST API 서버

간단한 TODO와 사용자 관리 REST API 서버입니다.

## 📋 목차
- [설치 방법](#설치-방법)
- [실행 방법](#실행-방법)
- [API 문서](#api-문서)
- [배포 방법](#배포-방법)

## 🔧 설치 방법

```bash
# 1. 의존성 설치
npm install

# 2. 개발 서버 실행
npm run dev

# 또는 일반 실행
npm start
```

## 🎯 실행 방법

### 로컬 개발
```bash
npm run dev
```

서버가 `http://localhost:3000`에서 실행됩니다.

## 📚 API 문서

### 기본 정보
- **Base URL**: `http://localhost:3000`
- **응답 형식**: JSON

### 엔드포인트

#### 🏠 홈
```
GET /
```
API 서버 정보를 반환합니다.

#### ❤️ 헬스 체크
```
GET /api/health
```
서버 상태를 확인합니다.

---

### 📝 TODO API

#### 모든 TODO 조회
```
GET /api/todos
GET /api/todos?completed=true
```

**응답 예시:**
```json
{
  "success": true,
  "count": 3,
  "data": [
    {
      "id": 1,
      "title": "프로젝트 계획 수립",
      "completed": false,
      "createdAt": "2024-01-01T00:00:00.000Z"
    }
  ]
}
```

#### 특정 TODO 조회
```
GET /api/todos/:id
```

#### TODO 생성
```
POST /api/todos
Content-Type: application/json

{
  "title": "새로운 할 일"
}
```

#### TODO 수정
```
PUT /api/todos/:id
Content-Type: application/json

{
  "title": "수정된 제목",
  "completed": true
}
```

#### TODO 삭제
```
DELETE /api/todos/:id
```

---

### 👥 User API

#### 모든 사용자 조회
```
GET /api/users
```

#### 특정 사용자 조회
```
GET /api/users/:id
```

#### 사용자 생성
```
POST /api/users
Content-Type: application/json

{
  "name": "홍길동",
  "email": "hong@example.com",
  "role": "user"
}
```

---

### 📊 통계 API

#### 통계 조회
```
GET /api/stats
```

**응답 예시:**
```json
{
  "success": true,
  "data": {
    "todos": {
      "total": 3,
      "completed": 1,
      "pending": 2,
      "completionRate": "33.33%"
    },
    "users": {
      "total": 2,
      "admins": 1,
      "regularUsers": 1
    }
  }
}
```

---

## 🧪 API 테스트

### cURL 예시

```bash
# TODO 조회
curl http://localhost:3000/api/todos

# TODO 생성
curl -X POST http://localhost:3000/api/todos \
  -H "Content-Type: application/json" \
  -d '{"title":"새로운 할 일"}'

# TODO 수정
curl -X PUT http://localhost:3000/api/todos/1 \
  -H "Content-Type: application/json" \
  -d '{"completed":true}'

# TODO 삭제
curl -X DELETE http://localhost:3000/api/todos/1
```

### Postman / Thunder Client
1. Postman 또는 VS Code의 Thunder Client 확장 설치
2. 위의 엔드포인트로 요청 테스트

---

## 🌐 배포 방법

### Vercel 배포

1. **vercel.json 생성** (이미 포함됨)
2. **배포 명령어**
```bash
npm i -g vercel
vercel login
vercel
```

### Railway 배포

1. **Railway CLI 설치**
```bash
npm i -g @railway/cli
```

2. **배포**
```bash
railway login
railway init
railway up
```

### Render 배포

1. [Render.com](https://render.com) 접속
2. "New Web Service" 클릭
3. GitHub 저장소 연결
4. 빌드 명령어: `npm install`
5. 시작 명령어: `npm start`
6. 배포!

---

## 📁 프로젝트 구조

```
.
├── server.js           # 메인 서버 파일
├── package.json        # 의존성 관리
├── vercel.json         # Vercel 배포 설정
├── README.md           # 문서
└── public/             # 정적 파일 (선택사항)
```

---

## 🛠️ 기술 스택

- **Node.js** - JavaScript 런타임
- **Express** - 웹 프레임워크
- **CORS** - Cross-Origin Resource Sharing

---

## 📝 라이선스

MIT

---

## 🤝 기여

이슈와 PR은 언제나 환영합니다!

---

## 📞 문의

질문이 있으시면 이슈를 등록해주세요.
