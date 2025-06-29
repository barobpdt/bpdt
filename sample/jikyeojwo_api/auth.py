from fastapi import Depends
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from jose import JWTError, jwt
from passlib.context import CryptContext
from datetime import datetime, timedelta
from model import Token, TokenData, User, UserInDB

# pip install python-jose[cryptography] passlib[bcrypt]

# JWT 설정
SECRET_KEY = "your-secret-key-here-change-in-production"  # 실제 운영환경에서는 환경변수로 관리
ALGORITHM = "HS256"
ACCESS_TOKEN_EXPIRE_MINUTES = 30

# 비밀번호 해싱
pwd_context = CryptContext(schemes=["bcrypt"], deprecated="auto")

# JWT Bearer 토큰
security = HTTPBearer()


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

def create_access_token(user_id):
	"""JWT 액세스 토큰 생성"""
	access_token_expires = timedelta(minutes=ACCESS_TOKEN_EXPIRE_MINUTES)
	expires_delta=access_token_expires
	data = {"sub": user_id } 	
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