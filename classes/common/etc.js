<script>
	logReader(name, fileName) {
		not(name) name='baro'
		obj=object("logReader.$name")
		if(obj.var(useModule)) {
			return obj;
		}
		path=System.path()
		if( fileName ) {
			ch=fileName.ch(1)
			if(ch==':') {
				logFileName=fileName
			} else {
				logFileName=Cf.val(path,'/',fileName)
			}
		} else {
			date=System.date("yyyyMMdd");
			logFileName=Cf.val(path,"/data/logs/${name}-${date}.log")
		}
		not( isFile(logFileName) ) {
			fileWrite(logFileName, "== $name 로그시작 ==\n");
		}
		return addModule(obj, 'logReader', name, logFileName)
	}
	logWriter(name, fileName) {
		not(name) name='baro'
		obj=object("logWriter.$name")
		if(obj.var(useModule)) {
			return obj;
		}
		path=System.path()
		if( fileName ) {
			ch=fileName.ch(1)
			if(ch==':') {
				logFileName=fileName
			} else {
				logFileName=Cf.val(path,'/',fileName)
			}
		} else {
			date=System.date("yyyyMMdd");
			logFileName=Cf.val(path,"/data/logs/${name}-${date}.log")
		}
		not( isFile(logFileName) ) {
			fileWrite(logFileName, "== $name 로그시작 ==\n");
		}
		return addModule(obj, 'logWriter', name, logFileName)
	}
</script>

<script>
	
	watcherNode(name) {
		map=Cf.rootNode().addNode('@watcherFiles')
		not(name) return map;
		return map.get(name)
	}
	watcherStart() {
		asize = args().size()
		if( asize < 3) {
			idx=watcherNode().childCount() + 1;
			code="watcher_$idx"
			if(asize==1) {
				args(path)
				callback = @callback.watcher
			} else if(asize==2) {
				args(path,callback)
			}
		} else {
			args(code,path,callback)
		} 
		watcher=System.watcherFile("testWatcher", callback)
		watcher.start(path)
	}
	

</script>

<script module="@logReader">
	init(name, fileName) {
		@startTime=System.localtime();
		@name=name;
		@logFileName=fileName;
		@fileLogReader=Baro.file("logRead_$name"); 
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
			if( fileLogReader.open() ) {
				this.member(status, 1);
				return;
			}
			if( fileLogReader.open(logFileName) ) {
				this.member(status,1);
				return true;
			}
			return false;
		case 1:
			not( fileLogReader.open() ) {
				this.member(status,0);
				return;
			}
			size=fileLogReader.size();
			startPos=when( size.gt(1024), size-1024, 0 );
			fileLogReader.seek(startPos);
			this.member(fileCurrentPos, size);
			this.member(status, 2);
			return fileLogReader.read();
		case 2:
			not( fileLogReader.open() ) {
				this.member(status, 0);
				return;
			}
			size=fileLogReader.size();
			if( size.eq(fileCurrentPos) ) return;
			if( size.lt(fileCurrentPos) ) {
				this.member(fileCurrentPos, size);
				return print("파일위치 다시 설정", size, fileCurrentPos);
			}
			fileLogReader.seek(fileCurrentPos);
			this.member(fileCurrentPos, size);
			return fileLogReader.read();
		default:
		}
		return null;
	}
	closeLog() {
		if( fileLogReader.open() ) fileLogReader.close();
		this.member(status, 0);
		this.member(startTime, 0);
	}
</script>

<script module="@logWriter">
	init(name, logFileName) {
		@name=name;
		@logFileName=logFileName;
		@startTime=System.localtime();
		@fileLogAppend=Baro.file("logWriter_$name")
	}	
	appendLog(data, skip) {
		if( skip ) return fileLogAppend.append(data);
		this.write("$data\r\n")
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
	closeLog() {
		if( fileLogAppend.open() ) fileLogAppend.close();
		this.member(status, 0);
		this.member(startTime, 0);
	}
</script>