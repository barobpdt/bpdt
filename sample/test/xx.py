

class PygameThread(QThread):
	changePixmap = pyqtSignal(QImage)
	def __init__(self, parent, logFile, outFile):
		super().__init__(parent)
		pygame.init()
		self.textFont = pygame.font.SysFont('timesnewroman',  30)
		self.msgFont = pygame.font.SysFont('timesnewroman',  50)
		self.surfaceList = []
		self.surfaceMain = pygame.Surface((800, 600), pygame.SRCALPHA)
		self.ypos = 0
		self.xpos = 0
		self.scrollPos = 0
		self.lineGap = 10
		self.updateTime = 60
		self.runCheck = True
		self.updateCheck = False
		self.msgText = ''
		self.lastPos = 0
		self.logFile = logFile
		self.logCheck = False
		try:
			self.fpIn=open(logFile, 'r', encoding='utf8')
			self.fpOut=open(outFile, 'a', encoding='utf8')
			self.lastPos=self.fpIn.seek(0, os.SEEK_END)
			self.logCheck = True
		except Exception as e:
			print(f"@@ log file exception : {e}")
		self.nextCommand = ''

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
		surface = renderOutlineText(text, self.textFont, clrText, clrLine, lineSize )
		w, h = self.surfaceMain.get_size()
		tw, th = surface.get_size()
		mw = w - self.lineGap
		self.log(f"addText {text} surface:{tw},{th} pos:{self.xpos},{self.ypos}")
		if space> 0 :
			self.xpos+=space
		if mw < (self.xpos+tw):
			self.xpos = self.lineGap
			self.ypos+=self.lineGap
		idx = int(self.ypos//h)
		sy = self.ypos % h
		self.log(f"@@ idx >= len(self.surfaceList) {idx} >= len({self.surfaceList})")
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

	def addTextWrap(self, text, clrText=pygame.Color('dodgerblue'), clrLine=(255,255,255), lineSize=2):
		lines = [word.split(' ') for word in text.splitlines()]  
		space = self.textFont.size(' ')[0]  # The width of a space.
		if self.ypos==0:
			self.ypos = self.lineGap
		for line in lines:
			num=0
			for word in line: 
				self.addText(word, clrText, clrLine, lineSize, space if num>0 else '')
				num+=1
			self.xpos=self.lineGap
			self.ypos+=self.lineGap
		self.ypos+=self.lineGap

	def makeSurface(self, fillCheck ):
		sy = self.scrollPos
		if fillCheck:
			self.surfaceMain.fill((0, 0, 0, 0))
		w, h = self.surfaceMain.get_size()
		idx = int(sy//h)
		cy = sy % h
		alen = len(self.surfaceList)
		self.log(f"make surface {w} {h} : idx < alen => {idx} < {alen} cy:{cy} {self.msgText}")
		if cy>0 and alen==(idx-1):
			cy=0
		if idx < alen:
			if cy==0:
				self.surfaceMain.blit(self.surfaceList[idx], (0,0))
			else:	
				hh = h-cy
				self.surfaceMain.blit(self.surfaceList[idx], (0,0),(0,cy,w,hh) )
				self.surfaceMain.blit(self.surfaceList[idx+1], (0,hh),(0,0,w,cy) )
		if self.msgText:
			self.renderMessage()
		return self.surfaceMain
	
	def addMessage(self, text):
		self.msgText = text
		self.updateCheck = True

	def renderMessage(self):
		self.log(f'@@ renderMessage start {self.msgText}')
		msgSurface = self.msgFont.render(self.msgText, False, (255, 255, 255)).convert()
		self.log(f'@@ msgSurface: {msgSurface}')
		w,h = self.surfaceMain.get_size()
		ow, oh = output.get_size()
		self.log(f'info => w,h ow,oh: {w},{h} {ow},{oh}')
		cx = w-(ow/2)
		cy = h-(oh/2)
		output = add_outline_to_image(msgSurface, 2, (255, 0, 0))
		self.surfaceMain.blit(output, (cx,cy))

	def close(self):
		self.runCheck = False
		try:
			self.fpIn.close()
			self.fpOut.close()
		except Exception as e:
			print('@@ log close exception')
		pygame.quit()

	def run(self):
		commands = ''
		ftype = ''
		line =''
		while self.runCheck:
			try:
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
						break 
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
					elif ftype=='updateTime':
						num = data.strip()
						try:
							self.updateTime = int(num)
						except Exception as e:
							print(f"@@ updateTime {data} not integer")
					else:
						self.log(f"errorType: {line}")
					self.lastPos=fsize
			except Exception as ex:
				print(f"@@ log parse exception : {ex}")

			if self.updateCheck:
				img = pygame_surface_to_qimage(self.makeSurface(True))
				if not img.isNull():
					self.changePixmap.emit(img)
				self.updateCheck = False
			pygame.time.wait(self.updateTime)

	self.thread = PygameThread(self, args.log, args.out)
	self.thread.changePixmap.connect(self.update_image)
	self.thread.start()

	@pyqtSlot(QImage)
	def update_image(self, img):
	self.imgMain = img
