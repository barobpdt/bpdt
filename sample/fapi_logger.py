import io
import os
import sys
import logging
from logging.handlers import TimedRotatingFileHandler

sys.stdout = io.TextIOWrapper(sys.stdout.detach(), encoding = 'utf-8')
sys.stderr = io.TextIOWrapper(sys.stderr.detach(), encoding = 'utf-8')

# localPath = os.path.dirname(os.path.abspath(os.path.dirname(__file__)))
# sys.path.append(f'{localPath}/sample')

logger = None
# 로거 초기화 (데이터베이스 연결 전에 먼저 초기화)
def setup_logging():
	global logger
	if logger:
		return logger
		
	"""일자별 로그 설정"""
	# 로그 디렉토리 생성
	log_dir = "logs"
	if not os.path.exists(log_dir):
		os.makedirs(log_dir)
	
	# 로거 설정
	logger = logging.getLogger("fastapi_logger")
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
		filename=os.path.join(log_dir, "fastapi_logger.log"),
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