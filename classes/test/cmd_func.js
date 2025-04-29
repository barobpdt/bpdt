<script>
	@timer.globalTimer(fc, addCheck, delay) {
		root = Cf.rootNode()
		fn = root.onTimeout
		if( typeof(addCheck,'bool')) {
			not( typeof(fc,'function')) return print("globalTimer 전역타이머가 실행 함수")
			not(System.globalTimer()) {
				if( typeof(fn,'func')) {
					System.globalTimer(true)
					find = arrayFind(fn.eventFuncList(), fc)
					not(find) fn.addFuncSrc(fc)
				} else {
					not(typeof(delay,'num')) delay = 80
					System.globalTimer(delay)
					setEvent(root,'onTimeout',fc)
				}
			}
			return root;
		}
		not(typeof(fc,'function')) fc = @timer.idle
		if( typeof(fn,'func')) {
			find = arrayFind(fn.eventFuncList(), fc)
			not( find ) fn.addFuncSrc(fc)
		} else {
			System.globalTimer(80)
			setEvent(root,'onTimeout', fc)
		}
		return root;
	}
	
	@timer.idle() {
		n=this.incrNum('globaltimer.idx)
		m=n%200 
		if( m==0) print('global timer idle...')
	}

	@webview.command(command, callback, target) {
		root=Cf.rootNode()
		arr=root.addArray('pythonCommandList')
		if(typeof(callback,'function')) {
			param = Cf.newNode().with(command, callback, target)
			arr.add(param)
		} else if(command) {
			arr.add(command)
		}
	} 
	@webview.commandTimeout() {		
		c=cmd('webview')
		not( c.isRun() && c.isStart() ) {
			if( this.webviewQuit ) return;
			print("webview not start 실행 리스트 초기화: ", this.webviewCommandList)
			arrayDeleteChild(this.webviewCommandList)
			return @webview.run()
		}
		if( this.webviewCommandCallTick ) {
			s=logReader('webview-out').timeout()
			print("webview timeout => $s")
			if( checkEnd(s) ) {
				cur = this.webviewCurrentNode not(typeof(cur,'node')) return;
				fc = cur.callback
				target = cur.target not(target) target = this
				if(typeof(callback,'function')) {
					call(fc, target, this.webviewResult)
				}
				this.webviewCurrentNode = null
				this.webviewCommandCallTick = 0
				cur.remove(true)
			}
			return;
		}
		cmdList =  this.webviewCommandList
		not(typeof(cmdList,'array')) return;
		param = cmdList.pop() not(param) return;
		print("webview command : $param")
		logReader('webview-out').timeout()
		if( typeof(param,'node') ) {
			this.webviewResult = ''
			this.webviewCurrentNode = param
			this.webviewCommandCallTick = System.tick()
			logWriter('webview-in').write(param.command)
		} else {			
			logWriter('webview-in').write(param)
		}
		checkEnd = func(&s) {
			not(s) return true;
			this.appendText('webviewResult', s)
			s.findPos('<next>')
			not(s.trim() ) return true
			return false;
		}
	}
	@webview.run() {
		c=cmd('webview')
		in = logWriter('webview-in')
		out = logReader('webview-out')

		pythonPath=conf('python.path')
		pythonPath.add('/python.exe')

		sourcePath = conf('include.path')
		sourcePath.add('/classes/pyapps')

		cmd=fv('#{pythonPath} "#{sourcePath}/webview.py" --log "#{in.member(logFileName)}" --out "#{out.member(logFileName)}" ')
		c.run(cmd)
		@timer.globalTimer(@webview.commandTimeout, true)
		return in;
	}
	@webview.close() {
		root = Cf.rootNode()
		root.webviewQuit = true
		c=cmd('webview')
		c.close()
	}
	@webview.quit() {
		c=cmd('webview')
		not( c.isRun() && c.isStart() ) return print("웹뷰 종료오류 [웹뷰 미실행중]")
		root = Cf.rootNode()
		root.webviewQuit = true
		logWriter('webview-in').write("##> quit:")
	}
	@webview.isGlobalTimer() {
		not(System.globalTimer()) return false;
		fn = Cf.rootNode().get('onTimeout') not(typeof(fn,'func')) return false;
		return arrayFind(fn.eventFuncList(), @webview.commandTimeout);
	}
	@webview.logPage() {
		Cf.sourceApply(#[
			<widgets>
				<page id="webviewLog">
					<editor id="e">
					<hbox>
						<button id="clear" text="clear">
						<space>
					</hbox>
				</page>
			</widgets>
		])
		p=page('test:webviewLog')
		p[
			init() {
				@out = logReader('webview-out')
				@e=widget('e')
				setEvent(widget('clear'), 'onClick', this.clickClear, this)
				this.timer(200)
				this.open()
			}
			onResize() {
				not(this.resizeTick) {
					this.resizeTick = System.tick()
					return
				}
				d=System.tick() - this.resizeTick
				if( d>800 ) {
					this.applyResize()
				}
			}
			applyResize() {
				e.geo().inject(x,y,w,h)
				cw=w.toInt()
				ch=h.toInt()
				this.resizeTick = System.tick()
				@webview.command("##>geo:1,1,$cw,$ch,0")
			}
			onTimer() {
				if( this.resizeTick) {
					this.applyResize()
					this.resizeTick = 0
				}
				if( @webview.isGlobalTimer() ) return;
				s=out.timeout()
				if(s) {
					e.append(s)
				}
			}
			clickClear() {
				e.value('')
			}
		]
		p.init()
		return p;
	}
	
	@python.pip(command, callback) {
		c=cmd('pip')
		pythonPath=conf('python.path')
		pythonPath.add('/python.exe')
		pip=fv('#{pythonPath} -m pip #{command}')
		if(typeof(callback,'function')) {
			c.cmdCallback(pip, callback)
		} else {
			c.run(pip)
		}
	}
	
	@python.commadStart() {
		c=cmd('python')
		in = logWriter('pythoncmd-in')
		out = logReader('pythoncmd-out')
		pythonPath=conf('python.path')
		pythonPath.add('/python.exe')

		sourcePath = conf('include.path')
		sourcePath.add('/classes/pyapps')

		cmd=fv('#{pythonPath} "#{sourcePath}/run_cmd.py" --log "#{in.member(logFileName)}" --out "#{out.member(logFileName)}" ')
		c.run(cmd)
		
		@clipManager.watcher(@clipManager.zipFileCopy, true)
		@timer.globalTimer(@python.commandTimeout, true)
		return in;
	}
	
	@python.commad(command, callback, target) {
		c=cmd('python')
		not( c.isRun() ) {
			@python.commadStart()
		}
		root=Cf.rootNode()
		in = logWriter('python-in')
		if( command.eq('zipinfo','zipinfo_kr') ) {
			fullPath = global('cm.zipFilePath')
			if( fullPath ) {
				root.zipinfoTick = System.tick()
				in.write("##>zipinfo:${fullPath}<>euc-kr")
				global('cm.zipFilePath', null)
			}
		}
		arr=root.addArray('pythonCommandList')
		if(typeof(callback,'function')) {
			node=Cf.newNode('pythonCommandNode')
			param = node.addNode().with(command, callback, target)
			arr.add(param)
		} else if(command) {
			arr.add(command)
		}
	}
	@python.isGlobalTimer() {
		not(System.globalTimer()) return false;
		fn = Cf.rootNode().get('onTimeout') not(typeof(fn,'func')) return false;
		return arrayFind(fn.eventFuncList(), @python.commandTimeout);
	}
	@python.commadTimeout() {
		c=cmd('python')
		not( c.isRun() && c.isStart() ) {
			if( this.webviewQuit ) return;
			print("webview not start 실행 리스트 초기화: ", this.pythonCommandList)
			arrayDeleteChild(this.pythonCommandList)
			return @python.commandStart()
		}
		if( this.zipinfoTick ) {
			out = logReader('runcmd-out').timeout()
			if( out ) {
				
			}
		} 
		cmdList =  this.pythonCommandList not(typeof(cmdList,'array')) return;
		param = cmdList.pop() not(param) return;
		print("webview command : $param")
		logReader('python-out').timeout()
		if( typeof(param,'node') ) {
			this.pythonResult = ''
			this.pythonCurrentNode = param
			this.pythonCommandCallTick = System.tick()
			logWriter('python-in').write(param.command)
		} else {			
			logWriter('python-in').write(param)
		}
		checkEnd = func(&s) {
			not(s) return true;
			this.appendText('webviewResult', s)
			s.findPos('<next>')
			not(s.trim() ) return true
			return false;
		}
	} 
	
	@cmd.command(command, callback, target) {		
		c=cmd('cmd')
		if(typeof(callback,'function')) {
			c.cmdCallback(command, callback, target)
		} else {
			c.run(command)
		}
	}
	
	@clipManager.watcher(fc, addCheck) {
		root=Cf.rootNode()
		fn = root.onChanageClipboard
		not(typeof(fn,'func')) {
			System.watcherClipboard()
			fn=setEvent(root, 'onChanageClipboard', @clipManager.watcherDefault)
		}
		if(typeof(fc,'function')) {
			not(typeof(addCheck,'bool')) addCheck = true
			if( addCheck ) {
				fn.removeFuncSrc(fc)
			} else {
				fn.addFuncSrc(fc)
			}
		}
	}
	@clipManager.watcherDefault(a,b,c) {
		root=Cf.rootNode()
		root.clipboardWatcherTick = System.tick()
	}
	@clipManager.zipFileCopy(a,b,c) {
		not(a.eq('url')) return;
		not(b.start('file:///')) return;
		path = b.value(8)
		ext=right(path,'.').lower()
		not(ext.eq('zip')) return;
		global('cm.zipFilePath', path)
		
	}
</func>
</script>