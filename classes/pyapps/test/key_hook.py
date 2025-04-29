import argparse
import os
import time
import keyboard

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
parser.add_argument('--output', action=CustomAction, nargs='+', required=True, help='출력파일')
args = parser.parse_args()
try:
	fin=open(args.log, 'r', encoding='utf8')
	fout=open(args.output, 'a', encoding='utf8')
	lastPos=fin.seek(0, os.SEEK_END)
	tm=time.time()
	def log (msg):
		fout.write(f"##> {msg}\n")
		fout.flush()

	def keyMap (trigger, hotkey):
		log(f"{trigger}: {hotkey}")

	log(f"키보드 모니터링 시작 로그파일 : {args.log}")
    
	while True:
		dist=time.time()-tm
		fsize=os.stat(args.log).st_size
		if fsize>lastPos :
			line=fin.read().strip()
			pos=line.find("##>")
			log(f"line:{line}")
			params=None
			val = ''
			ftype = ''
			if pos!=-1 :
				end=line.find(":", pos+3)
				if end!=-1 :
					ftype=line[pos+3:end].strip()
					val=line[end+1:]
					params=[v.strip() for v in val.split(',')]
			# pos
			log(f"key hook:{ftype} {params}")
				
			if ftype=='quit': 
				break
			if ftype=='add-hotkey':
				# keyboard.add_hotkey('alt+shift+c', print, args=('triggered', 'alt+shift+c'))
				keyboard.add_hotkey('alt+shift+c', lambda: log("hotkey:alt+shift+c"))
			elif ftype=='echo':
				log("echo")
			else:
				log(f"{ftype} not defined")
			lastPos=fsize
		time.sleep(0.2)
	log(f"close test python [filepath:{args.output}]")
	fout.close()
	fin.close()	
except Exception as e:
	print(f" error: {e}")
	
	