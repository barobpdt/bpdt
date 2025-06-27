from fastapi import FastAPI, HTTPException, Depends
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy import create_engine, Column, Integer, String, Text, Boolean, DateTime, Numeric
from sqlalchemy.orm import sessionmaker, Session, declarative_base
from pydantic import BaseModel, Field
from typing import List, Optional
from datetime import datetime
import os
import sys
import io
from sqlalchemy.sql import text

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

# 다양한 연결 문자열 시도
DATABASE_URLS = [
    f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4",
    f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4&ssl_mode=DISABLED",
    f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{host}:{port}/{DB_NAME}?charset=utf8mb4&ssl_mode=REQUIRED"
]

# 첫 번째 연결 문자열 사용
DATABASE_URL = DATABASE_URLS[0]

# MySQL 엔진 생성 (연결 오류 처리 개선)
def create_database_engine():
    """데이터베이스 엔진을 생성합니다."""
    for i, url in enumerate(DATABASE_URLS, 1):
        try:
            print(f"🔍 데이터베이스 연결 시도 {i}: {url}")
            
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
            
            print(f"✅ 데이터베이스 연결 성공! (연결 {i})")
            return engine
            
        except Exception as e:
            print(f"❌ 연결 {i} 실패: {e}")
            if i == len(DATABASE_URLS):
                print("❌ 모든 연결 시도 실패")
                raise e
            continue

# 엔진 생성
try:
    engine = create_database_engine()
except Exception as e:
    print(f"❌ 데이터베이스 엔진 생성 실패: {e}")
    # 기본 엔진 생성 (오류 발생 시)
    engine = create_engine(
        DATABASE_URL,
        pool_pre_ping=True,
        pool_recycle=300
    )

SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
Base = declarative_base()

# 데이터베이스 모델
class User(Base):
    __tablename__ = "users"
    
    id = Column(Integer, primary_key=True, index=True, autoincrement=True)
    name = Column(String(100), nullable=False)
    email = Column(String(100), unique=True, index=True, nullable=False)
    age = Column(Integer)
    is_active = Column(Boolean, default=True)
    created_at = Column(DateTime, default=datetime.utcnow)
    updated_at = Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)

# Pydantic 모델 (API 요청/응답용)
class UserBase(BaseModel):
    name: str
    email: str
    age: Optional[int] = None
    is_active: bool = True

class UserCreate(UserBase):
    pass

class UserUpdate(BaseModel):
    name: Optional[str] = None
    email: Optional[str] = None
    age: Optional[int] = None
    is_active: Optional[bool] = None

class UserResponse(UserBase):
    id: int
    created_at: datetime
    updated_at: datetime
    
    class Config:
        from_attributes = True

# QR 사용자 정보 Pydantic 모델
class QrUserResponse(BaseModel):
    memb_no: str
    qr_no: str
    qr_owner_cd: Optional[str] = None
    qr_use_cd: Optional[str] = None
    nick_nm: Optional[str] = None
    car_info: Optional[str] = None
    maker_car: Optional[str] = None
    park_text: Optional[str] = None
    pin_no: Optional[str] = None
    introduction_info: Optional[str] = None
    service_type_cd: Optional[str] = None
    add_dt: Optional[datetime] = None
    chg_dt: Optional[datetime] = None
    
    class Config:
        from_attributes = True

class QrUserDetailResponse(BaseModel):
    qr_info: QrUserResponse
    member_info: Optional[dict] = None
    qr_file_info: Optional[dict] = None
    call_history: Optional[List[dict]] = None
    
    class Config:
        from_attributes = True

# 데이터베이스 테이블 생성
def create_tables():
    try:
        Base.metadata.create_all(bind=engine)
        print("✅ 데이터베이스 테이블이 성공적으로 생성되었습니다.")
    except Exception as e:
        print(f"⚠️ 테이블 생성 중 오류 발생: {e}")
        print("⚠️ 기존 테이블을 사용합니다.")

# 애플리케이션 시작 시 테이블 생성 (오류 처리 개선)
try:
    create_tables()
except Exception as e:
    print(f"⚠️ 테이블 생성 실패: {e}")
    print("⚠️ 애플리케이션은 계속 실행되지만 일부 기능이 제한될 수 있습니다.")

# 데이터베이스 세션 의존성
def get_db():
    db = SessionLocal()
    try:
        yield db
    except Exception as e:
        print(f"⚠️ 데이터베이스 세션 오류: {e}")
        db.rollback()
        raise HTTPException(status_code=500, detail=f"Database error: {str(e)}")
    finally:
        db.close()

# CRUD 작업 함수들
def create_user(db: Session, user: UserCreate):
    try:
        db_user = User(**user.dict())
        db.add(db_user)
        db.commit()
        db.refresh(db_user)
        return db_user
    except Exception as e:
        db.rollback()
        print(f"⚠️ 사용자 생성 오류: {e}")
        raise e

def get_user(db: Session, user_id: int):
    try:
        return db.query(User).filter(User.id == user_id).first()
    except Exception as e:
        print(f"⚠️ 사용자 조회 오류: {e}")
        raise e

def get_users(db: Session, skip: int = 0, limit: int = 100):
    try:
        print("@@ get_users", skip, limit)
        return db.query(User).offset(skip).limit(limit).all()
    except Exception as e:
        print(f"⚠️ 사용자 목록 조회 오류: {e}")
        raise e

def update_user(db: Session, user_id: int, user_update: UserUpdate):
    try:
        db_user = db.query(User).filter(User.id == user_id).first()
        if db_user is None:
            return None
        
        update_data = user_update.dict(exclude_unset=True)
        for field, value in update_data.items():
            setattr(db_user, field, value)
        
        db_user.updated_at = datetime.utcnow()
        db.commit()
        db.refresh(db_user)
        return db_user
    except Exception as e:
        db.rollback()
        print(f"⚠️ 사용자 수정 오류: {e}")
        raise e

def delete_user(db: Session, user_id: int):
    try:
        db_user = db.query(User).filter(User.id == user_id).first()
        if db_user is None:
            return None
        
        db.delete(db_user)
        db.commit()
        return db_user
    except Exception as e:
        db.rollback()
        print(f"⚠️ 사용자 삭제 오류: {e}")
        raise e

# QR 사용자 정보 조회 함수들
def get_qr_users(db: Session, skip: int = 0, limit: int = 100):
    """QR 사용자 목록을 조회합니다."""
    try:
        return db.query(SgMembQr).offset(skip).limit(limit).all()
    except Exception as e:
        print(f"⚠️ QR 사용자 목록 조회 오류: {e}")
        return []

def get_qr_user_by_qr_no(db: Session, qr_no: str):
    """QR 번호로 QR 사용자 정보를 조회합니다."""
    try:
        return db.query(SgMembQr).filter(SgMembQr.QR_NO == qr_no).first()
    except Exception as e:
        print(f"⚠️ QR 사용자 조회 오류: {e}")
        return None

def get_qr_user_by_memb_no(db: Session, memb_no: str):
    """회원번호로 QR 사용자 정보를 조회합니다."""
    try:
        return db.query(SgMembQr).filter(SgMembQr.MEMB_NO == memb_no).all()
    except Exception as e:
        print(f"⚠️ 회원별 QR 사용자 조회 오류: {e}")
        return []

def get_qr_user_detail(db: Session, qr_no: str):
    """QR 사용자 상세 정보를 조회합니다."""
    try:
        # QR 사용자 기본 정보
        qr_user = db.query(SgMembQr).filter(SgMembQr.QR_NO == qr_no).first()
        if not qr_user:
            return None
        
        # 회원 정보
        member_info = None
        if qr_user.MEMB_NO:
            member = db.query(SgMemb).filter(SgMemb.MEMB_NO == qr_user.MEMB_NO).first()
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
        
        # QR 파일 정보
        qr_file_info = None
        qr_file = db.query(SgQrInfo).filter(SgQrInfo.QR_NO == qr_no).first()
        if qr_file:
            qr_file_info = {
                "qr_no": qr_file.QR_NO,
                "qr_file_no": qr_file.QR_FILE_NO,
                "qr_info": qr_file.QR_INFO,
                "low_qr_info": qr_file.LOW_QR_INFO,
                "use_yn": qr_file.USE_YN,
                "service_type_cd": qr_file.SERVICE_TYPE_CD
            }
        
        # 호출 이력 (최근 10건)
        call_history = []
        calls = db.query(SgMembCall).filter(SgMembCall.QR_NO == qr_no).order_by(SgMembCall.ADD_DT.desc()).limit(10).all()
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
        
        return {
            "qr_info": qr_user,
            "member_info": member_info,
            "qr_file_info": qr_file_info,
            "call_history": call_history
        }
    except Exception as e:
        print(f"⚠️ QR 사용자 상세 정보 조회 오류: {e}")
        return None

def search_qr_users(db: Session, search_term: str, search_type: str = "nick_nm"):
    """QR 사용자를 검색합니다."""
    try:
        query = db.query(SgMembQr)
        
        if search_type == "nick_nm":
            query = query.filter(SgMembQr.NICK_NM.contains(search_term))
        elif search_type == "car_info":
            query = query.filter(SgMembQr.CAR_INFO.contains(search_term))
        elif search_type == "qr_no":
            query = query.filter(SgMembQr.QR_NO.contains(search_term))
        elif search_type == "memb_no":
            query = query.filter(SgMembQr.MEMB_NO.contains(search_term))
        
        return query.all()
    except Exception as e:
        print(f"⚠️ QR 사용자 검색 오류: {e}")
        return []

def get_qr_user_stats(db: Session):
    """QR 사용자 통계를 조회합니다."""
    try:
        total_qr_users = db.query(SgMembQr).count()
        active_qr_users = db.query(SgMembQr).filter(SgMembQr.QR_USE_CD == "사용").count()
        master_qr_users = db.query(SgMembQr).filter(SgMembQr.QR_OWNER_CD == "M").count()
        sub_qr_users = db.query(SgMembQr).filter(SgMembQr.QR_OWNER_CD == "S").count()
        
        return {
            "total_qr_users": total_qr_users,
            "active_qr_users": active_qr_users,
            "master_qr_users": master_qr_users,
            "sub_qr_users": sub_qr_users
        }
    except Exception as e:
        print(f"⚠️ QR 사용자 통계 조회 오류: {e}")
        return {
            "total_qr_users": 0,
            "active_qr_users": 0,
            "master_qr_users": 0,
            "sub_qr_users": 0
        }

# API 엔드포인트들
@app.get("/")
def read_root():
    return {
        "message": "FastAPI MySQL CRUD Sample with QR User Management",
        "database": f"Connected to {DB_NAME} on {DB_HOST}",
        "docs": "/docs",
        "endpoints": {
            "users": "/users",
            "qr_users": "/qr-users",
            "qr_user_detail": "/qr-users/{qr_no}/detail",
            "qr_user_search": "/qr-users/search",
            "qr_user_stats": "/qr-users/stats"
        }
    }

@app.post("/users/", response_model=UserResponse, status_code=201)
def create_user_endpoint(user: UserCreate, db: Session = Depends(get_db)):
    """새로운 사용자를 생성합니다."""
    # 이메일 중복 확인
    existing_user = db.query(User).filter(User.email == user.email).first()
    if existing_user:
        raise HTTPException(status_code=400, detail="Email already registered")
    
    return create_user(db=db, user=user)

@app.get("/users/", response_model=List[UserResponse])
def read_users(skip: int = 0, limit: int = 100, db: Session = Depends(get_db)):
    """모든 사용자 목록을 조회합니다."""
    print("@@ read_users", skip, limit)
    users = get_users(db, skip=skip, limit=limit)
    return users

@app.get("/users/{user_id}", response_model=UserResponse)
def read_user(user_id: int, db: Session = Depends(get_db)):
    """특정 사용자를 조회합니다."""
    user = get_user(db, user_id=user_id)
    if user is None:
        raise HTTPException(status_code=404, detail="User not found")
    return user

@app.put("/users/{user_id}", response_model=UserResponse)
def update_user_endpoint(user_id: int, user_update: UserUpdate, db: Session = Depends(get_db)):
    """사용자 정보를 업데이트합니다."""
    user = update_user(db, user_id=user_id, user_update=user_update)
    if user is None:
        raise HTTPException(status_code=404, detail="User not found")
    return user

@app.delete("/users/{user_id}", response_model=UserResponse)
def delete_user_endpoint(user_id: int, db: Session = Depends(get_db)):
    """사용자를 삭제합니다."""
    user = delete_user(db, user_id=user_id)
    if user is None:
        raise HTTPException(status_code=404, detail="User not found")
    return user

@app.get("/users/search/{name}")
def search_users_by_name(name: str, db: Session = Depends(get_db)):
    """이름으로 사용자를 검색합니다."""
    users = db.query(User).filter(User.name.contains(name)).all()
    return users

@app.get("/stats/users")
def get_user_stats(db: Session = Depends(get_db)):
    """사용자 통계를 조회합니다."""
    total_users = db.query(User).count()
    active_users = db.query(User).filter(User.is_active == True).count()
    inactive_users = db.query(User).filter(User.is_active == False).count()
    
    return {
        "total_users": total_users,
        "active_users": active_users,
        "inactive_users": inactive_users
    }

# QR 사용자 관련 엔드포인트들
@app.get("/qr-users/", response_model=List[QrUserResponse])
def read_qr_users(skip: int = 0, limit: int = 100, db: Session = Depends(get_db)):
    """QR 사용자 목록을 조회합니다."""
    qr_users = get_qr_users(db, skip=skip, limit=limit)
    return qr_users

@app.get("/qr-users/{qr_no}", response_model=QrUserResponse)
def read_qr_user(qr_no: str, db: Session = Depends(get_db)):
    """QR 번호로 QR 사용자 정보를 조회합니다."""
    qr_user = get_qr_user_by_qr_no(db, qr_no=qr_no)
    if qr_user is None:
        raise HTTPException(status_code=404, detail="QR User not found")
    return qr_user

@app.get("/qr-users/{qr_no}/detail")
def read_qr_user_detail(qr_no: str, db: Session = Depends(get_db)):
    """QR 사용자 상세 정보를 조회합니다."""
    qr_user_detail = get_qr_user_detail(db, qr_no=qr_no)
    if qr_user_detail is None:
        raise HTTPException(status_code=404, detail="QR User not found")
    return qr_user_detail

@app.get("/qr-users/member/{memb_no}")
def read_qr_users_by_member(memb_no: str, db: Session = Depends(get_db)):
    """회원번호로 QR 사용자 정보를 조회합니다."""
    qr_users = get_qr_user_by_memb_no(db, memb_no=memb_no)
    return qr_users

@app.get("/qr-users/search/")
def search_qr_users_endpoint(
    search_term: str, 
    search_type: str = "nick_nm", 
    db: Session = Depends(get_db)
):
    """QR 사용자를 검색합니다."""
    if search_type not in ["nick_nm", "car_info", "qr_no", "memb_no"]:
        raise HTTPException(status_code=400, detail="Invalid search type")
    
    qr_users = search_qr_users(db, search_term=search_term, search_type=search_type)
    return qr_users

@app.get("/qr-users/stats")
def get_qr_user_stats_endpoint(db: Session = Depends(get_db)):
    """QR 사용자 통계를 조회합니다."""
    return get_qr_user_stats(db)

@app.get("/health")
def health_check():
    """데이터베이스 연결 상태를 확인합니다."""
    try:
        # 연결 테스트
        with engine.connect() as db:
            result = db.execute(text("SELECT 1 as test"))
            test_value = result.fetchone()[0]
            
            # 데이터베이스 정보 조회
            db_info = db.execute(text("SELECT DATABASE() as db_name, VERSION() as version"))
            db_info_result = db_info.fetchone()
            
            return {
                "status": "healthy",
                "database": "connected",
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
    except Exception as e:
        return {
            "status": "unhealthy",
            "database": "disconnected",
            "error": str(e),
            "error_type": type(e).__name__,
            "connection_info": {
                "host": host,
                "port": port,
                "database": DB_NAME,
                "user": DB_USER
            }
        }

if __name__ == "__main__":
    import uvicorn
    print(f"🚀 FastAPI MySQL CRUD 서버 시작")
    print(f"📊 데이터베이스: {DB_NAME} on {DB_HOST}")
    print(f"🌐 서버 주소: http://localhost:8000")
    print(f"📚 API 문서: http://localhost:8000/docs")
    uvicorn.run(app, host="0.0.0.0", port=8000) 