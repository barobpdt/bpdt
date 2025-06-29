# FastAPI MySQL CRUD Sample with QR User Management

비동기 FastAPI와 MySQL을 사용한 CRUD 애플리케이션입니다. QR 사용자 관리 기능을 포함합니다.

## 🚀 주요 기능

- **비동기 데이터베이스 연결**: SQLAlchemy의 비동기 기능을 사용하여 성능 최적화
- **QR 사용자 관리**: QR 코드 관련 사용자 정보 CRUD 작업
- **실시간 로깅**: 일자별 로그 파일 관리
- **RESTful API**: 표준 HTTP 메서드를 사용한 API 엔드포인트
- **자동 문서화**: FastAPI 자동 생성 API 문서

## 📋 요구사항

- Python 3.8+
- MySQL 5.7+
- aiomysql (비동기 MySQL 드라이버)

## 🛠️ 설치 및 실행

### 1. 의존성 설치

```bash
pip install -r requirements.txt
```

### 2. 데이터베이스 설정

`fastapi_mysql_crud.py` 파일에서 데이터베이스 연결 정보를 수정하세요:

```python
DB_HOST = "your_host:port"
DB_NAME = "your_database"
DB_USER = "your_username"
DB_PASSWORD = "your_password"
```

### 3. 애플리케이션 실행

```bash
python fastapi_mysql_crud.py
```

또는 Windows에서:

```bash
run.bat
```

### 4. 서버 접속

- **API 서버**: http://localhost:8000
- **API 문서**: http://localhost:8000/docs
- **대안 문서**: http://localhost:8000/redoc

## 🔧 비동기 기능

### 비동기 데이터베이스 연결

이 애플리케이션은 SQLAlchemy의 비동기 기능을 사용합니다:

- **aiomysql**: 비동기 MySQL 드라이버
- **AsyncSession**: 비동기 데이터베이스 세션
- **동시 요청 처리**: 여러 요청을 동시에 효율적으로 처리

### 성능 최적화

- **연결 풀링**: 데이터베이스 연결 풀 관리
- **비동기 쿼리**: 데이터베이스 쿼리의 비동기 실행
- **동시성**: 여러 요청의 동시 처리

## 📊 API 엔드포인트

### 기본 엔드포인트

- `GET /` - 애플리케이션 정보
- `GET /health` - 데이터베이스 연결 상태 확인

### QR 사용자 관리

- `GET /qr-users/` - QR 사용자 목록 조회
- `GET /qr-users/{qr_no}` - QR 번호로 사용자 조회
- `GET /qr-users/{qr_no}/detail` - QR 사용자 상세 정보
- `GET /qr-users/member/{memb_no}` - 회원별 QR 사용자 조회
- `GET /qr-users/search/` - QR 사용자 검색
- `GET /qr-users/stats` - QR 사용자 통계

## 🧪 테스트

### 비동기 API 테스트

```bash
python test_async_api.py
```

이 스크립트는 다음을 테스트합니다:
- 헬스 체크
- QR 사용자 목록 조회
- QR 사용자 검색
- QR 사용자 통계
- QR 사용자 상세 정보
- 동시 요청 처리

### 데이터베이스 연결 테스트

```bash
python test_db_connection.py
```

## 📝 로그 관리

### 로그 파일 위치

- **로그 디렉토리**: `logs/`
- **메인 로그**: `logs/fastapi_mysql_crud.log`
- **일자별 로테이션**: 자동으로 30일간 보관

### 로그 관리 도구

```bash
python log_manager.py list          # 로그 파일 목록
python log_manager.py show [lines]  # 최신 로그 조회
python log_manager.py clean [days]  # 오래된 로그 정리
python log_manager.py backup        # 로그 파일 백업
```

## 🔍 모니터링

### 헬스 체크

```bash
curl http://localhost:8000/health
```

응답 예시:
```json
{
  "status": "healthy",
  "database": "connected",
  "connection_type": "async",
  "message": "Successfully connected to secretguard on 106.246.249.162:23381",
  "test_query": 1,
  "database_name": "secretguard",
  "mysql_version": "8.0.xx",
  "connection_info": {
    "host": "106.246.249.162",
    "port": 23381,
    "database": "secretguard",
    "user": "muknoori"
  }
}
```

## 🚨 문제 해결

### 일반적인 문제

1. **데이터베이스 연결 실패**
   - 연결 정보 확인
   - 네트워크 연결 상태 확인
   - 방화벽 설정 확인

2. **비동기 드라이버 오류**
   - `aiomysql` 설치 확인
   - MySQL 서버 버전 호환성 확인

3. **로그 파일 권한 오류**
   - `logs/` 디렉토리 생성 권한 확인

### 디버깅

로그 파일을 확인하여 상세한 오류 정보를 확인할 수 있습니다:

```bash
python log_manager.py show 100
```

## 📈 성능 최적화

### 비동기 처리의 장점

- **동시성**: 여러 요청을 동시에 처리
- **리소스 효율성**: 스레드 대신 코루틴 사용
- **확장성**: 더 많은 동시 연결 처리 가능

### 모니터링 지표

- 응답 시간
- 동시 요청 처리량
- 데이터베이스 연결 풀 사용률

## 🤝 기여

버그 리포트나 기능 요청은 이슈로 등록해 주세요.

## 📄 라이선스

이 프로젝트는 MIT 라이선스 하에 배포됩니다. 