@baro.cmd(param) {
	if(param && typeof(param,'string')) {
		args(name, callback)
		cur = Baro.process(name)
		if( typeof(callback,'func')) {
			cur.set('@callbackResult', callback)
		}
		arr=cur.get('cmdList') 
		if(typeof(arr,'array')) return cur;
		cur.addArray('cmdList')
		cur.set('@firstCall', true)
		_event(cur, '@callback', @baro.cmdProc)		
		return cur;
	}
	map=object('baro.objectMap')
	arr=map.get('@cmdObjects')
	cnt=0 if( typeof(arr,'array') ) cnt=arr.size()
	addCmd = func(n) {
		cur = arr.add(Baro.process("cmdObject_$n"))
		cur.set('@firstCall', true)
		cur.addArray('cmdList').reuse()
		_event(cur, '@callback', @baro.cmdProc)
		return cur;
	};
	not(cnt) {
		arr=map.addArray('@cmdObjects')
		maxCmdCount=@baro.conf('maxCmdCount') 
		not(maxCmdCount) {
			maxCmdCount=4
			conf('baro.maxCmdCount', maxCmdCount, true)
		}
		while(n=1,maxCmdCount) addCmd(n)
	}
	obj=null
	while(cur, arr) {
		if(cur.cmp('@mode', 'persist')) continue;
		if(cur.cmp('@status','start')) continue;
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
		obj = addCmd(arr.size()+1)
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
		print("@@ ${cmd.id}가 이미 중지된 상태입니다")
		return;
	}
	cmd.stop()
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
				if(typeof(cb,'func')) {
					call(cb, this, this.ref(cmdResult))
				} else {
					print(">> cmd proc 결과:", this.cmdResult )
				}
			}
			this.set('@status','result')
			@baro.cmdRun(this)
		} else if(this.get('@logPrint')) {
			prog = this.ref('@line').move()
			print("$prog >> $data")
		}
	}
}

