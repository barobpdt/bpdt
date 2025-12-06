# 🔒 CORS (Cross-Origin Resource Sharing) 완벽 가이드

## 📋 목차
- [CORS란?](#cors란)
- [기본 사용법](#기본-사용법)
- [고급 설정](#고급-설정)
- [실전 예제](#실전-예제)
- [보안 고려사항](#보안-고려사항)

---

## 🤔 CORS란?

**CORS (Cross-Origin Resource Sharing)**는 웹 브라우저에서 다른 도메인의 리소스에 접근할 수 있도록 허용하는 메커니즘입니다.

### 왜 필요한가?

브라우저는 보안상의 이유로 **Same-Origin Policy (동일 출처 정책)**을 적용합니다.

```
❌ 차단되는 경우:
프론트엔드: http://localhost:3001
백엔드:     http://localhost:3000
→ 포트가 다르므로 다른 출처!

✅ 허용되는 경우:
프론트엔드: http://example.com
백엔드:     http://example.com
→ 같은 출처!
```

---

## 🚀 기본 사용법

### 1. 모든 출처 허용 (개발용)

```javascript
import cors from 'cors';

// 가장 간단한 방법 - 모든 출처 허용
app.use(cors());
```

⚠️ **주의**: 프로덕션에서는 사용하지 마세요!

---

### 2. 특정 출처만 허용 (권장)

```javascript
// 단일 출처
app.use(cors({
  origin: 'https://myapp.com'
}));

// 여러 출처
app.use(cors({
  origin: ['https://myapp.com', 'https://admin.myapp.com']
}));
```

---

### 3. 동적 출처 허용

```javascript
app.use(cors({
  origin: function (origin, callback) {
    const allowedOrigins = [
      'https://myapp.com',
      'https://admin.myapp.com',
      'http://localhost:3000'
    ];
    
    // origin이 undefined인 경우는 같은 출처 요청
    if (!origin || allowedOrigins.includes(origin)) {
      callback(null, true);
    } else {
      callback(new Error('CORS 정책에 의해 차단되었습니다.'));
    }
  }
}));
```

---

## ⚙️ 고급 설정

### 완전한 CORS 옵션

```javascript
app.use(cors({
  // 허용할 출처
  origin: 'https://myapp.com',
  
  // 허용할 HTTP 메서드
  methods: ['GET', 'POST', 'PUT', 'DELETE', 'PATCH'],
  
  // 허용할 헤더
  allowedHeaders: ['Content-Type', 'Authorization'],
  
  // 노출할 헤더 (클라이언트가 접근 가능)
  exposedHeaders: ['X-Total-Count', 'X-Page-Number'],
  
  // 인증 정보 포함 허용 (쿠키, Authorization 헤더 등)
  credentials: true,
  
  // Preflight 요청 캐시 시간 (초)
  maxAge: 86400, // 24시간
  
  // OPTIONS 요청에 대한 상태 코드
  optionsSuccessStatus: 204
}));
```

---

## 🎯 실전 예제

### 예제 1: 환경별 CORS 설정

```javascript
const corsOptions = {
  origin: function (origin, callback) {
    // 개발 환경
    if (process.env.NODE_ENV === 'development') {
      callback(null, true); // 모든 출처 허용
      return;
    }
    
    // 프로덕션 환경
    const allowedOrigins = [
      'https://myapp.com',
      'https://www.myapp.com',
      'https://admin.myapp.com'
    ];
    
    if (!origin || allowedOrigins.includes(origin)) {
      callback(null, true);
    } else {
      callback(new Error('Not allowed by CORS'));
    }
  },
  credentials: true,
  methods: ['GET', 'POST', 'PUT', 'DELETE'],
  allowedHeaders: ['Content-Type', 'Authorization']
};

app.use(cors(corsOptions));
```

---

### 예제 2: 라우트별 CORS 설정

```javascript
// 전역 CORS 비활성화
// app.use(cors()); ← 이거 사용 안 함

// 특정 라우트에만 CORS 적용
app.get('/api/public', cors(), (req, res) => {
  res.json({ message: '누구나 접근 가능' });
});

// 특정 출처만 허용하는 라우트
const restrictedCors = cors({
  origin: 'https://admin.myapp.com',
  credentials: true
});

app.get('/api/admin', restrictedCors, (req, res) => {
  res.json({ message: '관리자만 접근 가능' });
});

// CORS 없는 라우트 (같은 출처만 접근 가능)
app.get('/api/internal', (req, res) => {
  res.json({ message: '내부 API' });
});
```

---

### 예제 3: 정규식으로 출처 패턴 매칭

```javascript
app.use(cors({
  origin: function (origin, callback) {
    // localhost의 모든 포트 허용
    const localhostPattern = /^http:\/\/localhost:\d+$/;
    
    // 서브도메인 허용
    const subdomainPattern = /^https:\/\/[\w-]+\.myapp\.com$/;
    
    if (!origin || 
        localhostPattern.test(origin) || 
        subdomainPattern.test(origin)) {
      callback(null, true);
    } else {
      callback(new Error('Not allowed by CORS'));
    }
  }
}));
```

---

### 예제 4: 인증이 필요한 API

```javascript
// 쿠키와 함께 요청을 받을 때
app.use(cors({
  origin: 'https://myapp.com',
  credentials: true, // 중요!
  allowedHeaders: ['Content-Type', 'Authorization', 'Cookie']
}));

// 프론트엔드에서는 이렇게 요청
// fetch('http://api.example.com/data', {
//   credentials: 'include' // 쿠키 포함
// });
```

---

### 예제 5: Preflight 요청 최적화

```javascript
// Preflight 요청 결과를 24시간 캐시
app.use(cors({
  origin: 'https://myapp.com',
  maxAge: 86400, // 24시간 (초 단위)
  methods: ['GET', 'POST', 'PUT', 'DELETE'],
  allowedHeaders: ['Content-Type', 'Authorization']
}));

// 또는 특정 라우트에서 OPTIONS 요청 직접 처리
app.options('/api/posts', cors());
app.post('/api/posts', cors(), (req, res) => {
  // POST 처리
});
```

---

### 예제 6: 커스텀 헤더 사용

```javascript
app.use(cors({
  origin: 'https://myapp.com',
  
  // 클라이언트가 보낼 수 있는 헤더
  allowedHeaders: [
    'Content-Type',
    'Authorization',
    'X-API-Key',
    'X-Request-ID'
  ],
  
  // 클라이언트가 읽을 수 있는 헤더
  exposedHeaders: [
    'X-Total-Count',
    'X-Page-Number',
    'X-Rate-Limit-Remaining'
  ]
}));

// API에서 커스텀 헤더 사용
app.get('/api/posts', (req, res) => {
  res.set('X-Total-Count', '100');
  res.set('X-Page-Number', '1');
  res.json({ data: [] });
});
```

---

### 예제 7: 에러 핸들링

```javascript
const corsOptions = {
  origin: function (origin, callback) {
    const allowedOrigins = ['https://myapp.com'];
    
    if (!origin || allowedOrigins.includes(origin)) {
      callback(null, true);
    } else {
      // CORS 에러 발생
      callback(new Error(`출처 ${origin}는 허용되지 않습니다.`));
    }
  }
};

app.use(cors(corsOptions));

// CORS 에러 핸들러
app.use((err, req, res, next) => {
  if (err.message.includes('허용되지 않습니다')) {
    res.status(403).json({
      success: false,
      message: 'CORS 정책 위반',
      error: err.message
    });
  } else {
    next(err);
  }
});
```

---

## 🔐 보안 고려사항

### ✅ DO (해야 할 것)

```javascript
// 1. 프로덕션에서는 특정 출처만 허용
app.use(cors({
  origin: ['https://myapp.com', 'https://www.myapp.com']
}));

// 2. credentials: true 사용 시 origin을 명시
app.use(cors({
  origin: 'https://myapp.com',
  credentials: true
}));

// 3. 필요한 메서드만 허용
app.use(cors({
  methods: ['GET', 'POST', 'PUT', 'DELETE']
}));

// 4. 환경 변수 사용
app.use(cors({
  origin: process.env.ALLOWED_ORIGINS?.split(',') || []
}));
```

---

### ❌ DON'T (하지 말아야 할 것)

```javascript
// 1. 프로덕션에서 모든 출처 허용
app.use(cors()); // ❌ 위험!

// 2. credentials: true와 origin: '*' 함께 사용
app.use(cors({
  origin: '*',
  credentials: true // ❌ 작동하지 않음!
}));

// 3. 검증 없이 요청 헤더의 origin 사용
app.use(cors({
  origin: (origin, callback) => {
    callback(null, origin); // ❌ 위험!
  }
}));
```

---

## 🧪 테스트 방법

### cURL로 CORS 테스트

```bash
# Preflight 요청 (OPTIONS)
curl -X OPTIONS http://localhost:3000/api/posts \
  -H "Origin: https://myapp.com" \
  -H "Access-Control-Request-Method: POST" \
  -H "Access-Control-Request-Headers: Content-Type" \
  -v

# 실제 요청
curl -X GET http://localhost:3000/api/posts \
  -H "Origin: https://myapp.com" \
  -v
```

---

### 브라우저 콘솔에서 테스트

```javascript
// 프론트엔드에서 테스트
fetch('http://localhost:3000/api/posts', {
  method: 'GET',
  headers: {
    'Content-Type': 'application/json'
  },
  credentials: 'include' // 쿠키 포함
})
.then(res => res.json())
.then(data => console.log(data))
.catch(err => console.error('CORS 에러:', err));
```

---

## 📊 CORS 플로우

```
1. 브라우저가 요청 전송
   ↓
2. 브라우저가 Preflight 요청 (OPTIONS) 전송
   ↓
3. 서버가 CORS 헤더와 함께 응답
   - Access-Control-Allow-Origin
   - Access-Control-Allow-Methods
   - Access-Control-Allow-Headers
   ↓
4. 브라우저가 CORS 정책 확인
   ↓
5. 허용되면 실제 요청 전송
   ↓
6. 서버 응답
```

---

## 💡 자주 묻는 질문

### Q1: CORS 에러가 계속 발생해요!
```javascript
// 확인 사항:
// 1. origin이 정확한지
// 2. credentials: true 사용 시 origin: '*' 아닌지
// 3. 서버가 올바른 CORS 헤더를 보내는지
```

### Q2: localhost에서 테스트하려면?
```javascript
app.use(cors({
  origin: ['http://localhost:3000', 'http://localhost:3001']
}));
```

### Q3: 모바일 앱에서 접근하려면?
```javascript
// 모바일 앱은 CORS 제한이 없으므로
// 별도 설정 불필요. 하지만 API 키 등으로 인증 필요
```

---

## 🎓 추가 학습 자료

- [MDN CORS 문서](https://developer.mozilla.org/ko/docs/Web/HTTP/CORS)
- [cors npm 패키지](https://www.npmjs.com/package/cors)
- [CORS 테스트 도구](https://www.test-cors.org/)
