@baro.cmd(param) {
	arr=object('baro.objectMap').addArray('@cmdObjects')
	addCmd = func(id) {
		cur=Baro.process(id) if(arr.find(cur)) return cur;
		arr.add(cur) 
		if(cur.isset('@callback')) return cur;
		@baro.cmdStop(cur)
		event(cur, '@callback', @baro.cmdProc)
		return cur;
	};
	not(arr.size()) {
		cmdCount=conf('baro.maxCmdCount') 		
		not(cmdCount) cmdCount=4
		while(n=1,cmdCount) addCmd("cmdObject_$n"))
	}
	if(param && typeof(param,'string')) {
		args(name, callback)		
		cur = addCmd(name)
		if( typeof(callback,'func')) {
			cur.set('@callbackResult', callback)
		}		
		return @baro.cmdRun(cur, 'cd');
	}
	
	obj=null
	while(cur, arr) {
		not(cur.id.start('cmdObject_')) continue;
		if(cur.cmp('@mode', 'persist')) continue;
		if(cur.cmp('@status','start')) continue;
		if(cur.isset('@jobCallbackFunc')) continue;
		tick=cur.get('@endTick')
		if(tick) {
			dist=System.tick() - tick;
			if( dist < 500 ) {
				continue;
			}
		}
		obj = cur;
		break;
	}
	not( obj ) {
		idx=arr.size()+1;
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
		cmd.set('@logPrint', true)
	} else {
		args(command, callback)
		cmd=@baro.cmd(callback)
	}
	not(tagCheck(cmd,'process')) return print("@@ cmdRun 오류 $cmd 객체오류");
	not( cmd.run() ) {
		@baro.cmdStop(cmd)
		cmd.cmdList.add('chcp 65001')
		cmd.cmdList.add(command)
		cmd.run('cmd')
		return cmd;
	}
	if( cmd.cmp('@status','start') ) {
		if(command) {
			cmd.cmdList.add(command)
		}
	} else {
		not(command) command = cmd.cmdList.pop()
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
	cmd.cmdList.reuse()
	cmd.set('cmdResult','')
	cmd.set('@status', 'stop')
	cmd.set('@firstCall', true)
	cmd.set('@logPrint', false)
	not(cmd.run()) {
		// print("@@ ${cmd.id}가 이미 중지된 상태입니다")
		return;
	}
	cmd.stop()
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
				if( this.isset('@jobCallbackFunc') ) {
					print(">> job callback 사용중")
				} else {				
					cb=this.get('@callbackResult')
					if(typeof(cb,'func')) {
						call(cb, this, this.ref(cmdResult))
					} else {
						print(">> cmd proc 결과:", this.cmdResult )
					}
				}
			}
			this.set('@status','result')
			@baro.cmdRun(this)
		} else if(this.get('@logPrint')) {
			if( this.isset('@jobCallbackFunc') ) {
				return;
			}
			cb=this.get('@callbackResult')
			if(typeof(cb,'func')) {
				call(cb, this, true, data)
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