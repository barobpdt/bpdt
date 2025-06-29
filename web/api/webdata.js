<func>
	kill_netstatePort(c) {
		not(c) c=cmd('python')
		closePort = func(param) {
			ss=this.ref('cmdResult')
			while(ss.valid()) {
				line=ss.findPos("\n")
				not(line.ch()) continue;
				not(line.start('TCP',true)) continue;
				line.findPos('LISTENING')
				if(line.ch()) {
					pid = line.trim()
					print("netstate kill port pid=$pid")
					if(pid) c.run("taskkill /f /pid $pid")
					return true;
				}
			}
		};
		setEvent(c, 'onResult', closePort)
		c.run("netstat -ano | findstr 8000")
	}
</func>

emoji(req, param, &uri) { 
	conf('webdata.emoji',#[
표정
	😀,😃,😄,😁,😅,😂,🤣,😊,😇,🙂,🙃,😉,😌,😍,🥰
동물
	🐶,🐱,🐭,🐹,🐰,🦊,🐻,🐼,🐨,🐯,🦁,🐮,🐷,🐸,🐵
음식
	🍎,🍐,🍊,🍋,🍌,🍉,🍇,🍓,🍈,🍒,🍑,🥭,🍍,🥥,🥝	
])

	s = conf('webdata.emoji')
	s.ref()
	sp=-1;
	cur=null;
	while(s.valid()) {
		line = s.findPos("\n")
		indent = indentCount(line)
		not(line.ch()) continue;
		if(sp.eq(-1)) sp = indent
		dist = indent - sp;
		if(dist==0 ) {
			name = line.trim()
			cur = param.addNode().with(name)
		} else if(dist==1) {
			while(line.valid()) {
				not(line.ch()) break;
				emoji = line.findPos(',').trim()
				cur.addNode().with(emoji)
			}
		}
	}
	param.val('@treeMode', true)
	return param;
}

parseDdl(req,param,&uri,data) {
	s=stripSqlComment(fileRead('c:/temp/jkj.sql'))
	s=parseTable(s)
	fileWrite('c:/temp/jkj_table.txt', s)
	result = pythonSampleRun('path_test.py')
	not(result ) result = '실행결과가 없습니다'
	req.send(result)
	
	stripSqlComment = func(&s) {
		ss=''
		while(s.valid()) {
			ss.add(s.findPos('--'))
			not(s.ch()) break;
			s.findPos("\n")
		}
		return ss;
	};
	parseTable = func(&s) {
		ss = ''
		while(s.valid()) {
			a=s.move().lower()
			not(a.eq('create')) break;
			sp=skip(s)
			if(sp) s.pos(sp)
			a=s.findPos('(',1,1)
			c=a.ch()
			if(c.eq('`')) {
				table = a.match('`','`')
			} else {
				table = a.trim()
			}
			body = s.match(1)
			line = s.findPos(';')
			line.findPos('COMMENT=')
			c=line.ch()
			if(c.eq()) tableDesc=line.match() else tableDesc=''
			ss.add("$table<sep>$body<sep>$tableDesc<end>\r\n")
		}
		return ss;
	}
	skip = func(&s,type) {
		not(s.ch()) return 0;
		sp=0
		while(s.valid()) {
			a=s.move().lower()
			if(a.eq('table','if','not','exists')) {
				sp=s.cur()
				n++
				continue;
			}
			break;
		}
		return sp;
	}
}

<func>
	pythonSampleRun(runFile) {
			pythonPath=conf('python.path')
			bpdtPath='c:/bpdt'
			c=cmd('python')
			while(n=0,10) {
				if(c.status=='stay') break;
				System.sleep(500)
			}
			setEvent(c,'onResult', func(s) {
				fn=Cf.funcNode('parent')
				print("python result == $s")
				print("fn=>", fn)
				this.result = s
			})
			cmd=fv('#{pythonPath}/python #{bpdtPath}/sample/#{runFile}')
			print("python cmd=>$cmd")
			c.run(cmd)
			while(n=0,10) {
				if(c.result) break;
				System.sleep(500)
			}
			return getObjectResult(c)
	}
	getObjectResult(obj) {
		s=obj.result
		obj.result = ''
		return s;
	}
</func>