# FastAPI + Supabase 예제

이 프로젝트는 FastAPI와 Supabase를 사용한 간단한 사용자 관리 시스템 예제입니다.

## 🚀 기능

- ✅ 사용자 CRUD 작업 (생성, 조회, 수정, 삭제)
- ✅ 사용자 검색 (이름 또는 이메일로 검색)
- ✅ RESTful API 엔드포인트
- ✅ 자동 API 문서 생성 (Swagger UI)
- ✅ 웹 인터페이스
- ✅ 클라이언트 테스트 스크립트

## 📋 요구사항

- Python 3.8+
- Supabase 계정 및 프로젝트

## 🛠️ 설치 및 설정

### 1. 의존성 설치

```bash
pip install -r requirements.txt
```

### 2. Supabase 프로젝트 설정

1. [Supabase](https://supabase.com)에 가입하고 새 프로젝트를 생성합니다.
2. 프로젝트 대시보드에서 다음 정보를 확인합니다:
   - Project URL
   - Anon (public) key

### 3. 데이터베이스 테이블 생성

Supabase 대시보드의 SQL Editor에서 다음 SQL을 실행하여 `users` 테이블을 생성합니다:

```sql
-- users 테이블 생성
CREATE TABLE users (
    id BIGSERIAL PRIMARY KEY,
    name TEXT NOT NULL,
    email TEXT UNIQUE NOT NULL,
    age INTEGER,
    city TEXT,
    created_at TIMESTAMPTZ DEFAULT NOW()
);

-- RLS (Row Level Security) 비활성화 (예제용)
ALTER TABLE users DISABLE ROW LEVEL SECURITY;

-- 테이블에 대한 모든 권한 부여
GRANT ALL ON users TO anon;
GRANT ALL ON users TO authenticated;
GRANT USAGE, SELECT ON SEQUENCE users_id_seq TO anon;
GRANT USAGE, SELECT ON SEQUENCE users_id_seq TO authenticated;
```

### 4. 환경 변수 설정

프로젝트 루트에 `.env` 파일을 생성하고 다음 내용을 추가합니다:

```env
# Supabase 설정
SUPABASE_URL=https://your-project-id.supabase.co
SUPABASE_ANON_KEY=your-anon-key-here
```

## 🚀 실행 방법

### 1. FastAPI 서버 실행

```bash
python fastapi_supabase_example.py
```

또는 uvicorn을 직접 사용:

```bash
uvicorn fastapi_supabase_example:app --host 0.0.0.0 --port 8000 --reload
```

### 2. 웹 인터페이스 접속

브라우저에서 다음 URL에 접속합니다:
- **API 문서**: http://localhost:8000/docs
- **웹 인터페이스**: http://localhost:8000/static/fastapi_supabase.html

### 3. 클라이언트 테스트 실행

```bash
python fastapi_supabase_client.py
```

## 📚 API 엔드포인트

### 기본 정보
- **GET** `/` - API 정보 및 사용 가능한 엔드포인트 목록

### 사용자 관리
- **GET** `/users` - 모든 사용자 조회
- **GET** `/users/{user_id}` - 특정 사용자 조회
- **POST** `/users` - 새 사용자 생성
- **PUT** `/users/{user_id}` - 사용자 정보 수정
- **DELETE** `/users/{user_id}` - 사용자 삭제

### 검색
- **GET** `/users/search/{keyword}` - 사용자 검색 (이름 또는 이메일)

## 📝 사용 예제

### 1. 새 사용자 생성

```bash
curl -X POST "http://localhost:8000/users" \
     -H "Content-Type: application/json" \
     -d '{
       "name": "홍길동",
       "email": "hong@example.com",
       "age": 30,
       "city": "서울"
     }'
```

### 2. 모든 사용자 조회

```bash
curl "http://localhost:8000/users"
```

### 3. 특정 사용자 조회

```bash
curl "http://localhost:8000/users/1"
```

### 4. 사용자 정보 수정

```bash
curl -X PUT "http://localhost:8000/users/1" \
     -H "Content-Type: application/json" \
     -d '{
       "age": 31,
       "city": "대구"
     }'
```

### 5. 사용자 삭제

```bash
curl -X DELETE "http://localhost:8000/users/1"
```

### 6. 사용자 검색

```bash
curl "http://localhost:8000/users/search/홍"
```

## 🎨 웹 인터페이스 사용법

1. 브라우저에서 `http://localhost:8000/static/fastapi_supabase.html` 접속
2. 각 섹션에서 원하는 작업 수행:
   - **검색**: 이름이나 이메일로 사용자 검색
   - **생성**: 새 사용자 정보 입력 후 생성
   - **수정**: 사용자 ID와 수정할 정보 입력
   - **삭제**: 사용자 ID 입력 후 삭제
   - **목록**: 현재 등록된 모든 사용자 확인

## 🔧 개발 및 확장

### 새로운 엔드포인트 추가

```python
@app.get("/users/count")
async def get_user_count():
    """사용자 수 조회"""
    try:
        response = supabase.table("users").select("*", count="exact").execute()
        return {"count": response.count}
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"사용자 수 조회 실패: {str(e)}")
```

### 새로운 테이블 추가

1. Supabase 대시보드에서 새 테이블 생성
2. FastAPI에서 해당 테이블에 대한 CRUD 엔드포인트 추가
3. Pydantic 모델 정의

### 인증 추가

Supabase Auth를 사용하여 인증을 추가할 수 있습니다:

```python
from supabase import create_client, Client
from fastapi import Depends, HTTPException, status
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials

security = HTTPBearer()

async def get_current_user(credentials: HTTPAuthorizationCredentials = Depends(security)):
    try:
        user = supabase.auth.get_user(credentials.credentials)
        return user
    except Exception:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Invalid authentication credentials",
            headers={"WWW-Authenticate": "Bearer"},
        )
```

## 🐛 문제 해결

### 1. 데이터베이스 연결 오류

- Supabase URL과 API 키가 올바른지 확인
- `.env` 파일이 프로젝트 루트에 있는지 확인
- Supabase 프로젝트가 활성 상태인지 확인

### 2. 테이블이 존재하지 않는 경우

Supabase 대시보드에서 `users` 테이블을 생성했는지 확인하고, 위의 SQL 스크립트를 실행하세요.

### 3. CORS 오류

웹 인터페이스에서 API 호출 시 CORS 오류가 발생하면, FastAPI 앱의 CORS 설정을 확인하세요.

### 4. 포트 충돌

기본 포트 8000이 사용 중인 경우, 다른 포트를 사용하세요:

```bash
uvicorn fastapi_supabase_example:app --host 0.0.0.0 --port 8001 --reload
```

## 📁 프로젝트 구조

```
├── fastapi_supabase_example.py    # 메인 FastAPI 애플리케이션
├── fastapi_supabase_client.py     # API 테스트 클라이언트
├── templates/
│   └── fastapi_supabase.html      # 웹 인터페이스
├── requirements.txt               # Python 의존성
├── env_example.txt               # 환경 변수 예제
└── README_FASTAPI_SUPABASE.md    # 이 파일
```

## 🤝 기여하기

1. 이 저장소를 포크합니다
2. 새로운 기능 브랜치를 생성합니다 (`git checkout -b feature/amazing-feature`)
3. 변경사항을 커밋합니다 (`git commit -m 'Add some amazing feature'`)
4. 브랜치에 푸시합니다 (`git push origin feature/amazing-feature`)
5. Pull Request를 생성합니다

## 📄 라이선스

이 프로젝트는 MIT 라이선스 하에 배포됩니다.

## 🔗 유용한 링크

- [FastAPI 공식 문서](https://fastapi.tiangolo.com/)
- [Supabase 공식 문서](https://supabase.com/docs)
- [Pydantic 공식 문서](https://pydantic-docs.helpmanual.io/)
- [Uvicorn 공식 문서](https://www.uvicorn.org/) 