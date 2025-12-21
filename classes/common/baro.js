/* ==============================================================
*
* 모듈 wrap 함수 (기존 등록된 모듈을 실행하기위한 처리함수)
*
==============================================================*/
run() {
	not(typeof(@baro.cmd,'func')) {
		include('classes/common/baro-cmd')
	}
	fn=Cf.funcNode('parent')
	getCommand=func(s) {
		not( s.find('${') ) return s;
		return str(a, fn);
	};
	asize=args().size()
	cmdObject=when(this, this.get('@cmdObject'))
	switch(asize) {
	case 0:
		return @baro.cmd();
	case 1: 
		args(a)
		if( tagCheck(a,'process')) {
			if(this) {
				if(cmdObject) {
					if(cmdObject != a) {
						this.set('@cmdObject', a)
					}
				} else {
					this.set('@cmdObject', a)
				}
			}
			if(a.run() && a.cmp('@status','start') ) {
				return true;
			}
			return false;
		}
		if(cmdObject) {
			cmdCurrent = @baro.cmdRun(cmdObject,getCommand(a))
		} else {
			cmdCurrent = @baro.cmdRun(getCommand(a))
			if(this) this.set('@cmdObject', cmdCurrent)
		}
	case 2: 
		args(a,b)
		if(typeof(b,'func')) {
			if(cmdObject) {
				cmdCurrent = @baro.cmdRun(cmdObject, getCommand(a), b)
			} else {
				cmdCurrent = @baro.cmdRun(getCommand(a),b)
				if(this) this.set('@cmdObject', cmdCurrent)
			}
		} else {
			cmdCurrent = @baro.cmdRun(a,getCommand(b))
		}
	case 3:
		args(a,b,c)
		cmdCurrent = @baro.cmdRun(a,getCommand(b),c)
	default:
	}
	return cmdCurrent;
}
runPython() {
	py=pathJoin( conf('python.path'), 'python.exe')
	not(isFile(py)) return log('파이션이 설치되지 않았거나 파이션경로가 설정되지 않았습니다')
	runFunc=call('run')
	self=Cf.funcNode().get('@this') not(self) self=_node()
	arr=args()
	command=arr.get(0)
	arr.set(0,"$py $command")
	return call(runFunc, self, arr);
}
loadConfigService(serviceMode, fullpath) {
	not(isFile(fullpath)) return print("@@ loadConfigService 오류 파일 $fullpath 경로가 없습니다");
	src=fileRead(fullpath)
	not(src) {
		return print("@@ loadConfigService 오류 설정소스가 없습니다");
	}
	print(">> loadService 시작", serviceMode, fullpath )
	root=@baro.loadService(serviceMode, src)
	filePathInfo(fullpath).inject(folder, filename)		
	root.set('@currentFileName',filename)
	return root;
}

/* 전역 타이머 실행 */
startGlobalTimer() {
	if( global().get('@timerDelay') ) {
		return log('global timer가 실행중입니다 #{0}', global().get('@timerDelay'))
	}
	timerInfo = object('user.globalTimerInfo')
	timerInfo.addArray('@watchFileList').reuse()
	event(global(),'onTimeout', @baro.procGlobalTimer)
	// 500ms 마다 파일변경체크
	System.globalTimer(500)
	log("global timer 시작", timerInfo)
	return timerInfo;
}
/* 전역 타이머 중지 */
stopGlobalTimer() {
	global().set('@timerDelay',0)
	System.globalTimer(false)
	log("global timer 중지됨")
}
/* 전역 타이머처리 콜백함수 */	
@baro.procGlobalTimer() {
	timerInfo = object('user.globalTimerInfo')
	if(timerInfo.lock) {
		return;
	}
	while(cur, timerInfo) {
		cur.inject(serviceMode,fullpath,fileName,name,modifyTime)
		// 현재파일 시간과 등록된 시간이 다르다면 파일 변경처리
		if(fileTime(fullpath) != modifyTime) {
			log("$fileName 변경됨 서비스등록 처리")
			loadConfigService(serviceMode,fullpath)
			cur.modifyTime=fileTime(fullpath)
			return;
		} 
	}
	jobList = timerInfo.get('@jobList')
	endCheck = func() {
		if( timerInfo.jobStartTick ) {
			dist = System.tick() - timerInfo.jobStartTick;
			if(dist > 1500 ) {
				job=timerInfo.addArray('@penddingList').pop()
				if(job) {
					not(jobList.find(job)) jobList.add(job)
				}
			}
		}
	};
	if( typeof(jobList,'array')) {
		if(timerInfo.lock) return;
		job = jobList.pop() not(job) return endCheck();
		callback=job.get('@jobCallbackFunc')
		timerInfo.jobStartTick = System.tick()
		if(job.cmp('tag','process') ) {
			status=job.cmp('@status')
			not( job.run() ) {
				print(">> globalTimer : cmd 프로그램이 종료되었습니다")
				return;
			}
			pendding=timerInfo.addArray('@penddingList')
			if( status.eq('stay')) {				
				if(typeof(callback,'func')) {
					call(callback,job,job.ref('cmdResult'))					
				}
				not(pendding.find(job)) {
					job.set('@jobCallbackFunc', null)
				}
				return;				
			} else if( status.eq('start')) {
				not(pendding.find(job)) pendding.add(job)
			} else {
				print(">> globalTimer : $status cmd 상태체크 오류")
			}
		} else if(job.cmp('tag','web') ) {
			url=job.get('@jobUrl')
			result=webResult(job, url)
		} else {
			log('job 태그 미정의 #{0}', job)
		}		
	}
	if( timerInfo.timeoutCheck ) {
		timeoutList = timerInfo.get('@timeoutJobList')
		removeArr=timerInfo.addArray('@removeJobArray').reuse()
		while(cur, timeoutList) {
			cur.inject(startTick, duration)
			not(startTick) {
				removeArr.add(cur)
				continue;
			}
			not(duration) duration=1000
			dist = System.tick() - cur.startTick;
			if(dist>duration ) {
				runTimeoutJob(cur)
			}
		}
	}	
}

addTimeoutJob(node, type, duration) {
	timerInfo = object('user.globalTimerInfo')
	timerInfo.timeoutCheck = true
	node.startTick = System.tick()
}
runTimeoutJob(node) {
	print("run timeout job node==$node")
}

/* 글로벌 타이머에서 실행할 작업 등록 */
addJob(job,callback) {
	timerInfo = object('user.globalTimerInfo')
	timerInfo.lock = true
	if( timerInfo.isset('@jobList') ) {
		arr= timerInfo.get('@jobList')
	} else {
		arr= timerInfo.addArray('@jobList')
	}
	skip=false
	while(cur, arr) {
		if(cur==job) {
			print("@@ jobAdd 중복된 작업이 있습니다 노드:$cur")	
			skip=true
		}
	}
	not(skip) {
		if( typeof(callback,'func')) {
			job.set('@jobCallbackFunc', callback)
		}
		not(arr.find(job)) arr.add(job)
	}
	timerInfo.lock = false
	return job;
}

startFolderWatcher(path, callback) {
	not(isFolder(path)) return print('@@ start watcher $path 경로 미정의')
	if(path.find('/')) {
		path=path.replace('/','\')
	}
	filePathInfo(path).inject(folder,name)
	not(typeof(callback,'func')) {
		callback = @baro.procFolderWatcher
	}
	watcher = System.watcherFile(name, callback)
	watcher.start(path)
}

@baro.procFolderWatcher() {
	args(type, name)
	fn = Cf.funcNode()
	tick= fn.get('prevTick')	
	if(type.eq(3)) {
		if(tick) {
			dist = System.tick() - tick			
			if(dist<100) {
				return
			}
		}
		path = this.target
		print("watcher file name == $name")
		while(curName,this.watcherNames) {
			if(name.find(curName) ) {
				fullpath = Cf.val(path,'/',curName) 
				@baro.loadPage(fullpath)
			}	
		}
		
	}
	fn.set('prevTick', System.tick())
}

@baro.frontendProc(&s) {
	c=s.ch(-1)
	print(">> frontendProc [$c] :: $s")
}

@baro.configFileModifyCheck() {
	args().inject(type,name,check)
	print(">> configFileModifyCheck",type,name,check, this)
}
parseConfigProps(&s) {
	node=_node()
	@baro.parseConfig(findServiceNode(this),node,s)
	return node;
}

@baro.setNodeVersion() {
	@baro.cmdRun(c,'node -v',func(&s) {
		root=object('baro.services')
		s.findPos("\n")
		line=s.findPos("\n")
		root.set('@nodeVersion', line)
	})
}
/*
@baro.findBindPort(5173, func(port) {
	if(port) return print("front 데몬이 실행중입니다")
	print("xxxxxxx 프론트엔드 실행 시작 xxxxxxxxxxx")
	@baro.viteRunDev()
})
*/
@baro.viteCreate(projectPath, projectName) {
	cc=@baro.cmd('npm')
	not(projectPath) projectPath='c:/temp/vite'
	@baro.cmdRun(cc, 'cd c:/temp/vite')
	@baro.cmdRun(cc, 'npm create vite@latest sample-tailwind -- --template react')
	@baro.cmdRun(cc, 'cd sample-tailwind')
	@baro.cmdRun(cc, 'npm install react-router-dom')
	@baro.cmdRun(cc, 'npm install -D tailwindcss@3 postcss autoprefixer')
	@baro.cmdRun(cc, 'npx tailwindcss init -p')
	tailwinConfigFile = 'c:/temp/vite/tailwind.config.js'
	src=fileRead(tailwinConfigFile)
	saveConfig(src)
	saveConfig=func(&s) {
		ss=''
		content='"./index.html", "./src/**/*.{js,ts,jsx,tsx}"'
		left=s.findPos('content:') 
		c=s.ch()
		not(c)return print("@@ tailwinConfig 파일 수정오류 (경로:$tailwinConfigFile)")
		s.match()
		ss.add(left)
		ss.add("content: [$content]")
		ss.add(s)
		fileWrite(tailwinConfigFile,ss)
	};
	@baro.viteRunDev(pathJoin(projectPath,projectName) )
}
@baro.viteRunDev(projectPath, logPath) {
	not(projectPath) projectPath = 'C:/temp/vite/sample-baro1'
	not(logPath) logPath='log.txt'	
	npmCmd = 'npm run dev >> "$logPath" 2>&1'
	cc=@baro.cmd('frontend')
	@baro.cmdRun(cc, "cd $projectPath", @baro.frontendProc )
	@baro.cmdRun(cc, _s(npmCmd) )
	print("xxxxxxx 프론트엔드 실행중 xxxxxxxxxxx")
}

/* ==============================================================
*
* 모듈 공통함수
*
==============================================================*/
@baro.isSingleTag(tag,&s) {
	left=s.findPos('>')
	c=left.ch(-1)
	if(c.eq('/')) return true;
	return false;
}
@baro.isHtmlTag(key) {
	tag=key.lower()
	if(tag.start('h')) {
		c=key.value(1)
		if(typeof(c,'num')) return true;
	} else if(tag.eq('div','form','span','p','img','button','input','video','a','nav','header')) {
		return true;
	}
	return false;
}
@baro.isFunc(&s) {
	s.ch() not(c) return;
	if(c.eq('@')) s.incr()
	c=s.next().ch()
	while(c.eq('-','.')) c=s.incr().next().ch()
	return when(c.eq('('), true)
}
@baro.getClassName(pageNode, &s) {
	ss="#${pageNode.pageId} "
	if( s.find(' ') ) {
		while(s.valid(),n) {
			left = s.findPos(' ').trim() not(left) break;
			ss.add('.',left)
		}		
	} else {
		ss.add(".$s")
	}
	return ss;
}
@baro.findFieldValue(node,field,val) {
	not(typeof(node,'node')) return;
	asize=args().size()
	check=func() {
		if(asize==3) {
			if(cur.cmp(field,val)) return true;
		} else {
			if(cur.isset(field)) return true;
		}
	};
	while(cur, node) {
		if(check()) return cur()
		if(asize==3) 
			sub=@baro.findFieldValue(cur,field,val)
		else 
			sub=@baro.findFieldValue(cur,field)
		if(sub) return sub;
	}
	return;
}
@baro.colorMap(page, param) {
	map=page.addNode('@colorMap')
	idx = 1
	if(param ) {
		a=param.get(0)
		if(typeof(a,'bool') && a) {
			map.removeAll()
		}
		if(typeof(a,'num')) idx=a
	}
	not(map.childCount()) {
		db=Baro.db('data_map')
		not(db.open()) db.open('data_map.db')
		sp=Math.random(0,10).toInt();
		sp*=4;
		sql="select idx, color from color_map order by like_num desc limit $sp, 4"
		root = db.fetchAll(sql)
		while(cur, root) {
			cur.inject(idx, color)
			map.addNode().with(idx,color)
		}
	}
	n=idx-1;
	cur= map.child(idx)
	if(cur) {
		c=cur.color
	} else {
		c=randomColor()
	}
	return "$c"
}
/* ==============================================================
*
* 공통함수
*
==============================================================*/
log(s) {
	fn=Cf.funcNode('parrent')
	param=args(1)
	msg=format(s,fn,this,param)
	print("log>>$msg")
}

tagCheck(obj, type) {
	not(typeof(obj,'node')) return false;
	if(obj.cmp('tag',type)) return true;
	tag=obj.get('@tag')
	chk = tag && tag.start(type)
	return chk;
}
typeName(&s) {
	ss='', upper=false
	while(n=0, s.size()) {
		c=s.ch(n) not(c) break;
		if(c.eq('-')) {
			upper=true
			continue;
		}
		if(upper || n.eq(0)) {	
			ss.add(c.upper())
			upper=false
		} else {
			ss.add(c)
		}
	}
	return ss;
}

parseTemplate(&s, map) {
	print(">> parse template $s $map =========")
}
varEndPos(&s) {
	fn=Cf.funcNode('parent')
	type=''
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq()) break;
		if(c.eq(',')) break;
		if(c.is('oper')) {
			type='oper'
			s.incr()
			continue;
		}
		c=s.next().ch()
		while(c.eq('.')) {
			type='sub'
			c=s.next().ch()
		}
		if(c.eq('(')) {
			type='func'
			s.match()
		}
	}
	fn.set('@endPosType', type)
	return s.cur();
}
trimLine(&s) {
	nl=conf('cf.newline')
	ss=''
	while(s.valid(),n) {
		not(s.ch()) break;
		line=s.findPos("\n").trim() not(line)continue;
		if(n) ss.add(nl)
		ss.add(line)
	}
	return ss;
}
removeIndentText(&s) {
	ss='', firstIndent='', indentSize=-1;
	nl=conf('cf.newline')
	while(s.valid()) {
		if(lineBlankCheck(s)) {
			s.findPos("\n")
			continue;
		}
		line=s.findPos("\n")
		if(indentSize.eq(-1)) {
			firstIndent=indentText(line)
			indentSize=firstIndent.size()
			ss.add(line.trim())
		} else {
			ss.add(nl)
			if(line.start(firstIndent,true)) {
				ss.add(line.trim('right'))
			} else {
				ss.add(line.trim())
			}
		}
	}
	return ss;
}
insertIndentText(&s,indent) {
	if(indent.ch()) return print("@@ insertIndentText 인덴트 추가는 공백만 가능합니다");
	ss='',	nl=conf('cf.newline'), linenum=0;
	print("xxxxxxxx start xxxxxxxx")
	while(s.valid()) {
		if(lineBlankCheck(s)) {
			s.findPos("\n")
			continue;
		}
		if(linenum++) ss.add(nl)
		line=s.findPos("\n").trim('right')
		print(">> ", linenum, line)
		ss.add(indent, line)
	}
	return ss;
}
/* 마지막 0 문자 제거 예) 23.12000 => 23.12 */
trimZero(s) {
	not(s) return;
	last=s.size()-1;
	while(n=last, 0) {
		c=s.ch(n)
		if(c.eq('0')) {
			continue;
		}
		if(n.eq(last)) return s;
		last=n+1
		break;
	}
	return s.trim(0,last,true);
}
replaceFindText(s, replace, value, sep) {
	pos = _find(s)
	if(typeof(pos,'num')) {
		return _replace(s,pos)
	}
	return false;
	
	_replace = func(&str, pos) {		
		if(pos>0) {
			ss=s.value(0,pos,true)
		} else {
			ss=''
		}
		size = replace.size()
		str.pos(pos+size)
		str.findPos(sep,1,1)
		ss.add(value)
		if(str.ch()) ss.add(str)
		return ss;
	};
	_find = func(&str) { 
		while(str.valid()) {
			left = str.findPos(replace,1,1) not(str.valid()) return false;
			print("xxxxxx", replace, str)
			not(left.ch()) return str.cur();
			c=left.ch(-1) not(c) return str.cur();
			if( c.eq(sep) ) {
				return str.cur()
			}
		}
		return false;
	};
}
/* 노드 전체 초기화 (값만 null로 설정) */
nodeReuse(node) {
	not(typeof(node,'node')) return;
	while(k,node.keys(true)) {
		v=node.get(k)
		if(typeof(v,'node')) {
			nodeReuse(v)
		} else if(typeof(v,'array')) {
			v.reuse()
		} else {
			node.set(k,null)
		}
	}
	return node;
}
/* 노드에 이미 등록된 key가 있다면 무시하고 key, value 등록 */
setNodeKeyValue(node,k,v) {
	a=node.get('@keyArray') not(a) a=node.addArray('@keyArray')
	if(a) {
		if(a.find(k)) return;
		a.add(k)
	}
	print('@@ addNodeProp ', k, v, a)
	node.set(k,v)
}
getJsonArray(&s,node,arr,fn,map) {
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',')) {
			s.incr()
			continue;
		}
		if(c.eq('{','[')) {
			src=s.match(1)
			if(c.eq('{')) {
				cur=node.addNode()
				getJsonNode(src,cur,fn,map)
				cur.set('@parentArray', arr)
				arr.add(cur)
				
			} else {
				arrSub=node.addArray()
				getJsonArray(src,node,arrSub,fn,map)
				arr.add(arrSub)
			}
			continue;
		}
		val=''
		if(c.eq()) {
			if(c.eq("'")) {
				val=s.match()
			} else {
				val=fmt(s.match(),fn,map)
			}
		} else if(@baro.isFunc(s)) {
			fnm=s.findPos('(',0,1).trim()
			fparam=s.match()
			val=getVarValue("$fnm($fparam)")
		} else {
			ep=varEndPos(s)
			type=Cf.funcNode().set('@endPosType')
			if(sp<ep) {
				v=s.trim(sp,ep,true)
				if(type) {
					val=getVarValue(v)
				} else {
					if(fn.isset(v)) {
						val=fn.get(v)
					} else if(map.isset(v)) {
						val=map.get(v)
					} else {
						val=configValue(v)
					}
				}
			} else {
				break;
			}
		}
		arr.add(val)
	}
	return arr;
}

getJsonNode(&s,node,fn,map) {
	not(node) node=_node()
	not(fn) fn=Cf.funcNode('parent')
	not(map) map=this
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',')) {
			s.incr()
			continue;
		}
		k=s.move(),val=''
		c=s.ch() 
		not(c.eq(':')) {
			v=k
			if(fn.isset(v)) {
				val=fn.get(v)
			} else if(map.isset(v)) {
				val=map.get(v)
			} else {
				val=configValue(v)
			}
			node.set(k,v)
			continue;
		}
		c=s.incr().ch()
		sp=s.cur()
		if(c.eq('{','[')) {
			src=s.match(1)
			if(c.eq('{')) {
				cur=node.addNode(k)
				getJsonNode(src,cur,fn,map)
			} else {
				arr=node.addArray(k)
				getJsonArray(src,node,arr,fn,map)
			}
			continue;
		}
		if(c.eq()) {
			if(c.eq("'")) {
				val=s.match()
			} else {
				val=fmt(s.match(),fn,map)
			}
		} else if(@baro.isFunc(s)) {
			fnm=s.findPos('(',0,1).trim()
			fparam=s.match()
			val=getVarValue("$fnm($fparam)")
		} else {
			ep=varEndPos(s)
			type=Cf.funcNode().set('@endType')
			if(sp<ep) {
				v=s.trim(sp,ep,true)
				if(type) {
					val=getVarValue(v)
				} else {
					if(fn.isset(v)) {
						val=fn.get(v)
					} else if(map.isset(v)) {
						val=map.get(v)
					} else {
						val=configValue(v)
					}
				}
			} else {
				break;
			}
		}
		node.set(k,val)
	}
	print(">> getJsonNode json:{$node}")
	return node; 
}
configValue(key, root) {
	findService = func(cur) {
		p=cur.parentNode()
		while(isValid(p)) {
			if(p.isset('serviceName')) {
				return p;
			}
			p=p.parentNode()
		}
		return;
	};
	not(root) {
		root=findService(this) not(root) return print("configValue 오류 (service 루트로드를 찾을수 없습니다)");
	}
	return @baro.configKeyValue(root, this, key, 'value');
}
cv(key, rootCode) {
	if(rootCode) {
		root=object('baro.services').addNode(rootCode) 
		return configValue(key, root);
	}
	return configValue(key) 
}


@baro.setPythonPath() {
	userprofile = func(&s) {
		not( s.start('echo %userprofile%', true) ) return;
		not( s.ch()) return;
		userprofilePath=s.findPos("\n").trim()
		if(line) {
			conf('cf.userprofile', userprofilePath.replace('\','/'), true)	
		}
		pythonPath=pathJoin(basePath,'AppData/Local/Programs/Python')		
		node=getFolderList(pythonPath)
		if(node && node.childCount() ) {
			cur=node.child(0)
			path=getNodePath(cur)
			if(path) pythonPath=path
		}
		not(pythonPath ) return print("@@ baro.setPythonPath 파이션경로를 찾을수 없습니다")
		conf('python.path', pythonPath, true)
		log("파이션경로가 설정되었습니다 #{0}", pythonPath)
	};
	run('echo %userprofile%', userprofile)	
}

getParentNode(node, field, value) {	
	p=node.parentNode()
	while(isValid(p)) {
		if(value) {
			if(p.cmp(field,value)) return p;
		} else {
			if(p.isset(field)) return p;
		}
		p=p.parentNode()
	}
	return;
}
getFolderList(path, node, depthNumber, pathLength) {
	asize=args().size()
	if(asize==1 && typeof(path,'node') ) {
		node=path
		path=node.fullPath
	}
	not(node) {
		node=_node()		
	}
	not(path) return print("@@ getFolderList 경로 미설정 노드:$node")
	not(pathLength) {
		root=getParentNode(node,'@rootPath')
		if(root) {
			rootPath = root.get('@rootPath')
			pathLength= rootPath.size()
		} else {
			node.set('@rootPath', path)
			pathLength= path.size()
		}
	}
	not(depthNumber) depthNumber=1
	depthNumber--;
	fileObject = Baro.file('baro')
	fileObject.list(path, func(info) {
		while(info.next()) {			
			info.inject(type,fullPath,name,modifyDt)
			if(type.eq('folder')) {
				relativeName = fullPath.value(pathLength+1)
				cur=node.addNode().with(type, relativeName, name, fullPath, modifyDt)
				if(depthNumber>0 ) {
					cur.childLoad = true
					getFolderList(fullPath, cur, depthNumber, pathLength)
				} else {
					cur.childLoad = false
				}
			}
		}
	});
	return node;
}

getFileList(path, node ) {
	arr=_arr()
	relativePath = node.relativeName
	not(path) {
		not(relativePath) return print("@@ getFileList 부모경로가 없습니다")
		root=getParentNode(node,'@rootPath')
		not(root) return print("@@ getFileList 최상위 노드 경로가 없습니다")
		path=pathJoin(root.get('@rootPath'), relativePath)
		print("path == $path ")
	}
	fileObject = Baro.file('baro')
	fileObject.list(path, func(info) {
		while(info.next()) {			
			info.inject(type,fullPath,name,modifyDt)
			if(type.eq('file')) {				
				cur=node.addNode().with(type, name, fullPath, modifyDt)
				if( relativePath ) {
					cur.relativeName = pathJoin(relativePath, name)
				}
				arr.add(cur)
			}
		}
	});
	return arr;
}
