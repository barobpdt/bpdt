import argparse
import os
import time 

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
	fpLog=open(args.log, 'r')
	fpOut=open(args.output, 'a')
	lastPos=fpLog.seek(0, os.SEEK_END)
	tm=time.time()
	def log (msg):
		fpOut.write(f"##> {msg}\n")
		fpOut.flush()

	def keyMap (trigger, hotkey):
		log(f"{trigger}: {hotkey}")

	while True:
		dist=time.time()-tm
		fsize=os.stat(args.log).st_size
		if fsize>lastPos :
			line=fpLog.read().strip()
			pos=line.find("##>") 
			ftype='undefined'
			data=''
			if pos!=-1 :
				end=line.find(":", pos+3)
				if end!=-1 :
					ftype=line[pos+3:end].strip()
					data=line[end+1:]
			# pos
			print(f"type={ftype}, data={data}")
			if ftype=='quit': 
				break
			elif ftype=='eval': 
				try:
					result=eval(data)
					log(f"eval:{result}")
				except Exception as ee:
					log(f"evalException:{ee}")
			elif ftype=='exec': 
				try:
					result=exec(data)
				except Exception as ee:
					log(f"execException:{ee}")
			elif ftype=='echo':
				params=[v.strip() for v in data.split(',')]
				log(f"echo:params={params}")
			else:
				log(f"errorType:ftype={ftype} not defined")
			lastPos=fsize
		time.sleep(0.2)
	log(f"close test python [filepath:{args.output}]")
	fpOut.close()
	fpLog.close()	
except Exception as e:
	print(f" error: {e}")
