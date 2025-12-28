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
				logFileName=pathJoin(path,fileName)
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
