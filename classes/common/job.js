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
		node=_node()
		node.type=param
	}
	timerPostList.add(node)
	print("xxxxxxxxxx timer job add xxxxxxxxxx", node, timerPostList)
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
@job.start() {
	conf('job.webMaxNum', 10)
	jobs=Baro.worker('jobs')
	not(jobs.is('start')) {			
		jobs.start(@job.proc)
	}
	return jobs;
}
@job.proc(node) {
	print("@@ 작업시작 => ", node)
	not(node) return print("@@ 작업시작오류 (작업노드 미정의)")
	not(node.jobType) node.jobType='test'
	fc = call("@job.fc_${node.jobType}")
	if(typeof(fc,'func')) {
		call(fc, this, node)
	}
}
@job.addJob(param) {
	if(typeof(param,'node')) {
		args(node)
	} else {
		args(jobType, node)
		node.jobType=jobType
	}
	jobs=Baro.worker('jobs')
	if(jobs.is('start')) {			
		jobs.push(node)
	} else {
		print("@@ 작업이 시작되지 않았습니다 job addNode 오류")
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

// 작업추가
// @job.addJob(node) 
// 
@job.fc_test(node) {
	@job.addTimerJob(node)		
}
@job.post_test(node) {
	command=node.command
	not(command) command='dir'
	cmd().cmdNode(node, command, @job.test_callback )
	print("@@ post_test call end", node)
}
@job.test_callback(&s) {
	print("test callback s==$s")
}