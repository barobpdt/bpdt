import sys
import argparse
import os
import time
import win32.win32gui as win32gui

from PyQt5.QtWidgets import QWidget, QApplication, QVBoxLayout
from PyQt5.QtWidgets import QApplication
from PyQt5.QtGui import QDragEnterEvent, QDropEvent
from PyQt5.QtWebEngineWidgets import QWebEngineView, QWebEnginePage
from PyQt5.QtCore import Qt, QTimer, QTime, QUrl, QEvent

parser = argparse.ArgumentParser(description='프로그램 확장기능 처리')

class CustomAction(argparse.Action):
	def __call__(self, parser, namespace, values, option_string=None):
		setattr(namespace, self.dest, " ".join(values))

# 입력받을 인자값 등록
parser.add_argument('--log', action=CustomAction, nargs='+', required=True, help='로그파일')
parser.add_argument('--out', action=CustomAction, nargs='+', required=True, help='출력파일')
args = parser.parse_args()


class WebView(QWebEngineView):
	# Store external windows.
	external_windows = []
	
	def __init__(self, parent=None):
		super().__init__(parent)
		self.acceptDrops = True
		try:
			self.fp=open(args.log, 'r', encoding='utf8')
			self.fa=open(args.out, 'a', encoding='utf8')
			self.lastPos=self.fp.seek(0, os.SEEK_END)
			self.tm=time.time()
			self.setGeometry(0, 0, 350, 250)    
			self.timer = QTimer(self)
			self.timer.setInterval(100)
			self.timer.timeout.connect(self.timeout)
			self.timer.start()
			self.logCount = 0
			self.nextCommand = ''
			self.loadUrl('http://localhost/chat/chat.html')
			# self.setBackgroundColor(QtCore.Qt.transparent)
			self.setAcceptDrops(True)
			self.focusProxy().installEventFilter(self)
			self.hide()
		except Exception as e:
			print(f" error: {e}")

	def eventFilter(self, source, event):
		if (self.focusProxy() is source and event.type() == QEvent.MouseButtonPress ):
			self.logAppend("mouse press ok")
		return super().eventFilter(source, event)
	
	def dragEnterEvent(self, event: QDragEnterEvent):
		if event.mimeData().hasUrls():
			event.acceptProposedAction()
		else:
			event.ignore()

	def dropEvent(self, event: QDropEvent):
		files = [u.toLocalFile() for u in event.mimeData().urls()]		
		self.logAppend(f'Dropped file: {files}')
		event.accept()
		

	def acceptNavigationRequest(self, url,  _type, isMainFrame):
		if _type == QWebEnginePage.NavigationTypeLinkClicked:
			w = QWebEngineView()
			w.setUrl(url)
			w.show()

			# Keep reference to external window, so it isn't cleared up.
			self.external_windows.append(w)
			return False
		return super().acceptNavigationRequest(url,  _type, isMainFrame)

	def logAppend(self, msg):
		self.fa.write(f"##> {msg}\n")
		self.fa.flush()
		
	def loadUrl(self, url):
		self.setUrl(QUrl(url))
		
	def loadPage(self):
		with open('src/test.html', 'r') as f:
			html = f.read()
			self.setHtml(html)
			
	def timeout(self):
		# sender = self.sender()
		# currentTime = QTime.currentTime().toString("hh:mm:ss")
		# dist=time.time()-self.tm
		fsize=os.stat(args.log).st_size
		
		if self.logCount==0:
			self.logAppend(f"start:{args.log}")
			self.logCount = self.logCount + 1

		checkCommand = True
		if self.nextCommand:
			data = self.nextCommand
		elif fsize>self.lastPos :
			data = self.fp.read().strip()
		else:
			checkCommand = False

		if checkCommand:
			dist=time.time()-self.tm
			pos=data.find("##>")
			#self.logAppend(f"line:{data} dist={dist}")
			params=None
			val = ''
			ftype = ''
			if pos!=-1 :
				ep=data.find("##>", pos+3)
				if ep!=-1:
					line = data[pos+3:ep]
					self.nextCommand = data[ep:].strip()
				else:
					line = data[pos+3:]
					self.nextCommand = ''
				end=line.find(":", pos)
				if end!=-1 :
					ftype = line[0:end].strip()
					val = line[end+1:]
					params = [v.strip() for v in val.split(',')]
			# pos
			# self.logAppend(f">> {ftype} {params}")
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
				elif ftype=='setParent':
					self.setWindowFlag(Qt.WindowStaysOnTopHint)
					win32gui.SetParent(self.winId(), int(params[0]))
					self.setWindowFlag(Qt.FramelessWindowHint)
					self.setAttribute(Qt.WA_TranslucentBackground)
					self.show()
				elif ftype=='top':
					self.setWindowFlags(Qt.WindowStaysOnTopHint|Qt.SplashScreen)
					self.show()
				elif ftype=='geo':
					self.setGeometry(int(params[0]), int(params[1]), int(params[2]), int(params[3]))
					self.show()
				else:
					self.logAppend(f"{ftype} not defined")
			## if params
			self.lastPos=fsize
			self.logAppend(f"result:{ftype}<next>{self.nextCommand}")
		# end if print(f"currentTime=={currentTime}")
			
def main():
	app = QApplication(sys.argv)
	webview = WebView()
	webview.show()
	sys.exit(app.exec_())

if __name__ == '__main__':
	main()