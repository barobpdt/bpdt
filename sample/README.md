# FastAPI MySQL CRUD 샘플

FastAPI와 MySQL을 사용한 간단한 CRUD(Create, Read, Update, Delete) 애플리케이션입니다.

## 기능

- ✅ 사용자 생성 (CREATE)
- ✅ 사용자 조회 (READ) - 전체 목록 및 개별 조회
- ✅ 사용자 수정 (UPDATE)
- ✅ 사용자 삭제 (DELETE)
- ✅ 사용자 검색 (이름 기반)
- ✅ 사용자 통계 조회
- ✅ **QR 사용자 정보 조회** (jkj_model.py 활용)
- ✅ **QR 사용자 상세 정보 조회** (회원정보, QR파일정보, 호출이력 포함)
- ✅ **QR 사용자 검색** (닉네임, 차량정보, QR번호, 회원번호 기반)
- ✅ **QR 사용자 통계** (전체, 활성, 마스터, 서브 사용자 수)
- ✅ 자동 API 문서 생성 (Swagger UI)
- ✅ CORS 지원
- ✅ MySQL 데이터베이스 연결
- ✅ 데이터베이스 연결 상태 확인

## 설치 및 실행

### 1. 의존성 설치

```bash
pip install -r requirements.txt
```

### 2. 데이터베이스 연결 테스트

```bash
python test_db_connection.py
```

### 3. 서버 실행

```bash
python fastapi_mysql_crud.py
```

서버가 `http://localhost:8000`에서 실행됩니다.

### 4. API 문서 확인

브라우저에서 다음 URL에 접속하여 API 문서를 확인할 수 있습니다:
- Swagger UI: http://localhost:8000/docs
- ReDoc: http://localhost:8000/redoc

### 5. 테스트 실행

```bash
# 기본 API 테스트
python test_api.py

# QR 사용자 정보 조회 API 테스트
python test_qr_api.py
```

## 데이터베이스 설정

현재 설정된 MySQL 데이터베이스 정보:
- **호스트**: 106.246.249.162:23381
- **데이터베이스명**: secretguard
- **사용자**: secretguard
- **비밀번호**: snflWkd22!

## API 엔드포인트

### 기본 정보
- `GET /` - API 기본 정보 및 엔드포인트 목록
- `GET /health` - 데이터베이스 연결 상태 확인

### 사용자 관리
- `POST /users/` - 새 사용자 생성
- `GET /users/` - 모든 사용자 조회 (페이지네이션 지원)
- `GET /users/{user_id}` - 특정 사용자 조회
- `PUT /users/{user_id}` - 사용자 정보 수정
- `DELETE /users/{user_id}` - 사용자 삭제

### 추가 기능
- `GET /users/search/{name}` - 이름으로 사용자 검색
- `GET /stats/users` - 사용자 통계 조회

### QR 사용자 관리
- `GET /qr-users/` - QR 사용자 목록 조회 (페이지네이션 지원)
- `GET /qr-users/{qr_no}` - QR 번호로 QR 사용자 정보 조회
- `GET /qr-users/{qr_no}/detail` - QR 사용자 상세 정보 조회 (회원정보, QR파일정보, 호출이력 포함)
- `GET /qr-users/member/{memb_no}` - 회원번호로 QR 사용자 정보 조회
- `GET /qr-users/search/` - QR 사용자 검색 (닉네임, 차량정보, QR번호, 회원번호 기반)
- `GET /qr-users/stats` - QR 사용자 통계 조회

## 데이터 모델

### User 모델
```python
{
    "id": int,           # 자동 생성되는 고유 ID (AUTO_INCREMENT)
    "name": str,         # 사용자 이름 (필수)
    "email": str,        # 이메일 (필수, 고유)
    "age": int,          # 나이 (선택)
    "is_active": bool,   # 활성 상태 (기본값: True)
    "created_at": datetime,  # 생성 시간 (자동)
    "updated_at": datetime   # 수정 시간 (자동)
}
```

### QR 사용자 모델
```python
{
    "memb_no": str,              # 회원번호 (필수)
    "qr_no": str,                # QR번호 (필수)
    "qr_owner_cd": str,          # QR소유자구분 (M:MASTER, S:SUB)
    "qr_use_cd": str,            # QR상태 (사용,대기,미사용)
    "nick_nm": str,              # 명칭
    "car_info": str,             # 차량번호
    "maker_car": str,            # 차종
    "park_text": str,            # 주차문구 (기본값: '잠시')
    "pin_no": str,               # PIN번호
    "introduction_info": str,    # 소개글
    "service_type_cd": str,      # 서비스타입
    "add_dt": datetime,          # 등록일
    "chg_dt": datetime           # 수정일
}
```

## 사용 예시

### 1. 데이터베이스 연결 상태 확인
```bash
curl -X GET "http://localhost:8000/health"
```

### 2. QR 사용자 목록 조회
```bash
curl -X GET "http://localhost:8000/qr-users/"
```

### 3. QR 사용자 상세 정보 조회
```bash
curl -X GET "http://localhost:8000/qr-users/QR001/detail"
```

### 4. QR 사용자 검색
```bash
# 닉네임으로 검색
curl -X GET "http://localhost:8000/qr-users/search/?search_term=김&search_type=nick_nm"

# 차량정보로 검색
curl -X GET "http://localhost:8000/qr-users/search/?search_term=12가&search_type=car_info"

# QR번호로 검색
curl -X GET "http://localhost:8000/qr-users/search/?search_term=QR&search_type=qr_no"
```

### 5. 회원번호로 QR 사용자 조회
```bash
curl -X GET "http://localhost:8000/qr-users/member/MEMB001"
```

### 6. QR 사용자 통계 조회
```bash
curl -X GET "http://localhost:8000/qr-users/stats"
```

### 7. 기존 사용자 관리 (기존 기능)
```bash
# 사용자 생성
curl -X POST "http://localhost:8000/users/" \
     -H "Content-Type: application/json" \
     -d '{
       "name": "김철수",
       "email": "kim@example.com",
       "age": 25,
       "is_active": true
     }'

# 사용자 목록 조회
curl -X GET "http://localhost:8000/users/"
```

## 프로젝트 구조

```
├── fastapi_mysql_crud.py  # 메인 애플리케이션 (MySQL + QR 사용자 관리)
├── jkj_model.py          # 데이터베이스 모델 정의 (SQLAlchemy)
├── test_db_connection.py   # 데이터베이스 연결 테스트
├── test_api.py            # 기본 API 테스트 스크립트
├── test_qr_api.py         # QR 사용자 API 테스트 스크립트 (신규)
├── web_interface.html     # 웹 인터페이스
├── requirements.txt       # 의존성 목록
├── README.md             # 프로젝트 문서
└── run.bat              # Windows 실행 스크립트
```

## 주요 특징

1. **MySQL 데이터베이스**: 원격 MySQL 서버 연결
2. **SQLAlchemy ORM**: 데이터베이스 작업을 위한 ORM 사용
3. **PyMySQL**: MySQL 연결 드라이버
4. **Pydantic 모델**: 데이터 검증 및 직렬화
5. **의존성 주입**: FastAPI의 Depends를 사용한 데이터베이스 세션 관리
6. **자동 문서화**: Swagger UI와 ReDoc을 통한 자동 API 문서 생성
7. **에러 처리**: 적절한 HTTP 상태 코드와 에러 메시지
8. **CORS 지원**: 웹 애플리케이션에서의 API 호출 지원
9. **연결 풀링**: 효율적인 데이터베이스 연결 관리
10. **SSL 지원**: 보안 연결 설정
11. **QR 사용자 관리**: jkj_model.py 기반의 QR 사용자 정보 조회 기능
12. **상세 정보 조회**: QR 사용자의 회원정보, QR파일정보, 호출이력을 포함한 상세 조회
13. **다양한 검색**: 닉네임, 차량정보, QR번호, 회원번호 기반 검색
14. **통계 기능**: QR 사용자 현황 통계 제공

## QR 사용자 관리 기능 상세

### QR 사용자 목록 조회
- 페이지네이션 지원 (skip, limit 파라미터)
- QR 사용자의 기본 정보 조회

### QR 사용자 상세 정보 조회
- QR 사용자 기본 정보
- 연관된 회원 정보 (이름, 이메일, 전화번호, 가입일 등)
- QR 파일 정보 (QR 이미지 URL, 서비스 타입 등)
- 호출 이력 (최근 10건의 호출 기록)

### QR 사용자 검색
- **닉네임 검색**: 사용자 닉네임으로 검색
- **차량정보 검색**: 차량번호로 검색
- **QR번호 검색**: QR 번호로 검색
- **회원번호 검색**: 회원번호로 검색

### QR 사용자 통계
- 전체 QR 사용자 수
- 활성 QR 사용자 수
- 마스터 QR 사용자 수 (QR_OWNER_CD = 'M')
- 서브 QR 사용자 수 (QR_OWNER_CD = 'S')

## 개발 환경

- Python 3.7+
- FastAPI 0.104.1
- SQLAlchemy 2.0.23
- PyMySQL 1.1.0
- MySQL 5.7+
- Pydantic 2.5.0

## 문제 해결

### 데이터베이스 연결 실패 시

1. **네트워크 연결 확인**:
   ```bash
   telnet 106.246.249.162 23381
   ```

2. **데이터베이스 연결 테스트**:
   ```bash
   python test_db_connection.py
   ```

3. **방화벽 설정 확인**: 포트 23381이 열려있는지 확인

4. **SSL 설정 문제**: 필요시 SSL 설정을 조정

### QR 사용자 데이터가 없는 경우

1. **데이터베이스 확인**: `sg_memb_qr` 테이블에 데이터가 있는지 확인
2. **테이블 구조 확인**: jkj_model.py의 모델과 실제 테이블 구조가 일치하는지 확인
3. **권한 확인**: 데이터베이스 사용자가 해당 테이블에 대한 조회 권한이 있는지 확인

### 일반적인 오류

- **Connection refused**: 서버가 실행되지 않았거나 포트가 잘못됨
- **Access denied**: 사용자명/비밀번호 오류
- **Database not found**: 데이터베이스명 오류
- **SSL connection error**: SSL 설정 문제
- **Table not found**: 테이블이 존재하지 않음
- **Column not found**: 컬럼명이 모델과 다름

## 라이선스

MIT License 