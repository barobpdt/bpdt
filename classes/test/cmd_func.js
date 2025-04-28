<script>
	@timer.idle() {
		n=this.incrNum('globaltimer.idx)
		m=n%200 
		if( m==0) print('global timer idle...')
	}

	globalTimer() {
		root = Cf.rootNode()
		not(System.globalTimer()) {
			System.globalTimer(80)
			setEvent(root,'onTimeout',@timer.idle)
		}
		return root;
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
		return in;
	}
	@webview.command(cmd) {
		root=Cf.rootNode()
		a=root.addNode('webviewCommandList')
		a.push(cmd)
	}
	@webview.commandTimeout() {
		if(this.webviewCommandList) {
			if(this.webviewCommandCall) {
				s=logReader('webview-out').timeout()
				if(s) {
					this.webviewCommandCall=false
				}
				return;
			}
			cmd=this.webviewCommandList.pop()
			if(cmd) {
				c=cmd('webview')
				not(c.isRun() && c.isStart() ) {
					print("webview not start 실핼리스트", this.webviewCommandList)
					this.webviewCommandList.reuse()
					return @webview.run()
				}
				print("webview command : $cmd")
				logWriter('webview-in').write(cmd)
				this.webviewCommandCall=true
			}
			
		}
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

	@cmd.command(command, callback) {
		c=cmd('cmd')
		if(typeof(callback,'function')) {
			c.cmdCallback(command, callback)
		} else {
			c.run(command)
		}
	}
</script>