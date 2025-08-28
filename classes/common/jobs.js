@job.start() {
	conf('job.webMaxNum', 10)
	jobs=Baro.worker('jobs')
	not(jobs.is('start')) {			
		jobs.start(@job.proc)
	}
	return jobs;
}
@job.event(obj, eventName, fc, target) { return _event(obj, eventName, fc, target) }
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
	@job.event(global,'onTimeout', @job.timerProc)
	System.globalTimer(250)
}
@job.timerProc() {
	this.inject(@timerPostList)
	job = timerPostList.pop() not(job) return;
	type=job.jobType not(type) type='test'; 
	fc=call("@job.${type}#post")
	if(typeof(fc,'function') ) {
		call(fc, this, job)
	} else {
		print("@@ timer job function not defined node==>$job")
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

@job.webObject() {
	ws=object('baro.webObjectMap')
	arr = ws.get('@webObjects')
	cnt = 0
	if( typeof(arr,'array') ) cnt=arr.size()
	print("@@ web object size: ", cnt)
	not(cnt) {			
		arr=ws.addArray('@webObjects')
		while(n=1,5) {
			cur = arr.add(Baro.web("webObject_$n"))
			@job.event(cur, '@callback', @job.webTypeResult)
		}
	}
	obj=null
	while(cur, arr) {
		if(cur.is('run')) continue;
		obj = cur;
	}
	not( obj ) {
		idx = arr.size()+1;
		if( idx > conf('job.webMaxNum') ) {
			return null;
		}
		cur = arr.add(Baro.web("webObject_$idx"))
		@job.event(cur, '@callback', @job.webTypeResult)
		obj = cur
	}
	print("obj==>$obj")
	return obj;
}

@job.webTypeResult(type, data) {
	if(type=='read') this.appendText('result', data)
	if(type=='finish') {
		type=this.jobType not(type) type='test'
		fc = call("@job.${type}#web")
		if(typeof(fc,'function')) {
			call(fc, this, this.ref(result), target )
		} else {
			print("@@ webTypeResult 콜백 함수 미정의 : job.${type}#web")
		}
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
			line=s.findPos("\n");
			not(line.ch()) continue;
			k=line.findPos(':').trim()
			if(lineCheck(line,';')) {
				v=line.findPos(';').trim()
			} else {
				v=line.trim()
			}
			hh.set(k,v)
		}
	};
}

@job.web_default(&s, target) {
	print("## web result ==> $s")
}
@job.web_sido_info(s, target) {
	node=object('naver.sidoInfo')
	node.parseJson(s)
	while(cur,node.regionList) { 
		@job.addJob('gungoInfo', cur)
	}
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
/* 웹url 호출후 노트패드에서 열기
	node=_node()
	@job.addWebJob('openUrl', 'https://www.tjmedia.com/chart/top100', node)
*/
@job.openUrl#post(node) {
	print("@@ post test", node)
	node.inject(@saveFileName)
	cmd().run( _s('notepad "$saveFileName" '))
} 
@job.openUrl#web(&s, node) {
	path=_s('c:/temp/${System.date("yyyyMMdd")}.html')
	fileWrite(path,s)
	node.set('@saveFileName', path)
	@job.addPost('openUrl', node)
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
data = _s('chartType=TOP&searchStartDate=2025-$month-01&searchEndDate=2025-$month-$days&strType=')
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

/* 
page test 
ghp_ h3x1xuT3vc22T9RuocAPFUsg3yaHvj1 soqT4
*/
@baro.main() {
	p = _page('app:main', V[
		<page margin="0">
			<div id="container">
		</page>
		init() {
			@container= this.get('container')
			this.size(800,600)
			this.open()
		}
		addStack(page) {
			container.addPage(page, true)
		}
	])
	p.addStack( @baro.pageChatbot() )
	return p;
}
@baro.pageChatbot() {
	p =_page('app:chatbot', V[
		<page>
			<canvas id="c">
			<hbox>
				<button id="ok" text="ok">
				<space>
			</hbox>
		</page>
		
		init() {
			@cmd = cmd('python')
			@in = logWriter('webvew-in')
			@out = logReader('webview-out')
			@python = _s('${@python.path}/python')
			@srcPath = _s('${@sample.path}/apps')
			@canvas = this.get('c')
			@btnOk = this.get('ok')
			print("## init line==>",python,srcPath)	
			this.setPage()
			this.setPageEvent()
			this.setWebview()
			this.timer(50)
		}
		onTimer() {
			log = out.timeout()
			if( log ) {
				print("@@ timer log == $log")
				this.parseWebLog(log)
			}
		}
		onResize() {
			canvas.geo().inject(x,y,w,h)
			line = _s('##> geo:$:x,$:y,$:w,$:h,0')
			print("xx resize xx", line, x, y)
			in.append(line)
		}
		onClose() {
			this.killTimer()
			in.append('##> quit:')
		}
		setPage() {
			box=this.child(0)
			box.margin(0)
			hbox=box.child(1)
			hbox.margin(4,2,2,2)
		}
		setPageEvent() {
			_event(canvas,'onDraw', this.drawCanvas, this)
			_event(btnOk, 'onClick', this.clickBtnOk, this)
		}
		setWebview() {
			line=_s('$python "$srcPath/webpage.py" --log "${in.logFileName}" --out "${out.logFileName}"')
			cmd.cmdAdd(this, line, this.webviewStarted )
		}
		drawCanvas(dc,rc) {
			dc.fill('#344')
		}
		clickBtnOk() {
			alert('clickBtnOk')
		}
		webviewStarted(&s) {
			print("@@ web view start => $s")
		}
		parseWebLog(&s) {
			if(s.find('##> start:')) {
				winId = canvas.winId()
				in.append("##>setParent:${winId}")
			}
		}
	])
	return p;
}
