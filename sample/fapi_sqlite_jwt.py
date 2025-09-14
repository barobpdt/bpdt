from jose import JWTError, jwt
from passlib.context import CryptContext
from datetime import datetime, timedelta
from pydantic import BaseModel
from typing import Optional

# JWT 설정
SECRET_KEY = "your-secret-key-here-change-in-production"  # 실제 운영환경에서는 환경변수로 관리
ALGORITHM = "HS256"
ACCESS_TOKEN_EXPIRE_MINUTES = 30

# 비밀번호 해싱
pwd_context = CryptContext(schemes=["bcrypt"], deprecated="auto")

# JWT 토큰 관련 클래스들
class Token(BaseModel):
	access_token: str
	token_type: str

class TokenData(BaseModel):
	userid: str
	username: Optional[str] = None

class AdminUser(BaseModel):
	uesrid: str
	username: str
	email: Optional[str] = None
	full_name: Optional[str] = None
	disabled: Optional[bool] = None

class UserInDB(AdminUser):
	hashed_password: str

class UserLogin(BaseModel):
	userid:str
	username: str
	password: str


# 관리자 사용자 정보 (실제 운영환경에서는 데이터베이스에서 관리)
ADMIN_USERS = {
	"admin": {
		"userid": "admin",
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

def get_adminUser(userid: str):
	"""사용자 정보 조회"""
	if userid in ADMIN_USERS:
		user_node = ADMIN_USERS[userid]
		return UserInDB(**user_node)
	return None

def authenticate_user(userid: str, password: str):
	"""사용자 인증"""
	user = get_adminUser(userid)
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

