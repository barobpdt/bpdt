/* ==============================================================
*
* 모듈 wrap 함수 (기존 등록된 모듈을 실행하기위한 처리함수)
*
==============================================================*/
initService() {
	include('classes/common/baro-services')
	getServiceNode('baro', 'apps')
	getServiceNode('baro', 'tools')
	// conf('cf.useWatch', true, true)
	// conf('cf.useDebug', true, true)
	initModules()
	initConfig()
	Cf.debug(true,"data/logs")
	include("@apps#SourceRun")
	startGlobalTimer()
}
closeApp() {
	if(checkFunc('closePythonCommand')) {
		closePythonCommand()
		System.sleep(100)
	}
	Cf.debug(false)
	Cf.exit()
}
initWas() {
	was().start(80, conf('web.rootPath'))
}
includeService(serviceMode, fullPath, reset, evalAll) {
	ext=right(fullPath,'.')
	if(ext.eq('html')) {
		if(typeof(serviceMode,'node')) {
			service=serviceMode.serviceName
		} else {
			service=serviceMode
		}
		include("$service#$fullPath")
	} else if(ext.eq('js')) {
		if(typeof(serviceMode,'node')) {
			serviceNode=serviceMode
		} else {
			serviceNode=getServiceNode(serviceMode, true)	
		}		
		not(typeof(serviceNode,'node')) {
			return print("@@ 설정 로드오류 [$serviceMode]에 등록된 정보가 없습니다 (경로: $path)") 
		}
		not(isFile(fullPath)) {
			return print("@@ includeService 오류 파일 $fullPath 경로가 없습니다");
		}
		src=fileRead(fullPath)
		not(src) {
			return print("@@ includeService 오류 설정소스가 없습니다");
		}
		// print(">> loadService 시작", serviceNode, fullPath )
		root=@baro.loadService(serviceNode, src, reset, evalAll)
		filePathInfo(fullPath).inject(folder, filename)		
		root.set('@currentFileName',filename)
		return root;
	} else {
		print("@@ include service $fullPath 파일무시")
	}
}

initModules() {
	serviceNode=getServiceNode('baro', 'modules')
	path = conf('path.modules')
	not(isFolder(path)) {
		print("initModules : $modulePath 모듈경로 미정의")
		return;
	}
	regServiceFolder(serviceNode, path )
}
regServiceFile(serviceMode, fullPath) {
	serviceNode=getServiceNode(serviceMode)
	callback = func(serviceNode, fullpath) {
		return includeService(serviceNode, fullpath)
	};
	addWatchFile(serviceNode, fullPath, callback)
}

regServiceFolder(serviceMode, path) {
	serviceNode=getServiceNode(serviceMode)
	not(typeof(serviceNode,'node')) {
		return print("@@ 서비스 등록 오류 [$serviceMode]에 등록된 정보가 없습니다 (경로: $path)") 
	}
	service=serviceNode.serviceName
	not(service) {
		return print("@@ 서비스 등록 오류 서비스명 미정의 (노드: $serviceNode)")
	}
 
	while(cur, getFileList(path)) {
		cur.inject(name, fullPath)
		includeService(serviceNode, fullPath)
		regServiceFile(serviceNode, fullPath)	
	}
}

run() {
	not(typeof(@baro.cmd,'func')) {
		include('classes/common/baro-cmd')
	}
	fn=Cf.funcNode('parent')
	getCommand=func(&s) {
		not( s.find('${') ) return s;
		return str(s, fn);
	};
	asize=args().size()	
	switch(asize) {
	case 0:
		return @baro.cmd('@run');
	case 1: 
		args(a)
		if( tagCheck(a,'process')) {			
			if( a.run() && a.cmp('@status','start') ) {
				return true;
			}
			return false;
		}
		cmdCurrent = @baro.cmdRun('@run',getCommand(a))
	case 2: 
		args(a,b)
		if(typeof(b,'func')) {
			cmdCurrent = @baro.cmdRun(getCommand(a),b)
		} else {
			if(typeof(a,'string')) {
				cmd=@baro.cmd(a)
			} else {
				cmd=a
			}
			cmdCurrent = @baro.cmdRun(cmd,getCommand(b))
		}
	case 3:
		args(a,b,c)
		if(typeof(a,'string')) {
			cmd=@baro.cmd(a)
		} else {
			cmd=a
		}
		cmdCurrent = @baro.cmdRun(cmd,getCommand(b),c)
	default:
	}
	return cmdCurrent;
}
runPython() {
	py=pathJoin( conf('python.path'), 'python.exe')
	not(isFile(py)) {
		return log('error::파이션이 설치되지 않았거나 파이션경로가 설정되지 않았습니다')
	}
	runFunc=call('run')
	self=Cf.funcNode().get('@this') not(self) self=_node()	 
	asize=args().size() not(asize) return py;
	callback=null
	if(asize==1) {
		args(command)
		id='@python'
	} else if(asize==2) {
		args(a,b)
		if(typeof(b,'func')) {
			id='@python'
			command=a
			callback=b
		} else {
			id=a, command=b
		}
	} else if(asize==3) {
		args(id,command,callback)
	}
	params=_arr()
	params.add(id,"$py $command")
	if(typeof(callback,'func')) params.add(callback)
	print(">> runPython params => $params")
	return call(runFunc, self, params);
}

/* 웹호출 결과 출력 (api 호출) */
webResult(url, method, data, headerJson) {
	web=webObject()
	not(method) method='GET'
	if(method=='POST') {
		web.set('data',data)
	}		
	if(headerJson && typeof(headerJson,'string')) {
		header=web.addNode('@header').reuse()
		header.parseJson(headerJson)
	}
	web.call(url,method, func(type,data) {
		if(type=='error') return log('error::webResult 오류 객체:#{0} 메시지#{1}', this, data);
		if(type=='read') this.appendText('@webResult', data)
	})
	return web.ref('@webResult')
} 
was(port,path,devMode) {
	obj = object("baro.was")
	if(obj.var(useModule)) return obj;
	include('classes/common/was.js')
	addModule(obj,'@was',devMode)
	if(port && path ) {
		obj.start(port,path)
	}
	return obj;
}
json(node, childPrefix, useIndent) {
	obj = object("baro.json")
	not(obj.var(useModule)) {
		include('classes/common/json.js')
		addModule(obj,'json',childPrefix)
	}
	not(node) {
		return obj;
	}
	if(typeof(childPrefix,'bool')) {
		not(childPrefix) obj.member(childPrefix, '')
	} 
	else if(childPrefix) {
		obj.member(childPrefix, childPrefix)
	}  
	obj.member(useIndent, useIndent)
	return obj.jsonValue(node)
}

/* 파일감시 경로를 추가한다 */
addWatchFile(serviceMode, fullpath, callback) {
	serviceNode=getServiceNode(serviceMode, true)	
	not(typeof(serviceNode,'node')) {
		return print("@@ 파일감시 경로추가 오류 [$serviceMode]에 등록된 정보가 없습니다 (경로: $path)") 
	}
	not(typeof(callback,'func')) {
		callback=func(serviceNode, fullpath) { 
			print("@watch 파일변경 $fullpath (서비스:${serviceNode.serviceName})") 
		}
	}
	timerInfo = object('baro.globalTimerInfo')
	timerInfo.useWatchFile = true	
	filePathInfo(fullpath).inject(folder, filename, name)
	modifyTime=fileTime(fullpath)
	cur=timerInfo.addNode('@watchFileInfo').addNode(fullpath)
	cur.with(serviceNode,fullpath,filename,name,modifyTime, callback)
	return cur;
}

/* API 결과처리 워커등록 */
webRequestInfo(url, method, data, header) {
	not(method) method='GET'
	ss=str('url:"$url", method:"$method"')
	if(data||header) {
		node=_node()
		if(data) {
			data=parseJson(node,data,true)
			ss.add("@data $data")
		}
		if(header) {
			header=parseJson(node,header,true)
			ss.add("@header:$header")
		}
	}
	return ss;
}
/* API 결과처리 워커등록 */
addApiWorker(id, targetNode, logCallback ) {
	not(typeof(targetNode,'node')) {
		return print("@@addApiWorker 대상노드 미정의");
	}	
	timerInfo = object('baro.globalTimerInfo')
	timerInfo.useWorker = true
	timerInfo.lock=true
	web=webObject(id)
	if(typeof(logCallback,'func')) {
		web.logCallback=logCallback
	} else {
		not(web.logCallback) {
			web.logCallback=func(result,info) { print("##apiWorker callback ${this.url}  result::$result") };
		}
	}
	not(web.workerMode) {
		web.workerMode = 'ApiWorker'
		web.logTail = logTail("workerapi_$id")
		web.logAppend = logAppend("workerapi_$id")
		event(web,'@callback',@baro.workerWebProc)
	}	
	// requestInfo: url, method, data, headerJson
	nodeArrayVar(web,'@workerJobList').add(targetNode)
	addArrayVar(timerInfo,'@workerList',web)
	timerInfo.lock=false
	return web;
}

/* 다운로드처리 워커등록 */
addWebDownloadWorker(id, targetNode, logCallback ) {
	not(typeof(targetNode,'node')) {
		return print("@@addWebDownloadWorker 대상노드 미정의");
	}
	timerInfo = object('baro.globalTimerInfo')
	timerInfo.useWorker = true
	timerInfo.lock=true
	wid="download_$id"
	web=webObject(wid)
	if(typeof(logCallback,'func')) {
		web.logCallback=logCallback
	} else {
		not(web.logCallback) {
			web.logCallback=func(result,info) { print("##apiWorker callback ${this.url}  result::$result") };
		}
	}
	not(web.workerMode) {
		web.workerMode = 'DownloadWorker'
		web.logTail = logTail(wid)
		web.logAppend = logAppend(wid)
		event(web,'@callback',@baro.workerWebProc)
	}
	// requestInfo: url, method, data, headerJson
	nodeArrayVar(web,'@workerJobList').add(targetNode)
	addArrayVar(timerInfo,'@workerList',web)
	timerInfo.lock=false
	return web;
}

/* cmd 결과처리 워커등록 */
addCmdWorker(id, command, logCallback) {	
	timerInfo = object('baro.globalTimerInfo')
	timerInfo.useWorker=true
	timerInfo.lock=true
	cmd=Baro.process(id)
	if(notValid(command)) {
		return cmd;
	}
	if(typeof(logCallback,'func')) {
		cmd.logCallback=logCallback
	} else {
		not(cmd.logCallback) {
			cmd.logCallback = func(result, info) { print("##cmdWorker callback result::$result") };
		}
	}
	not(cmd.workerMode) {
		cmd.workerMode='CmdWorker'
		cmd.logTail = logTail("workercmd_$id")
		cmd.logAppend = logAppend("workercmd_$id")
	}
	
	if(typeof(command,'node')) {
		node=command
		node.lastCommand = ''
		if(typeof(node.commandList,'array')) {
			if(isValid(node.commandList)) {
				node.lastCommand = node.commandList.get(-1)
			} else {
				node.error='error::addCmdWorker commandList 내용이 없습니다')
			}
		} else {
			not(node.command) {
				node.error='error::addCmdWorker command 내용이 없습니다')
			}
		}
		not(node.error) {
			nodeArrayVar(cmd,'@workerJobList').add(node)
		}
	} else {
		if(command) {
			nodeArrayVar(cmd,'@workerJobList').add(command)
		} else {
			log('error::addCmdWorker command 내용이 없습니다')
		}
	}
	addArrayVar(timerInfo,'@workerList',cmd)
	timerInfo.lock=false
	return cmd;
}

@baro.workerWebProc(type,&data,bytes) {
	if(type=='progress') {
		n=this.incrNum('@progressCnt')
		m=n%10;
		if(m==1) this.logAppend.append("##progress ${n}th ${this.url} ${bytes}")
		return;
	}
	if(type=='error') {
		this.set('@error',data)
	}
	if(type=='finish') {
		this.inject(url, targetNode, @error)
		msg=str('@#>finish: web ${error?[error="$error" ]}url="$url" target="$targetNode"')
		this.logAppend.append("\r\n$msg\r\n")
		this.logCallback(this.logTail.timeout(), targetNode)
		this.set('@progressCnt', 0)
		this.set('@workerStatus','finish')
	}
}
@baro.workerCmdProc(type,&data) {
	if(type=='read') {
		this.appendText('@cmdResult', data)
		this.inject(id,logAppend,logCallback,@command,@workerInfo)
		logAppend.write(data)
		c=data.ch(-1)
		if(workerInfo && typeof(workerInfo.logCallback,'func')) {
			logCallback=workerInfo.logCallback
		} 
		if(c=='>') {
			this.set('@workerStatus','finish')
			msg=str('@#>finish: cmd id="$id" command="$command"')
			logAppend.append("\r\n$msg\r\n")
			logData=this.ref('@cmdResult')
			if( workerInfo) {
				workerInfo.inject(command, commandList)
				if(typeof(commandList,'array')) {
					not(commandList.size()) this.set('@workerInfo', null)
				} else {
					this.set('@workerInfo', null)
				}
				call(logCallback,this,logData,workerInfo)
			} else {
				call(logCallback,this,logData)
			}
			this.set('@cmdResult','')
		} else {
			call(logCallback,this,data,true)
		}
		return;
	}
	if(type=='error') {
		this.inject(id,@command)
		msg=str('##error>> cmd error="$data" id="$id" command="$command"')
		this.logAppend.apeend("\r\n$msg\r\n")
		return this.var(error, data)
	}
}


/* 사용자 워커등록 */
addUserWorker(id, targetNode, callbackWorker ) {
	not(typeof(targetNode,'node')) {
		return print("@@addWebDownloadWorker 대상노드 미정의");
	}
	worker = Baro.worker(id)
	workerCallback = worker.get('@callback')
	not(typeof(workerCallback,'func')) 
	{
		worker.start(func(node) {
			not(typeof(node,'node')) {
				print("user worker stay mode !!!")
				return;
			}
			callback = node.callbackWorker
			if(typeof(callback,'func')) {
				callback(node)
			} else {
				print("@@ userWorekr 콜백함수 미정의 (노드:$node)")
			}
		});
	}
	
	if(typeof(callbackWorker,'func')) {
		targetNode.callbackWorker=callbackWorker
	} else {
		not(targetNode.callbackWorker) {
			targetNode.callbackWorker=func(node) { print("##userWorker callback  node::$node") };
		}
	}
	worker.push(targetNode)
	return worker;
}

/* 전역 타이머 실행 */
startGlobalTimer() {
	if( global().var(@timerDelay) ) {
		return print('global timer가 실행중입니다', global().var(@timerDelay) )
	}
	event(global(),'onTimeout', @baro.procGlobalTimer)
	// 500ms 마다 타이머 실행
	System.globalTimer(500)
	timerInfo = object('baro.globalTimerInfo')
	timerInfo.var(@startTick, System.tick())
	print("global timer 시작", timerInfo)
	return timerInfo;
}
/* 전역 타이머 중지 */
stopGlobalTimer(workerClose) {
	timerInfo = object('baro.globalTimerInfo')
	global().var(@timerDelay,0)
	System.globalTimer(false)
	print("global timer 중지됨")
	if(workerClose) {
		watcher=global().var(@watcherFiles)
		if( isObject(watcher) ) {
			while( cur, watcher) {
				cur.set('@callback', null)
			}
		}
		while(worker, timerInfo.var(@workerList) ) {
			if( tagCheck(worker,'process')) {
				worker.set('@workerStatus','stop')
				worker.get('@workerJobList').reuse()
				worker.logTail.closeLog(true)
				worker.logAppend.closeLog(true)
				worker.close()
			}
			else if( tagCheck(worker,'process')) {
				worker.stop()
			}
		}
	}
}
/* 전역 타이머처리 콜백함수 */	
@baro.procGlobalTimer() {
	timerInfo = object('baro.globalTimerInfo')
	if(timerInfo.lock) {
		if( timerInfo.lockCheckTick ) {
			timerInfo.lockCheckTick=0
			timerInfo.lock=false
		} else {
			timerInfo.lockCheckTick=System.tick()
		}
		return;
	}
	if(timerInfo.useWatchFile) {
		while(cur, timerInfo.get('@watchFileInfo') ) {
			cur.inject(serviceNode,fullpath, modifyTime, callback)
			// 현재파일 시간과 등록된 시간이 다르다면 파일 변경처리
			if(fileTime(fullpath) != modifyTime) {
				log("$fullpath 파일 변경됨")
				callback(serviceNode,fullpath)
				cur.modifyTime=fileTime(fullpath)
				return;
			} 
		}
	}
	if(timerInfo.useAppCommand) {
		command = timerInfo.get('@appCommandList').pop()
		if( command ) {
			print(">> globalTimer appCommand==$command")
			runGlobalAppCommand(command)
			return;
		}
	}
	if( timerInfo.useWorker ) {
		while(cur, timerInfo.get('@workerList')) {
			runGlobalWorker(cur)
		}
	}
}

/* 폴더변경 감시 시작 */
startFolderWatcher(path, callback) {
	not(isFolder(path)) return print('@@ startFolderWatcher [$path] 경로오류')
	if(path.find('/')) {
		path=path.replace('/','\')
	}
	filePathInfo(path).inject(folder,name)
	watcher = System.watcherFile(name, @baro.procFolderWatcher)
	watcher.start(path)
	watcherInfo=global().addNode('@watcherFiles').get(name)
	not(typeof(watcherInfo,'node')) {
		return print("@@ startFolderWatcher [$name] 등록오류")
	}
	nodeVar(watcherInfo,'@status', 'start')
	if(typeof(callback,'func')) {
		watcherInfo.watcherCallback = callback
	}
}
/* 폴더변경 감시 기본콜백 함수 */
@baro.procFolderWatcher() {
	args(type, name)
	if( nodeVar(this,'@status').eq('ready','stop') ) {
		return;
	}
	fn = Cf.funcNode()
	tick= fn.get('prevTick')
	this.inject(code, target, watcherCallback)
	if(type.eq(3)) {
		if(tick) {
			dist = System.tick() - tick			
			if(dist<100) {
				return
			}
		}		
		type='modify'
	}
	if(type.eq(2)) {
		type='delete'
	}
	fullpath = pathJoin( target, name)
	if(typeof(watcherCallback,'func')) {
		watcherCallback(type, fullpath)
	} else {
		log("watcher changed : $type, $fullpath")
	}
	fn.set('prevTick', System.tick())
}

runSource(&src, node) {
	if(typeof(src,'bool')) {
		return print("@@ evalValue 실행오류 괄포매칭 오류")
	}
	fn=Cf.funcNode()
	if(typeof(node,'node')) {
		fn.set('@this',node)
	}
	if(src.start('@eval=>',true)) {
		root=findParentNode(fn.get('@this'),'seriveName')
		log("${root.serviceName} 소스실행 시작 =====")
	}
	return eval(stripComment(src), fn)
}
getSource(&s,node,fn) {
	not(typeof(node,'node')) {
		node=this
		not(node) node=_node()
	}
	not(fn) {
		fn=Cf.funcNode('parent')
	}
	fnCur=Cf.funcNode()
	fnCur.set('endPos', 0)
	ss=''
	while(s.valid()) {
		left = s.findPos('@[',0,1)
		ss.add(left)
		c=s.incr().ch()
		not(c.eq('[')) break;
		key=s.match(1).trim()
		if(typeof(key,'bool')) break;
		if(isVar(key)) {
			ss.add(getValue(key,node,fn))
		} else {
			ss.add(eval(key, fn))
		}
	}
	return ss;
	
	isVar = func(&s) {
		c=s.next().ch() 
		while(c.eq('.')) c=s.next().ch()
		fnCur.set('endPos', s.cur())
		not(c) return true;	
		if(startWith(s,'&&','||','?')) return true;
		return when(c,false,true);
	};	
	getValue=func(&s,node,fn) {
		not(typeof(s,'string')) return;
		if( s.start('conf.',true)) {
			return conf(s)
		}
		sp=s.cur()
		key=s.trim(sp,endPos,true)
		val=getVarValue(key,fn,node)
		if(isNull(val)) {
			val=configValue(key, node)
		}
		s.pos(endPos)
		if(startWith(s,'&&','||','?')) {
			c=s.ch()
			if(c.eq('?')) {
				c=s.incr().ch()
				if(c.eq('(')) {
					src=s.match(1)
				} else {
					src=s.findPos(':',1,1)
				}
				if(val) {
					val=getSource(src,node,fn)
				} else {
					c=s.ch()
					if(c.eq(':')) {
						c=s.incr().ch()
						if(c.eq('(')) {
							src=s.match(1)
						} else {
							src=s
						}
						val=getSource(src,node,fn)
					} else {
						val=''
					}
				}
			} 
			else if(s.start('&&',true)) {
				if(val) {
					val=getSource(s,node,fn)
				} else {
					val=''
				}
			} else if(s.start('||',true)) {
				if(isNull(val)) {
					val=getSource(s,node,fn)
				}
			}
		}
		return val;
	};
}

@baro.workerDownPath(obj) {
	not(typeof(obj,'node')) {
		return log("error::workerDownPath 객체오류");
	}
	obj.inject(tag,id)
	// path='C:/temp/worker'
	path=pathJoin(System.path(),'data/worker')
	not(isFolder(path)) Baro.file().mkdir(path)
	not(tag) tag='object'
	not(id) id='common'
	idx=obj.incrNum('@downloadIndex')
	return pathJoin(path, str('$tag-$id-$idx.data'));
}

runGlobalWorker(node) {
	not(typeof(node,'node')) {
		return log("error::runGlobalWorker 대상노드가 없습니다");
	}
	node.inject(@workerStatus, @workerJobList, workerMode)
	not(workerStatus) workerStatus='stay'
	if(tagCheck(node,'process')) {
		cmd=node
		not(cmd.run()) {
			cmd.set('@workerStatus', 'stay')
			cmd.run('cmd', @baro.workerCmdProc)
			return;
		}
		if( workerStatus.eq('ready','finish')) {
			if(isValid(workerJobList)) {
				jobFinish = true
				info=workerJobList.get(0)
				if(typeof(info,'node')) {
					info.inject(type, command, commandList)
					if(typeof(commandList,'array') ) {
						command=commandList.pop()
						if( commandList.size()) {
							jobFinish = false
						}
					}
					cmd.set('@workerInfo', info)
				} else {
					if(info.find('=>')) {
						splitSep(info,'=>').inject(type,command)
					} else {
						type=''
						command=info
					}
					cmd.set('@workerInfo', null)
				}
				if(jobFinish) {
					workerJobList.pop()
				}
				if(command) {
					cmd.set('@workerStatus', 'run')
					cmd.set('@cmdStartTick', System.tick())
					cmd.set('@workerType', type)
					cmd.set('@command', command)
					cmd.write(command)
					print(">> worker cmd next : $command")
				}
			}
		}
		return;
	} 
	callDownload = func(targetNode, defaultHeader) {
		targetNode.inject(url, method, data, header, downloadFileName)
		targetNode.requestStartTick=System.tick()
		not(downloadFileName) {
			downloadFileName=@baro.workerDownPath(web)
		}
		web.data=''
		if( defaultHeader && ~(header)) {
			header=defaultHeader
		}
		parseJsonNode(web,webRequestInfo(url, method, data, header))
		web.set('@webStartTick', System.tick())
		web.set('@workerStatus', 'ready')
		web.targetNode=targetNode
		web.download(url,downloadFileName)
	};
	if(tagCheck(node,'web')) {
		web=node
		if( web.is('start')) {
			web.set('@workerStatus', 'run')
		} else {
			targetNode=workerJobList.get(0) not(targetNode ) return;
			if(targetNode.childCount()) {
				num=0, header=targetNode.get('header')
				while(cur, targetNode) {
					if(cur.requestStartTick) continue;
					callDownload(cur,header)
					num++;
				}
				not(num) workerJobList.pop()
			} else {
				callDownload(targetNode)
				workerJobList.pop()
			}
		}
	}
	
}
/*
	공통 어플리케이션 커멘드 실행
	command 형태
		@@>커멘드타입: 수행내용
*/
runGlobalAppCommand(&s) {
	comment=''
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq('/')) {
			c=s.ch(1)
			if(c.eq('/')) cmt=s.findPos("\n") else cmt=s.match();
			comment.add(cmt.trim())
			continue;
		}
		s.findPos('@@>')		
		type=s.findPos(':').trim()
		line=s.findPos("\n")
		if(type.eq('useCmd')) {
			id=getParam()
			cmd=getParam()
			not(cmd) cmd='cd'
			addCmdWorker(id, cmd)
		}
	}
	
	getParam = func() {
		c=line.ch() not(c) return;
		if(c.eq()) {
			return line.match();
		} else {
			val= line.findPos(" ,\t\n",4).trim()
			return val;
		}
	};
}
addGlobalAppCommand(command) {
	timerInfo = object('baro.globalTimerInfo')
	timerInfo.useAppCommand=true
	nodeArrayVar(timerInfo,'@appCommandList').add(command)
}

/*
service config
*/
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
convertSource(&s) {
	ss='', nl=conf('cf.newline')
	endPos = func() {
		c=s.ch()
		while(c.eq('.')) {
			c=s.incr().next().ch()
			if(c.eq('(')) {
				s.match(1)
				c=s.ch()
			}
		}
		return s.cur()
	};
	while(s.valid()) {
		left=s.findPos('(',1,1)
		line=''
		if( isMultiFunc(left, s) ) {
			line.ref()
			if(line.find('=')) {
				vnm=line.findPos('=').trim()
				fnm=line.trim()
			} else {
				vnm=''
				fnm=line.trim()
			}
			param=s.match(1)
			sp=s.cur()
			ss.add(nl,indent,"_tempVar=${fnm}(${param})",nl,indent)
			if(vnm) ss.add("$vnm=")
			ss.add("_tempVar")
			ep=endPos()
			if(sp<ep) {
				ss.add(s.value(sp,ep,true))
			} else {
				print("@@ conver source 오류 ", sp, ep)
			}			
		} else {			
			sp=s.cur()
			v=s.match(1) if(typeof(v,'bool')) break;
			funcType=line.trim()
			if(funcType.eq('if','whild')) {				
				b=v
				b.findPos('(',0,1)
				if( isMultiFunc(null, b) ) {
					fnm=v.findPos('(',0,1)
					param=v.match(1)
					ss.add(nl,"_tempVar=${fnm}(${param})")
					ss.add(nl, funcType, "(_tempVar" )
					sp=v.cur()
					c=v.ch()
					while(c.eq('.')) {
						c=v.incr().next().ch()
						if(c.eq('(')) {
							v.match(1)
							c=v.ch()
						}
					}
					ep=v.cur()
					ss.add(v.value(sp,ep,true))
					ss.add(") ")					
					continue;
				}
			}
			ss.add(left)
			ep=endPos()
			ss.add(s.value(sp,ep,true))
		}
		not(s.valid()) break;
	}
	return ss;
	
	isMultiFunc = func(a,b) {
		ok=false;
		left=''
		if(a) {			
			if(a.find("\n")) {
				left=a.findLast("\n")
				k=left.right()
			} else {				
				k=a
			}
			line.add(k)
		}
		not(b.ch()) return;
		v=b.match(1) if(typeof(v,'bool')) return;
		c=b.ch()
		if(b.ch('.')) {
			if(v.find(',')) {
				ok=true
			}
		}
		if(ok && left ) {
			ss.add(left)
		}
		return ok;
	};
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
			if(firstIndent) {
				if(line.start(firstIndent,true)) {
					ss.add(line.trim('right'))
				} else {
					ss.add(line.trim())
				}
			} else {
				ss.add(line.trim('right'))
			}
		}
	}
	return ss;
}
insertIndentText(&s,indent) {
	if(indent.ch()) return print("@@ insertIndentText 인덴트 추가는 공백만 가능합니다");
	ss='',	nl=conf('cf.newline'), linenum=0;
	while(s.valid()) {
		if(lineBlankCheck(s)) {
			s.findPos("\n")
			continue;
		}
		if(linenum++) ss.add(nl)
		line=s.findPos("\n").trim('right')
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
configValueEndPos(&s) {
	c=s.ch()
	if(c.eq('=',':')) c=s.incr().ch()
	if(c.eq()) s.match()
	else if(c.eq('{','[')) s.match(1)
	else if(c.eq('<')) {
		if(s.start('<>')) {
			s.match('<>','</>')
		} else {
			sp=s.cur()
			c=s.incr().next().ch()
			if(c.eq('-','.')) c=s.incr().next().ch()
			ep=s.cur()
			tag=s.trim(sp+1,ep,true)
			s.pos(sp)
			s.match("<$tag","</$tag>")
		}
	} 
	else if(isFunc(s)) {
		s.findPos('(',0,1)
		s.match()
		c=s.ch()
		if(c.eq('{')) {
			s.match(1)
		}
	}
	else {
		s.findPos(" ,\t\n")
	}
	return s.cur();
}
/* 문자열중 찾는 문자가 있다면 속성까지 모드 바꾸줜다 
	예) s: width=100, height=50
		replace:	width
		value:		width=500
		결과: width=500, height=50
*/

nodeAppendText(node,key,value,sep,replace) {
	ss=node.get(key)
	if(replace && ss ) {
		result = replaceConfigText(ss,replace,value,sep)
		if(result) {
			node.set(key,result)
			return;
		}
	}
	if(sep) {
		if(ss) node.appendText(key,sep)
	}
	node.appendText(key,value)
}
replaceConfigText(s, replace, value, sep) {
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
		if(sep) {
			str.findPos(sep,1,1)
		} else {
			sp=s.cur()
			ep=configValueEndPos(s)
			if(sp<ep) {
				str.pos(ep)
			}
		}
		ss.add(value)
		if(str.ch()) ss.add(str)
		return ss;
	};
	_find = func(&str) { 
		while(str.valid()) {
			left = str.findPos(replace,1,1) not(str.valid()) return false;			
			c=left.ch(-1)
			if(c.is('alphanum')) continue;
			c=str.ch(1)
			if(c.is('alphanum')) continue;
			return s.cur();
		}
		return false;
	};
}
randomColor() {
	hue=System.rand(360).toInt(); 
	return Baro.color('hsl', hue, 100, 100);
}
randomIcon() {
	num=System.rand(360).toInt();
	Baro.db('icons').fetch("select type, id from icons where type='vicon' limit $num,1"  ).inject( type, id);
	return "$type.$id";
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
/* array 값을 json 형태 문자열로 생성(함수/태그 등 확장기능포함) */
parseJsonArray(node,&s,arr,fn,map) {
	fnCur=Cf.funcNode()
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
				parseJsonNode(cur,src,fn,map)
				cur.set('@parentArray', arr)
				arr.add(cur)
				
			} else {
				arrSub=node.addArray()
				parseJsonArray(node,src,arrSub,fn,map)
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
			if(sp<ep) {
				v=s.trim(sp,ep,true)
				if(typeof(v,'num')) {
					val=v
				}
				else if(fnCur.get('@endPosType')) {
					val=getVarValue(v)
				} 
				else {
					if(fn.isset(v)) {
						val=fn.get(v)
					} else if(map.isVar(v)) {
						val=map.get(v)
					} else {
						val=cv(v)
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
/* node 값을 json 형태 문자열로 생성(함수/태그 등 확장기능포함) */
parseJsonNode(node,&s,fn,map) {
	fnCur=Cf.funcNode()
	not(node) node=_node()
	not(fn) fn=Cf.funcNode('parent')
	not(map) {
		map=fnCur.get('@this')
		not(map) map=node
	}
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',')) {
			s.incr()
			continue;
		}
		bref=false
		if(c.eq()) {
			k=s.match()
			c=s.ch()
		} else {
			sp=s.cur()
			if(c.eq('@')) {
				bref=true
				s.incr()
			}
			c=s.next().ch()
			k=s.trim(sp,s.cur(),true)
		}
		if( bref && c.eq('{')) {
			ss=s.match(1)
			sub=parseJsonNode(_node(),ss,fn,map)
			node.set(k.trim(1), json(sub,'data'))
			continue;
		}
		not(c.eq(':')) {
			if(fn.isset(k)) {
				val=fn.get(k)
			} else if(map.isVar(k)) {
				val=map.get(k)
			}
			node.set(k,val)
			break;
		}
		c=s.incr().ch()
		if(c.eq('{','[')) {
			src=s.match(1)
			if(c.eq('{')) {
				cur=node.addNode(k)
				cur.removeAll(true)
				parseJsonNode(cur,src,fn,map)
			} else {
				arr=node.addArray(k)
				arr.reuse()
				parseJsonArray(node,src,arr,fn,map)
			}
			continue;
		}
		sp=s.cur()
		if(c.eq()) {
			if(c.eq("'")) {
				val=s.match()
			} else {
				val=str(s.match(),fn,map)
			}
		} else if(@baro.isFunc(s)) {
			fnm=s.findPos('(',0,1).trim()
			fparam=s.match()
			val=getVarValue("$fnm($fparam)")
		} else {
			ep=varEndPos(s) 
			if(sp<ep) {
				v=s.trim(sp,ep,true)
				if(typeof(v,'num')) {
					val=v
				}
				else if(fnCur.get('@endType')) {
					val=getVarValue(v)
				} 
				else {
					if(fn.isset(v)) {
						val=fn.get(v)
					} else if(map.isVar(v)) {
						val=map.get(v)
					} else {
						val=cv(v)
					}
				}
				s.pos(ep)
			} else {
				break;
			}
		}
		node.set(k,val)
	}
	print(">> parseJsonNode json:{$node}")
	return node; 
}

/* Config Value 설정값 리턴 (설정 serviceNode 가 없다면 현재노드 기준 부모 노드를 찾는다)*/
configValue(key, self, serviceNode) {
	not(self) return print("configValue 대상노드 미정의 [키:$key]");
	not(serviceNode) {
		serviceNode=findParentNode(self,'serviceName') 
		not(serviceNode) {
			// return print("configValue 오류 (service 루트로드를 찾을수 없습니다)");
			return;
		}
	}
	return @baro.configKeyValue(serviceNode, self, key, 'value');
}

/* Config Value 처리 wrap 함수 */
cv(key) {
	if(key.find(':')) {
		splitSep(key,':').inject(rootCode,key)
	}
	if(rootCode) {
		serviceNode=getServiceNode(rootCode)
		not(serviceNode) return print("cv 오류 (service 루트로드를 찾을수 없습니다)");
		return configValue(key, this, serviceNode);
	} else {
		return configValue(key, this) 
	}
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
		root=findParentNode(node,'@rootPath')
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
	node.childLoad = true
	return node;
}

getFileList(path, node, arr, depth ) {
	not(node) node=_node()
	if(typeof(arr,'array')) {
		checkChild= true
	} else {
		checkChild = false
		arr=_arr()
	}
	relativePath = node.relativeName
	not(path) {
		not(relativePath) return print("@@ getFileList 경로가 없거나 부모노드 경로가 없습니다")
		root=findParentNode(node,'@rootPath')
		not(root) return print("@@ getFileList 최상위 노드 경로가 없습니다")
		path=pathJoin(root.get('@rootPath'), relativePath)
		print("path == $path ")
	}
	not(depth) {
		fileObject = Baro.file('baro')
		node.inject(@fileSort, @fileFilter, @nameFilter)
		if(fileSort) {
			// name, time, size, type, case
			fileObject.var(sort, fileSort)
		}
		if(fileFilter) {
			// folder, files, hidden
			fileObject.var(filter, fileFilter)
		}
		if(nameFilter) {
			// *.svg
			fileObject.var(nameFilter, nameFilter)
		}
		depth=0
	}
	depth+=1;
	fileObject.list(path, func(info) {
		while(info.next()) {			
			info.inject(type,fullPath,name,modifyDt)
			if(type.eq('file')) {				
				cur=node.addNode().with(type, name, fullPath, size, modifyDt)
				if( relativePath ) {
					cur.relativeName = pathJoin(relativePath, name)
				}
				arr.add(cur)
			} else if(checkChild) {
				getFileList(fullPath,node,arr,depth)
			}
		}
	});
	return arr;
}
