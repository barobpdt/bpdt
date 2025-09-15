from fastapi import FastAPI, HTTPException, Depends, status
from fastapi.middleware.cors import CORSMiddleware
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from sqlalchemy import create_engine, Column, Integer, String, Text, Boolean, DateTime, Numeric
from sqlalchemy.ext.asyncio import create_async_engine, AsyncSession, async_sessionmaker
from sqlalchemy.orm import sessionmaker, Session, declarative_base
from sqlalchemy.sql import text
from sqlalchemy.future import select
from pydantic import BaseModel, Field
from typing import List, Optional
from datetime import datetime, timedelta
import os
import sys
import io
import logging
from logging.handlers import TimedRotatingFileHandler
import asyncio
import signal
from jose import JWTError, jwt
from passlib.context import CryptContext


# JWT 설정
SECRET_KEY = "your-secret-key-here-change-in-production"  # 실제 운영환경에서는 환경변수로 관리
ALGORITHM = "HS256"
ACCESS_TOKEN_EXPIRE_MINUTES = 30

# 비밀번호 해싱
pwd_context = CryptContext(schemes=["bcrypt"], deprecated="auto")

# JWT Bearer 토큰
security = HTTPBearer()

sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'utf-8')
sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'utf-8')

# jkj_model.py에서 모델들 import (실제 사용하는 모델들만)
localPath = os.path.dirname(os.path.abspath(os.path.dirname(__file__)))
sys.path.append(f'{localPath}/sample')

try:
    # 현재 디렉토리에서 jkj_model.py 찾기
    current_dir = os.path.dirname(os.path.abspath(__file__))
    if current_dir not in sys.path:
        sys.path.insert(0, current_dir)
    
    from jkj_model import (
        SgMemb, SgMembQr, SgQrInfo, SgMembCall
    )
    print("@@ jkj_model.py 모델 import 성공")
except ImportError as e:
    print(f"@@ jkj_model.py 모델 import 실패: {e}")
    print("QR 사용자 기능이 비활성화됩니다.")
    # 더미 클래스 생성
    class SgMemb:
        pass
    class SgMembQr:
        pass
    class SgQrInfo:
        pass
    class SgMembCall:
        pass

# 전역 변수로 서버 상태 관리
server_running = True
# JWT 토큰 관련 클래스들
class Token(BaseModel):
    access_token: str
    token_type: str

class TokenData(BaseModel):
    username: Optional[str] = None

class User(BaseModel):
    username: str
    email: Optional[str] = None
    full_name: Optional[str] = None
    disabled: Optional[bool] = None

class UserInDB(User):
    hashed_password: str

# 관리자 사용자 정보 (실제 운영환경에서는 데이터베이스에서 관리)
ADMIN_USERS = {
    "admin": {
        "username": "admin",
        "full_name": "Administrator",
        "email": "admin@example.com",
        "hashed_password": pwd_context.hash("admin123"),  # 실제 운영환경에서는 강력한 비밀번호 사용
        "disabled": False
    }
}

# JWT 토큰 관련 함수들
def verify_password(plain_password, hashed_password):
    """비밀번호 검증"""
    return pwd_context.verify(plain_password, hashed_password)

def get_password_hash(password):
    """비밀번호 해싱"""
    return pwd_context.hash(password)

def get_user(username: str):
    """사용자 정보 조회"""
    if username in ADMIN_USERS:
        user_dict = ADMIN_USERS[username]
        return UserInDB(**user_dict)
    return None

def authenticate_user(username: str, password: str):
    """사용자 인증"""
    user = get_user(username)
    if not user:
        return False
    if not verify_password(password, user.hashed_password):
        return False
    return user

def create_access_token(data: dict, expires_delta: Optional[timedelta] = None):
    """JWT 액세스 토큰 생성"""
    to_encode = data.copy()
    if expires_delta:
        expire = datetime.utcnow() + expires_delta
    else:
        expire = datetime.utcnow() + timedelta(minutes=15)
    to_encode.update({"exp": expire})
    encoded_jwt = jwt.encode(to_encode, SECRET_KEY, algorithm=ALGORITHM)
    return encoded_jwt

async def get_current_user(credentials: HTTPAuthorizationCredentials = Depends(security)):
    """현재 인증된 사용자 조회"""
    credentials_exception = HTTPException(
        status_code=status.HTTP_401_UNAUTHORIZED,
        detail="Could not validate credentials",
        headers={"WWW-Authenticate": "Bearer"},
    )
    try:
        payload = jwt.decode(credentials.credentials, SECRET_KEY, algorithms=[ALGORITHM])
        username: str = payload.get("sub")
        if username is None:
            raise credentials_exception
        token_data = TokenData(username=username)
    except JWTError:
        raise credentials_exception
    
    user = get_user(username=token_data.username)
    if user is None:
        raise credentials_exception
    return user

async def get_current_active_user(current_user: User = Depends(get_current_user)):
    """현재 활성 사용자 조회"""
    if current_user.disabled:
        raise HTTPException(status_code=400, detail="Inactive user")
    return current_user

# 시그널 핸들러
def signal_handler(signum, frame):
    """시그널 핸들러"""
    global server_running
    logger.info(f"🛑 종료 시그널 수신: {signum}")
    server_running = False

# 시그널 등록
signal.signal(signal.SIGINT, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)

# FastAPI 앱 생성
app = FastAPI(
    title="FastAPI MySQL CRUD Sample",
    description="A simple CRUD application using FastAPI and MySQL with QR User Management",
    version="1.0.0"
)

# CORS 미들웨어 추가
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# MySQL 데이터베이스 설정
DB_HOST = "106.246.249.162:23381"
DB_NAME = "secretguard"
DB_USER = "muknoori"
DB_PASSWORD = "snflWkd22!"

# 호스트와 포트 분리
if ':' in DB_HOST:
    host, port = DB_HOST.split(':')
    port = int(port)
else:
    host = DB_HOST
    port = 3306

# 비동기 데이터베이스 URL
ASYNC_DATABASE_URLS = [
    f"mysql+aiomysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4",
    f"mysql+aiomysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4&ssl_mode=DISABLED",
    f"mysql+aiomysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4&ssl_mode=REQUIRED"
]

# 동기 데이터베이스 URL (기존 호환성용)
DATABASE_URLS = [
    f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4",
    f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4&ssl_mode=DISABLED",
    f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4&ssl_mode=REQUIRED"
]

# 첫 번째 연결 문자열 사용
ASYNC_DATABASE_URL = ASYNC_DATABASE_URLS[0]
DATABASE_URL = DATABASE_URLS[0]

# 로거 초기화 (데이터베이스 연결 전에 먼저 초기화)
def setup_logging():
    """일자별 로그 설정"""
    # 로그 디렉토리 생성
    log_dir = "logs"
    if not os.path.exists(log_dir):
        os.makedirs(log_dir)
    
    # 로거 설정
    logger = logging.getLogger("fastapi_mysql_crud")
    logger.setLevel(logging.INFO)
    
    # 콘솔 핸들러
    console_handler = logging.StreamHandler()
    console_handler.setLevel(logging.INFO)
    console_formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    console_handler.setFormatter(console_formatter)
    
    # 파일 핸들러 (일자별 로테이션)
    file_handler = TimedRotatingFileHandler(
        filename=os.path.join(log_dir, "fastapi_mysql_crud.log"),
        when="midnight",
        interval=1,
        backupCount=30,  # 30일간 보관
        encoding='utf-8'
    )
    file_handler.setLevel(logging.INFO)
    file_formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(funcName)s:%(lineno)d - %(message)s'
    )
    file_handler.setFormatter(file_formatter)
    
    # 핸들러 추가
    logger.addHandler(console_handler)
    logger.addHandler(file_handler)
    
    return logger

# 로거 초기화 (가장 먼저 실행)
logger = setup_logging()

# 비동기 데이터베이스 엔진 생성
def create_async_database_engine():
    """비동기 데이터베이스 엔진을 생성합니다."""
    for i, url in enumerate(ASYNC_DATABASE_URLS, 1):
        try:
            logger.info(f"🔍 비동기 데이터베이스 연결 시도 {i}: {url}")
            
            engine = create_async_engine(
                url,
                pool_pre_ping=True,
                pool_recycle=300,
                pool_timeout=30,
                max_overflow=10,
                pool_size=5,
                echo=False,  # SQL 로그 비활성화 (성능 향상)
                connect_args={
                    "connect_timeout": 30,
                    "read_timeout": 30,
                    "write_timeout": 30
                }
            )
            
            logger.info(f"✅ 비동기 데이터베이스 연결 성공! (연결 {i})")
            return engine
            
        except Exception as e:
            logger.error(f"❌ 비동기 연결 {i} 실패: {e}")
            if i == len(ASYNC_DATABASE_URLS):
                logger.error("❌ 모든 비동기 연결 시도 실패")
                raise e
            continue

# 동기 데이터베이스 엔진 생성 (기존 호환성용)
def create_database_engine():
    """동기 데이터베이스 엔진을 생성합니다."""
    for i, url in enumerate(DATABASE_URLS, 1):
        try:
            logger.info(f"🔍 동기 데이터베이스 연결 시도 {i}: {url}")
            
            engine = create_engine(
                url,
                pool_pre_ping=True,
                pool_recycle=300,
                pool_timeout=30,
                max_overflow=10,
                pool_size=5,
                connect_args={
                    "connect_timeout": 30,
                    "read_timeout": 30,
                    "write_timeout": 30
                }
            )
            
            # 연결 테스트
            with engine.connect() as conn:
                conn.execute(text("SELECT 1"))
            
            logger.info(f"✅ 동기 데이터베이스 연결 성공! (연결 {i})")
            return engine
            
        except Exception as e:
            logger.error(f"❌ 동기 연결 {i} 실패: {e}")
            if i == len(DATABASE_URLS):
                logger.error("❌ 모든 동기 연결 시도 실패")
                raise e
            continue

# 데이터베이스 엔진 초기화 (개선된 버전)
def initialize_database_engines():
    """데이터베이스 엔진들을 초기화합니다."""
    global async_engine, engine, AsyncSessionLocal, SessionLocal
    
    # 비동기 엔진 생성 시도
    async_engine = None
    try:
        async_engine = create_async_database_engine()
        AsyncSessionLocal = async_sessionmaker(
            async_engine, 
            class_=AsyncSession, 
            expire_on_commit=False
        )
        logger.info("✅ 비동기 데이터베이스 엔진 생성 완료")
    except Exception as e:
        logger.error(f"❌ 비동기 데이터베이스 엔진 생성 실패: {e}")
        logger.info("⚠️ 동기 데이터베이스 엔진으로 대체합니다.")
    
    # 동기 엔진 생성 (비동기 실패 시 또는 호환성을 위해)
    try:
        engine = create_database_engine()
        SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
        logger.info("✅ 동기 데이터베이스 엔진 생성 완료")
    except Exception as e:
        logger.error(f"❌ 동기 데이터베이스 엔진 생성 실패: {e}")
        # 최후의 수단: 기본 설정으로 엔진 생성
        try:
            engine = create_engine(DATABASE_URL, pool_pre_ping=True, pool_recycle=300)
            SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
            logger.info("✅ 기본 설정으로 동기 데이터베이스 엔진 생성 완료")
        except Exception as e2:
            logger.error(f"❌ 모든 데이터베이스 연결 시도 실패: {e2}")
            raise e2

# 데이터베이스 엔진 초기화 실행
initialize_database_engines()

Base = declarative_base()

# QR 사용자 정보 Pydantic 모델
class QrUserResponse(BaseModel):
    memb_no: str = Field(alias="MEMB_NO")
    qr_no: str = Field(alias="QR_NO")
    qr_owner_cd: Optional[str] = Field(default=None, alias="QR_OWNER_CD")
    qr_use_cd: Optional[str] = Field(default=None, alias="QR_USE_CD")
    nick_nm: Optional[str] = Field(default=None, alias="NICK_NM")
    car_info: Optional[str] = Field(default=None, alias="CAR_INFO")
    maker_car: Optional[str] = Field(default=None, alias="MAKER_CAR")
    park_text: Optional[str] = Field(default=None, alias="PARK_TEXT")
    pin_no: Optional[str] = Field(default=None, alias="PIN_NO")
    introduction_info: Optional[str] = Field(default=None, alias="INTRODUCTION_INFO")
    service_type_cd: Optional[str] = Field(default=None, alias="SERVICE_TYPE_CD")
    add_dt: Optional[datetime] = Field(default=None, alias="ADD_DT")
    chg_dt: Optional[datetime] = Field(default=None, alias="CHG_DT")
    
    class Config:
        from_attributes = True
        populate_by_name = True

class QrUserDetailResponse(BaseModel):
    qr_info: QrUserResponse
    member_info: Optional[dict] = None
    qr_file_info: Optional[dict] = None
    call_history: Optional[List[dict]] = None
    
    class Config:
        from_attributes = True

# 로그인 관련 Pydantic 모델
class UserLogin(BaseModel):
    username: str
    password: str

# 데이터베이스 테이블 생성
async def create_tables_async():
    try:
        async with async_engine.begin() as conn:
            await conn.run_sync(Base.metadata.create_all)
        logger.info("✅ 비동기 데이터베이스 테이블이 성공적으로 생성되었습니다.")
    except Exception as e:
        logger.warning(f"⚠️ 비동기 테이블 생성 중 오류 발생: {e}")
        logger.info("⚠️ 기존 테이블을 사용합니다.")

def create_tables():
    try:
        Base.metadata.create_all(bind=engine)
        logger.info("✅ 동기 데이터베이스 테이블이 성공적으로 생성되었습니다.")
    except Exception as e:
        logger.warning(f"⚠️ 동기 테이블 생성 중 오류 발생: {e}")
        logger.info("⚠️ 기존 테이블을 사용합니다.")

# 애플리케이션 시작 시 테이블 생성 (오류 처리 개선)
async def init_database():
    """데이터베이스 초기화"""
    try:
        await create_tables_async()
    except Exception as e:
        logger.warning(f"⚠️ 비동기 테이블 생성 실패: {e}")
        try:
            create_tables()
        except Exception as e2:
            logger.warning(f"⚠️ 동기 테이블 생성도 실패: {e2}")
        logger.info("⚠️ 애플리케이션은 계속 실행되지만 일부 기능이 제한될 수 있습니다.")

# 애플리케이션 시작 이벤트
@app.on_event("startup")
async def startup_event():
    """애플리케이션 시작 시 실행"""
    logger.info("🚀 FastAPI MySQL CRUD 서버 시작")
    logger.info(f"📊 데이터베이스: {DB_NAME} on {DB_HOST}")
    logger.info(f"🌐 서버 주소: http://localhost:8000")
    logger.info(f"📚 API 문서: http://localhost:8000/docs")
    logger.info(f"📋 로그 파일: logs/fastapi_mysql_crud.log")
    
    # 데이터베이스 초기화
    await init_database()

# 애플리케이션 종료 이벤트
@app.on_event("shutdown")
async def shutdown_event():
    """애플리케이션 종료 시 실행"""
    logger.info("👋 FastAPI MySQL CRUD 서버 종료")
    
    # 비동기 엔진 종료
    if 'async_engine' in globals():
        await async_engine.dispose()
        logger.info("✅ 비동기 데이터베이스 연결 종료")
    
    # 동기 엔진 종료
    if 'engine' in globals():
        engine.dispose()
        logger.info("✅ 동기 데이터베이스 연결 종료")

# 데이터베이스 세션 의존성
def get_db():
    db = SessionLocal()
    try:
        yield db
    except Exception as e:
        logger.error(f"⚠️ 데이터베이스 세션 오류: {e}")
        db.rollback()
        raise HTTPException(status_code=500, detail=f"Database error: {str(e)}")
    finally:
        db.close()

# 비동기 데이터베이스 세션 의존성
async def get_async_db():
    async with AsyncSessionLocal() as db:
        try:
            yield db
        except Exception as e:
            logger.error(f"⚠️ 비동기 데이터베이스 세션 오류: {e}")
            await db.rollback()
            raise HTTPException(status_code=500, detail=f"Database error: {str(e)}")

# QR 사용자 정보 조회 함수들
async def get_qr_users(db: AsyncSession, skip: int = 0, limit: int = 100):
    """QR 사용자 목록을 조회합니다."""
    try:
        logger.info(f"QR 사용자 목록 조회: skip={skip}, limit={limit}")
        result = await db.execute(
            select(SgMembQr).offset(skip).limit(limit)
        )
        qr_users = result.scalars().all()
        logger.info(f"QR 사용자 목록 조회 완료: {len(qr_users)}건")
        return qr_users
    except Exception as e:
        logger.error(f"⚠️ QR 사용자 목록 조회 오류: {e}")
        return []

async def get_qr_user_by_qr_no(db: AsyncSession, qr_no: str):
    """QR 번호로 QR 사용자 정보를 조회합니다."""
    try:
        logger.info(f"QR 사용자 조회: qr_no={qr_no}")
        result = await db.execute(
            select(SgMembQr).filter(SgMembQr.QR_NO == qr_no)
        )
        qr_user = result.scalar_one_or_none()
        if qr_user:
            logger.info(f"QR 사용자 조회 완료: {qr_no}")
        else:
            logger.warning(f"QR 사용자를 찾을 수 없음: {qr_no}")
        return qr_user
    except Exception as e:
        logger.error(f"⚠️ QR 사용자 조회 오류: {e}")
        return None

async def get_qr_user_by_memb_no(db: AsyncSession, memb_no: str):
    """회원번호로 QR 사용자 정보를 조회합니다."""
    try:
        logger.info(f"회원별 QR 사용자 조회: memb_no={memb_no}")
        result = await db.execute(
            select(SgMembQr).filter(SgMembQr.MEMB_NO == memb_no)
        )
        qr_users = result.scalars().all()
        logger.info(f"회원별 QR 사용자 조회 완료: {len(qr_users)}건")
        return qr_users
    except Exception as e:
        logger.error(f"⚠️ 회원별 QR 사용자 조회 오류: {e}")
        return []

async def get_qr_user_detail(db: AsyncSession, qr_no: str):
    """QR 사용자 상세 정보를 조회합니다."""
    try:
        logger.info(f"QR 사용자 상세 정보 조회: qr_no={qr_no}")
        
        # QR 사용자 기본 정보
        result = await db.execute(
            select(SgMembQr).filter(SgMembQr.QR_NO == qr_no)
        )
        qr_user = result.scalar_one_or_none()
        if not qr_user:
            logger.warning(f"QR 사용자를 찾을 수 없음: {qr_no}")
            return None
        
        # 회원 정보
        member_info = None
        if qr_user.MEMB_NO:
            member_result = await db.execute(
                select(SgMemb).filter(SgMemb.MEMB_NO == qr_user.MEMB_NO)
            )
            member = member_result.scalar_one_or_none()
            if member:
                member_info = {
                    "memb_no": member.MEMB_NO,
                    "f_memb_nm": member.F_MEMB_NM,
                    "l_memb_nm": member.L_MEMB_NM,
                    "email": member.EMAIL,
                    "phone_no": member.PHONE_NO,
                    "nick_nm": member.NICK_NM,
                    "memb_stat_cd": member.MEMB_STAT_CD,
                    "memb_join_dt": member.MEMB_JOIN_DT,
                    "last_day": member.LAST_DAY
                }
                logger.info(f"회원 정보 조회 완료: {qr_user.MEMB_NO}")
        
        # QR 파일 정보
        qr_file_info = None
        qr_file_result = await db.execute(
            select(SgQrInfo).filter(SgQrInfo.QR_NO == qr_no)
        )
        qr_file = qr_file_result.scalar_one_or_none()
        if qr_file:
            qr_file_info = {
                "qr_no": qr_file.QR_NO,
                "qr_file_no": qr_file.QR_FILE_NO,
                "qr_info": qr_file.QR_INFO,
                "low_qr_info": qr_file.LOW_QR_INFO,
                "use_yn": qr_file.USE_YN,
                "service_type_cd": qr_file.SERVICE_TYPE_CD
            }
            logger.info(f"QR 파일 정보 조회 완료: {qr_no}")
        
        # 호출 이력 (최근 10건)
        call_history = []
        call_result = await db.execute(
            select(SgMembCall)
            .filter(SgMembCall.QR_NO == qr_no)
            .order_by(SgMembCall.ADD_DT.desc())
            .limit(10)
        )
        calls = call_result.scalars().all()
        for call in calls:
            call_history.append({
                "call_seq": call.CALL_SEQ,
                "call_type_cd": call.CALL_TYPE_CD,
                "call_req_msg": call.CALL_REQ_MSG,
                "call_reply_msg": call.CALL_REPLY_MSG,
                "call_stat_cd": call.CALL_STAT_CD,
                "call_stat_dt": call.CALL_STAT_DT,
                "add_dt": call.ADD_DT
            })
        logger.info(f"호출 이력 조회 완료: {len(call_history)}건")
        
        logger.info(f"QR 사용자 상세 정보 조회 완료: {qr_no}")
        return {
            "qr_info": qr_user,
            "member_info": member_info,
            "qr_file_info": qr_file_info,
            "call_history": call_history
        }
    except Exception as e:
        logger.error(f"⚠️ QR 사용자 상세 정보 조회 오류: {e}")
        return None

async def search_qr_users(db: AsyncSession, search_term: str, search_type: str = "nick_nm"):
    """QR 사용자를 검색합니다."""
    try:
        logger.info(f"QR 사용자 검색: term='{search_term}', type='{search_type}'")
        query = select(SgMembQr)
        
        if search_type == "nick_nm":
            query = query.filter(SgMembQr.NICK_NM.contains(search_term))
        elif search_type == "car_info":
            query = query.filter(SgMembQr.CAR_INFO.contains(search_term))
        elif search_type == "qr_no":
            query = query.filter(SgMembQr.QR_NO.contains(search_term))
        elif search_type == "memb_no":
            query = query.filter(SgMembQr.MEMB_NO.contains(search_term))
        
        result = await db.execute(query)
        qr_users = result.scalars().all()
        logger.info(f"QR 사용자 검색 완료: {len(qr_users)}건")
        return qr_users
    except Exception as e:
        logger.error(f"⚠️ QR 사용자 검색 오류: {e}")
        return []

async def get_qr_user_stats(db: AsyncSession):
    """QR 사용자 통계를 조회합니다."""
    try:
        logger.info("QR 사용자 통계 조회 시작")
        
        # 전체 QR 사용자 수
        total_result = await db.execute(select(SgMembQr))
        total_qr_users = len(total_result.scalars().all())
        
        # 활성 QR 사용자 수
        active_result = await db.execute(
            select(SgMembQr).filter(SgMembQr.QR_USE_CD == "사용")
        )
        active_qr_users = len(active_result.scalars().all())
        
        # 마스터 QR 사용자 수
        master_result = await db.execute(
            select(SgMembQr).filter(SgMembQr.QR_OWNER_CD == "M")
        )
        master_qr_users = len(master_result.scalars().all())
        
        # 서브 QR 사용자 수
        sub_result = await db.execute(
            select(SgMembQr).filter(SgMembQr.QR_OWNER_CD == "S")
        )
        sub_qr_users = len(sub_result.scalars().all())
        
        stats = {
            "total_qr_users": total_qr_users,
            "active_qr_users": active_qr_users,
            "master_qr_users": master_qr_users,
            "sub_qr_users": sub_qr_users
        }
        logger.info(f"QR 사용자 통계 조회 완료: {stats}")
        return stats
    except Exception as e:
        logger.error(f"⚠️ QR 사용자 통계 조회 오류: {e}")
        return {
            "total_qr_users": 0,
            "active_qr_users": 0,
            "master_qr_users": 0,
            "sub_qr_users": 0
        }

# API 엔드포인트들
@app.get("/")
def read_root():
    logger.info("루트 엔드포인트 호출")
    return {
        "message": "FastAPI MySQL CRUD Sample with QR User Management",
        "database": f"Connected to {DB_NAME} on {DB_HOST}",
        "docs": "/docs",
        "endpoints": {
            "login": "/login",
            "qr_users": "/qr-users",
            "qr_user_detail": "/qr-users/{qr_no}/detail",
            "qr_user_search": "/qr-users/search",
            "qr_user_stats": "/qr-users/stats",
            "shutdown": "/shutdown (requires authentication)"
        }
    }

@app.post("/login", response_model=Token)
async def login_for_access_token(user_credentials: UserLogin):
    """사용자 로그인 및 JWT 토큰 발급"""
    logger.info(f"로그인 시도: username={user_credentials.username}")
    try:
        user = authenticate_user(user_credentials.username, user_credentials.password)
        if not user:
            logger.warning(f"로그인 실패: 잘못된 사용자명 또는 비밀번호 - {user_credentials.username}")
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="Incorrect username or password",
                headers={"WWW-Authenticate": "Bearer"},
            )
        
        access_token_expires = timedelta(minutes=ACCESS_TOKEN_EXPIRE_MINUTES)
        access_token = create_access_token(
            data={"sub": user.username}, expires_delta=access_token_expires
        )
        
        logger.info(f"로그인 성공: username={user_credentials.username}")
        return {"access_token": access_token, "token_type": "bearer"}
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"로그인 처리 중 오류: {e}")
        raise HTTPException(status_code=500, detail="Login processing error")

# QR 사용자 관련 엔드포인트들
@app.get("/qr-users/", response_model=List[QrUserResponse])
async def read_qr_users(skip: int = 0, limit: int = 100, db: AsyncSession = Depends(get_async_db)):
    """QR 사용자 목록을 조회합니다."""
    logger.info(f"QR 사용자 목록 API 호출: skip={skip}, limit={limit}")
    try:
        qr_users = await get_qr_users(db, skip=skip, limit=limit)
        # Pydantic 모델로 변환
        result = []
        for qr_user in qr_users:
            try:
                qr_response = QrUserResponse.model_validate(qr_user)
                result.append(qr_response)
            except Exception as e:
                logger.warning(f"⚠️ QR 사용자 변환 오류: {e}")
                # 기본값으로 변환
                result.append(QrUserResponse(
                    memb_no=qr_user.MEMB_NO,
                    qr_no=qr_user.QR_NO,
                    qr_owner_cd=getattr(qr_user, 'QR_OWNER_CD', None),
                    qr_use_cd=getattr(qr_user, 'QR_USE_CD', None),
                    nick_nm=getattr(qr_user, 'NICK_NM', None),
                    car_info=getattr(qr_user, 'CAR_INFO', None),
                    maker_car=getattr(qr_user, 'MAKER_CAR', None),
                    park_text=getattr(qr_user, 'PARK_TEXT', None),
                    pin_no=getattr(qr_user, 'PIN_NO', None),
                    introduction_info=getattr(qr_user, 'INTRODUCTION_INFO', None),
                    service_type_cd=getattr(qr_user, 'SERVICE_TYPE_CD', None),
                    add_dt=getattr(qr_user, 'ADD_DT', None),
                    chg_dt=getattr(qr_user, 'CHG_DT', None)
                ))
        logger.info(f"QR 사용자 목록 API 응답 완료: {len(result)}건")
        return result
    except Exception as e:
        logger.error(f"⚠️ QR 사용자 목록 조회 오류: {e}")
        raise HTTPException(status_code=500, detail=f"QR users fetch error: {str(e)}")

@app.get("/qr-users/{qr_no}", response_model=QrUserResponse)
async def read_qr_user(qr_no: str, db: AsyncSession = Depends(get_async_db)):
    """QR 번호로 QR 사용자 정보를 조회합니다."""
    logger.info(f"QR 사용자 조회 API 호출: qr_no={qr_no}")
    try:
        qr_user = await get_qr_user_by_qr_no(db, qr_no=qr_no)
        if qr_user is None:
            logger.warning(f"QR 사용자를 찾을 수 없음: {qr_no}")
            raise HTTPException(status_code=404, detail="QR User not found")
        
        # Pydantic 모델로 변환
        try:
            result = QrUserResponse.model_validate(qr_user)
            logger.info(f"QR 사용자 조회 API 응답 완료: {qr_no}")
            return result
        except Exception as e:
            logger.warning(f"⚠️ QR 사용자 변환 오류: {e}")
            # 기본값으로 변환
            result = QrUserResponse(
                memb_no=qr_user.MEMB_NO,
                qr_no=qr_user.QR_NO,
                qr_owner_cd=getattr(qr_user, 'QR_OWNER_CD', None),
                qr_use_cd=getattr(qr_user, 'QR_USE_CD', None),
                nick_nm=getattr(qr_user, 'NICK_NM', None),
                car_info=getattr(qr_user, 'CAR_INFO', None),
                maker_car=getattr(qr_user, 'MAKER_CAR', None),
                park_text=getattr(qr_user, 'PARK_TEXT', None),
                pin_no=getattr(qr_user, 'PIN_NO', None),
                introduction_info=getattr(qr_user, 'INTRODUCTION_INFO', None),
                service_type_cd=getattr(qr_user, 'SERVICE_TYPE_CD', None),
                add_dt=getattr(qr_user, 'ADD_DT', None),
                chg_dt=getattr(qr_user, 'CHG_DT', None)
            )
            logger.info(f"QR 사용자 조회 API 응답 완료 (수동 변환): {qr_no}")
            return result
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"⚠️ QR 사용자 조회 오류: {e}")
        raise HTTPException(status_code=500, detail=f"QR user fetch error: {str(e)}")

@app.get("/qr-users/{qr_no}/detail")
async def read_qr_user_detail(qr_no: str, db: AsyncSession = Depends(get_async_db)):
    """QR 사용자 상세 정보를 조회합니다."""
    logger.info(f"QR 사용자 상세 정보 API 호출: qr_no={qr_no}")
    try:
        qr_user_detail = await get_qr_user_detail(db, qr_no=qr_no)
        if qr_user_detail is None:
            logger.warning(f"QR 사용자 상세 정보를 찾을 수 없음: {qr_no}")
            raise HTTPException(status_code=404, detail="QR User not found")
        logger.info(f"QR 사용자 상세 정보 API 응답 완료: {qr_no}")
        return qr_user_detail
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"⚠️ QR 사용자 상세 정보 조회 오류: {e}")
        raise HTTPException(status_code=500, detail=f"QR user detail fetch error: {str(e)}")

@app.get("/qr-users/member/{memb_no}")
async def read_qr_users_by_member(memb_no: str, db: AsyncSession = Depends(get_async_db)):
    """회원번호로 QR 사용자 정보를 조회합니다."""
    logger.info(f"회원별 QR 사용자 API 호출: memb_no={memb_no}")
    try:
        qr_users = await get_qr_user_by_memb_no(db, memb_no=memb_no)
        logger.info(f"회원별 QR 사용자 API 응답 완료: {len(qr_users)}건")
        return qr_users
    except Exception as e:
        logger.error(f"⚠️ 회원별 QR 사용자 조회 오류: {e}")
        raise HTTPException(status_code=500, detail=f"Member QR users fetch error: {str(e)}")

@app.get("/qr-users/search/")
async def search_qr_users_endpoint(
    search_term: str, 
    search_type: str = "nick_nm", 
    db: AsyncSession = Depends(get_async_db)
):
    """QR 사용자를 검색합니다."""
    logger.info(f"QR 사용자 검색 API 호출: term='{search_term}', type='{search_type}'")
    try:
        if search_type not in ["nick_nm", "car_info", "qr_no", "memb_no"]:
            logger.warning(f"잘못된 검색 타입: {search_type}")
            raise HTTPException(status_code=400, detail="Invalid search type")
        
        qr_users = await search_qr_users(db, search_term=search_term, search_type=search_type)
        logger.info(f"QR 사용자 검색 API 응답 완료: {len(qr_users)}건")
        return qr_users
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"⚠️ QR 사용자 검색 오류: {e}")
        raise HTTPException(status_code=500, detail=f"QR user search error: {str(e)}")

@app.get("/qr-users/stats")
async def get_qr_user_stats_endpoint(db: AsyncSession = Depends(get_async_db)):
    """QR 사용자 통계를 조회합니다."""
    logger.info("QR 사용자 통계 API 호출")
    try:
        stats = await get_qr_user_stats(db)
        logger.info(f"QR 사용자 통계 API 응답 완료: {stats}")
        return stats
    except Exception as e:
        logger.error(f"⚠️ QR 사용자 통계 조회 오류: {e}")
        raise HTTPException(status_code=500, detail=f"QR user stats fetch error: {str(e)}")

@app.get("/health")
async def health_check():
    """데이터베이스 연결 상태를 확인합니다."""
    logger.info("헬스 체크 API 호출")
    try:
        # 비동기 연결 테스트
        async with AsyncSessionLocal() as db:
            result = await db.execute(text("SELECT 1 as test"))
            test_value = result.fetchone()[0]
            
            # 데이터베이스 정보 조회
            db_info = await db.execute(text("SELECT DATABASE() as db_name, VERSION() as version"))
            db_info_result = db_info.fetchone()
            
            response = {
                "status": "healthy",
                "database": "connected",
                "connection_type": "async",
                "message": f"Successfully connected to {DB_NAME} on {DB_HOST}",
                "test_query": test_value,
                "database_name": db_info_result[0] if db_info_result else None,
                "mysql_version": db_info_result[1] if db_info_result else None,
                "connection_info": {
                    "host": host,
                    "port": port,
                    "database": DB_NAME,
                    "user": DB_USER
                }
            }
            logger.info("헬스 체크 성공")
            return response
    except Exception as e:
        logger.error(f"헬스 체크 실패: {e}")
        return {
            "status": "unhealthy",
            "database": "disconnected",
            "connection_type": "async",
            "error": str(e),
            "error_type": type(e).__name__,
            "connection_info": {
                "host": host,
                "port": port,
                "database": DB_NAME,
                "user": DB_USER
            }
        }

@app.post("/shutdown")
async def shutdown_server(current_user: User = Depends(get_current_active_user)):
    """서버를 안전하게 종료합니다. (인증된 사용자만 가능)"""
    global server_running
    logger.info(f"🛑 서버 종료 요청 수신: user={current_user.username}")
    server_running = False
    
    # 백그라운드에서 서버 종료
    asyncio.create_task(shutdown_background())
    
    return {
        "message": "서버 종료 요청이 수신되었습니다. 잠시 후 서버가 종료됩니다.",
        "status": "shutting_down",
        "requested_by": current_user.username
    }

async def shutdown_background():
    """백그라운드에서 서버 종료"""
    await asyncio.sleep(2)  # 2초 대기
    logger.info("🛑 서버 종료 실행")
    sys.exit(0)

if __name__ == "__main__":
    import uvicorn
    
    try:
        uvicorn.run(
            app, 
            host="0.0.0.0", 
            port=8000,
            log_config=None,  # uvicorn 기본 로깅 비활성화
            access_log=False  # uvicorn 액세스 로그 비활성화 (우리 로거 사용)
        )
    except KeyboardInterrupt:
        logger.info("🛑 서버 종료 요청됨")
    except Exception as e:
        logger.error(f"❌ 서버 실행 중 오류: {e}") 