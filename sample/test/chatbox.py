import pygame
import sys
from PyQt5.QtGui import QImage, QPixmap, QPainter, QLinearGradient, QColor
from PyQt5.QtWidgets import QApplication, QLabel
from PyQt5.QtWidgets import QApplication, QWidget 
from PyQt5.QtCore import Qt, pyqtSignal, pyqtSlot, Qt, QThread, QTimer
from PyQt5.QtGui import QImage
from particle import Particle, ParticleSystem
import argparse
import random
import math
import os


sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
	
class CustomAction(argparse.Action):
	def __call__(self, parser, namespace, values, option_string=None):
		setattr(namespace, self.dest, " ".join(values))

# 인자값을 받을 수 있는 인스턴스 생성
parser = argparse.ArgumentParser(description='채팅메시지 처리')

# 입력받을 인자값 등록
parser.add_argument('--log', action=CustomAction, nargs='+', required=True, help='로그파일')
parser.add_argument('--out', action=CustomAction, nargs='+', required=True, help='출력파일')
args = parser.parse_args()


_circle_cache = {}
def _circlepoints(r):
	r = int(round(r))
	if r in _circle_cache:
		return _circle_cache[r]
	x, y, e = r, 0, 1 - r
	_circle_cache[r] = points = []
	while x >= y:
		points.append((x, y))
		y += 1
		if e < 0:
			e += 2 * y - 1
		else:
			x -= 1
			e += 2 * (y - x) - 1
	points += [(y, x) for x, y in points if x > y]
	points += [(-x, y) for x, y in points if x]
	points += [(x, -y) for x, y in points if y]
	points.sort()
	return points

def add_outline_to_image(image: pygame.Surface, thickness: int, color: tuple, color_key: tuple = (255, 0, 255)) -> pygame.Surface:
	mask = pygame.mask.from_surface(image)
	mask_surf = mask.to_surface(setcolor=color)
	mask_surf.set_colorkey((0, 0, 0))
	iw, ih = image.get_size()
	new_img = pygame.Surface((iw + 2, ih + 2))
	new_img.fill(color_key)
	new_img.set_colorkey(color_key)
	for i in -thickness, thickness:
		new_img.blit(mask_surf, (i + thickness, thickness))
		new_img.blit(mask_surf, (thickness, i + thickness))
	new_img.blit(image, (thickness, thickness))
	return new_img

def renderOutlineText(text, font, gfcolor, ocolor, opx):
	textsurface = font.render(text, True, gfcolor).convert_alpha()
	w = textsurface.get_width() + 2 * opx
	h = font.get_height()
	osurf = pygame.Surface((w, h + 2 * opx)).convert_alpha()
	osurf.fill((0, 0, 0, 0))
	surf = osurf.copy()
	osurf.blit(font.render(text, True, ocolor).convert_alpha(), (0, 0))
	for dx, dy in _circlepoints(opx):
		surf.blit(osurf, (dx + opx, dy + opx))
	surf.blit(textsurface, (opx, opx))
	return surf

def pygame_surface_to_qimage(surface): 
	width, height = surface.get_size() 
	raw_data = pygame.image.tostring(surface, 'RGBA')  
	bytes_per_line = width * 4  # 4 bytes for RGBA 
	return QImage(raw_data, width, height, bytes_per_line, QImage.Format_RGBA8888)

def circle_surf(radius, color):
	surf = pygame.Surface((radius * 2, radius * 2))
	pygame.draw.circle(surf, color, (radius, radius), radius)
	surf.set_colorkey((0, 0, 0))
	return surf

class ChatboxWidget(QWidget):
	def __init__(self):
		super().__init__()
		self.setGeometry(100, 100, 800, 600)
		self.setWindowTitle('QPainter 이미지 그리기')
		self.imgMain = QImage()
		self.timer = QTimer(self)
		self.timer.timeout.connect(self.updateWidget)
		self.timer.start(30) # 약 60 FPS => 16
		self.surfaceList = []
		self.surfaceMain = pygame.Surface((800, 600), pygame.SRCALPHA)
		self.fontText = pygame.font.SysFont('malgungothic',  30)
		self.ypos = 0
		self.xpos = 0
		self.scrollPos = 0
		self.maxHeight = 0
		self.lineGap = 10
		self.mousePosArr = []
		self.updateTime = 60
		self.runCheck = True
		self.updateCheck = False
		self.msgText = ''
		self.setMouseTracking(True)
		self.mouseButton = ''
		self.mousePos = None
		self.mouseShift = False
		self.logImage = pygame.image.load('c:/bpdt/data/sprites/down_130.jpg')
		# self.logImage.convert()
		# self.logImage.convert()
		self.logImageRc = None
		self.logImageSurface = None
		# particle
		self.particleType = ''
		self.particles = []
		self.ps = ParticleSystem(self.surfaceMain.get_width(), self.surfaceMain.get_height())
		# log
		self.lastPos = 0
		self.logFile = args.log
		self.logCheck = False	
		try:
			self.fpIn=open(args.log, 'r', encoding='utf8')
			self.fpOut=open(args.out, 'a', encoding='utf8')
			self.lastPos=self.fpIn.seek(0, os.SEEK_END)
			self.logCheck = True
		except Exception as e:
			print(f"@@ log file exception : {e}")
		self.nextCommand = ''

	def mousePressEvent(self, event):
		self.mousePos = (event.x(), event.y())
		modifiers = event.modifiers()
		if modifiers & Qt.ShiftModifier:
			self.mouseShift = True
		if event.button() == Qt.LeftButton:
			self.mouseButton = 'left'
		elif event.button() == Qt.RightButton:
			self.mouseButton = 'right'
		elif event.button() == Qt.MiddleButton:
			self.mouseButton = 'middle'
		self.log(f'@@ mouse press pos={self.mousePos}')
		super().mousePressEvent(event)

	def mouseReleaseEvent(self, event):
		self.mouseButton = ''
		self.logImageRc = None
		self.mouseShift = False
		self.log(f'@@ mouse release')

	def mouseMoveEvent(self, event):
		x, y = event.x(), event.y()
		if self.particleType=='spark':
			self.mousePosArr.append((x,y))
		elif self.mouseButton=='left':
			w, h = self.surfaceMain.get_size()			
			cx, cy = w//2, h//2
			dx, dy = x-cx, y-cy
			dist = math.sqrt(dx ** 2 + dy ** 2)
			if self.logImageRc is None and dist<10:
				return
			angle = math.degrees(-math.atan2(dy, dx))
			scale = abs(5 * dist / w)
			self.logImageSurface = pygame.transform.rotozoom(self.logImage, angle, scale)
			self.logImageRc = self.logImageSurface.get_rect()
			self.logImageRc.center = cx, cy
			self.log(f'mouse move angle:{angle:2}, scale:{scale:.2}, self.logImageRc:{self.logImageRc}')
			if self.mouseShift and dx>2 and dy >2 :
				self.mousePosArr.append((x,y))
		super().mouseMoveEvent(event) 

	def updateWidget(self):
		if self.logImageRc:
			self.updateStart()
		if self.particleType!='':
			self.updateStart()
		try:
			self.parseCommand()
		except Exception as ex:
			print(f"@@ log parse exception : {ex}")

		if self.updateCheck:
			self.updateCheck = False
			self.update()
	
	def paintEvent(self, event):
		painter = QPainter(self)
		gradient = QLinearGradient(0, 0, self.width(), self.height()) # Start (x1,y1) and End (x2,y2) points
		gradient.setColorAt(0.0, QColor("lightblue")) # Color at 0% position
		gradient.setColorAt(1.0, QColor("darkblue"))  # Color at 100% position
		painter.setBrush(gradient)
		painter.setPen(Qt.NoPen)
		painter.drawRect(self.rect())

		self.imgMain = pygame_surface_to_qimage(self.makeSurface())
		if not self.imgMain.isNull():
			# 이미지 크기 및 위치 지정하여 그리기
			painter.drawImage(0, 0, self.imgMain) # (x, y, image)

	def log (self, msg):
		if not self.logCheck: 
			return
		try:
			self.fpOut.write(f"##> {msg}\n")
			self.fpOut.flush()
		except Exception as e:
			print(f"@@ log error : {e}")

	def addSurface(self):
		self.surfaceList.append(pygame.Surface((800, 600), pygame.SRCALPHA))
		self.log(f"@@ add surface len: {len(self.surfaceList)}")

	def addText(self, text, clrText, clrLine, lineSize, space):
		try:
			surface = add_outline_to_image(self.fontText.render(text, True, clrText), lineSize, clrLine)
			w, h = self.surfaceMain.get_size()
			tw, th = surface.get_size()
			mw = w - self.lineGap
			self.log(f"addText {text} surface:{tw},{th} pos:{self.xpos},{self.ypos}")
			if self.maxHeight < th:
				self.maxHeight = th
			if space> 0 :
				self.xpos+=space
			if mw < (self.xpos+tw):
				self.xpos = self.lineGap
				self.ypos+= self.maxHeight + self.lineGap
				self.maxHeight = 0
			idx = int(self.ypos//h)
			sy = self.ypos % h
			self.log(f"@@ idx >= len(self.surfaceList) {idx} >= {len(self.surfaceList)} add surface")
			if idx >= len(self.surfaceList):
				self.addSurface()
			ep = self.ypos + th
			idxNext = int(ep//h)
			if idxNext > idx:
				self.addSurface()
				hh = th-sy
				self.surfaceList[idx].blit(surface, (self.xpos,sy), (0,0,tw,hh))
				self.surfaceList[idx+1].blit(surface, (self.xpos,0), (0,0,tw,sy))
			else:
				self.surfaceList[idx].blit(surface, (self.xpos,sy))
			self.xpos += tw
		except Exception:
			self.log(f'@@ addText pass {text}')

	def addTextWrap(self, text, clrText=pygame.Color('dodgerblue'), clrLine=(255,255,255), lineSize=2):
		lines = [word.split(' ') for word in text.splitlines()]
		space = self.fontText.size(' ')[0]  # The width of a space.
		self.log(f'@@ addTextWrap : {text} {lines}')
		if self.ypos==0:
			self.ypos = self.lineGap
		for line in lines:
			num=0
			self.log(f'line=>{line}')
			for word in line: 
				self.log(f'word {num}=>{word}')
				self.addText(word, clrText, clrLine, lineSize, space if num>0 else '')
				num+=1
			self.xpos=self.lineGap
			self.ypos+=self.maxHeight + self.lineGap
			self.maxHeight = 0
		self.ypos+=self.lineGap
		self.updateStart()

	def makeSurface(self ):
		sy = self.scrollPos
		w, h = self.surfaceMain.get_size()
		idx = int(sy//h)
		cy = sy % h
		alen = len(self.surfaceList)
		if cy>0 and alen==(idx-1):
			cy=0
		if idx < alen:
			self.log(f"make surface {w} {h} : idx < alen => {idx} < {alen} cy:{cy}")
			if cy==0:
				self.surfaceMain.blit(self.surfaceList[idx], (0,0))
			else:	
				hh = h-cy
				self.surfaceMain.blit(self.surfaceList[idx], (0,0),(0,cy,w,hh) )
				self.surfaceMain.blit(self.surfaceList[idx+1], (0,hh),(0,0,w,cy) )
		if self.logImageRc:
			self.render_logImg()

		if self.particleType=='particle02':
			self.render_particle02()
		elif self.particleType=='spark':
			self.ps.draw_spark(self.surfaceMain)
		elif self.particleType=='dust':
			self.ps.add_random()
			self.ps.update()
			self.ps.draw(self.surfaceMain)

		if self.msgText:
			self.renderMessage()
		return self.surfaceMain
	
	def addMessage(self, text):
		self.msgText = text
		self.updateCheck = True

	def renderMessage(self):
		try:
			fontMsg = pygame.font.SysFont('malgungothic',  50)
			self.log(f'@@ renderMessage start {self.msgText}')
			# msgSurface = fontMsg.render(self.msgText, False, (255, 255, 255)).convert()
			msgSurface = fontMsg.render(self.msgText, True, (255, 255, 255))
			output = add_outline_to_image(msgSurface, 4, (255, 0, 0))
			w,h = self.surfaceMain.get_size()
			ow, oh = output.get_size()
			cx = (w-ow)/2
			cy = (h-oh)/2
			self.log(f'@@ renderMessage info => w,h ow,oh: {w},{h} {ow},{oh} {cx},{cy}')
			self.surfaceMain.blit(output, (cx,cy))
		except Exception as e:
			self.log(f'@@ renderMessage except : {e}')

	def render_particle02(self):
		w, h = self.surfaceMain.get_size()
		self.particles.append([[w/2, h/2], [random.randint(0, 20) / 10 - 1, -5], random.randint(6, 11)])
		for particle in self.particles:
			particle[0][0] += particle[1][0]
			particle[0][1] += particle[1][1]
			particle[2] -= 0.1
			particle[1][1] += 0.15
			pygame.draw.circle(self.surfaceMain, (255, 255, 255), 
					[int(particle[0][0]), int(particle[0][1])], int(particle[2]))

			radius = particle[2] * 2
			self.surfaceMain.blit(circle_surf(radius, (20, 20, 60)), 
						(int(particle[0][0] - radius), int(particle[0][1] - radius)), special_flags=pygame.BLEND_RGB_ADD)

			if particle[2] <= 0:
				self.particles.remove(particle)

	def render_logImg(self):
		if self.logImageRc:
			self.surfaceMain.blit(self.logImageSurface, self.logImageRc)

	def updateStop(self):
		self.runCheck = False
	def updateStart(self):
		if self.particleType=='spark':
			if len(self.mousePosArr) > 0 :
				mx, my = self.mousePosArr.pop(0)
				self.ps.add_spark(mx, my)
		self.updateCheck = True
		self.surfaceMain.fill((0,0,0,0))
		
	def closeEvent(self, event):
		"""창이 닫힐 때 스레드 정리"""
		try:
			self.fpIn.close()
			self.fpOut.close()
		except Exception as e:
			print('@@ log close exception')
		event.accept()
	def parseCommand(self):
		fsize=os.stat(self.logFile).st_size
		checkCommand = True
		if self.nextCommand:
			commands = self.nextCommand
		elif fsize > self.lastPos :
			commands = self.fpIn.read().strip()
		else:
			commands = ''
			checkCommand = False

		ftype='undefined'
		if checkCommand:
			pos=commands.find("##>")
			data=''
			if pos!=-1 :
				ep = commands.find("##>", pos+3)
				if ep!=-1 :
					line = commands[pos: ep]
					self.nextCommand = data[ep:].strip()
				else :
					line = commands[pos:]
					self.nextCommand = ''
				end=line.find(":")
				if end!=-1 :
					ftype=line[pos+3:end].strip()
					data=line[end+1:].strip()
				else:
					ftype='typeNotFound'
			else:
				ftype = 'endNotMatch' 

			if ftype=='quit': 
				self.close()
				return 
			elif ftype=='exec': 
				try:
					result=exec(data)
					self.log(f"exec:{result}")
				except Exception as ex:
					self.log(f"execException:{ex}")
			elif ftype=='echo':
				params=[v.strip() for v in data.split(',')]
				self.log(f"echo:params={params}")
			elif ftype=='addMessage':
				self.addMessage(data)
			elif ftype=='addText':
				self.addTextWrap(data)
			elif ftype=='particleType':
				name = data.strip()
				self.particleType = '' if name=='no' else name
				self.log(f'@@ particleType {name} start')
			elif ftype=='updateTime':
				num = data.strip()
				try:
					self.updateTime = int(num)
				except Exception as e:
					print(f"@@ updateTime {data} not integer")
			else:
				self.log(f"errorType: {line}")
			self.lastPos=fsize

app = QApplication(sys.argv)
pygame.init()
pygame.font.init() 
# pygame.display.set_mode((800,600),pygame.HIDDEN)
widget = ChatboxWidget()
widget.show()
sys.exit(app.exec())
