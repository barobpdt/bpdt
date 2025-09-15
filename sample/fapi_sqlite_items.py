from fastapi import FastAPI, Depends, HTTPException
from sqlalchemy import create_engine, Column, Integer, String
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker, Session
from pydantic import BaseModel
from typing import List
import os
import logging
from logging.handlers import TimedRotatingFileHandler


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

# FastAPI app instance
# root_path='/api/v1'
app = FastAPI(title="My API", redirect_slashes=False) 

# Database setup
DATABASE_URL = "sqlite:///./test.db"
engine = create_engine(DATABASE_URL)
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
Base = declarative_base()


# Database model
class Item(Base):
    __tablename__ = "items"
    id = Column(Integer, primary_key=True, index=True)
    name = Column(String, index=True)
    description = Column(String)


# Create tables
Base.metadata.create_all(bind=engine)


# Dependency to get the database session
def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()


# Pydantic model for request data
class ItemCreate(BaseModel):
    name: str
    description: str


# Pydantic model for response data
class ItemResponse(BaseModel):
    id: int
    name: str
    description: str

@app.get("/")
async def read_root():
    logger.info(f'@@ read root ')
    return { "message": "@@@ Hello World @@@" }

# API endpoint to create an item
@app.post("/addItem/", response_model=ItemResponse)
async def create_item(item: ItemCreate, db: Session = Depends(get_db)):
    logger.info(f'@@ create item :{item}')
    db_item = Item(**item.dict())
    db.add(db_item)
    db.commit()
    db.refresh(db_item)
    return db_item
    
@app.post('/testData')
async def testData(request: Request):
    content_type = request.headers.get('Content-Type')    
    if content_type is None:
        raise HTTPException(status_code=400, detail='No Content-Type provided')
    elif content_type == 'application/json':
        try:
            return await request.json()
        except JSONDecodeError:
            raise HTTPException(status_code=400, detail='Invalid JSON data')
    else:
        raise HTTPException(status_code=400, detail='Content-Type not supported')

# API endpoint to read an item by ID
@app.get("/items/{item_id}", response_model=ItemResponse)
async def read_item(item_id: int, db: Session = Depends(get_db)):
    db_item = db.query(Item).filter(Item.id == item_id).first()
    if db_item is None:
        raise HTTPException(status_code=404, detail="Item not found")
    return db_item
        
@app.get("/items/", response_model=List[ItemResponse])
async def read_itemAll(db: Session = Depends(get_db)):
    return db.query(Item).all()

