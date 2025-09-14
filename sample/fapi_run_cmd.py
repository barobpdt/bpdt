@python.cmdPip('pip install fastapi uvicorn')
@python.cmdPip('pip install sqlalchemy')
@python.cmdPip('pip install python-dotenv')
@python.cmdPip('pip install "python-jose[cryptography]"')
@python.cmdPip('pip install passlib')
@python.cmdPip('pip install python-multipart')
@python.cmdPip('pip install bcrypt==4.0.1') 


## fastApi 실행
name = 'fapi_sqlite'
uvicorn = _s('${@python.path}/Scripts/uvicorn')
cc=@job.cmdRun('cd c:/bpdt/sample')
cmd = _s('$uvicorn ${name}:app --reload --root-path=/api/v1')
@job.cmdRun(cc, cmd)


name = 'fapi_sqlite'
cc=@job.cmdRun('cd c:/bpdt/sample')
cmd = _s('$py $name.py')
@job.cmdRun(cc, cmd)


## 파이션 커멘드 시작
cc=@python.cmdExec()
@job.cmdStop(cc)
@python.cmdExec(#[##> print: start python command tool ])
@python.cmdExec(#[##> quit: end python command tool ])

## 파이션 종료
@job.cmdRun('taskkill /im python.exe /F')
@job.cmdRun('taskkill /im uvicorn.exe /F')

## sys.path 설정
@python.cmdExec(#[##> exec:
import sys
from pathlib import Path
localPath = os.path.dirname(os.path.abspath(os.path.dirname(__file__)))
log(f'print: localPath = {localPath} {sys.path}')
])

@python.cmdExec(#[##> exec:	
from fapi_logger import setup_logging
logger = setup_logging()
log(f'print: logger = {logger}')
])
~~
@python.cmdExec(#[##> exec:	
from fapi_sqlite_model import model
log(f'print: model = {model} {model.Item}')
])

