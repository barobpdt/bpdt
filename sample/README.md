# FastAPI SQLite CRUD 샘플

FastAPI와 SQLite를 사용한 간단한 CRUD(Create, Read, Update, Delete) 애플리케이션입니다.

## 기능

- ✅ 사용자 생성 (CREATE)
- ✅ 사용자 조회 (READ) - 전체 목록 및 개별 조회
- ✅ 사용자 수정 (UPDATE)
- ✅ 사용자 삭제 (DELETE)
- ✅ 사용자 검색 (이름 기반)
- ✅ 사용자 통계 조회
- ✅ 자동 API 문서 생성 (Swagger UI)
- ✅ CORS 지원

## 설치 및 실행

### 1. 의존성 설치

```bash
pip install -r requirements.txt
```

### 2. 서버 실행

```bash
python fastapi_sqlite_crud.py
```

서버가 `http://localhost:8000`에서 실행됩니다.

### 3. API 문서 확인

브라우저에서 다음 URL에 접속하여 API 문서를 확인할 수 있습니다:
- Swagger UI: http://localhost:8000/docs
- ReDoc: http://localhost:8000/redoc

### 4. 테스트 실행

```bash
python test_api.py
```

## API 엔드포인트

### 기본 정보
- `GET /` - API 기본 정보 및 엔드포인트 목록

### 사용자 관리
- `POST /users/` - 새 사용자 생성
- `GET /users/` - 모든 사용자 조회 (페이지네이션 지원)
- `GET /users/{user_id}` - 특정 사용자 조회
- `PUT /users/{user_id}` - 사용자 정보 수정
- `DELETE /users/{user_id}` - 사용자 삭제

### 추가 기능
- `GET /users/search/{name}` - 이름으로 사용자 검색
- `GET /stats/users` - 사용자 통계 조회

## 데이터 모델

### User 모델
```python
{
    "id": int,           # 자동 생성되는 고유 ID
    "name": str,         # 사용자 이름 (필수)
    "email": str,        # 이메일 (필수, 고유)
    "age": int,          # 나이 (선택)
    "is_active": bool,   # 활성 상태 (기본값: True)
    "created_at": datetime,  # 생성 시간 (자동)
    "updated_at": datetime   # 수정 시간 (자동)
}
```

## 사용 예시

### 1. 사용자 생성
```bash
curl -X POST "http://localhost:8000/users/" \
     -H "Content-Type: application/json" \
     -d '{
       "name": "김철수",
       "email": "kim@example.com",
       "age": 25,
       "is_active": true
     }'
```

### 2. 모든 사용자 조회
```bash
curl -X GET "http://localhost:8000/users/"
```

### 3. 특정 사용자 조회
```bash
curl -X GET "http://localhost:8000/users/1"
```

### 4. 사용자 정보 수정
```bash
curl -X PUT "http://localhost:8000/users/1" \
     -H "Content-Type: application/json" \
     -d '{
       "age": 26,
       "is_active": false
     }'
```

### 5. 사용자 삭제
```bash
curl -X DELETE "http://localhost:8000/users/1"
```

### 6. 사용자 검색
```bash
curl -X GET "http://localhost:8000/users/search/김"
```

### 7. 통계 조회
```bash
curl -X GET "http://localhost:8000/stats/users"
```

## 프로젝트 구조

```
├── fastapi_sqlite_crud.py  # 메인 애플리케이션
├── test_api.py            # API 테스트 스크립트
├── requirements.txt       # 의존성 목록
├── README.md             # 프로젝트 문서
└── crud_sample.db        # SQLite 데이터베이스 (자동 생성)
```

## 주요 특징

1. **SQLAlchemy ORM**: 데이터베이스 작업을 위한 ORM 사용
2. **Pydantic 모델**: 데이터 검증 및 직렬화
3. **의존성 주입**: FastAPI의 Depends를 사용한 데이터베이스 세션 관리
4. **자동 문서화**: Swagger UI와 ReDoc을 통한 자동 API 문서 생성
5. **에러 처리**: 적절한 HTTP 상태 코드와 에러 메시지
6. **CORS 지원**: 웹 애플리케이션에서의 API 호출 지원

## 개발 환경

- Python 3.7+
- FastAPI 0.104.1
- SQLAlchemy 2.0.23
- SQLite3
- Pydantic 2.5.0

## 라이선스

MIT License 