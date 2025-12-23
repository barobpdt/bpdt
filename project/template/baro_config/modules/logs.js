##> config 
	test = @eval{
		logRead = logTail('test')
		logWrite = logAppend('test')
		log("logs test : $logRead, $logWrite member:${0}", logRead.member() )
		logWrite.append('test log start')
		logWrite.append('hello world !!!')
		data = logRead.timeout()
		log("log timeout data==$data")
		logData = logTail('logs').timeout()
		print(">> logData == $logData")
	}
	
##> func {name=logs}	 
	logTail(name, fileName) {
		not(name) name='baro'
		obj=object("logTail.$name")
		if( obj.var(useModule)) {
			return obj;
		}
		path=System.path()
		if( fileName ) {			
			if( isFullpath(fileName) ) {
				fullpath=fileName
			} else {
				fullpath=pathJoin(path,fileName)
			}
		} else {
			date=System.date("yyyyMMdd");
			fullpath=pathJoin(path,"data/logs/${name}-${date}.log")
		}
		not( isFile(fullpath) ) {
			fileWrite(fullpath, "== $name log tail 시작 ==\n");
		}
		return addModule(obj, 'logTail', name, fullpath)
	}
	logAppend(name, fileName) {
		not(name) name='baro'
		obj=object("logAppend.$name")
		if(obj.var(useModule)) {
			return obj;
		}
		path=System.path()
		if( fileName ) {
			if(isFullpath(fileName)) {
				fullpath=fileName
			} else {
				fullpath=pathJoin(path,fileName)
			}
		} else {
			date=System.date("yyyyMMdd");
			fullpath=pathJoin(path,"/data/logs/${name}-${date}.log")
		}
		not( isFile(fullpath) ) {
			fileWrite(fullpath, "== $name log append 시작 ==\n");
		}
		return addModule(obj, 'logAppend', name, fullpath)
	}	

##> module { name=logTail }
	init(name, fileName) {
		@startTime=System.localtime();
		@name=name;
		@logFileName=fileName;
		@fileLog=Baro.file("logTail_$name"); 
		@lastReadCheck=false;
		@status=0;
		@logTick=0;
		@fileCurrentPos=0;
		@lastRead=false;
		this.timeout();
	}	
	start() {
		this.member(startTime, System.localtime())
		this.timeout()
	}
	stop() {
		this.member(startTime, null)
	}
	timeout() {
		not( startTime ) return;
		switch( status ) {
		case 0: 
			if( fileLog.open() ) {
				this.member(status, 1);
				return;
			}
			if( fileLog.open(logFileName) ) {
				this.member(status,1);
				return true;
			}
			return false;
		case 1:
			not( fileLog.open() ) {
				this.member(status,0);
				return;
			}
			size=fileLog.size();
			startPos=when( size.gt(1024), size-1024, 0 );
			fileLog.seek(startPos);
			this.member(fileCurrentPos, size);
			this.member(status, 2);
			return fileLog.read();
		case 2:
			not( fileLog.open() ) {
				this.member(status, 0);
				return;
			}
			size=fileLog.size();
			if( size.eq(fileCurrentPos) ) return;
			if( size.lt(fileCurrentPos) ) {
				this.member(fileCurrentPos, size);
				return print("파일위치 다시 설정", size, fileCurrentPos);
			}
			fileLog.seek(fileCurrentPos);
			this.member(fileCurrentPos, size);
			return fileLog.read();
		default:
		}
		return null;
	}
	closeLog() {
		if( fileLog.open() ) fileLog.close();
		this.member(status, 0);
		this.member(startTime, 0);
	}	

##> module { name=logAppend }
	init(name, logFileName) {
		@name=name;
		@logFileName=logFileName;
		@startTime=System.localtime();
		@fileLogAppend=Baro.file("logAppend_$name")
	}	
	write(data) {
		not( fileLogAppend.open() ) {
			not( fileLogAppend.open(logFileName,'append') ) return print("로그파일 첨부오류 (파일명:$logFileName)");
		}
		fileLogAppend.append(data);
		fileLogAppend.flush();
	}
	append(data) {
		this.write("$data\r\n")
	}
	appendLog(data) {
		fileLogAppend.append(data);
	}
	closeLog() {
		if( fileLogAppend.open() ) fileLogAppend.close();
		this.member(status, 0);
		this.member(startTime, 0);
	}