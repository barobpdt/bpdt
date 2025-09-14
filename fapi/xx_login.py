from datetime import datetime, timedelta
from fastapi import Depends, FastAPI, Request
from fastapi.security import OAuth2PasswordRequestForm
from jose import jwt
from fastapi.responses import HTMLResponse
from typing import Union
from fastapi.templating import Jinja2Templates

SECRET_KEY = "09d25e094faa6ca2556c818166b7a9563b93f7099f6f0f4caa6cf63b88e8d3e7"
ALGORITHM = "HS256"
ACCESS_TOKEN_EXPIRE_MINUTES = 30

app = FastAPI()

templates = Jinja2Templates(directory="templates")

def create_access_token(data: dict, expires_delta: Union[timedelta, None] = None):
	to_encode = data.copy()
	if expires_delta:
		expire = datetime.utcnow() + expires_delta
	else:
		expire = datetime.utcnow() + timedelta(minutes=15)
	to_encode.update({"exp": expire})
	encoded_jwt = jwt.encode(to_encode, SECRET_KEY, algorithm=ALGORITHM)
	return encoded_jwt

@app.get('/login',response_class=HTMLResponse)
def login(request : Request):
	return templates.TemplateResponse("login.html", {"request": request})

#creates token upon user validation
@app.post('/login', response_class=HTMLResponse)
def login(request : Request, f: OAuth2PasswordRequestForm = Depends()):
	data = {"username": f.username, "password": f.password}
	access_token_expires = timedelta(minutes=ACCESS_TOKEN_EXPIRE_MINUTES)
	access_token = create_access_token(
		data={"sub": f.username}, expires_delta=access_token_expires
	)
	return templates.TemplateResponse(
		"authenticated.html", {"request": request, "data" : data, 
		"access_token": access_token, "token_type": "bearer"}
	)  

if __name__ == "__main__":
	import uvicorn
	print(f"🌐 서버 주소: http://localhost:8000")
	print(f"📚 API 문서: http://localhost:8000/docs")
	uvicorn.run(app, host="0.0.0.0", port=8000) 