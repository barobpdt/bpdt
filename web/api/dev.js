<api>
	test(req, param, &uri) {
		type=uri.findPos('/').trim()
		param=uri.findPos('/').trim()
		if(type.eq('navIcons')) {
			return '["application_add","application_cascade","application_delete","application_double"]'
		}
		return "api test URI=> $uri type:$type ($param)";
	}
	watcherPath(req,param,&uri) {
		type=uri.findPos('/').trim() not(type ) type = 'webpageWatcher'
		path=uri.trim()
		root = Cf.rootNode()		
		config = root.get('@watcherFiles').get(type)
		if(typeof(config,'node')) {
			if(path) {
				config.set('target',path)
			} else {
				param.path = config.target.replace('\','/')
			}
		} else {
			param.error = true
			param.message = "$type watcher 유형 미존재"
		}
		return param;
	}
</api>
