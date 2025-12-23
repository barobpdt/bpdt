##> func {name=jobs }
	@eval {
		print("jobs function => webResult
	}

	/* 웹호출 결과 출력 (api 호출) */
	webResult(web, url, method, data, headerJson) {
		not(method) method='GET'
		if(method=='POST') {
			web.set('data',data)
		}		
		if(headerJson && typeof(headerJson,'string')) {
			header=web.addNode('@header').reuse()
			header.parseJson(headerJson)
		}
		web.call(url,method, @user.webCallback)
		return web.ref('@webResult')
	}
	@user.webCallback(type,data) {
		if(type=='error') return log('webResult 오류 객체:#{0} 메시지#{1}', this, data);
		if(type=='read') this.appendText('@webResult', data)
	}


##> func {name=fileCheck, note=파일변경여부 체크해서 자동반영}

	/* 서비스별(프로트,백앤드,api 등) 파일감시 목록추가  */
	addWatchFile(serviceMode, fullpath) {
		not(isFile(fullpath)) return log('addWatchFile #{0} 파일을 찾을수 없습니다 (소스감시 등록오류)',fullpath);
		watchInfo=object('user.watchFileInfo')
		files=watchInfo.get('@watchFileList')
		not(typeof(files,'array')) {
			return log('addWatchFile 감시타이머가 실해중이 아닙니다 (소스감시 객체오류 #{0} 파일등록 실패)',fullpath);
		}
		if(fullpath.find('\')) {
			fullpath = fullpath.replace('\','/')
		}
		if( files.find(fullpath) ) {
			return log("$fullpath 는 파일변경목록에 이미 추가되었습니다")
		}
		// 전체경로에서 폴더, 파일명, 확장자제외 이름 추출
		filePathInfo(fullpath).inject(folder,fileName,name)
		// 파일변경시간 추출
		modifyTime = fileTime(fullpath)
		// 파일명으로 소스감시 목록추가
		watchInfo.addNode(fileName).with(serviceMode,fullpath,folder,fileName,name,modifyTime)
		files.add(fullpath)		
		// 설정서비스 미등록시 기본값으로 등록
		root=object("baro.services").get(serviceMode)
		not(typeof(root,'node')) {			
			user.serviceName=serviceMode
		}
	}
	
	/* 전역 타이머 실행 */
	startGlobalTimer() {
		if( global().get('@timerDelay') ) {
			return log('global timer가 실행중입니다 #{0}', global().get('@timerDelay'))
		}
		watchInfo = object('user.watchFileInfo')
		watchInfo.addArray('@watchFileList').reuse()
		event(global(),'onTimeout', @user.timerProc)
		// 500ms 마다 파일변경체크
		System.globalTimer(500)
		log("global timer 시작", watchInfo)
		return watchInfo;
	}
	/* 전역 타이머 중지 */
	stopGlobalTimer() {
		global().set('@timerDelay',0)
		System.globalTimer(false)
		log("global timer 중지됨")
	}
	/* 전역 타이머처리 콜백함수 */	
	@user.timerProc() {
		watchInfo = object('user.watchFileInfo')
		while(cur, watchInfo) {
			cur.inject(serviceMode,fullpath,fileName,name,modifyTime)
			// 현재파일 시간과 등록된 시간이 다르다면 파일 변경처리
			if(fileTime(fullpath) != modifyTime) {
				log("$fileName 변경됨 서비스등록 처리")
				@baro.loadService(serviceMode,fullpath)
				return;
			} 
		}		
		jobList = watchInfo.get('@jobList')
		if(typeof(jobList,'array')) {
			job = jobList.pop() not(job) return;
			if(job.cmp('tag','precess') ) {
				cmd=job.get('@jobCommand')
				if(cmd) {
					job.set('@jobCommand', null)
					job.write(cmd)
				}
			} else if(job.cmp('tag','web') ) {
				url=job.get('@jobUrl')
				result=webResult(job, url)
			} else {
				log('job 태그 미정의 #{0}', job)
			}
			return;
		}
	}
	
