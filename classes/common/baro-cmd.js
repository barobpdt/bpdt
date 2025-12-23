@baro.cmd(param) {
	cmdObjects=object('baro.objectMap').addArray('@cmdObjects')
	addCmd = func(id, proc) {
		cur=Baro.process(id) 
		if(cmdObjects.find(cur) ) return cur;
		cmdObjects.add(cur)
		@baro.cmdStop(cur)
		if(typeof(proc, 'func')) 
			event(cur, '@callback', proc)
		else 
			event(cur, '@callback', @baro.cmdProc)
		return cur;
	};
	if(typeof(param,'string')) {
		args(name, callback, proc)		
		cur = addCmd(name, proc)
		cur.set('@mode','persist')
		if( typeof(callback,'func')) {
			cur.set('@callbackResult', callback)
		}
		// @baro.cmdRun(cur, 'cd');
		return cur;
	}
	obj=null
	while(cur, cmdObjects) {
		if(cur.cmp('@mode','persist') || cur.cmp('@status','start') ) continue;
		tick=cur.get('@endTick')
		if(tick) {
			dist=System.tick() - tick;
			if( dist < 500 ) continue;
		}
		obj = cur;
		break;
	}
	not( obj ) {
		idx=cmdObjects.size()+1;
		obj = addCmd("cmdObject_$idx")
	}
	if( typeof(param,'func')) {
		obj.set('@callbackResult', param)
	}
	print("@@ cmdObject ok => ", obj.id)
	return obj;
}

@baro.cmdRun(param) {
	if(tagCheck(param,'process')) {
		args(cmd,command,callback)
		if( typeof(callback,'func')) {
			cmd.set('@callbackResult', callback)
		}
	} else {
		args(command, callback)
		cmd=@baro.cmd(callback)
	}
	not(tagCheck(cmd,'process')) return print("@@ cmdRun 오류 $cmd 객체오류");
	not( cmd.run() ) {
		@baro.cmdStop(cmd)
		cmdList = nodeArrayVar(cmd,'@cmdList')
		cmdList.add('chcp 65001')
		cmdList.add(command)
		cmd.run('cmd')
		return cmd;
	}
	if( cmd.cmp('@status','start') ) {
		cmdList = nodeArrayVar(cmd,'@cmdList')
		if(command) {
			cmdList.add(command)
		}
	} else {
		not(command) command = nodeArrayVar(cmd,'@cmdList').pop()
		if( command ) {
			print("@@ baro.cmdRun COMMAND:$command")
			cmd.set('cmdResult','')
			cmd.set('@status','start')
			cmd.write(command)
		} else {
			cmd.set('@status','stay')			
		}
	}
	return cmd;
} 
@baro.cmdStop(cmd) {
	nodeArrayVar(cmd,'@cmdList',true)
	cmd.set('cmdResult','')
	cmd.set('@status', 'stop')
	cmd.set('@firstCall', true)
	if(cmd.run()) {
		cmd.stop()
	}
}
@baro.cmdAllStop() {
	arr=object('baro.objectMap').addArray('@cmdObjects')
	while(cur, arr) {
		@baro.cmdStop(cur)
	}
}

@baro.cmdProc(type,data) {
	if(type=='read') {
		this.appendText('cmdResult', data);
		c=data.ch(-1,true);
		if(c=='>') {
			this.set('@endTick', System.tick())
			if( this.get('@firstCall') ) {
				print("@@ firstCall RESULT:${this.cmdResult}")
				this.set('@firstCall', false)
			} else {				
				cb=this.get('@callbackResult')
				result=this.ref(cmdResult)
				if(typeof(cb,'string')) {
					logAppend(cb).write("##> $result\r\n\r\n")
				} else if(typeof(cb,'func')) {
					call(cb, this, this.ref(cmdResult))
				} else {
					print(">> cmd proc 결과:", this.cmdResult )
				}
			}
			this.set('@status','result')
			@baro.cmdRun(this)
		} else if(this.get('@logPrint')) {			
			cb=this.get('@callbackResult')
			if(typeof(cb,'string')) {
				logAppend(cb).write(data)
			}
			prog = this.ref('@line').move()
			log("$prog >> $data")
		}
	}
}

@baro.parseCmdResult(s) {
	ss=''
	while(s.valid()) {
		left = s.findPos('[')
		ss.add(left)
		not(s.ch()) break;
		s.findPos('m').trim()
		// print(">>$k")
	}
	return ss;
}
@baro.cmdCallback(&s) {
	not(s.ch()) return;
	if( s.start('echo %userprofile%', true) ) {
		not(s.ch()) return;
		line = s.findPos("\n").trim()
		conf('cf.userprofile', line, true)
		print("@@ cmd callbakc >>userprofile = $line 설정")
		return;
	}
	print("cmd callbakc>> $s")
}
@baro.findBindPort(tcpPort, checkCallback) {
	cmd=@baro.cmd('userProc')
	funcParam(cmd,'type','findBindPort')
	funcParam(cmd,'tcpPort,checkCallback')
	@baro.cmdRun(cmd, "netstat -ano | findstr $tcpPort", @baro.userProc);
}

@baro.userProc(&s) {
	// tcpPort close
	switch(funcParam('type')) {
	case findBindPort:	
		funcParam('tcpPort,checkCallback').inject(port,callback)
		print(">> findBindPort start ======= PORT:$port")
		while(s.valid()) {		
			s.findPos('TCP')
			not(s.ch()) break;
			line = s.findPos("\n")			
			line.findPos(":$port")
			c=line.ch(0)
			print(">> line:$line [$c]")
			not(typeof(c,'num')) {
				line.findPos('LISTENING')
				pid=line.trim()
				if(pid) {
					if(typeof(callback,'func')) {
						callback(pid)
					} else if(callback) {
						@baro.cmdRun(this, "taskkill /PID $pid /F" )
					}
					return;
				}
			}
		}
		print(">> findBindPort end $port 포트를 찾을수 없습니다")
		if(typeof(callback,'func')) callback()
	default:
	}
	// baro userProc end
}

funcParam() {
	asize=args().size()
	switch(asize) {
	case 1:
		args(&s)
		target=this
		not(s.find(',')) {
			name=s.trim()
			key="@$name" 
			not(target.isset(key)) return print("@@ funcParam 노드 키오류 ($name 변수가 설정되지 않았습니다)", target)
			value=target.get(key) 
			target.set(key,null)
			return value;
		}
		arr=_arr()
		while(s.valid()) {
			not(s.ch()) break;
			name=s.findPos(',').trim()
			key="@$name"
			arr.add(target.get(key))
			target.set(key,null)
		}
		return arr;
	case 2:
		args(target, &s)
		fn=Cf.funcNode('parent')
		while(s.valid()) {
			not(s.ch()) break;
			name=s.findPos(',').trim()
			not(fn.isset(name)) {
				return print("@@ funcParam 이름오류 (현재함수에 $name 변수 미정의)")
			}
			key="@$name"
			target.set(key, fn.get(name));
		}
	case 3:
		args(target, name, value)
		key="@$name"
		target.set(key, value)
	default:
	} 
}