@job.start() {
	conf('job.webMaxNum', 10)
	jobs=Baro.worker('jobs')
	not(jobs.is('start')) {			
		jobs.start(@job.proc)
	}
	return jobs;
}

@job.event(obj, eventName, fc, reset) {		
	not(typeof(obj,'node')) return print('@@ job event 객체 오류', obj, fc) 
	fn = obj.get(eventName)
	if( typeof(fn,'func')) {
		print("xxxxxxxx", args())
		not(reset) {
			print("＠＠ $eventName 함수가 이미등록되었습니다")
			return fn;
		}
	}
	fcType = typeof(fc)
	not( fcType.eq('funcRef') ) {		
		if(fc) print("@@ job event  함수타입 오류 (타입:$fcType)")
		return;
	}
	fn=Cf.funcNode(fc, obj)
	obj.set(eventName, fn) 
	return fn;		
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
	@job.event(global,'onTimeout', @job.timerProc)
	System.globalTimer(250)
}
@job.timerProc() {
	this.inject(@timerPostList)
	job = timerPostList.pop() not(job) return;
	print("timer job == $job")
	fc=call("@job.post_${job.type}")
	if(typeof(fc,'function') ) {
		call(fc, this, job)
	} else {
		print("@@ timer job function not defined node==>$job")
	}
} 

@job.proc(node) {
	print("@@ 작업시작 => ", node)
	not(node) return print("@@ 작업시작오류 (작업노드 미정의)")
	type = node.jobType
	not(type) {
		type=node.type
		not(type) type='test'
	}
	fc = call("@job.fc_${type}")
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
	if(jobs.is('start')) {			
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
	type = node.jobType
	not(type) {
		type=node.type
		not(type) type='test'
	}
	fc = call("@job.worker_${type}")
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
		fc = call("@job.web_${this.resultType}")
		if(typeof(fc,'function')) {
			target = this.get('@target')
			call(fc, this, this.ref(result), target)
		} else {
			print("@@ webTypeResult 콜백 함수 미정의 : job.web_${this.resultType}")
		}
	}		
}

@job.webResult(param) {
	if( typeof(param,'node') ) {
		args(wo,resultType,url,target)
	} else {
		args(resultType, url,target)
		wo=@job.webObject()
		not(wo) return;
	}		
	not(resultType) resultType='default'
	not(url) url = wo.url
	wo.set('@target', target)
	wo.set('result','')
	wo.resultType=resultType
	wo.call(url)
	return true;
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
@job.fc_test(node) {
	@job.addWorker(node)
}
@job.worker_test(node) {
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

@job.post_pythonTest(node) {
	not( isFolder(conf('python.path')) ) {
		c=cmd()
		c.cmdAdd('c:')
		c.cmdNode(node, 'cd %userprofile%', @job.setPythonPath)	
	}
}	

/*

*/
firstLine(&s) {
	not(s.ch()) return '[null]';
	return s.findPos("\n").trim();
}

_getVarValue(&s,fn) {
	isVar = func(s) {
		c=s.next().ch() not(c) return true;
		while(c.eq('.')) c=s.next().ch()
		return when(c,false,true);
	};
	c=s.ch() not(c) return; 
	if(c.eq('=')) {
		k=s.incr().trim()
		v=conf(k) not(v) v="[conf $k 미정의]";
		return v;
	}
	not(fn) fn=Cf.funcNode('parent')
	not( isVar(s) ) return eval(s);
	k=s.move(), c=s.ch()
	not(c) {
		if( fn.isset(k)) {
			v=fn.get(k)	
		} else { 
			if(k.eq('nl')) v="\n"
			else if(k.eq('tab')) v="\t"
			else v=' ';
		}
		return v;
	}
	ss=fn.get(k)
	while(c.eq('.')) {
		not(typeof(ss,'node')) return '';
		k=s.incr().move()
		c=s.ch()
		not(c) {
			v=ss.member("$k")
			not(v) v=ss.get(k)
			return v;
		}
		ss=ss.get(k)
	}
	return ss;
}
_sv(&s, node) {
	fn=Cf.funcNode('parent')
	ss=''
	while(s.valid()) {
		left = s.findPos('$')
		ss.add(left)
		c=s.ch() not(c) break;
		if(c.eq('{')) {
			src=s.match(1)
			if(typeof(src,'bool')) break;
			ss.add(_getVarValue(src,fn))
			continue;
		}
		k=s.move()
		v=''
		if(fn.isset(k)) {
			v=fn.get(k)
		} else if(node && node.isVar(k)) {
			v=node.get(k)
		}
		if(v) ss.add(v)
	}
	return ss;
} 
_confInfo(&s) {
	db=Baro.db('config')
	a=s.move(), filter=''
	if(a.eq('*') ) {
		c=s.ch()
		not(c.eq('.')) return print("@@ error conf list $a 하위 정보 오류");
		b=s.incr().trim()
		print("b==$b")
		if(b.find('%')) {
			filter = "and cd like '$b'"
		} else {
			filter = "and cd='$b' "
		}
	} else {
		c=s.ch()
		if(c.eq('.')) {
			b=s.incr().trim()
			filter = "and grp='$a' and cd like '$b'"
		} else {
			filter = "and grp='$a'"
		}
	}
	node=db.fetchAll("select grp, cd, data from conf_info where 1=1 $filter")
	ss=''
	while(cur, node) {
		cur.inject(grp, cd, data)
		line=sv('$grp.$cd ${firstLine(data)} ${nl}')
		ss.add(line)
	}
	return ss;
}
