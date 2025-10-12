import os
import sys
import time 
import threading
import logging
from CommCode import CommCode
from logging.handlers import TimedRotatingFileHandler
from sqlalchemy import create_engine, text
from sqlalchemy.ext.asyncio import create_async_engine, AsyncSession, async_sessionmaker
from sqlalchemy.orm import sessionmaker


# localPath = os.path.dirname(os.path.abspath(os.path.dirname(__file__)))
localPath = os.path.dirname(__file__)
sys.path.insert(0, localPath)

print(f"@@ localpath = {localPath} ")
class ApiConfig:
	_instance = None
	_lock = threading.Lock()
	def __new__(cls, *args, **kwargs):
		if not cls._instance:
			with cls._lock:
				if not cls._instance:
					cls._instance = super().__new__(cls)
		return cls._instance

	def __init__(self):
		if hasattr(self, '_initialized'):
			return
		self.async_engine = None
		self.async_session = None
		self.engine = None
		self.session = None
		self.commCode = None	# setCommCode
		self.logger = None
		self.fpIn = None		# setInPath
		self.inFileLastPos = 0
		self.inFilePath = ''
		self.nextCommand = ''
		self.fpOut = None		# setLogPath
		self.startTm = time.time()
		self._initialized = True
	def setAyncDb(self, url):
		try:
			self.async_engine = create_async_engine(url, echo=True)
			self.async_session = async_sessionmaker(
				self.async_engine, 
				class_=AsyncSession, 
				expire_on_commit=False
			)
		except Exception as e:
			self.loginfo(f"{url} 테이블 생성 중 오류 발생: {e}")

	def setDb(self, url):
		try:
			self.engine = create_engine(url, echo=True)
			self.session = sessionmaker(autocommit=True, autoflush=True, bind=self.engine)
		except Exception as e:
			self.loginfo(f"{url} 테이블 생성 중 오류 발생: {e}")
	def exec(self, query):
		result = None
		try:
			with self.engine.connect() as connection:
				result = connection.execute(text(query))
		except Exception as e:
			self.loginfo(f'{query} query exec exception : {e}')
		return result
	def loginfo(self, msg):
		logger = self.logger
		if not logger:
			logger = self.getLogger('api_log')
		try:
			logger.info(msg)
		except Exception as e:
			print(f'log print exception message:{msg} {logger} {e}')
	def log(self, msg):
		try:
			if self.fpOut:
				self.fpOut.write(f"##> {msg}\n")
				self.fpOut.flush()
		except Exception as e:
			pass

	def setInPath(self, path):
		try:
			if self.fpIn:
				self.fpIn.close()
			self.fpIn=open(path, 'r', encoding='utf8')
			self.inFileLastPos = fpIn.seek(0, os.SEEK_END)
			self.inFilePath = path
		except Exception as e:
			pass
			
	def getInData(self):
		if not self.fpIn:
			return
		commands = ''
		try:
			fsize=os.stat(self.inFilePath).st_size
			if fsize > self.inFileLastPos:
				commands = self.fpIn.read().strip()
				self.inFileLastPos = fsize
		except Exception as e:
			self.log(f'print: getInData exception {e}')
		return commands
			
	def setLogPath(self, path):
		try:
			if self.fpOut:
				self.fpOut.close()
			self.fpOut=open(path, 'a', encoding='utf8')
		except Exception as e:
			pass
	def getLogger(self, name='test'):
		if self.logger:
			return self.logger
			
		"""일자별 로그 설정"""
		# 로그 디렉토리 생성
		log_dir = "logs"
		if not os.path.exists(log_dir):
			os.makedirs(log_dir)
		
		# 로거 설정
		logger = logging.getLogger(name)
		logger.setLevel(logging.INFO)
		
		# 콘솔 핸들러 %(name)s - %(levelname)s 
		console_handler = logging.StreamHandler()
		console_handler.setLevel(logging.INFO)
		console_formatter = logging.Formatter(
			'%(asctime)s - %(message)s'
		)
		console_handler.setFormatter(console_formatter)
		
		# 파일 핸들러 (일자별 로테이션)
		file_handler = TimedRotatingFileHandler(
			filename=os.path.join(log_dir, f"{name}.log"),
			when="midnight",
			interval=1,
			backupCount=30,  # 30일간 보관
			encoding='utf-8'
		)
		file_handler.setLevel(logging.INFO)
		file_formatter = logging.Formatter(
			'%(asctime)s - %(funcName)s:%(lineno)d - %(message)s'
		)
		file_handler.setFormatter(file_formatter)
		
		# 핸들러 추가
		logger.addHandler(console_handler)
		logger.addHandler(file_handler)
		
		return logger	
			
	def setCommCode(self, root):
		self.commCode=root

	def getCode(self, code=None):
		if self.commCode is None:
			return None
		if not code:
			return self.commCode
		return self.commCode.getObject(code)
	
	def getCodeTitle(self, code):
		if self.commCode is None:
			return ''
		return self.commCode.getTitle(code)
	def getCodeKey(self, code, title):
		if self.commCode is None:
			return ''
		node = self.getCode(code)
		if not node:
			return ''
		for cur in node.children.values():
			if cur.title==title:
				return cur.code
		return ''
	def getCodeCount(self, code):
		if self.commCode is None:
			return 0
		node = self.getCode(code)
	
if __name__=='__main__':
	api = ApiConfig()
	print(f'@@ ApiConfig => {api}')
