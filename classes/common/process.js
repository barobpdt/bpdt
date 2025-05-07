<script module="@web">
	init(name, fc) {
		@web = Baro.web(name)
		@finishResultFunc = fc
		setCallback(web, this.webProc, this)
	}
	call(url) {
		this.set('resultData','')
		web.call(url)
	}
	setData(data) {
		web.set('method', 'POST')
		web.set('data', data)
	}
	setHeader(k,v) {
		h = web.addNode("@header")
		h.set(k,v)
	}
	webProc(type, data ) {
		if(type.eq('read')) return this.appendText('resultData', data)
		if(type.eq('finish')) {
			fc = this.member(finishResultFunc)
			if( fc ) call(fc, this, this.ref('resultData'))
		}
	} 
</script>

<script module="@cmd">
	init(name, program) {
		not(program) program = 'cmd'
		@proc=Baro.process(name)
		@cmdList=this.addArray("cmd.list");
		@paddingList=this.addArray("padding.list");
		@status = 'first'
		@currentProgram = program
		@currentNode = null
		setCallback(proc, this.cmdProc, this)
		this.runStartTick = System.localtime()
		this.start()
	}
	isRun() {
		return proc.run();
	}
	isStart() {
		return status.eq('start');
	}
	start(prog) {
		if( proc.run() ) {
			proc.close()
		}
		@status = 'first'
		if(prog) {
			@currentProgram = prog
		} else {
			prog=this.member(currentProgram)
		}
		this.cmdAdd('chcp 65001')
		proc.run(prog, 0x400)
	}
	
	stop() {
		@status = 'stop'
		if( proc.run() ) {
			proc.kill()
		} else {
			print("cmd 가 실행중이 아닙니다")
		}
	}
	close() {
		while(cur, cmdList ) {
			paddingList.add(cur)
		}
		cmdList.reuse()
		this.stop()
	}
	run(cmd) {
		if(status.eq('first')) {
			return this.cmdAdd(cmd)
		}
		not( proc.run() ) {
			return this.start()
		}
		not(cmd) {
			node = cmdList.pop()
			if(typeof(node,'node')) {
				cmd = node.cmd
				this.member(currentNode, node)
			} else {
				cmd = node
				this.member(currentNode, null)
			}
		}
		if(cmd) {
			@status = 'start'
			this.runStartTick = System.localtime()
			this.set('cmdResult','')
			proc.write(cmd);
		} else {
			dist = System.localtime() - this.runStartTick
			print("run time == $dist")
			if( dist > 5 ) {
				print("모든 명령을 실행 했습니다")
			}
		}
	}
	cmdCallback(cmd, callback, target) {
		node=this.addNode().with(cmd, callback, target)
		cmdList.add(node)
		if( status.eq('first','stay')) {
			this.run()
		}
	}
	cmdProc(type,data) {
		if(type=='read') {
			this.appendText('cmdResult', data);
			c=data.ch(-1,true);
			if(c=='>') {
				if(status.eq('first')) {
					print("@@ ${program} ${status} 실행")
				} else {
					this.parseResult()
				}
				@status = 'stay'
				this.run()
			}
		}
	}
	cmdAdd(cmd, run) {
		cmdList.add(cmd)
		if(run) this.run()
	}
	parseResult() {
		// result = this.ref(cmdResult)
		result = this.get('cmdResult')
		print("current node=> ", currentNode)
		if( currentNode) {
			ftype=typeof(currentNode.callback)
			if(ftype.eq('funcRef')) {
				target = currentNode.target not(target) target=this
				call(currentNode.callback, target, result)
			} else if(ftype.eq('func')) {
				fn = currentNode.callback
				fn.callFuncParams(result)
				fn.callFuncSrc()
			}
			if( this.find(currentNode) ) {
				this.remove(currentNode, true)
			}
			this.member(currentNode, null)
		} else {
			fn = this.onLogChange
			if(typeof(fn,'func')) {
				fn.callFuncParams(result)
				fn.callFuncSrc()
			} else {
				print("cmd>> $result")
				logWriter('cmd').appendLog(result)
			}
		}
	}
</script>

<script module="@job">
	init(name, type, callback) {
		@worker = Baro.worker(name)
		this.workerStartTick = System.localtime()
		this.jobType(type, callback)
	}
	workerId() {
		return worker.id;
	}
	jobType(type) {
		not(args().size()) return this.member(type)
		this.member(type, type)
	}
	jobCallback(fc) {
		not(typeof(fc,'func')) fc= this.jobDefault
		this.jobType('jobFunc')
		setEvent(worker,'@callback', fc, this)
	}
	jobStart(fc) {
		if( typeof(fc,'func') ) {
			worker.stop()
			if( worker.isValid('@callback')) {
				arr = worker.val('@callback').eventFuncList()
				if(arr.find(fc)) {
					print('이미 추가된 작업함수입니다')
				} else {
					arr.add(fc)
				}
			} else {
				this.jobCallback(fc)
			}
		}
		if(worker.is()) {
			print("작업이 이미 시작중입니다")
			return;
		}
		worker.start(this)
	}	
	jobStop() {
		worker.stope()
	}
	jobAdd(job) {
		worker.push(job)
	}
	jobList() {
		return worker.list()
	}
	jobCount() {
		return worker.list().size()
	}
	jobDefault(job) {
		name = worker.id
		not(job) {
			print("worker $name 작업내용 모두처리")
		}
		print("worker $name 시작 [작업 타입: ${job.type}]")
	}
	removeFunc(fc) {
		not(worker.is()) {
			print("작업 시작중이 아닙니다")
			return;
		}
		if( worker.isValid('@callback')) {
			worker.stop()
			worker.val('@callback').removeFuncSrc(fc)
		}
		this.jobStart()
	}
</script>


<script module="@webDownload">
	init(name, savePath, count) {
		not(savePath) savePath = Cf.val(System.path(),'/data/download')
		not(isFolder(savePath)) Baro.file().mkdir(savePath, true)
		not(count) count = 2
		@serviceName = name
		@savePath = savePath
		@maxDownloadCount = count
	}
	downloadStart() {
		while(n=0, n<maxDownloadCount, n++ ) { 
			this.downloadNext(Baro.web("${serviceName}-$n"));
		}
	}
	downloadAdd(node) {
		node.inject(url, fileName, savePath)
		not(url) return print("download URL 미정의 [노드: $node]")
		
		idx=this.incrNum("webdownload_index") 
		not(savePath) {
			savePath = this.member(savePath)
		}
		not(fileName) fileName=right(url,'/');
		n=idx%maxDownloadCount;
		web=Baro.web("${serviceName}-$n")
		cur=web.addNode()
		cur.url = url
		cur.savePath = savePath
		cur.fileName = fileName
		cur.downState=0
		print("@@ download ADD cur==>$cur")
	}
	downloadClear() {
		while(n=0, n<maxDownloadCount, n++ ) {
			web=Baro.web("${serviceName}-$n")
			web.removeAll()
		}
	}
	downloadInfo() {
		while(n=0, n<maxDownloadCount, n++ ) {
			web=Baro.web("${serviceName}-$n")
			print("n => ", web.childCount())
		}
	}
	downloadNext(web) {
		while(cur, web) {
			if( cur.downState.eq(1,2,3,9) ) continue;
			return this.downloadFile(web, cur);
		}
		return false;
	}
	downloadFile(web, cur) { 
		not(cur) return;
		cur.inject(url, fileName, savePath)
		not(url) return print("@@ downloadFile URL 미정의")
		not(fileName) return print("@@ downloadFile 파일명 미정의")
		not(savePath) return print("@@ downloadFile 파일경로 미정의")
		cur.downState=1
		web.currentNode=cur;
		web.download(cur.url, "$savePath/${fileName}", "GET", this, this.downloadProcess );
	}
	downloadProcess(type,data) {
		if(type=='process') {
			return;
		}
		web=sendor
		cur=web.currentNode;
		if(type=='finish') {
			cur.downState=3;
			return this.downloadNext(web);
		}
		if(type=='error') {
			cur.downState=9;
			return print("다운로드오류: ${cur}");
		}
	}
</script>


