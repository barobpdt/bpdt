##>config
	RUN_PATH = c:/temp/python
	USE_RESET = false

	@eval{
		src= cv('db-blob-insert')
		// runSource(src)
		print("eval src", src )
	}
	apply-all = @eval {
		configPath = pathJoin( conf('path.libs'), 'tools/python_command.js' )
		ss=''
		nl=conf('cf.newline')
		path = cv('RUN_PATH')
		while(cur, getFileList(path) ) {
			cur.inject(fullPath, name, modifyDt)
			key=left(name,'.')
			src = insertIndentText(fileRead(fullPath), "\t")
			ss.add(nl, "$key {",nl)
			ss.add(src, nl,"}",nl)
		}
		configSourceRplace(configPath, 'source', ss)
	}
	test-setUrl = @eval {
		url='https://www.freepik.com/search?format=search&img=1&last_filter=img&last_value=1&query=2d+sprites'
		call( @python.runCommand, this, 'exec', 'command.setUrl')
	}
	quit-all = @eval {
		call( @python.runCommand, this, "quit")
		call( @python.webCommand, this, "quit")
	}
	 
	save = @eval {
		reset = cv('USE_RESET')
		print( "python command>> start [USE_RESET:$reset]", typeof(reset) )
		@python.saveSource('runCommand.py', reset)
		@python.saveSource('webCommand.py', reset)
	}
	run = @eval {
		py	= pathJoin(conf('python.path'), 'python.exe')
		// addCmdWorker('runCommand', "$py -m pip list", @python.runProc)
		a=@python.execLine('runCommand.py')
		cmd=Baro.process('runCommand')
		if(cmd.cmp('@workerStatus', 'run') ) {
			print("runCommand가 실행중입니다");
		} else {
			addCmdWorker('runCommand', a, @python.runProc)
		}		
		a=@python.execLine('webCommand.py')
		cmd=Baro.process('webCommand')
		if(cmd.cmp('@workerStatus', 'run') ) {
			print("webCommand가 실행중입니다");
		} else {
			addCmdWorker('webCommand', a, @python.webProc)
		}
		return true;
	}
	db-blob-insert {
		sub=_node()
		sub.data=Cf.toBinary(ss)
		x=db.exec("insert into test (id, data, prop) values ('test',#{data},'222')", sub)
	}

##> func {name=python}
	@python.runProc(result, target) {
		print("##@python.runProc $result", target)
	} 
	@python.webProc(result, target) {
		print("##@python.webProc $result", target)
	} 
	@python.saveSource(filename, reset) {
		savePath	= pathJoin(cv('RUN_PATH'), filename)
		filePathInfo(savePath).inject(folder, null, name)
		source=cv("source.$name")
		not(source) return log("error:@python.saveSource 소스저장 실패 소스내용이 없습니다 (경로:$savePath, 설정:source.$name)");
		if(reset) {
			fileDelete(savePath)
		}
		not(isFile(savePath)) {
			fileWrite(savePath, source)
		}
	}
	@python.execLine(filename) {	
		py	= pathJoin(conf('python.path'), 'python.exe')
		sourcePath	= pathJoin(cv('RUN_PATH'), filename)
		not(isFile(sourcePath)) {
			log("error::@python.getCommand 소스파일 경로가 없습니다 (경로:$sourcePath)")
		}
		filePathInfo(sourcePath).inject(folder, null, name)
		command	= logAppend("python-${name}-command")
		out			= logTail("python-${name}-out")
		return str('"$py" "$sourcePath" --command="${command.logFileName}" --out="${out.logFileName}"')
	}
	@python.runCommand(command, confCode) {
		log=logAppend("python-runCommand-command")
		not(command.find(':') ) {
			command.add(':')
		}
		if(confCode) {
			fn=Cf.funcNode('parent')
			source = getSource( cv(confCode), this, fn)
			if(source) command.add(source)
		}
		print(">> runCommand ==> $command")
		log.append("@#>$command")
	}
	@python.webCommand(command, confCode) {
		log=logAppend("python-webCommand-command")
		not(command.find(':') ) {
			command.add(':')
		}
		if(confCode) {
			fn=Cf.funcNode('parent')
			source = getSource( cv(confCode), this, fn)
			if(source) command.add(source)
		}
		log.append("@#>$command")
	}
	/* 설정에 저장된 아이콘 정보를 읽어서 baro.icons 노드 생성*/
	@python.makeIconNode() {
		node=object('baro.icons')
		s=conf('icons.lucideSvg')
		s.ref()
		while(s.valid()) {
			not(s.ch()) break;
			k=s.move()
			c=s.ch()
			not(c.eq('{')) break;
			v=s.match()
			node.set(k,v)
		}
		return node;
	}
	/* 자바스크립트 svg path 정보를 읽어서 설저저장 (설정명:icons.lucideSvg) */
	@python.makeSvgIcons(&s) {
		not(s) {
			s=fileRead('c:/temp/icons.js')
			s.ref()
		}
		cnt=0
		tick=System.tick()
		arr=_arr()
		data=''
		while(s.valid()) {
			s.findPos('"path",')
			not(s.ch()) break;
			arr.reuse()
			name=null
			while(isNull(name)) {
				s.findPos('d:')
				c=s.ch()
				arr.add(s.match())
				s.findPos(']')
				c=s.ch()
				if(c.eq(']')) {			
					s.findPos('const') not(s.ch()) break;
					name=s.move()
					break;
				}
			}
			if(name) {		
				ss=''
				while(d, arr) {
					ss.add(str('<path d="$d" />'), "\n")
				}
				data.add("$name {\n$ss}\n")
			}
			cnt++;
		}
		conf('icons.lucideSvg', data, true)
		d=System.tick()-tick;
		print("makeSvgIcons==$cnt", d, data.size())
		return data;
	}

##> source
runCommand {
	    import sys
    import os
    # 현재 스크립트의 상위 디렉토리를 Python path에 추가
    # sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    import argparse
    import time
    class CustomAction(argparse.Action):
        def __call__(self, parser, namespace, values, option_string=None):
            setattr(namespace, self.dest, " ".join(values))
    # 인자값을 받을 수 있는 인스턴스 생성
    parser = argparse.ArgumentParser(description='프로그램 확장기능 처리')
    # 입력받을 인자값 등록
    parser.add_argument('--command', action=CustomAction, nargs='+', required=True, help='로그파일')
    parser.add_argument('--out', action=CustomAction, nargs='+', required=True, help='출력파일')
    args = parser.parse_args()

    try:
        fpCommand=open(args.command, 'r', encoding='utf8')
        fpOut=open(args.out, 'a', encoding='utf8')
        lastPos=fpCommand.seek(0, os.SEEK_END)
        nextCommand = ''
        tm=time.time()
        def log (msg):
            fpOut.write(f"$#> {msg}\n")
            fpOut.flush()
            
        log(f"파이션 실행툴 시작 {tm}")
        while True:
            dist=time.time()-tm
            fsize=os.stat(args.command).st_size
            checkCommand = True
            if nextCommand:
                commands = nextCommand
            elif fsize > lastPos :
                commands=fpCommand.read().strip()
            else:
                commands = ''
                checkCommand = False
            if checkCommand:
                pos=commands.find("$#>")
                ftype='undefined'
                data=''
                if pos!=-1 :
                    ep = commands.find("$#>", pos+3)
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
        fpCommand.close()
    except Exception as e:
        print(f" error: {e}")
}
webCommand {
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
    parser.add_argument('--command', action=CustomAction, nargs='+', required=True, help='로그파일')
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
            print("init ", args.command)
            try:
                self.fp=open(args.command, 'r', encoding='utf8')
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
                self.logAppend(f'@#>mousePress: {event.pos()}')
            elif event.type() == QEvent.MouseMove:
                self.logAppend(f'@#>mouseMove: {event.pos()}')
            return super().eventFilter(source, event)
        def logAppend(self, msg):
            self.fa.write(f"@#> {msg}\n")
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
            fsize=os.stat(args.command).st_size
            checkCommand = True
            if self.nextCommand:
                data = self.nextCommand
            elif fsize>self.lastPos :
                data = self.fp.read().strip()
            else:
                checkCommand = False
            if checkCommand:
                dist=time.time()-self.tm
                pos=data.find("@#>")
                self.logAppend(f"line:{data} dist={dist}")
                params=None
                val = ''
                ftype = ''
                if pos!=-1 :
                    ep=data.find("@#>", pos+3)
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
}

##> command
chromeDriver {
	from selenium import webdriver
	from selenium.webdriver.chrome.options import Options
	from selenium.webdriver.common.by import By
	from selenium.webdriver.common.keys import Keys
	from selenium.webdriver.common.action_chains import ActionChains
	global driver
	options = Options()
	options.add_experimental_option("detach", True)
	options.add_argument("--window-size = x,y")
	options.add_argument('--disable-popup-blocking') 
	driver = webdriver.Chrome(options=options)
	driver.implicitly_wait(3)
	driver.get(url='@[url]')
}
setUrl {
	driver.get('@[url]')
	log(f'chromeDriver: {driver.page_source}')
}
chromeOpen {
	from selenium import webdriver
	from selenium.webdriver.chrome.options import Options
	from selenium.webdriver.common.by import By
	from selenium.webdriver.common.keys import Keys
	from selenium.webdriver.common.action_chains import ActionChains
	options = Options()
	options.add_experimental_option("detach", True)
	options.add_argument('--disable-popup-blocking')	 
	driver = webdriver.Chrome(options=options)
	driver.implicitly_wait(3)
	driver.get(url='@[url]')

	# class name으로 찾기
	driver.find_element(By.CLASS_NAME,'gLFyf')
	# tag name으로 찾기
	driver.find_element(By.TAG_NAME,'textarea')
	# id로 찾기
	el = driver.find_element(By.ID,'APjFqb')

	# 클릭하기
	el.click()
	# 값 입력하기
	el.send_keys("tistory")
	# 키보드 입력하기
	el.send_keys(Keys.ENTER)
	# iframe 이동
	driver.switch_to.frame(' iframe id ')
	driver.switch_to.default_content()
	# 붙여넣기
	ActionChains(driver).key_down(Keys.COMMAND).send_keys('v').key_up(Keys.COMMAND).perform()

	log(f'pageSource: url => {driver.page_source}')
}
save-png {
	#pip install pillow
	import cairosvg
	svg_content = """
	<svg
	  xmlns="http://www.w3.org/2000/svg"
	  width="24"
	  height="24"
	  viewBox="0 0 24 24"
	  fill="none"
	  stroke="black"
	  stroke-width="2"
	  stroke-linecap="round"
	  stroke-linejoin="round"
	>
	  <path d="m14 11 4-4 4 4" />
	  <path d="M18 16V7" />
	  <path d="m2 16 4.039-9.69a.5.5 0 0 1 .923 0L11 16" />
	  <path d="M3.304 13h6.392" />
	</svg>
	"""

	cairosvg.svg2png(
		bytestring=svg_content.encode("utf-8"),
		write_to="icon.png",
		output_width=256,
		output_height=256
	)
	print("icon.png 생성 완료")
}

