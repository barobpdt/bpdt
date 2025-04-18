c=cmd()
in=logWriter('webview-in')
out=logReader('webview-out')


pp=conf('python.path')
pp.add('/python')

target = System.path()
target.add('/pyapps')

p=fv('#{pp} "#{target}/webview.py" --log "#{in.member(logFileName)}" --out "#{out.member(logFileName)}" ')
c.run(p)

~~
<widgets>
	<page id="p1">
		<editor id="e">
		<hbox>
			<button id="clear" text="clear">
			<space>
		</hbox>
	</page>
</widgets>
~~
page=page('test:p1')
page.open()
page[
	init() {
		@e=widget('e')
		@out=logReader('webview-out')
		@in = logWriter('webview-in')
		setEvent(widget('clear'), 'onClick', this.clickClear, this)
		this.moveTick = 0
		this.timer(100)
	}
    movePos() {
        rc=e.geo()
        this.mapGlobal(rc).inject(x,y,w,h)
        x=x.toInt()
        y=y.toInt()
        w=w.toInt()
        h=h.toInt()
        in.append("##>geo:$x,$y,$w,$h")
    }
	onTimer() {
        if(this.topMode) {
            this.movePos()
            this.topMode = false
        }
        if(this.moveTick) {
            dist = System.tick() - this.moveTick
            if(dist>500) {
                this.movePos()
                this.moveTick = 0
            }
            return
        }
        if(this.activationChange) {
			a=this.is('min')
			if(a) {
				this.minMode = true
			} else if(this.minMode) {
				in.append('##>show:')
				this.minMode = false
			}
			b=this.is('active')
            if(b) {
                in.append('##>top:')
                this.topMode = true
            }
			this.activationChange = false
			return
		}
		if(this.minMode) {
			in.append('##>hide:')
			this.minMode=false
		}
		s=out.timeout()
		if(s) e.append(s)
	}
	clickClear() {
		print("click clear")
		e.value('')
	} 
	onMove() {
		not(this.moveTick) {
			in.append('##>hide:')
		}
		this.moveTick = System.tick()
	}
]

~~

## ace editor 함수
 // 서버 메시지 처리 함수
    function handleServerMessage(data) {
        switch (data.type) {
            case 'code_update':
                // 서버에서 코드 업데이트를 받은 경우
                if (data.code !== undefined) {
                    editor.setValue(data.code);
                }
                break;
            case 'language_change':
                // 서버에서 언어 변경을 받은 경우
                if (data.language) {
                    document.getElementById('language-select').value = data.language;
                    editor.session.setMode(`ace/mode/${data.language}`);
                }
                break;
            case 'theme_change':
                // 서버에서 테마 변경을 받은 경우
                if (data.theme) {
                    document.getElementById('theme-select').value = data.theme;
                    editor.setTheme(`ace/theme/${data.theme}`);
                }
                break;
            default:
                console.log('알 수 없는 메시지 타입:', data.type);
        }
    }
    
## hotkey 
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
	
## webview
import sys
import argparse
import os
import time

from PyQt5.QtWidgets import QWidget, QApplication, QVBoxLayout
from PyQt5.QtWidgets import QApplication
from PyQt5.QtWebEngineWidgets import QWebEngineView, QWebEnginePage
from PyQt5.QtCore import Qt, QTimer, QTime, QUrl

parser = argparse.ArgumentParser(description='프로그램 확장기능 처리')

class CustomAction(argparse.Action):
	def __call__(self, parser, namespace, values, option_string=None):
		setattr(namespace, self.dest, " ".join(values))

# 입력받을 인자값 등록
parser.add_argument('--log', action=CustomAction, nargs='+', required=True, help='로그파일')
parser.add_argument('--out', action=CustomAction, nargs='+', required=True, help='출력파일')
args = parser.parse_args()


class CustomWebEnginePage(QWebEnginePage):
	""" Custom WebEnginePage to customize how we handle link navigation """
	# Store external windows.
	external_windows = []

	def acceptNavigationRequest(self, url,  _type, isMainFrame):
		if _type == QWebEnginePage.NavigationTypeLinkClicked:
			w = QWebEngineView()
			w.setUrl(url)
			w.show()

			# Keep reference to external window, so it isn't cleared up.
			self.external_windows.append(w)
			return False
		return super().acceptNavigationRequest(url,  _type, isMainFrame)

class Example(QWidget):
	def __init__(self):
		super().__init__()
		print("init ", args.log)
		
		try:
			self.fp=open(args.log, 'r')
			self.fa=open(args.out, 'a')
			self.lastPos=self.fp.seek(0, os.SEEK_END)
			self.tm=time.time()
		except Exception as e:
			print(f" error: {e}")
			
		self.initUI()
		
	def initUI(self):
		vbox = QVBoxLayout(self) 
		self.webEngineView = QWebEngineView()
		#self.loadPage()
		self.loadUrl('http://localhost')

		vbox.addWidget(self.webEngineView)
		# vbox.setMargin(0)
		vbox.setContentsMargins(0, 0, 0, 0)
		self.setLayout(vbox)

		self.setGeometry(0, 0, 350, 250)
		self.setWindowTitle('QWebEngineView')        
		self.timer = QTimer(self)
		self.timer.setInterval(150)
		self.timer.timeout.connect(self.timeout)
		self.timer.start()
		
		self.setWindowFlags(Qt.SplashScreen)
		self.hide()
	
	def logAppend(self, msg):
		self.fa.write(f"##> {msg}\n")
		self.fa.flush()
		
	def loadUrl(self, url):
		self.webEngineView.setUrl(QUrl(url))
		
	def loadPage(self):
		with open('src/test.html', 'r') as f:
			html = f.read()
			self.webEngineView.setHtml(html)
			
	def timeout(self):
		sender = self.sender()
		currentTime = QTime.currentTime().toString("hh:mm:ss")
		dist=time.time()-self.tm
		fsize=os.stat(args.log).st_size
		if fsize>self.lastPos :
			line=self.fp.read().strip()
			pos=line.find("##>")
			self.logAppend(f"line:{line} dist={dist}")
			params=None
			val = ''
			ftype = ''
			if pos!=-1 :
				end=line.find(":", pos+3)
				if end!=-1 :
					ftype=line[pos+3:end].strip()
					val=line[end+1:]
					params=[v.strip() for v in val.split(',')]
					#params=map(str.strip, val.split(','))
			# pos
			self.logAppend(f">> {ftype} {params}")
			if params!=None :
				if ftype=='quit': 
					sys.exit()
				elif ftype=='echo':
					self.logAppend(f"echo = {params}")
				elif ftype=='hide':
					self.hide()
				elif ftype=='show': 
					self.show()
				elif ftype=='url':
					self.loadUrl(val.strip())
				elif ftype=='top':
					self.setWindowFlags(Qt.WindowStaysOnTopHint|Qt.SplashScreen)
					self.show()
				elif ftype=='geo':
					self.setGeometry(int(params[0]), int(params[1]), int(params[2]), int(params[3]))
					self.setWindowFlags(Qt.SplashScreen)
					self.show()
				else:
					self.logAppend(f"{ftype} not defined")
			## if params
			self.lastPos=fsize
		# end if print(f"currentTime=={currentTime}")
			
			
def main():
	app = QApplication(sys.argv)
	ex = Example()
	sys.exit(app.exec_())

if __name__ == '__main__':
	main()

