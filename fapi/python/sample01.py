from ApiConfig import ApiConfig
from datetime import datetime, timedelta

from jose import JWTError, jwt
from passlib.context import CryptContext
import uuid

def generate_unique_id():
	unique_id = uuid.uuid5(uuid.NAMESPACE_DNS, 'example.com') # uuid.uuid1(node=get_mac_address())
	while check_id_duplicate(unique_id):
		unique_id = uuid.uuid4()
	return unique_id

def check_id_duplicate(id):
	# 중복 여부 확인 로직
	return False # 중복되지 않은 경우


# 비밀번호 해싱
# @python.cmdPip('pip install bcrypt==4.0.1')
pwd_context = CryptContext(schemes=["bcrypt"], deprecated="auto")

# JWT 설정
SECRET_KEY = pwd_context.hash('bpdttest')  # 실제 운영환경에서는 환경변수로 관리
ALGORITHM = "HS256"
ACCESS_TOKEN_EXPIRE_MINUTES = 30


print(f'SECRET_KEY=={SECRET_KEY}')

if __name__=='__main__':
	try:
		expire = datetime.utcnow() + timedelta(minutes=30)
		data={"sub": "isitna"}
		meta = data.copy()
		meta.update({"exp":expire})
		passwd = pwd_context.hash("admin123")
		verify = pwd_context.verify("admin123", passwd)
		encoded_jwt = jwt.encode(meta, SECRET_KEY, algorithm=ALGORITHM)
		payload = jwt.decode(encoded_jwt, SECRET_KEY, algorithms=[ALGORITHM])
		api = ApiConfig()
		api.info("start sample01 ")
		print(f'meta=>{meta} {passwd} {verify} {encoded_jwt} {payload}')
	except Exception as e:
		ApiConfig().info(f"sample01: {e}")
