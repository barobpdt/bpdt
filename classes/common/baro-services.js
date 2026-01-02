@baro.evalSourceParse(&s,node) {
	not(s.ch()) return;
	ss=''
	while(s.valid()) {
		left=s.findPos('{',1,1)
		ss.add(left)
		c=s.ch() not(c) break;
		c=left.ch(-1,true)
		if(c.eq('=',',','(')) {
			str=s.match(1)
			ss.add("getJsonNode(V[${str}])")
		} else {
			s.incr()
			ss.add('{')
		}
	}
	return ss;
}


@baro.hashmap(node, key, &s, reset) {	
	if(node.isVar(key)) {
		map=node.get(key)
		if(reset) map.reuse()
	} else {
		not(s.ch()) return print("맵생성오류 (json소스가 없습니다)");
		reset = true
		map = object('baro.hashmaps').addNode()
	}
	if(reset && s.ch() ) {
		map.parseJson(@baro.parseService(node,map,s))
	}
	return map	
}

@baro.createServiceFolder(root, mode) {
	config = root.get('@config') not(config) return print("@@createServiceFolder 설정노드가 없습니다(모드:$mode)")
	projectPath = @baro.configKeyValue(root,root,'PROJECT_PATH')
	projectName = @baro.configKeyValue(root,root,'PROJECT_Name')
	not(projectPath) return print("@@createServiceFolder 설정오류 (모드:$mode, PROJECT_PATH 미정의)", root)
	not(projectName) return print("@@createServiceFolder 설정오류 (모드:$mode, PROJECT_NAME 미정의)", root)
	fullPath = pathJoin(projectPath, projectName)
	if(isFolder(fullPath)) return print("@@createServiceFolder 프로젝트 생성오류 (경로:$fullPath 경로가 이미 존재합니다)", root)
	Baro.file().mkdir(fullPath, true)
	base=config.get('@default')
	base.set('FULL_PATH', fullPath)
}

@baro.loadService(serviceNode, &s, reset, evalAll) {
	if(typeof(serviceNode,'string')) {
		group=object("baro.serviceGroup").addNode('baroCommon')
		serviceName=serviceNode
		serviceNode=group.addNode(serviceName)
		serviceNode.set('serviceName', serviceName)
	}
	isProps = func(s) {
		c=s.ch()
		return when(c.eq('{'),true);
	};
	map=global().addNode('@evalCallMap')
	debug=conf('cf.useDebug')
	serviceName = serviceNode.get('serviceName')
	print(">> baro.loadService [$serviceName] start 소스사이즈:", s.size())
	evalCheckArray = []
	error=false
	type='', base=null, cur=null, evalDefault=null
	okCnt=0
	while(s.valid()) {
		left = s.findPos('##>',1)
		if(type) { 
			ok=true
			name = cur.get('name')			
			confCode="${serviceName}.${type}:${name}#modify"			
			not(cur.isVar('@confCode')) {
				cur.set('@confCode',confCode)
			}
			if( left.eq(conf(confCode)) ) {
				if( cur.isVar('@modify')) { 					
					print(">> $confCode 변경된 내용이 없습니다 ")
					not(reset) ok=false
				}
				cur.set('@modify',false)
			} else {
				serviceNode.set("@${type}_modify", true)				
				cur.set('@modify',true)
				conf(confCode,left,true)
			}
			if(ok ) {
				okCnt++;
				print(">> loadService type=====$type $confCode start") 
				if( left.find('@eval')) {
					if(type.eq('config')) {
						evalCheckArray.add(cur)
					} else {
						not(map.get(confCode)) evalCheckArray.add(cur)
					}
				}
				if(type.eq('config')) {
					@baro.parseConfig(serviceNode, cur, left)					
				} else if(type.eq('funcs','modules')) {
					currentFileName = serviceNode.get('@currentFileName') 
					not(currentFileName) currentFileName=cur.name
					// source=@baro.getFuncSource(cur,left)
					if(debug) {
						not(@baro.parseFuncs(cur,left)) {
							error=true
						}
					}
					not(error) {
						funcInfo=Cf.rootNode('@funcInfo')
						funcInfo.set('includeFileName', "${serviceNode.serviceName}::${currentFileName}")				
						if(type.eq('modules')) {
							if(name.eq('@default')) {
								name='common'
							}
							applyFunc(left, name)
						} else {						
							applyFunc(left)
						}
						funcInfo.set('includeFileName','')
					}
				} else if(type.eq('template')) {
					@baro.parseConfig(serviceNode, cur, left)
				} else if(type.eq('pages')) {
					print("pages==".left.size() )
				} else if(type.eq('sql')) {
					@baro.sqlFuncVal(serviceNode, cur, left)
				} else if(type.eq('routes')) {
					@baro.parseRoute(serviceNode, cur, left)
				} else if(type.eq('tables')) {
					@baro.tableFuncVal(serviceNode, cur, left)
				} else {
					@baro.parseConfig(serviceNode, cur, left)
				}
				print("parse serviceNode type=====$type end")
			} else {
				if( type.eq('config') && name.eq('@default')) {
					// 한번실행 체크
					check = map.get(confCode)
					if( left.find('@eval') && ~(check) ) {
						evalDefault = cur
					}
				}
			}
		}
		not(s.ch()) break
		type=s.move().lower(), name=''
		if(type.eq('page','app')) {
			if(type.eq('app')) name='app'
			type='pages'
		} else if(type.eq('module')) {
			type='modules'
		} else if(type.eq('func','function')) {
			type='funcs'
		}
		base=serviceNode.addNode("@$type")
		params=null
		if(isProps(s)) { 
			s.ch()
			params=s.match(1)
			nm=propValue(params,'name')
			if(nm) name=nm
		}
		not(name) name='@default'
		cur=base.get(name)
		cur.set('@type', type)
		if(cur) {
			if(reset) {
				arr=base.get('@keyArray') if(arr) arr.reuse()
				cur.removeAll(true)
				@baro.parseConfig(serviceNode, cur, params)
			}
		} else {
			cur=base.addNode(name)
			@baro.parseConfig(serviceNode, cur, params)
		}
	}
	if( okCnt && evalDefault ) {		
		evalCheckArray.add(evalDefault)
	}
	if( evalCheckArray.size() ) {
		while(cur, evalCheckArray) {
			@baro.evalCall(serviceNode,cur,evalAll,map)
		}
	}
	return serviceNode;
}

@baro.parseFuncs(cur, &s, moduleName) {	
	comment=''
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq('/')) {
			c=s.ch(1)
			if(c.eq('/')) cmt=s.findPos("\n") else cmt=s.match(1);
			if(comment) comment.add(conf('cf.newline'))
			comment.add(cmt)
			continue;
		}
		sp=s.cur()
		if(c.eq('@')) s.incr()
		c=s.next().ch()
		while(c.eq('.','#','-')) {
			c=s.incr().next().ch()
		}
		ep=s.cur()
		if(c.eq('(')) {
			fnm=s.trim(sp,ep,true)
			fparam=s.match()
			c=s.ch()
			if(c.eq('{')) {
				fsrc=s.match(1)
				if(typeof(fsrc,'bool')) {
					log("error::parseFuncs 함수소스 매치오류 (함수명:$fnm)")
					return false;
				}
			} else {
				log("error::parseFuncs 함수소스 시작오류 $fnm [$c]")
				return false;
			}
			sub=cur.addNode(fnm)
			sub.funcName=fnm
			sub.funcParam=fparam
			sub.funcSource=fsrc
			
		} else {
			fnm=s.trim(sp,ep,true)
			log("error::parseFuncs 함수 시작오류 $fnm [$c]")
			return false;
		}
	}
	return true;
}
@baro.getFuncSource(cur, &s) {
	not(s.ch()) return;
	src='', arr=cur.addArray('@keyArray')
	arr.reuse()
	nl=conf('cf.newline')
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(';',',')) {
			s.incr()
			continue;
		}
		sp=s.cur()
		if(c.eq('@')) {
			s.incr()
		}
		c=s.next().ch()
		while(c.eq('.')) c=s.next().ch()
		k=s.trim(sp,s.cur(),true)
		if(c.eq('=')) {
			k=s.trim(sp,s.cur(),true)
			c=s.incr().ch()
			if(c.eq()) {
				cur.set(k,s.match())
				not(arr.find(k)) arr.add(k)
				continue;
			}
			sp=s.cur()
			if(c.eq('@')) c=s.incr().ch()
			c=s.next().ch()
			while(c.eq('.')) c=s.next().ch()
			not(c.eq('(','{')) {
				print(">> 함수소스 생성오류: 키 [$k] 형식오류 함수내 키설정시 따옴표 또는 괄호만 허용")
				break;
			}
			kk=s.trim(sp,s.cur(),true)
			if(kk.eq('func','function')) {
				fparam=''
				if(c.eq('(')) {
					fparam=s.match()
					c=s.ch()
				} 
				if(c.eq('{')) {
					src=s.match(1)
					if(typeof(src,'bool')) {
						print(">> 함수소스 생성오류: [$k = $kk] 괄호매치 오류")
						break;
					}
				} else {
					print(">> 함수소스 생성오류: [$k = $kk] 시작오류")
					break;
				}
				src.add("${k}($fparam) {$src}" nl)
			} else {
				if(c.eq('(')) {
					print(">> 함수소스 생성오류: [$kk] 값이 func 또는 function만 허용")
					break;
				}
				source = s.match(1)
				if(typeof(source,'bool')) {
					print(">> 함수소스 생성오류: [$k = $kk] 괄호매칭 오류")
					break;
				}
				cur.set(k, "$kk=>$source")
				not(arr.find(k)) arr.add(k)
			}
			continue;
		}
		if(c.eq('{')) {
			k=s.trim(sp,s.cur(),true)
			val=s.match(1)
			if(typeof(val,'bool')) {
				log("$k 괄호 매칭오류")
				break;
			}
			if(k.eq('@eval')) {
				cur.set(k, "@eval=>$val")
			} else {
				cur.set(k, removeIndentText(val))
			}
			not(arr.find(k)) arr.add(k)
			continue;
		} 
		if(c.eq('(')) {
			k=s.trim(sp,s.cur(),true)
			s.match()
			if(s.ch('{')) {
				val=s.match(1)
				if(typeof(val,'bool')) {
					log("$k 함수 괄호 매칭오류")
					break;
				}
				ep=s.cur()
				src.add(s.value(sp,ep,true), nl)
			} else {
				log("$k 함수 시작오류")
				break;
			}
		} else {
			k=s.trim(sp,s.cur(),true)
			log("$k 함수 시작 오류 [$c]")
			break;
		}
	}
	return src;
}

@baro.evalCall(root, config, evalAll, map) {
	confCode=config.get('@confCode')
	fn=Cf.funcNode('parent')
	ka=config.get('@keyArray')
	if(cur.get('SET_EVAL')) {
		evalAll=true
	}
	while(k, ka) {
		s=config.ref(k)
		not(typeof(s,'string')) continue;
		if(k.ch('#')) continue;
		 
		ss=null
		if(k.eq('@eval')) {
			applyCheck=false
			ss=s
		} else if(evalAll) {
			if(s.start('@eval=>',true)) {
				applyCheck=true
				ss=s
			}
		}
		if(ss) {
			// src=@baro.evalSourceParse(ss)
			src = stripJsComment(ss)
			result = runSource(src,config)
			if(applyCheck) {
				config.set("@$k",result)
			} else if( typeof(result,'bool') && result ) {
				// 실행결과가 true 라면 한번만 실행되도록 한다
				map.set(confCode, true)
			}
		}
	}
}

@baro.storeCreateFile(root, stores) {
	nl=conf('cf.newline')
	useTypescript = conf('baro.useTypescript')
	va=['@state','@persist','@funcs']
	while(store, stores) {
		ka=store.get('@keyArray')
		not(ka) {
			continue;
		}
		while(k,va) {
			if(store.isVar(k)) store.set(k,'')
		}
		while(k,ka) {
			src=@baro.configValue(root, store, store.ref(k))
			if(k.eq('auto')) {
				auto(src)
			} else if('state')) {
				nodeAppendText(store,'@state',src, ",$nl")
			}
		}
	}
	auto=func(&s) {
		comment=''
		while(s.valid()) {
			c=s.ch() not(c) break;
			if(c.eq(',',';')) {
				s.incr()
				continue;
			}
			if(c.eq('/')) {
				if(comment) comment.add(' ')
				c=s.ch(1)
				if(c.eq('/')) {
					s.incr(2)
					comment.add(s.findPos("\n").trim())
				} else {
					comment.add(s.match())
				}
				continue;
			}
			line='', body='', types='', sep=''
			if(lineCheck(s,'&')) {
				line=s.findPos('&')
				if(lineCheck(s,'//')) {
					types=s.findPos('//')
					if(comment) comment.add(' ')
					comment.add(s.findPos("\n").trim())
				} else {					
					types = s.findPos("\n").trim()
				}
			} 
			else if(lineCheck(s,'{') || lineCheck(s,'[') ) {
				if(lineCheck(s,'{')) {
					sep='node'
					line=s.findPos('{',0,1)
				} else {
					sep='array'
					line=s.findPos('[',0,1)
				}
				body=s.match(1)
				if(lineCheck(s,'//')) {
					s.findPos('//')
					if(comment) comment.add(' ')
					comment.add(s.findPos("\n").trim())
				}
			} else {
				if(lineCheck(s,'//')) {
					line=s.findPos('//')
					if(comment) comment.add(' ')
					comment.add(s.findPos("\n").trim())
				} else {					
					line = s.findPos("\n").trim()
				}
			}
			async=false
			k='@state'
			c=line.ch()
			not(c) {
				print("@@ store 추가오류 (라인오류) 소스:[$s]")
				break;
			}
			if(c.eq('#')) {
				comment=''
				continue;
			}
			if(c.eq('*')) {
				k='@persist'
				c=line.incr().ch()
			}
			if(line.start('async',true)) {
				async=true
			}
			val=''
			if(isFunc(line)) {
				vnm=line.findPos('(',0,1).trim()
				param=line.match()
				if(body) {
					if(body.find("\n")) {
						val.add("($param) => {$body}")
					} else {
						val.add("($param) => set(state=>({$body}))")
					}
				} else {
					val.add("($param) => set({$param})")
				}
			} else {
				vnm=line.move()
				c=line.ch()
				if(c.eq(':','=')) {
					if(sep) {
						v=body
						if(sep.eq('node')) {
							val = "{$v}"								
						} else {
							val = "[$v]"
						}
					} else {
						c=line.incr().ch()
						if(c.eq()) {
							val=Cf.jsValue(line.match())
						} else {
							v=line.trim()
							if(typeof(v,'num') || v.eq('true','false','null')) {
								val=v
							} else {
								val=Cf.jsValue(v)
							}
						}
					}					
				} else {
					val='""'
				}
			}
			c=s.ch()
			if(c.eq(',',';')) {
				s.incr()
			}
			if(s.ch()) {
				val.add(',')
			}
			if(comment) {
				val.add("\t\t/* $comment */")
				comment=''
			}
			nodeAppendText(store,k,"${vnm}:${val}", nl)
		}
	};
}

@baro.makeBackendFiles(root, infoFile) {
	fo=Baro.file()
	projectPath = @baro.configKeyValue(root, 'backend.projectPath')
	not(projectPath) {
		basePath = @baro.configKeyValue(root, 'projectPath')
		not(basePath) return print("@@ makeBackendFiles 백앤드 생성실패 (프로젝트 경로 미정의)")
		projectPath = pathJoin(basePath,'backend')
	}
	not(isFullPath(projectPath)) {
		return print("@@ makeBackendFiles 백앤드 경로오류 ($projectPath 는 전체경로가 아닙니다)")
	}
	not(isFolder(projectPath)) {
		fo.mkdir(projectPath)
	}
	srcPath = pathJoin(projectPath,'src')
	not(isFolder(srcPath)) {
		folders=@baro.configKeyValue(backend, node,'backend.folders')
		not(folders) {
			folders=_arr('backend.folders').add('config','controllers','routes')
		}
		fo.mkdir(srcPath)
		while(sub, folders) {
			fo.mkdir(pathJoin(srcPath,sub))
		}
	} 
}
@baro.configKeyValue(root, node, &s, type, value) {
	not(s.ch()) return;
	not(node) node=root
	cur=null, refNode = null
	getName = func(incr) {
		if(incr) s.incr()
		sp=s.cur()
		c=s.next().ch()
		while(c.eq('-')) c=s.next().ch()
		return s.trim(sp,s.cur(),true);
	};
	if( isFunc(s)) {
		fnm=s.findPos('(',0,1)
		fparam=s.match(1)
		cur=@baro.serviceFuncVal(root, node, fnm, fparam)
	} else {
		name=getName()
		c=s.ch()
		cf=root.get('@config') 
		not(cf) {
			// print("@@ configKeyValue config변수 정의되지 않았습니다 name=$name")
			cf=root
		}
		if(name.ch('@')) {
			name=name.value(1)
			cfDef=cf.get('@default')
			type=node.get('@type')
			if( cfDef && cfDef.isVar(name) ) {
				cur=cfDef.get(name)
			} else if(type && root.isVar("@$type")) {
				base=root.get("@$type")
				cur=base.get(name)
			}
			not(cur) return;
		}
		else {
			if(node.isVar(name)) {
				cur=node.get(name)
			} else if(root.isVar("@$name")) {
				base=root.get("@$name")		
				if(c.eq('.')) {
					name=getName(true)
					refNode=base.get(name)
					if(typeof(refNode,'node')) {
						cur=refNode
					} else {
						refNode=base
					}
				} else {
					cur=base
				}
			} else if(cf.isVar(name)) {
				refNode=cf.get(name)
				if(typeof(refNode,'node')) {
					cur=refNode
				} else {
					refNode=cf
				}
			} else if(cf.isVar('@default')) { 
				def=cf.get('@default')
				if(typeof(def,'node') && def.isVar(name)) {
					refNode=def
					cur=def.get(name)
				} else {
					refNode=cf
				}
			}
		}
		if(isNull(cur)) {
			not(refNode) { 
				refNode=cf
			}
			if(node.isVar(name)) {
				cur=node.get(name)
			} else if(refNode.isVar(name)) {
				cur=refNode.get(name) 
			} else {
				parent=node.parentNode() not(parent) parent=root
				cur=parent.get(name)
			}	
		}		
		if(isNull(cur)) {
			if(refNode==root) {
				print(">> refNode == root")
				while(sub,refNode ) {
					while(base,sub) {
						if(base.isVar(name)) {
							cur=base.get(name)
							break;
						}
					}
				}
			} else {
				print(">> refNode========$refNode")
				while(sub,refNode ) {
					if(sub.isVar(name)) {
						refNode=sub
						cur=sub.get(name)
						break;
					}
				}
				if(isNull(cur)) {
					while(sub,root) {
						while(base,sub) {
							if(base.isVar(name)) {
								cur=base.get(name)
								break;
							}
						}
					}
				}
			}
		}
	}
	c=s.ch()
	while(s.valid()) {
		not(c.eq('.')) break;
		s.incr()
		if(isFunc(s)) {
			fnm=s.findPos('(',0,1)
			fparam=s.match(1)
			ref=when(cur,cur,refNode)
			cur=@baro.serviceFuncVal(root, ref, fnm, fparam)
			continue;
		}
		name=getName()
		if(typeof(cur,'node')) {
			cur=cur.get(name)
		} else {
			print("@@ configKeyValue 하위키를 찾을수 없습니다 name:$name", cur)
			cur=null
		}
	}
	if( isNull(cur)) return;
	if( type.eq('ref')) {
		@baro.parseService(root,refNode,cur,name);
		print("keyValue ref >> ", name, refNode, cur)
		return refNode.get("&$name")
	} 
	if( type.eq('value','string') ) {		
		return valueOf(cur, type.eq('string'));
	}
	if( type.eq('set')) {		
		if(typeof(cur,'node')) {
			cur.set(name,@baro.configValue(root,refNode,value))
		}
	} 
	return cur;
}

@baro.parseService(root, node, &s, parentKey) {
	Cf.error(true)
	nl=conf('cf.newline')
	ss='', skip=false, skipCnt=0;
	inValue=func(&str) {
		if(str.find('@[')) return @baro.parseService(root,node,str);
		sp=str.cur()		
		c=str.ch()
		if(isFunc(str)) {
			name=str.move()
			if(name.eq('get')) {
				name=str.match().trim()
				return @baro.configKeyValue(root,node,name)
			}
		} else if(c.eq('@')) {
			name=str.incr().trim()
			return @baro.configKeyValue(root,node,name)
		}
		str.pos(sp)
		return str.trim();
	};
	while(s.valid(),idx) {
		if(Cf.error()) return;
		left=s.findPos('@[') 
		ss.add(left)
		not(s.ch()) break;
		indent=''
		if(left.find("\n")) {
			indent=@baro.lastIndent(left)
		}
		k=s.findPos(']'), def=''
		if(lineCheck(k,'||')) {
			v=@baro.configKeyValue(root, node, k.findPos('||'),'value') 
			not(v) v=inValue(k.trim())
			if(v) ss.add(@baro.configValue(root,node,v))
			continue;
		}
		ok=false
		if(lineCheck(k,'?')) {
			ok=@baro.configKeyValue(root, node, k.findPos('?'))
			c=k.ch()
			if(c.eq('(')) {
				val=k.match(1)
				if(k.ch(':')) k.incr()
			} else {
				val=k.findPos(':')
			}
			if(ok) {
				ss.add(inValue(val))
			} else if(k.valid()) {
				ss.add(inValue(k))
			}
			continue;
		}
		sp=s.cur()
		c=s.ch()
		if(c.eq('=')) {
			s.incr()
			val=_checkValue()
			not(skip) {
				@baro.configValue(root,node,k,'set',val)
			}
			continue
		}
		 
		v=@baro.configKeyValue(root,node, k, indent)
		if(v) ok=true
		if(c.eq('.')) {
			fnm=s.incr().move()
			if(fnm.eq('map','filter')) {
				c=s.ch()
				if(c.eq('(')) {
					fparam = s.match(1)
					vnm=fparam.findPos('=>').trim()
					c=fparam.ch()
					if(c) {
						print("parentKey >> $parentKey", k, v, fparam) 
						if(typeof(v,'node','array')) {
							if(fparam.start('<>')) {
								fparam=fparam.match('<>','</>')
							}							
							local=_node()
							local.var(localVar, true)
							if(node.parentNode()) local.parentNode(node.parentNode())
							root=null
							src=fparam
							if(c.eq('{') && parentKey ) {
								root=node.addNode("&$parentKey").removeAll()
								src=fparam.match(1)
							}
							isNode=when(typeof(v,'node'),true)
							while(sub,v) {
								if(typeof(sub,'node')) {
									if(isNode && sub.get('@parentArray')) continue;
								}
								local.set(vnm, sub)
								data= @baro.parseService(root,local,src)
								print(">> $fm while ", local, src, data)
								if(root) {
									cur=root.addNode()
									cur.parseJson(data)
								} else {
									ss.add(data, nl)
								}
							}
						} else {
							print("@@ $fnm 함수 $k 변수값 미정의 실행값 $fparam")
						}
						continue;
					}
				}
			}
		}
		if(c.eq('?')) {
			s.incr()
			v=_checkValue()
			if(ok) {
				not(skip) ss.add(@baro.configValue(root,node,v,indent),nl)
			}  
			if(_isElse(s)) {
				c=s.ch()
				s.start('else', true)
				c=s.ch()				
				if(s.start('@[')) {
					if(ok) {
						skipCnt++;
						skip=true
					}
					continue;
				}
				v=_checkValue()
				not(ok) {
					not(skip) ss.add(@baro.configValue(root,node,v,indent),nl)
				}
			}
			if(skip) {
				skipCnt--;
				if(skipCnt==0) {
					skip=false
				}
			}
			continue;
		} 
		if(s.start('||',true)) {
			not(ok) v=_checkValue()
			if(skip) continue;
			if(v) {				
				ss.add(@baro.configValue(root,node,v,indent),nl)
			}
			continue;
		}
		if(sp<s.cur()) {
			s.pos(sp)
		}
		if(skip) continue;
		if(ok) {
			ss.add(@baro.configValue(root,node,v))
		} else if(def) {
			ss.add(def)
		}
	}
	return ss;
	
	_checkNext = func(&s) {
		c=s.ch()
		return c.eq('<','(','{','[');
	};
	_checkFunc = func(&s) {
		c=s.ch() not(c) return;
		s.start('export',true)
		s.start('async',true)
		if( s.start('function',true) ) {
			c=s.ch()
			not(c.eq('(')) c=s.next().ch()
			if(c.eq('(')) {
				s.match()
				c=s.ch()
				if(c.eq('{')) return true;
			}
		} else if(lineCheck(s,'=')) {
			s.findPos('=')
			c=s.ch()
			if(s.start('async',true)) c=s.ch()
			if(c.eq('(')) s.match() else s.next()
			c=s.ch()
			if(s.start('=>',true)) {
				return true;
			}
		}
		return;
	};
	_checkValue = func() {
		c=s.ch() not(c) return;
		if(_checkNext(s)) {
			if(c.eq('<')) {
				v=_tagValue()
			} else {
				src=s.match()
				if(c.eq('{','[')) {
					if(c.eq('{')) v="{$src}" else v="[$src]"
				} else {
					v=src
				}
			}
		} else if(_checkFunc(s)) {
			sp=s.cur()
			s.start('export',true)
			s.start('async',true)
			if( s.start('function',true) ) {
				c=s.ch()
				if(c.eq('(')) s.match() else s.next()
				c=s.ch()
				if(c.eq('{')) s.match(1)				
			} else {
				s.findPos('=>')
				c=s.ch()
				if(c.eq('{')) s.match(1) else s.findPos("\n")
			}
			ep=s.cur()
			c=s.ch()
			if(c.eq(',',';')) {
				s.incr()
				ep=s.cur()
			}
			if(sp<ep) v=s.trim(sp,ep,true)
		} else {
			if(lineBlankCheck(s) ) {
				s.findPos("\n")
				v=s.findPos("\n")
			} else {
				s.ch()
				v=s.findPos("\n")
			}
		}
		return v;
	};
	_tagValue = func() {
		c=s.ch() not(c.eq('<')) return;
		if(s.start('<>')) {
			tag='value'
			props=''
			body=s.match('<>','</>')
		} else {
			sp=s.cur()
			tag=s.incr().move()
			s.pos(sp)
			body=s.match("<$tag","</$tag>")
			props=body.findPos('>')
		}
		if(typeof(body,'bool')) return print("@@ parseService $tag 태그 매칭오류")
		return body;
	};
	_isElse = func(&s) {
		c=s.ch()
		if(s.start('else')) return true;
		return false;
	};
}
@baro.getFuncParam(root, node, &s) {
	arr=_arr()
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',',';')) {
			s.incr()
			continue;
		}		
		if(c.eq()) {
			val=s.match()			
		} else if(isFunc()) {
			fnm=s.move()
			fparam=s.match(1)
			val=@baro.serviceFuncVal(root,node,fnm,fparam)
		} else {
			vnm=s.findPos(',').trim()
			val=@baro.configKeyValue(root,node,vnm,'value')
		}
		arr.add(val)
	}
	return arr;
}
@baro.serviceFuncVal(root,node,fnm,fparam) {
	if( fnm.eq('eval')) {
		return getVarValue(fparam)
	}
	
	funcParams = @baro.getFuncParam(root,node,fparam)
	print(">> serviceFuncVal $fnm ($funcParams)")
	if( fnm.eq('value','ref')) {
		funcParams.inject(p0,p1)
		if(p1) {
			base=root.get("@$p0")
			name=p1
		} else if(p0) {
			base=root.get('@config')
			name=p0
		}
		not(typeof(base,'node')) {
			return print("@@ serviceFuncVal::value 함수 오류 설정노드 미정의 ", p0,p1);
		}		
		val=''
		if(base.isVar(name)) {
			refNode=base
			val= base.get(name)
		} else {
			while(cur,base) {
				if(cur.isVar(name)) {
					refNode=cur
					val=cur.get(name);
				}
			}
		}
		if(fnm.eq('ref')) {
			return refNode;
		} else if(val) {
			return @baro.configValue(root,refNode,val);
		}
		return;
	}
	if(fnm.eq('filter')) {
		funcParams.inject(p0,p1,p2)
		oper='eq'
		if(p0.eq('not')) {
			oper='not'
			field=p1, checkVal=p2
		} else {
			field=p0, checkVal=p1
			if(p2) oper=p2
		}
		aa=_arr()
		while(cur,node) {
			if(oper.eq('eq','not')) {
				ok=false;
				if(typeof(cur,'node')) {
					if(checkVal) {
						if(cur.cmd(field,checkVal)) {
							ok=true;
						}
					} else {
						if(cur.get(field)) {
							ok=true;
						}
					}					
				} else {
					if(checkVal) {
						if(checkVal==cur) ok=true
					} else if(cur) {
						ok=true
					}
				}
				if(oper.eq('eq')) {
					if(ok) aa.addNode(cur) 
				} else {
					not(ok) aa.addNode(cur)
				}
			}
		}
		return aa;
	}
	if(fnm.eq('path')) {
		ss='pathJoin('
		while(fp,funcParams,idx) {
			if(idx)ss.add(',')
			ss.add("'$fp'")		
		}
		ss.add(')')
		return getVarValue(ss);
	}
	if(fnm.eq('conf')) {
		code = fparam.findPos(',').trim()
		if(code.find('.')) {
			confCode=code
		} else {
			confCode="baro.$code"			
		}
		if(fparam.ch()) {
			value = fparam.findPos(',').trim()
			return conf(@baro.configValue(root,node,value),true)
		} else {
			return conf(confCode)
		}
	}
	if( fnm.eq('fields','binds','pk','tableinfo')) {
		node = @baro.configKeyValue(root,node,"table.$fparam")
		return json(node)
	}	
	if( fnm.eq('eq')) {
		a=fparam.findPos(',').trim()
		b=fparam.findPos(',').trim()
		if(root.isVar(a)) {
			return root.cmp(a,b)
		} else if(node.isVar(a)) {
			return node.cmp(a,b)
		}
		print(">> not eq $a, $b")
		return
	} 
	if( fnm.eq('query')) {
		
	} else {
		print("@@ serviceFuncVal $fnm 함수가 정의되지 않았습니다")
	}
	return;
}
@baro.parseRoute(root, node, &s) {
	node.set('@comment', '')
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq('/')) {
			c=s.ch(1)
			if(c.eq('*')) comment=s.match() else comment=s.findPos("\n");
			nodeAppendText(node,'@comment',comment,conf('cf.newline'))
			continue;
		}
		not(c.eq('[')) return print("route 시작오류 ", node, s)
		sp=s.cur()
		method = s.match().trim().lower()
		s.ch()		
		if(lineCheck(s,'{')) {
			uri = s.findPos('{',0,1).trim()
		} else {
			uri = s.findPos("\n").trim()
		}
		cur=node.addNode(uri)
		cur.with(method, uri)
		if(s.ch('{')) {
			props=s.match(1)
			@baro.parseConfig(root, cur, props)
		}
		confCode = "baro.route:${node.name}#${uri}"
		ep=funcsAppend(s)
		if(sp<ep) {
			s.pos(ep)
			if(cur.get('@funcs')!=conf(confCode) ) {
				node.set('@modify',true)
				conf(confCode, cur.get('@funcs'),true)
			}
		} else {
			break;
		}
	}
	funcsAppend = func(&s) {
		cur.set('@funcs','')
		while(s.valid()) {
			c=s.ch() not(c) return s.cur();
			if(c.eq('/')) {
				c=s.ch(1)
				if(c.eq('*')) s.match(1) else s.findPos("\n");
				continue;
			}
			not(isFunc(s)) break;
			fnm = s.findPos('(',0,1).lower()
			fparam = s.match(1)
			cur.appendText('@funcs',"${fnm}(${fparam})")
			if(c.eq(',',';')) s.incr()
		}
		return s.cur();
	};
}

@baro.routeFuncVal(root,node,&s) {
	funcVal = func(fnm, &fparam) {
		if(fnm.eq('if','not')) {
			node.set('newline','')
			if(fnm.eq('if')) return "if($fparam) ";
			chk=''
			while(fparam.valid(),n) {
				a=fparam.move()
				chk.add("isEmpty(!$a)")
				c=fparam.ch() not(c) break;
				if(c.eq(',')) {
					chk.add('||')
				}
			}			
			return "if($chk) ";
		}
		ss=''
		node.set('newline',conf('cf.newline'))
		switch(fnm) {
		case js: 
			ss=fparam
		case auth:
			vnm = fparam.trim() not(vnm) vnm='userId'
			node.get('@varNames').add(vnm)
			ss="const {$vnm} = getAuth(req)"
		case sql:
			node.set('@bindTarget', '')
			code=fparam.move(), vnm='', binds=''
			if(fparam.ch('(')) {
				binds=fparam.match()
				if(binds.ch(':')) {
					target=binds.trim(1)
					node.set('@bindTarget', target)
				} else {				
					if(binds.find(':')) {
						left = binds.findPos(':')
						target = binds.trim()
					} else {
						left = binds
						target = 'req.body'
					}
					ss.add("const {$names} = $target", conf('cf.newline'))
				} 
			}
			fparam.ch()
			if(fparam.start('=>',true)) {
				vnm= fparam.trim()
			}
			sql = @baro.configKeyValue(root, node, "sql.$code")
			not(sql) {
				sql="쿼리 코드 $code 미정의"
				print("@@ $sql")
			}
			query = @baro.parseQuery(sql, target)
			if(vnm) ss.add(#[const ${vnm}=await sql`${query}`])
			node.set('@async',true)
		case res:
			ss="res.status(200).json($fparam)"
		case error:
			if(isNum(fparam)) {
				num = fparam.findPos(',').trim()				
			} else {
				num = 400
			}
			c=fparam.ch()
			if(c.eq('{')) {
				val=fparam.trim()
			} else {
				if(c.eq()) {
					err=fparam.match()
				} else {
					err=fparm
				}
				val=Cf.val(#[{error:`${err}`}])
			}
			ss="res.status($num).json($val)"
		default: print("@@ 라우트 함수 $fnm 미정의")
		}
		return ss;
	}; 
	isNum = func(s) {
		v=s.findPos(',').trim()
		return when(typeof(v,'num'), true)
	};
	node.set('@funcsVal', '')
	node.addArray('@varNames').reuse()
	while(s.valid()) {
		not(isFunc(s)) break;
		fnm =s.findPos('(',0,1)
		fparam = s.match(1)
		fsrc=funcVal(fnm, fparam)
		print("####### routeFuncVal == $fnm $fsrc")
		node.appendText('@funcsVal', fsrc)
		if(node.newline) node.appendText('@funcsVal', node.newline)
	}
}
@baro.makeRoute(root) {
	not(root) root = object('baro.services')
	base=root.get('@route')
	not(base) return print("routes 기준노드 미정의")
	while(cur, base ) {
		print(">>",cur)
		while(sub,cur) {			
			not(sub.isVar('@funcs')) continue;
			src=@baro.parseService(root, sub, sub.ref('@funcs'))
			print(">> make route cur src==$src", sub.uri)
			@baro.routeFuncVal(root, sub, src)
		}
	}
}

@baro.tableFuncVal(root,node,&s) {
	comment=''
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',')) {
			s.incr()
			continue;
		}
		if(c.eq('/')) {
			c=s.ch(1)
			if(c.eq('/')) cmt=s.findPos("\n") else cmt=s.match(1);
			if(comment) comment.add(conf('cf.newline'))
			comment.add(cmt)
			continue
		}
		a=s.move().lower()
		if(a.eq('create')) {			
			b=s.move().lower()
			if(b.eq('table')) {
				name = s.move().lower()
			} else {
				name = b
			}
		} else {
			name=a
		}
		c=s.ch()
		not(c.eq('(')) return print("table $name 시작오류")
		table=node.addNode(name)
		table.set('@name',name)
		if(comment) {
			table.set('comment', comment)
			comment=''
		}
		confCode = "baro.table#$name"
		src=s.match(1)
		prevSrc = conf(confCode)
		if(src.eq(prevSrc) ) {
			node.set('@modifySource',true)
			table.set('@modify', true)
		}
		table.set('@source', src)
		if(a.eq('create')) {
			table.set('@value', src)
		} else {
			table.set('@useParse', true)
			table.set('@value', @baro.makeCreateQuery(root,table,src))
		}
	}
}
@baro.makeCreateQuery(root,table,&s) {
	while(s.valid()) {
		if(lineBlankCheck(s)) {
			s.findPos("\n")
			continue;
		}
		c=s.ch()
		if(c.eq(',')) {
			s.incr()
			continue;
		}
		if(lineCheck(s,'//')) {
			info = s.findPos('//')
			comment=s.findPos("\n").trim()
		} else if( lineCheck(s,'/*')) {
			info = s.findPos('/*',0,1)
			comment=s.match(1)
		} else {
			info = s.findPos("\n")
			comment=''
		}
		name=info.move(), type='text', size=''
		cur=table.addNode(name)
		if(isFunc(info)) {
			type=info.findPos('(',0,1).trim().lower()
			size=info.match().trim()
			cur.with(name,type,size)
		} else {
			type=info.move().lower()
			cur.with(name,type)
		}
		if(comment) cur.set('comment', comment)
		cur.set('fieldName', @baro.dbFieldName(name).upper())
		while(info.valid()) {
			c=info.ch() not(c) break;
			if(c.eq(',',';')) {
				info.incr()
				continue;
			}
			if(isFunc(info)) {
				fnm = info.move().lower(), fparam=''
				c=info.ch()
				if(c.eq('(')) {
					fparam=info.match()
				}				
				if(fnm.eq('def','default')) {
					def=''
					if(fparam) {
						c=fparam.ch()
						if(c.eq()) {
							def=fparam.match().trim()
						} else {
							def=fparam.trim()
						}
					}
					cur.set('def',true)
					if(def) cur.set('defValue',def)
				} else if(fnm.eq('fk')) {
					cur.set('fk',true)
					cur.set('fkValue',fparam)
				} else if(fnm.eq('in')) {
					fieldName=cur.set('fieldName')
					ss=''
					if(fieldName && ss ) {
						while(fparam.valid()) {
							a=fparam.findPos(',').trim()							
							if(typeof(a,'num') ) {
								ss.add(a)
							} else {
								c=a.ch()
								if(c.eq()) ss.add(a) else ss.add("'",a,"'")
							}
						}
						nodeAppendText('fieldCheck', "CHECK ($fieldName IN ($ss))", ' ')
					}
				} else if(fnm.eq('check')) {
					nodeAppendText('fieldCheck', "CHECK ($fparam)", ' ')					
				} else {
					print("@@ $fnm 테이블 필드속성 미정의")
				}
			} else {
				name = info.move()
				if(name.eq('unique')) name='uniq'
				if(name.eq('pk','fk','notnull','def','uniq')) {
					cur.set(name,true)
				} else {
					nodeAppendText('fieldInfo', name, ' ')
				}
			}			
		}
		print(">> 테이블필드정보 : $cur")
	}
	nl=conf('cf.newline')
	ss=''
	fkCnt=0
	while(cur, table) {
		if( cur.isVar('fkValue') ) fkCnt++;
	}
	while(cur, table, idx) {
		cur.inject(fieldName, name, def, defValue, fk, pk, uniq, type, size, notnull, fieldCheck, fieldInfo)
		if(idx) ss.add(',', nl)
		not(type) type='text'
		if(type.eq('flot')) {
			type='decimal'
			not(size) size='10, 2'
		} else if(type.eq('datetime','dt')) {
			type='timestamp'
		} else if(type.eq('num')) {
			if(size) type='decimal' else type='integer'
		} else if(type.eq('vc')) {
			type='varchar'
			not(size) size='255'
		} else if(type.eq('uuid')) {
			def=true
			defValue='gen_random_uuid()'
			// defValue='uuid_generate_v4()'
		} else if(type.eq('auto')) {
			type='serial'
		} else if(type.eq('int','num')) {
			type='integer'
		}
		ss.add("$fieldName ", type.upper())
		if(size) {
			ss.add("($size)")
		}
		if(notnull) ss.add(" NOT NULL")
		if(pk) ss.add("PRIMARY KEY")
		if(def) {
			if(defValue) {
				if(defValue.eq('now')) defValue='NOW()'
			}
			else {
				if(type.eq('integer','decimal')) defValue='0'
				else if(type.eq('timestamp')) defValue='CURRENT_TIMESTAMP'
				else if(type.eq('date')) defValue='CURRENT_DATE'
				else if(type.eq('time')) defValue='CURRENT_TIME'
				else if(type.eq('char') && size.eq('1')) defValue='Y'
				else if(type.eq('bool')) defValue='true'
				else defValue=''
			}
			if(isFunc(defValue)|| typeof(defValue,'num') || defValue.eq('CURRENT_DATE','CURRENT_TIME','true','false')) {
				ss.add(" DEFAULT $defValue")
			} else if(defValue) {
				ss.add(" DEFAULT '$defValue'")
			}
		}
		if(fk) {
			s=cur.ref('fkValue')
			aa=s.move(), bb=''
			c=s.ch()
			if(c.eq('.')) {
				bb=s.incr().move()
			}
			if(aa) {
				if(fieldInfo) fieldInfo.add(' ')
				fieldInfo.add(aa) if(bb)fieldInfo.add(" ($bb)")
			}			
		}
		if(uniq) { 
			nodeAppendText(table, '@tableUniques', fieldName, ',')
		}		
		if(fieldCheck) {
			ss.add(" $fieldCheck")
			// ALTER TABLE ${table} ADD CHECK (${fieldName} >= 30000)
		}
		if(fieldInfo) {
			ss.add(" $fieldInfo")
		}
	}
	fields=table.get('@tableUniques')
	if(fields) {
		ss.add(',',nl,'UNIQUE(',fields,')')
	}
	return ss;
}

@baro.sqlFuncVal(root,node,&s) {
	comment=''
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',')) {
			s.incr()
			continue;
		}
		if(c.eq('/')) {
			c=s.ch(1)
			if(c.eq('/')) cmt=s.findPos("\n") else cmt=s.match(1);
			if(comment) comment.add(conf('cf.newline'))
			comment.add(cmt)			
			continue
		} 
		not(isFunc(s)) return print("sql 설정 시작오류 ", s)
		name = s.findPos('(',0,1).trim() not(name) break;		
		sql=node.addNode(name)
		sql.set('@name',name)
		sql.set('@source',s.match(1))
	}
}
@baro.dbFieldName(&s) {
	not(typeof(s,'string')) return;
	ss='', upper=false
	while(n=0,s.size()) {
		c=s.ch(n)
		if(c.is('upper')) {
			if(n) ss.add('_',c.lower())
		} else {
			ss.add(c)
		}
	}
	return ss;
}

@baro.configValue(root,node,&s,indent) {
	not(s) return;	
	if(typeof(s,'string')) {		
		return valueOf(s);
	}
	if(typeof(s,'node')) {
		cur=s
		if(cur.isVar('@value')) return cur.get('@value');
	}
	return "$s";
	
}
@baro.parseConfigArray(parent, node, arr, &s ) {
	not(typeof(arr,'array')) return print('@baro.parseConfigArray 배열 미정의');
	if(typeof(s,'bool')) return print('@baro.parseConfigArray 매치오류');
	arr.reuse()
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',')) {
			s.incr()
			continue;
		}
		if(c.eq()) { 
			arr.add(s.match())
			continue;
		}
		if(c.eq('{')) {
			cur= node.addNode()
			@baro.parseConfig(parent,cur,s.match())
			arr.add(cur)
			cur.set('@parentArray', arr)
		} else if(c.eq('[')) {
			a = node.addArray()
			@baro.parseConfigArray(parent,node,a,s.match())
			arr.add(a)
		} else if(lineCheck(s,',')) {
			v=s.findPos(',').trim()
			arr.add(v)
		} else {
			v=s.findPos("\n").trim()
			arr.add(v)
		}
	}
	return arr;
}

@baro.parseConfig(parent, node, &s, mode) {
	not(s) return;
	not(typeof(node,'node')) return print('@baro.parseConfig target노드 오류');
	if(typeof(s,'bool')) return print('@baro.parseConfig 매치오류');
	// node.set('@serviceNode', parent)
	arr=node.addArray('@keyArray')	
	if(mode.eq('reset')) {
		node.reuse()
		arr.reuse()
	}
	isDef = func(&s) {
		if(s.ch('@')) s.incr()
		c=s.next().ch()
		while(c.eq('.')) c=s.next().ch()
		return c.eq('{');
	}
	addProp = func(k,v) {
		if(arr.find(k)) {			
			if(mode.eq('overwrite')) {
				arr.remove(k)
				arr.add(k)
			} else if(mode.eq('base')) {
				name=node.name
				if( name && k.ne('name')) {
					sub=node.addNode(name)
					Cf.funcNode('parent').set('node',sub)
				}
			} else {
				if(conf('cf.skipOverwrite')=='Y') {
					print("@@ parseConfig $k 가 중복등록되었습니다", node)
					return;
				}
			}
		} else {
			arr.add(k)
		}
		node.set(k,v)
	};
	comment=''
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',',';')) {
			s.incr()
			continue;
		}
		if(c.eq('/')) {
			c=s.ch(1)
			if(c.eq('/')) cc=s.findPos("\n") else cc=s.match()
			if(comment) comment.add(' ')
			comment.add(cc)
			continue;
		}
		if(c.eq()) {
			k=s.match()
		} 
		else {
			sp=s.cur()
			if(c.eq('@','#')) {
				s.incr()
			}
			c=s.next().ch()
			while(c.eq('-')) {
				c=s.incr().next().ch()
			}
			ep=s.cur()
			k=s.trim(sp,ep,true)
		}
		if(comment ) {
			node.set("#${k}:comment", comment)
			comment=''
		}
		if(c.eq('{','(')) {
			source = s.match(1)
			if(c.eq('{')) {
				addProp(k,removeIndentText(source))
			} else {
				addProp(k,trimLine(source))
			}
			continue
		}
		bprop=false
		if(c.eq(':','=')) {			
			bprop=true
			c=s.incr().ch()
		}
		if(isDef(s)) {
			sp=s.cur()
			if(s.ch('@')) s.incr()
			c=s.next().ch()
			while(c.eq('.')) c=s.next().ch()
			kk=s.trim(sp,s.cur(),true)
			if(c.eq('{','(')) {
				source=s.match(1)					
				addProp(k,"$kk=>$source")
			} else {
				return print("$kk def 시작오류")
			}
			continue;
		}
		if(c.eq()) {
			addProp(k,s.match())
			continue
		} 
		
		if(isFunc(s)) {
			sp=s.cur()
			while(isFunc(s)) {
				s.findPos('(',0,1)
				s.match()
				c=s.ch() 
				if(c.eq(',',';')) s.incr().ch()
			}
			v = s.trim(sp,s.cur(),true)
			addProp(k,v)
			continue
		}
		if(c.eq('<')) {
			if(s.start('<>')) {
				addProp(k, s.match('<>','</>'))
			} else {
				sp=s.cur()
				c=s.incr().next().ch()
				while(c.eq('-','.')) {
					c=s.incr().next().ch()
				}
				tag = s.trim(sp+1,s.cur(),true)
				if( @baro.isSingleTag(tag,s) ) {
					prop=s.findPos('>')
					c=prop.ch(-1)
					if(c.eq('/')) {
						sp=prop.cur()
						prop=prop.value(sp,-1)
					}
				} else {
					s.pos(sp)
					src=s.match("<$tag","</$tag>",8) 
					if(typeof(src,'bool')) {
						return print("@@ $tag 매칭오류")
					}
					prop=src.findPos('>')
				}
				addProp("@k","${tag}:${prop}")
				addProp(k,src)
			}
			continue
		}
		if(c.eq('{','[')) {
			sub=s.match(1)
			if(c.eq('{')) {
				cur = node.addNode(k)
				@baro.parseConfig(parent,cur,sub)
			} else {
				arr = node.addArray(k)
				@baro.parseConfigArray(parent,node,arr,sub)
			}
			continue
		}
		
		if( bprop) { 
			if( lineCheck(s,'@[') ) {
				v=s.findPos("\n").trim()
			} else if(lineCheck(s,';')) {
				v=s.findPos(';').trim()
			} else if(lineCheck(s,',')) {
				v=s.findPos(',').trim()
			} else {
				v=s.findPos("\n").trim()
				if(v.eq('null','true','false')) {
					if(v.eq('null')) v=null else if(v.eq('true')) v=true else v=false;
				}
			}
		} else {
			v=true
		}
		addProp(k,v)
	}
	return node;
}
