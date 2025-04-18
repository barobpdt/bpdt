<api>
	iconList(req, param, &uri) {
		path=webRoot()
		fo=Baro.file()
		fo.var(nameFilter, "*.png")
		fo.list("$path/images/icons",
		func(info){
			while(info.next()) {
				info.inject(type,name)
				if(type=='folder') continue
				param.addNode().with(name)
			}
		})
		return param;
	}
	icon(req, param, &uri) {
		path=webRoot()
		name=uri.trim()
		not( req.sendFile("$path/images/icons/$name") ) req.close()
		return;
	}
	pythonUse(req, param, &uri) {
		path=conf('path.python')
		cmd=Baro.process('cmd')
		cmd.args.command="cd $path"
		logClass('cmd').timeout()
		Cf.postEvent('cmdExec', cmd)
		while(10) {
			System.sleep(100)
			out=logClass('cmd').timeout()
			if(out) return out;
		}
		return "cd $path 실행오류";
	}
	pythonRunTest(req, param, &uri) { 
		cmd=Baro.process('cmd')
		cmd.args.command="python src/test.py"
		Cf.postEvent('cmdExec', cmd)
		logClass('cmd').timeout()
		while(250) {
			System.sleep(100)
			out=logClass('cmd').timeout()
			if(out) {
				c=out.ch(-1,true)
				if(c.eq('>')) return out;
			}
		}
		return "pythonRunTest 실행오류";
	}
	pythonLibs(req, param, &uri) {
		cmd=Baro.process('cmd')
		cmd.args.command="python Lib/site-packages/pip freeze"
		Cf.postEvent('cmdExec', cmd)
		logClass('cmd').timeout()
		while(100) {
			System.sleep(100)
			out=logClass('cmd').timeout()
			if(out) {
				c=out.ch(-1,true)
				if(c.eq('>')) return cmd.commandResult;
			}
		}
		return "pythonLibs 실행오류";
	}
	pythonInstall(req, param, &uri) {
		name=uri.trim();
		cmd=Baro.process('cmd')
		cmd.args.command="python Lib/site-packages/pip install $name"
		Cf.postEvent('cmdExec', cmd)
		logClass('cmd').timeout()
		while(250) {
			System.sleep(250)
			out=logClass('cmd').timeout()
			if(out) {
				c=out.ch(-1,true)
				if(c.eq('>')) return cmd.commandResult;
			}
		}
		return "pythonLibs 실행오류";
	}
	pythonPip(req, param, &uri) {
		name=uri.trim()
		cmd=Baro.process('cmd')
		cmd.args.command="python Lib/site-packages/pip $name"
		Cf.postEvent('cmdExec', cmd)
		logClass('cmd').timeout()
		while(250) {
			System.sleep(250)
			out=logClass('cmd').timeout()
			if(out) {
				c=out.ch(-1,true)
				if(c.eq('>')) return cmd.commandResult;
			}
		}
		return "pythonRun 실행오류";
	}
	test(req, param, uri) {
		return "api test URI=> $uri";
	}
</api>
