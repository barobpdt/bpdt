import sys
import os
# 현재 스크립트의 상위 디렉토리를 Python path에 추가
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# 또는 pyapps 폴더의 절대 경로를 직접 지정
# sys.path.append("C:/path/to/your/project")

import argparse
import time 
from pyapps import check_file_exists, file_to_base64, change_extension, save_file, list_zip_contents, list_zip_info


def print_key_event(e):
	print(f"Key {e.name} was {e.event_type}")

# keyboard.hook(print_key_event)

	
class CustomAction(argparse.Action):
	def __call__(self, parser, namespace, values, option_string=None):
		setattr(namespace, self.dest, " ".join(values))

# 인자값을 받을 수 있는 인스턴스 생성
parser = argparse.ArgumentParser(description='프로그램 확장기능 처리')

# 입력받을 인자값 등록
parser.add_argument('--log', action=CustomAction, nargs='+', required=True, help='로그파일')
parser.add_argument('--out', action=CustomAction, nargs='+', required=True, help='출력파일')
args = parser.parse_args()

		
try:
	fpIn=open(args.log, 'r', encoding='utf8')
	fpOut=open(args.out, 'a', encoding='utf8')
	lastPos=fpIn.seek(0, os.SEEK_END)
	nextCommand = ''
	tm=time.time()
	def log (msg):
		fpOut.write(f"##> {msg}\n")
		fpOut.flush()


	log(f"파이션 실행툴 시작 {tm}")
	
	while True:
		dist=time.time()-tm
		fsize=os.stat(args.log).st_size

		checkCommand = True
		if nextCommand:
			commands = nextCommand
		elif fsize > lastPos :
			commands=fpIn.read().strip()
		else:
			commands = ''
			checkCommand = False

		if checkCommand:			
			pos=commands.find("##>")
			ftype='undefined'
			data=''
			if pos!=-1 :
				ep = commands.find("##>", pos+3)
				if ep!=-1 :
					line = commands[pos: ep]
					nextCommand = data[ep:].strip()
				else :
					line = commands[pos:]
					nextCommand = ''
				end=line.find(":")
				if end!=-1 :
					ftype=line[pos+3:end].strip()
					data=line[end+1:].strip()
			# pos
			
			if ftype=='quit': 
				break
			elif ftype=='zipdetail': 
				try:
					sucess, json = list_zip_contents(data)
					log(f"zipdetail:{sucess}@{json}")
				except Exception as ex:
					log(f"zipdetail:false@{ex}")
			elif ftype=='zipinfo': 
				try:
					pos=data.find("<>")
					if pos>0:
						path = data[0:pos].strip()
						encode = data[pos+2:].strip()
					else:
						path = data.strip()
						encode = 'euc-kr'
					log(f"@@ pos={pos}, path={path}, encode={encode}")
					sucess, json = list_zip_info(path,encode)
					log(f"zipinfo:{sucess}@{json}")
				except Exception as ex:
					log(f"zipinfo:false@{ex}")
			elif ftype=='eval': 
				try:
					result=eval(data)
					log(f"eval:{result}")
				except Exception as ex:
					log(f"evalException:{ex}")
			elif ftype=='exec': 
				try:
					result=exec(data)
					log(f"exec:{result}")
				except Exception as ex:
					log(f"execException:{ex}")
			elif ftype=='echo':
				params=[v.strip() for v in data.split(',')]
				log(f"echo:params={params}")
			elif ftype=='save_base64':
				path = data
				if not check_file_exists(path):
					log(f"error:file not exists {path}")
					continue
				save_path = change_extension(path, 'base64')
				base64_data = file_to_base64(path)
				ret = save_file(save_path, base64_data)
				log(f"save_base64:path={save_path}, ret={ret}")	
			else:
				log(f"errorType:ftype={ftype} not defined")
			lastPos=fsize
			log(f"result:{ftype}<next>{nextCommand}")
		time.sleep(0.2)
	log(f"close test python [filepath:{args.output}]")
	fpOut.close()
	fpIn.close()	
except Exception as e:
	print(f" error: {e}")
