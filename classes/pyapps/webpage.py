import sys
import argparse
import os
import time

from PyQt5.QtWidgets import QWidget, QApplication, QVBoxLayout
from PyQt5.QtWidgets import QApplication
from PyQt5.QtWebEngineWidgets import QWebEngineView, QWebEnginePage
from PyQt5.QtGui import QDragEnterEvent, QDropEvent
from PyQt5.QtCore import Qt, QTimer, QTime, QUrl, QEvent
from PyQt5 import QtCore

import win32.win32gui as win32gui

parser = argparse.ArgumentParser(description='프로그램 확장기능 처리')

class CustomAction(argparse.Action):
	def __call__(self, parser, namespace, values, option_string=None):
		setattr(namespace, self.dest, " ".join(values))

# 입력받을 인자값 등록
parser.add_argument('--log', action=CustomAction, nargs='+', required=True, help='로그파일')
parser.add_argument('--out', action=CustomAction, nargs='+', required=True, help='출력파일')
args = parser.parse_args()


class MyWebView(QWebEngineView):
	# Store external windows.
	external_windows = []
	
	def __init__(self, parent=None):
		super().__init__(parent)
		self.acceptDrops = True
		try:
			# self.setBackgroundColor(QtCore.Qt.transparent)
			self.setAcceptDrops(True)
			self.setMouseTracking(True)
		except Exception as e:
			print(f" error: {e}")
	
	def dragEnterEvent(self, event: QDragEnterEvent):
		if event.mimeData().hasUrls():
			event.acceptProposedAction()
		else:
			event.ignore()

	def dropEvent(self, event: QDropEvent):
		files = [u.toLocalFile() for u in event.mimeData().urls()]
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

class WebWidget(QWidget):
	def __init__(self):
		super().__init__()
		print("init ", args.log)
		
		try:
			self.fp=open(args.log, 'r', encoding='utf8')
			self.fa=open(args.out, 'a', encoding='utf8')
			self.lastPos=self.fp.seek(0, os.SEEK_END)
			self.tm=time.time()
		except Exception as e:
			print(f" error: {e}")
			
		self.initUI()
		
	def initUI(self):
		self._glwidget = None
		self.webEngineView = MyWebView(self)
		self.loadUrl('http://localhost/chat/chat.html')

		vbox = QVBoxLayout(self) 
		vbox.addWidget(self.webEngineView)
		# vbox.setMargin(0)
		vbox.setContentsMargins(0, 0, 0, 0)
		self.setLayout(vbox)

		self.setGeometry(0, 0, 350, 250)
		self.setWindowTitle('QWebEngineView')        
		self.timer = QTimer(self)
		self.timer.setInterval(100)
		self.timer.timeout.connect(self.timeout)
		self.timer.start()
		self.nextCommand = ''
		self.parent_hwnd = None
		self.setAttribute(Qt.WA_TranslucentBackground)
		# self.setWindowFlags(Qt.SplashScreen)
		# self.hide()
		self.webEngineView.installEventFilter(self)

	def eventFilter(self, source, event):
		# self.logAppend(f'web-view event filter: {event.type()}')
		if (event.type() == QEvent.ChildAdded and
			source is self.webEngineView and
			event.child().isWidgetType()):
			self._glwidget = event.child()
			self._glwidget.installEventFilter(self)
		elif event.type() == QEvent.MouseButtonPress:
			self.logAppend(f'##>mousePress: {event.pos()}')
		elif event.type() == QEvent.MouseMove:
			self.logAppend(f'##>mouseMove: {event.pos()}')
		return super().eventFilter(source, event)

	def logAppend(self, msg):
		self.fa.write(f"##> {msg}\n")
		self.fa.flush()
		
	def loadUrl(self, url):
		self.webEngineView.setUrl(QUrl(url))
		
	def loadFile(self):
		with open('src/test.html', 'r') as f:
			html = f.read()
			self.webEngineView.setHtml(html)
			
	def timeout(self):
		# sender = self.sender()
		# currentTime = QTime.currentTime().toString("hh:mm:ss")
		fsize=os.stat(args.log).st_size
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
			self.logAppend(f"line:{data} dist={dist}")
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
					#params=map(str.strip, val.split(','))
			# pos
			self.logAppend(f">> {ftype} {params}")
			if params!=None :
				if ftype=='quit': 
					sys.exit()
				elif ftype=='echo':
					self.logAppend(f"echo = {params}")
				elif ftype=='pageActive':
					if self.parent_hwnd != None:
						win32gui.SetForegroundWindow(self.parent_hwnd)
				elif ftype=='setParent':
					# self.setWindowFlag(Qt.WindowStaysOnTopHint)
					parent = int(params[0])
					child_hwnd = self.winId()
					win32gui.SetParent(child_hwnd, parent)
					self.setWindowFlags(Qt.Window | Qt.FramelessWindowHint)
					self.show()
					# win32gui.ShowWindow(child_hwnd, win32con.SW_SHOW)
					win32gui.SetForegroundWindow(parent)
					self.parent_hwnd = parent
				elif ftype=='hide':
					self.hide()
				elif ftype=='show': 
					self.show()
				elif ftype=='url':
					self.loadUrl(val.strip())
				elif ftype=='top':
					self.setWindowFlags(Qt.Window | Qt.WindowStaysOnTopHint|Qt.SplashScreen)
					self.show()
				elif ftype=='splash':
					self.setWindowFlags(Qt.SplashScreen)
					self.show()
				elif ftype=='geo':
					self.setGeometry(int(params[0]), int(params[1]), int(params[2]), int(params[3]))
					self.setWindowFlags(Qt.SplashScreen)
					self.show()
				else:
					self.logAppend(f"{ftype}:not defined")
			## if params
			self.lastPos=fsize
			self.logAppend(f"result:{ftype}<next>{self.nextCommand}")
		# end if print(f"currentTime=={currentTime}")
			
			
def main():
	app = QApplication(sys.argv)
	ex = WebWidget()
	sys.exit(app.exec_())

if __name__ == '__main__':
	main()