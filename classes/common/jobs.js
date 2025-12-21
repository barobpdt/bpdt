@job.start() {
	conf('job.webMaxNum', 10)
	jobs=Baro.worker('jobs')
	not(jobs.is('start')) {			
		jobs.start(@job.proc)
	}
	@job.timer()
	return jobs;
}

@job.addPost(param) {
	not(System.globalTimer()) @job.timer();
	global=Cf.rootNode() 
	global.inject(@timerPostList)
	not(timerPostList) print("@@ timer job add error", global)
	if( typeof(param,'node')) {
		node=param;
	} else {
		args(type,node)
		not( typeof(node,'node')) node=_node()
		node.jobType=type
	}
	timerPostList.add(node)
} 
@job.timer() {
	if(System.globalTimer()) {
		print("timer 이미 설정됨")
		return;
	}
	global=Cf.rootNode()
	global.addArray('@timerPostList') 
	event(global,'onTimeout', @job.timerProc)
	System.globalTimer(250)
}
@job.timerProc() {
	this.inject(@timerPostList,@timerProc)
	if( typeof(timerProc,'func')) {
		if(timerProc()) return;
	}
	job = timerPostList.pop() not(job) return;
	if(typeof(job,'func')) {
		
	} else if(typeof(job,'node')) {
		if(job.cmp('tag','precess') ) {
			if( job.nextCommand ) {
				command = job.nextCommand
				job.nextCommand=null
				job.write(command)
			}
			return;
		}
		if(job.cmp('tag','web') ) {
			if( job.nextUrl ) {
				url = job.nextUrl
				job.nextUrl = null;
				job.call(url)
			}
		}
		type=job.jobType not(type) type='test'; 
		fc=call("@job.${type}#post")
		if(typeof(fc,'function') ) {
			call(fc, this, job)
		} else {
			print("@@ timer job function not defined node==>$job")
		}
	}
}

@job.proc(node) {
	print("@@ 작업시작 => ", node)
	not(node) return print("@@ 작업시작오류 (작업노드 미정의)")
	type = node.jobType not(type) type='test';
	fc = call("@job.${type}#job")
	if(typeof(fc,'func')) {
		call(fc, this, node)
	} else {
		return print("@@ 작업 처리 함수 오류(node:$node)")
	}
}

@job.addJob(param) {
	if(typeof(param,'node')) {
		args(node)
	} else {
		args(type, node)
		not(typeof(node,'node')) node=_node()
		node.jobType=type
	}
	jobs=Baro.worker('jobs')
	if( jobs.is('start')) {			
		jobs.push(node)
	} else {
		print("@@ 작업이 시작되지 않았습니다 job addNode 오류")
	}
}
@job.addWorker(param) {
	if(typeof(param,'node')) {
		args(node)
	} else {
		args(type, node)
		not(typeof(node,'node')) node=_node()
		node.jobType=type
	}
	jobs=Baro.worker('jobs')
	cnt=jobs.get('@subWorkerCount')
	subWorker = null
	if(cnt) {
		while(n=0, cnt) {
			cur=Baro.worker("jobs_$n")
			if(cur.get('@workerStartTick')) continue;
			subWorker = cur
		}
	}
	not(subWorker) {
		idx=jobs.incrNum('@subWorkerCount')
		subWorker=Baro.worker("jobs_$idx")
		subWorker.start(@job.workerProc)
	}
	subWorker.push(node)
	return subWorker;
}
@job.workerProc(node) {
	print("@@ job worker start => ", node)
	not(node) return print("@@ 작업시작오류 (작업노드 미정의)")
	type = node.jobType not(type) type='test';
	fc = call("@job.${type}#worker")
	if(typeof(fc,'func')) {
		this.set('@workerStartTick', System.tick())
		call(fc, this, node)
		this.set('@workerStartTick', 0)
	} else {
		return print("@@ worer 처리 함수 오류(node:$node)")
	}
}

@job.cmdObject(resultFunc) {
	map=object('baro.objectMap')
	arr = map.get('@cmdObjects')
	cnt = 0 if( typeof(arr,'array') ) cnt=arr.size()
	addCmd = func(n) {
		cur = arr.add(Baro.process("cmdObject_$n"))
		cur.set('@firstCall', true)
		cmdList = cur.addArray('cmdList').reuse()
		_event(cur, '@callback', @job.cmdProc)
		return cur;
	};
	not(cnt) {
		arr=map.addArray('@cmdObjects')
		while(n=1,8) addCmd(n)
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
	if( typeof(resultFunc,'func')) {
		obj.set('@callbackResult', resultFunc)
	}
	print("@@ cmdObject ok => ", obj.id)
	return obj;
}

@job.cmdRun(param) {
	if(_tagCheck(param,'process')) {
		args(cmd,command,callback)
		if( typeof(callback,'func')) {
			cmd.set('@callbackResult', callback)
		}
		cmd.set('@logPrint', true)
	} else {
		args(command, callback)
		cmd=@job.cmdObject(callback)
	}
	not(_tagCheck(cmd,'process')) return print("@@ cmdRun 오류 $cmd 객체오류");
	not( cmd.run() ) {
		@job.cmdStop(cmd)
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
			print("@@ job.cmdRun COMMAND:$command")
			cmd.set('cmdResult','')
			cmd.set('@status','start')
			cmd.write(command)
		} else {
			cmd.set('@status','stay')			
		}
	}
	return cmd;
} 
@job.cmdStop(cmd) {
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
@job.cmdProc(type,data) {
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
			@job.cmdRun(this)
		} else if(this.get('@logPrint')) {
			prog = this.ref('@line').move()
			print("$prog >> $data")
		}
	}
}

@job.webObject() {
	map=object('baro.objectMap')
	arr = map.get('@webObjects')
	cnt = 0
	if( typeof(arr,'array') ) cnt=arr.size()
	not(cnt) {			
		arr=map.addArray('@webObjects')
		while(n=1,8) {
			cur = arr.add(Baro.web("webObject_$n"))
			_event(cur, '@callback', @job.webTypeResult)
		}
	}
	obj=null
	while(cur, arr) {
		if(cur.is('run')) continue;
		tick=cur.get('@endTick')
		if(tick) {
			dist=System.tick() - tick;
			if( dist < 1000 ) {
				continue;
			}
		}
		obj = cur;
		break;
	}
	not( obj ) {
		idx = arr.size()+1;
		if( idx > conf('job.webMaxNum') ) {
			return null;
		}
		obj = arr.add(Baro.web("webObject_$idx"))
		print("@@ new web object size: ", obj)
		_event(obj, '@callback', @job.webTypeResult)
	}
	print("@@ webObject ok => ", obj.id)
	return obj;
}

@job.webTypeResult(type, data) {
	if(type=='read') this.appendText('result', data)
	if(type=='finish') {
		this.set('@endTick',System.tick())
		type=this.jobType not(type) type='test'
		fc = call("@job.${type}#web")
		if(typeof(fc,'function')) {
			target=this.get('@target')
			call(fc, this, this.ref(result), target )
		} else {
			print("@@ webTypeResult 콜백 함수 미정의 : job.${type}#web")
		}
	}
	if(type=='error') {
		this.set('@error', data)
	}
}

@job.addWebJob(jobType, url, target) {
	wo=@job.webObject()
	not(wo) return;
	not(jobType) jobType='test'
	wo.jobType=jobType
	hh=wo.get('@header')
	if( typeof(hh,'node')) {
		hh.removeAll(true)
	}
	if( wo.get('data')) {
		wo.set('data','')
	}
	method='GET'
	if( target ) {
		target.inject(@method, @data, @header)
		if(header) {
			not(hh) hh=wo.addNode('@header')
			if(typeof(header,'node')) {
				hh.copyNode(header)
			} else {
				setHeader(header)
			}
		}
		if( data) {
			wo.set('data', data)
		}
		target.set('@webObject', wo)
	}
	wo.set('@target', target)
	wo.set('result','')
	wo.call(url, method)
	return true;
	
	setHeader = func(&s) {
		while(s.valid()) {
			line=s.findPos("\n") not(line.ch()) continue;
			k=line.trim()
			v=s.findPos("\n").trim()
			hh.set(k,v)
		}
		print("@@ set http header => ", hh)
	};
}

/* ## 테스트 작업추가
	@job.addJob(node) 
*/
@job.test#job(node) {
	@job.addWorker(node)
}
@job.test#worker(node) {
	print("test worker start node=>", node)
} 
/* ## 파이션 eval 처리 실행
	@job.start()
	@job.addPost('pythonTest')
*/
@job.setPythonPath(&s) {
	s.findPos("\n")
	s.ch()
	print("s==$s")
	if(lineCheck(s,'>')) {
		ss=s.findPos('>').trim()
		path = ss.replace('\','/')
		pp=pythonPath("$path/AppData/Local/Programs/Python")
		print("@@ python path == $pp", path)
		if( pp ) {
			conf('python.path', pp, true)
		}
	}
	pythonPath = func(basePath) {
		pp=''
		fo=Baro.file('jobs')
		print("xxxxxxxxxxxxxx python path start : $basePath")
		fo.list(basePath,func(info) {
			while(info.next()) {
				info.inject(type, name, fullPath)
				print("@@ name==$name $type")
				if(type=='folder') {
					ss=name.lower()
					if(ss.start('python')) {
						return pp.add(fullPath)
					}
				}
			}
		});
		return pp;
	};
} 

@job.openUrl#web(&s, node) {
	not(s.size()) {
		print("@@ openUrl 응답결과가 없습니다", node, this)
		return;
	}
	
	not(node) node=_node()
	if( node.parseResult ) {
		return node.parseResult(s);
	}
	name=node.name not(name) name=System.date("yyyyMMdd");
	path=str('c:/temp/${name}.html')
	fileWrite(path,s)
	node.set('@saveFileName', path)
	@job.addPost('openUrl', node)
}
@job.openUrl#post(node) {
	if( node.nextUrl ) {
		@job.addWebJob('openUrl', node.nextUrl, node)
	} else {
		print("@@ post test", node)
		fileName = node.get('@saveFileName')
		program=conf('job.sourceEditor') not(program) program='code'
		cmd().run( str('$program "$fileName" '))
	}
}

@job.pythonTest#post(node) {
	not( isFolder(conf('python.path')) ) {
		c=cmd()
		c.cmdAdd('c:')
		c.cmdNode(node, 'cd %userprofile%', @job.setPythonPath)	
	}
}
/*
month='01'
days=System.date(System.localtime("2025-$month-01"),'daysInMonth')
data = str('chartType=TOP&searchStartDate=2025-$month-01&searchEndDate=2025-$month-$days&strType=')
node.set('work_month', month)
node.set('@method','POST')
node.set('@data', data)
@job.addWebJob('apiCall_tjTop1000', 'https://www.tjmedia.com/legacy/api/topAndHot100', node)
*/
@job.apiCall_tjTop1000#web(&s, node) {
	node.inject(apiName, work_month)
	cur=node.addNode('@current').removeAll(true)
	cur.parseJson(s)
	items = cur.resultData.items
	row=items.get(0)
	keys=row.keys()
	tm=System.localtime()
	db=Baro.db('tj')
	db.open('tj_info.db')
	not(db.open()) db.exec('create table top100 (work_month, tm, indexTitle, indexSong, word, mv_yn, imgthumb_path, rank, pro, com, icongubun)')
	sql = #[
		insert into top100 (
			work_month, tm,
			indexTitle, indexSong, word, mv_yn, imgthumb_path, rank, pro, com, icongubun
		) values(
			${work_month}, ${tm},
			#{indexTitle}, #{indexSong}, #{word}, #{mv_yn}, #{imgthumb_path}, #{rank}, #{pro}, #{com}, #{icongubun}
		)
	]
	while(row, items) {
		db.exec(sql, row)
	}
}

_tagCheck(obj, type) {
	not(typeof(obj,'node')) return false;
	if(obj.cmp('tag',type)) return true;
	tag=obj.get('@tag')
	chk = tag && tag.start(type)
	return chk;
}
_dbFetch(db, sql, node, filter) {
	type=typeof(db) not(type.eq('node(db)')) return print("@@ _dbFetch 데이터베이스 객체오류 SQL:$sql");
	not(node) node=_node()
	not(filter) filter=''
	if(filter) {
		fn=Cf.funcNode()
		query=str(sql,fn)
	} else {
		query=sql
	}
	return db.fetchAll(query,node,true);
}
_dbFetchStr(db, sql, node, filter) {
	root=_dbFetch(db,sql,node,filter)
	fa=root.get('@fields')
	ss=''
	while(c,fa,n) {
		if(n)ss.add(',')
		ss.add(c)
	}
	while(cur, root) {
		ss.add("\n")
		while(c,fa,n) {
			if(n)ss.add("\t")
			ss.add(cur.get(c))
		}
	}
	return ss;
}
_dbExec(db,sql,node) {
	type=typeof(db) not(type.eq('node(db)')) return print("@@ _dbExec 데이터베이스 객체오류 SQL:$sql");
	db.exec(sql,node)
	if(db.error()) return;
	return true;
}

@python.setPath(&s) {
	s.findPos("\n")
	s.ch()
	print("s==$s")
	if(lineCheck(s,'>')) {
		ss=s.findPos('>').trim()
		path = ss.replace('\','/')
		pp=pythonPath("$path/AppData/Local/Programs/Python")
		print("@@ python path == $pp", path)
		if(pp ) {
			conf('python.path', pp, true)
		}
	}
	pythonPath = func(basePath) {
		pp=''
		fo=Baro.file('jobs')
		print("xxxxxxxxxxxxxx python path start : $basePath")
		fo.list(basePath,func(info) {
			while(info.next()) {
				info.inject(type, name, fullPath)
				print("@@ name==$name $type")
				if(type=='folder') {
					ss=name.lower()
					if(ss.start('python')) {
						return pp.add(fullPath)
					}
				}
			}
		});
		return pp;
	};
}

@python.cmdExec(param) {
	map=object('baro.objectMap')
	cmd = map.get('@cmdExec')
	cmdline = func() {
		log=logWriter('runcmd-in')
		out=logReader('runcmd-out')
		python=str('${@python.path}/python')
		pythonFilename = str('$python.exe') not(isFile(pythonFilename)) return print("python 설치파일 찾기오류 경로:$pythonFilename");
		srcPath = str('${@sample.path}/apps') not(isFolder(srcPath)) return print("python 소스경로 오류=");
		return str('$python "$srcPath/run_cmd.py" --log "${log.logFileName}" --out "${out.logFileName}"');
	}
	if( cmd ) {
		runCheck = ~(cmd.run()) || cmd.cmp('@status','stay');
		if( runCheck ) {
			@job.cmdRun(cmd, cmdline())
		}
		if(param) {
			log=logWriter('runcmd-in')
			log.append(param)
		}
	} else {
		line = cmdline() not(line) return;
		cmd = @job.cmdRun(cmdline(), @python.result )
		cmd.set('@logPrint',true)
		cmd.set('@type','cmdExec')
		cmd.set('@mode','persist')
		cmd.set('@detail', '파이션 콘솔 소스실행')
		if( typeof(param,'func')) {
			execTimeout = param;
		} else {
			if(param) {
				@job.addPost(cmd)
			}
			execTimeout = @python.execTimeout
		}
		root = Cf.rootNode()
		root.set('@timerProc', execTimeout )
		map.set('@cmdExec', cmd)
	}
	return cmd;
}
@python.cmdPip(line) {
	not(line) line='pip list'
	not(line.start('pip')) line = Cf.val('pip ',line)
	python=str('${@python.path}/python')
	pythonFilename = str('$python.exe') not(isFile(pythonFilename)) return print("python 설치파일 찾기오류 경로:$pythonFilename");
	map=object('baro.objectMap')
	cmd = map.get('@cmdPip')
	cmdline = str('$python -m $line')
	if( cmd ) {
		@job.cmdRun(cmd, cmdline)
	} else {
		cmd = @job.cmdRun(cmdline, @python.result )
		cmd.set('@type','cmdPip')
		cmd.set('@mode','persist')
		cmd.set('@detail', '파이션 설치용 커멘드')
		map.set('@cmdPip', cmd)
	}
	return cmd;
}
@python.result(s) {
	print("@@ python result :$s")
}

@job.openSource#post(node) {
	fullpath = node.fullpath
	print(">> job.openSource start", node.fullpath)
	@job.cmdRun(str('notepade "$fullpath'))
}

@python.execTimeout() {
	out=logReader('runcmd-out')
	result = out.timeout()
	if( this.ref(pythonExecResult) ) {
		if(result) this.appendText('pythonExecResult', result)
		@python.execResult(this.ref(pythonExecResult))
		this.set('pythonExecResult', '')
		return true;
	}
	not( result) return false;
	this.set('pythonExecResult',result)
}
@python.execResult(&s) {
	cmd=object('baro.objectMap').get('@cmdExec')
	s.findPos('##>')
	not(s.ch()) {
		print("@python.execResult 실행 결과가 없습니다")
		return;
	} 
	while(s.valid()) {
		type=s.findPos(':').trim()
		ss=s.findPos('##>') not(ss.ch()) continue;
		if(type.eq('set')) {
			type=s.move()
			if(type.eq('webdriver')) {
				
			}
		}
		if(type.eq('print','execException')) {
			print("$type >> $ss")
			continue;
		}
		if(type.eq('result','exec')) {
			continue;
		}
		fc=call("@parse.$type")
		if(typeof(fc,'func')) {
			call(fc, cmd, ss, type)
		} else {
			print("@python.execResult 실행결과처리함수 @parse.$type 함수 미정의")
		}
	}
}

funcParam(param) {
	if( typeof(param,'node') ) {
		asize=args().size()
		if(asize==3) {			
			args(node,k,v)
			node.set("@$k", v)
		} else {
			fn=null
			if(asize==1) {
				args(node)
				fn=Cf.funcNode('parent')
			} else {
				args(node, name)
				if(typeof(name,'func')) {
					fn=name
				} else {
					k=name
					fnParent=Cf.funcNode('parent')
					v=fnParent.get(k)
					node.set("@$k", v)
				}
			}
			if( typeof(fn,'func') ) {
				node.set('@currentFunc', fn)
			}
		}
		return;
	} 
	k=param
	v=this.get("@$k")
	not(v) {		
		fn = this.get('@currentFunc')
		if(typeof(fn,'func')) {
			v=fn.get(k);
		}
	}
	return v;
}
@job.killPort(port) {
	c=@job.cmdRun('cd') 
	funcParam(c,'port')
	@job.cmdRun(c, str('netstat -ano | findstr $port'), func(&s) {
		port = funcParam('port')
		print("kill port ", this, port)
		cc=@job.cmdRun('cd')
		while(s.valid()) {		
			s.findPos('TCP')
			not(s.ch()) break;
			s.findPos(':')
			curPort=s.move()
			if(port.eq(curPort) ) {
				line=s.findPos("\n")
				line.findPos('LISTENING')
				pid=line.trim()
				print("pid:$pid")
				if(pid) {
					@job.cmdRun(cc,str('taskkill /PID $pid /F'))
				}
			}
		}
	});
} 
@job.killProg(name) {
	c=@job.cmdRun('cd')
	funcParam(c, 'name')
	@job.cmdRun(c, str('tasklist | findstr $name'), func(&s) {
		name = funcParam('name')
		cc=@job.cmdRun('cd')
		while(s.valid()) {
			s.findPos(name)
			c=s.ch() not(c) break;
			if(c.eq('.')) {
				ext=s.incr().move()
				name=Cf.val(name,".$ext")
			}
			pid = s.move()
			print("pid:$pid")
			// @job.cmdRun(cc,str('taskkill /im $name /F'))
			@job.cmdRun(cc,str('taskkill /PID $pid /F'))
		}
	});
}