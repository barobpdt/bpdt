##> config 
	PROJECT_PATH = @eval(
		path=conf('baro.defaultProjectPath') 
		not(path) {
			path=pathJoin(System.path(),'services')
			conf('baro.defaultProjectPath', path, true) 
		}
		return pathJoin(path, 'frontend')
	)
	PROJECT_NAME = sample-config
	USE_TAILWIND = false
	USE_TYPESCRIPT = false
	TEMPLATE_PATH = @eval( pathJoin(conf('baro.templatePath'))
	run = @eval(
		this.set(
			'PROJECT_FULLPATH', 
			pathJoin(this.PROJECT_PATH, this.PROJECT_NAME) 
		)
		print("this==>$this")
	)

##> app { routes=[ Home, Login, Setup ], runLast  }
	run = @eval(
		fullpath=configValue('PROJECT_FULLPATH')
		screenPath = pathJoin(fullpath,'src/screens')		
		template = conf('react.appTemplate') not(template) return log("page 템플릿소스 미정의 (저장경로:${0})", screenPath);		
		log(">> page template : $template")
		while(name, this.get('routes') ) {
			moduleName = "${name}Screen"
			params = { 
				STORE: 'global',  
				MODULE_NAME: moduleName
			}
			savePath = pathJoin(screenPath, "${moduleName}.jsx")
			saveSource = parseTemplate(template, params)

			// fileWrite(savePath, saveSource)
			print("app::run => name=$name")
		}
	)


##> store {name=global}
	auto(

	) 
	run(
		storePath = pathJoin(fullpath,'src/stores')
		templateStore = conf('react.storeTemplate') not(templateStore) return log("page store 템플릿 미정의 (저장경로:${0})", screenPath);
		src=makeStore(this.auto)
	)
	
