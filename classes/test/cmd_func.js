<script module="@test">
	runWebview() {
		c=cmd('webview')
		in = logWriter('webview-in')
		out = logReader('webview-out')

		pythonPath=conf('python.path')
		pythonPath.add('/python.exe')

		sourcePath = conf('include.path')
		sourcePath.add('/classes/pyapps')

		cmd=fv('#{pythonPath} "#{sourcePath}/webview.py" --log "#{in.member(logFileName)}" --out "#{out.member(logFileName)}" ')
		
		pip=fv('#{pythonPath} -m pip install zipfile')
		pip=fv('#{pythonPath} -m pip install pywin32==306')
		cmdInstall = fv('#{pythonPath} "#{conf("python.path")}/Scripts/pywin32_postinstall.py" -install')
	}
	pipCommand(command, callback) {
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

	runCommand(command, callback) {
		c=cmd('cmd')
		if(typeof(callback,'function')) {
			c.cmdCallback(command, callback)
		} else {
			c.run(command)
		}
	}
</script>