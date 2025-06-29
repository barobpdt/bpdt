## fastAPI 실행정보

## 일자별 로그생성
# log.ini 파일 생성 
	[loggers]
	keys=root

	[handlers]
	keys=logfile,logconsole

	[formatters]
	keys=logformatter

	[logger_root]
	level=INFO
	handlers=logfile, logconsole

	[formatter_logformatter]
	format=[%(asctime)s.%(msecs)03d] %(levelname)s [%(thread)d] - %(message)s

	[handler_logfile]
	#class=app.common.logger_handler.SafeRotatingFileHandler
	class=logging.handlers.TimedRotatingFileHandler
	level=INFO
	formatter=generic
	args=('log/uvicorn.log', 'midnight', 1, 365, 'utf-8')

	[formatter_generic]
	format=%(asctime)s - %(name)s - %(levelname)s - %(message)s	

	[handler_logconsole]
	class=handlers.logging.StreamHandler
	level=INFO
	args=()
	formatter=logformatter

# 명령어로 기동시 log.ini 설정 + Dockerfile
	# In Linux
	uvicorn app.main:app --host 0.0.0.0 --port 8000 --workers 10 --log-config log.ini
	# In Dockerfile
	CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000", "--workers", "20", "--log-config", "log.ini"]


## 로깅
import logging
from fastapi import FastAPI
app = FastAPI()
# 로깅 구성
logging.basicConfig(level=logging.INFO,
                    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

@app.get("/items/{item_id}")
def read_item(item_id: int, q: str = None):
    logger.info(f"Received request for item_id: {item_id} with query: {q}")
    return {"item_id": item_id, "q": q}


# 로깅 구성
def get_logger():
    logger = logging.getLogger("app_logger")
    logger.setLevel(logging.INFO)
    if not logger.handlers:
        handler = logging.StreamHandler()
        handler.setFormatter(logging.Formatter('%(asctime)s - %(levelname)s - %(message)s'))
        logger.addHandler(handler)
    return logger

@app.get("/items/{item_id}")
def read_item(item_id: int, logger=Depends(get_logger)):
    try:
        if item_id > 100:
            raise HTTPException(status_code=404, detail="항목을 찾을 수 없습니다.")
        logger.info(f"{item_id} 항목을 성공적으로 가져왔습니다.")
        return {"item_id": item_id}
    except HTTPException as e:
        logger.error(f"{item_id} 항목을 가져오는 중 오류 발생: {e.detail}")
        raise e

# Request 로깅 미들웨어
class RequestLoggingMiddleware(BaseHTTPMiddleware):
    async def dispatch(self, request: Request, call_next):
        logging.info(f"Request: {request.method} {request.url}")
        body = await request.body()
        logging.info(f"Request Body: {body.decode()}")
        response = await call_next(request)
        return response

# Response 로깅 미들웨어
class ResponseLoggingMiddleware(BaseHTTPMiddleware):
    async def dispatch(self, request: Request, call_next):
        response = await call_next(request)
        logging.info(f"Response: {response.status_code}")
        response_content = await response.content.read()
        logging.info(f"Response Body: {response_content.decode()}")
        return response

# 미들웨어 등록
app.add_middleware(RequestLoggingMiddleware)
app.add_middleware(ResponseLoggingMiddleware)		

## uvicon
def access_log() :
''' 엑세스 로그 포맷은 다음과 같이 지정'''
    logger = logging.getLogger('uvicorn.access')
    console_formatter = uvicorn.logging.ColourizedFormatter("{asctime} - {message}", style="{", use_colors=True)
    handler = logging.handlers.TimedRotatingFileHandler(ACCESS_LOG_PATH, when='midnight', interval=1, backupCount=1)
    handler.setFormatter(console_formatter)
    logger.addHandler(handler)

@app.on_event("startup")
async def startup_event() :
	'''on_event로 지정하면 request 올 때마다 해당 함수를 실행시킬 수 있다'''
    access_log() 
 
if __name__ == '__main__' :
''' 1.access_log = False로 지정하면 시스템 로그에 쌓이는 걸 막을 수 있다.
    2. reload=True, reload_dirs=[path] 를 지정하면 해당 디렉토리에 .py 파일이 
    업데이트 될 때마다 프로세스를 자동으로 리로드한다. '''
    uvicorn.run('main:app', host='0.0.0.0', port=8000, access_log=False,
                reload_dirs=[path], reload=True
    )

## rich
from rich.console import Console
from rich.table import Table
from rich.text import Text
import io
import os

# HTML 파일 저장 경로
output_dir = "rich_output"
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

def get_console_html(console):
    """콘솔에 기록된 내용을 HTML로 변환합니다."""
    buffer = io.StringIO()
    console.save_html(buffer, clear=False)
    return buffer.getvalue()

# 1. Rich 콘솔 생성 및 출력 내용 기록
console = Console(record=True)

# 예시 출력 (표)
table = Table(title="Example Table")
table.add_column("Column 1")
table.add_column("Column 2")
table.add_row("Row 1, Cell 1", "Row 1, Cell 2")
table.add_row("Row 2, Cell 1", "Row 2, Cell 2")
console.print(table)

# 예시 출력 (텍스트)
text = Text("This is some styled text.")
text.stylize("bold red")
console.print(text)

# 2. HTML 파일 저장
html_output = get_console_html(console)
html_file_path = os.path.join(output_dir, "rich_output.html")
with open(html_file_path, "w", encoding="utf-8") as f:
    f.write(html_output)

print(f"Rich 출력 내용이 {html_file_path} 에 저장되었습니다.")

## 호출	

conf('python.path', " %userprofile%/AppData/Local/Programs/Python/Python313", true)
 
c=cmd()
path=conf('python.path' )
pp="$path/python"
papi = "c:/bpdt/sample/fapi.py"
ptest= "c:/bpdt/sample/path_test.py"
c.run("$pp -m pip install fastapi pydantic uvicorn")
c.run("$pp -m pip list")
c.run("$pp $papi")
c.run("netstat -ano | findstr 8000")
c.run("taskkill /f /pid 5080")
c.run("$pp $ptest")

cc=cmd('test')
cc.run('ping 192.168.219.1')
cc.run("$pp $papi")

cm = Baro.process('fapi')
cb = call('cb_fapi')
setCallback(cm, cb)
cm.run("c:/Users/user/AppData/Local/Programs/Python/Python313/python $papi")

c.run("cd ")


~~
<func>
	cb_fapi(type, data) {
		print("fapi $type>>$data ")
	}
</func>
 