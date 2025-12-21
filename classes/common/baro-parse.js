 

@baro.propValue(&s, propName) {
	while(s.valid()) {
		left=s.findPos(propName)
		c=s.ch() not(c) break;
		if(c.eq('=',':')) {
			c=left.ch(-1)
			if(c.is('alphanum')) continue;
			c=s.incr().ch()
			if(c.eq()) {
				val=s.match()
			} else {
				val=s.findPos(", \t\n",4)
			}
			return value;
		}
	}
	return;
}

@baro.isHtmlTag(key) {
	tag=key.lower()
	if(tag.start('h')) {
		c=key.value(1)
		if(typeof(c,'num')) return true;
	} else if(tag.eq('div','form','span','p','img','button','input','video','a','nav','header')) {
		return true;
	}
	return false;
}
@baro.ignoreStyleProp(k) { return k.eq('id','key','class','varName','render','tag') }
@baro.loadPage(fullpath) {
	not(fullpath) {
		fullpath = conf('baro.loadPagePath')
		not(fullpath) fullpath='c:/bpdt/project/page-test.js'
	}
	filePathInfo(fullpath).inject(watchPath, fileName)	
	not(conf("cf.newline")) conf("cf.newline","\r\n",true)
	parent = object('baro.pages')
	parent.set('@style','')
	
	watcherId = 'webpageWatcher'
	watcherConfig=Cf.rootNode().get('@watcherFiles').get(watcherId)
	not(watcherConfig) {
		watcherConfig = @baro.startWatcher(watcherId, watchPath)
		not(watcherConfig) return print("$fullpath 경로 모니터 모듈 실행 오류");
		watcherConfig.set('pages',parent)
		print("@@ watcherConfig 새로 생성 ", watcherConfig)
		
	}
	not(watcherConfig.watcherNames.find(fileName)) {
		watcherConfig.watcherNames.add(fileName)
	}
	src = fileRead(fullpath)
	Cf.error(true)
	@baro.parsePages(parent, src)
	if(Cf.error() ) {
		err = cf.error()
		print("페이지 로딩실패 : $err")
	}
}
@baro.initPage(parent,page,src,template) {
	nl = conf('cf.newline')
	pageCode=page.pageCode
	prev = conf("pageSrc.${pageCode}")
	if(prev.eq(src)) {
		print("######## @baro.parsePage not chanage", page)
		// conf('baro.debugMode', 'true', true)
		not(conf('baro.debugMode')) return;
	}
	pageTemplatePath = conf('baro.pageTemplatePath')
	not(pageTemplatePath) {
		pageTemplatePath = 'c:/bpdt/project/page-template.txt'
		conf('baro.pageTemplatePath', pageTemplatePath)
	}
	not(isFile(pageTemplatePath)) return print("$pageCode 페이지 초기화 실패 페이지 템플릿 파일이 없습니다");
	not(template) template = fileRead(pageTemplatePath)
	nodeInitVal(page,'@pageCss')
	nodeInitVal(page,'@pageFuncs')
	nodeInitVal(page,'@pageInit')
	page.set('@varIndex',1)
	page.addNode('@varMap').reuse()
	page.pageId = @baro.varName("${page.pageCode}-page")

	Cf.error(true)
	@baro.makeHtmlLayout(parent, page, src) if(catchError()) return;
	@baro.makeHtmlProps(parent, page) if(catchError()) return;
	@baro.makePageScript(parent, page) if(catchError()) return;
	if( parent.isset('@style')) {
		parent.set('@style','')
		nodeAppendText(page, '@pageCss', parent.get('@style'))
	}
	if( page.isset('@css')) {
		nodeAppendText(page, '@pageCss', page.get('@css'))
	}
	if( page.isset('@cssVar')) {
		nodeAppendText(page, '@pageCss', page.get('@cssVar'), nl)
	}
	nodeAppendText(page, '@pageFuncs', page.get('@funcs'))
	nodeAppendText(page, '@pageInit', page.get('@funcsInit'))
	nodeInitVal(page,'@css')
	nodeInitVal(page,'@cssVar')
	nodeInitVal(page,'@funcs')
	nodeInitVal(page,'@funcsInit')

	conf("pageSrc.${pageCode}", src, true)
	pageSrc = @baro.parseSource(parent, page, page, template,'value')
	print("## pageSrc ==> $pageSrc")
	@baro.changePageScript(parent, page, pageSrc)
}
@baro.initLayoutVar(page, node, key) {
	not(key) return print("레이아웃 노드 추가오류 키미정의", node);
	varMap = page.get('@varMap')
	if(@baro.isHtmlTag(key)) {
		node.set('tag',key)
	}
	node.set('class',key)
	if(key.eq('space')) {
		node.set('flex','1')
	}
	if(varMap.isset(key)) {
		key = Cf.val(key,'-',page.incrNum('@varIndex'))
	} else {
		varMap.set(key,true)
	}
	node.set('key', key)
	nodeInitVal(node,'@html')
	nodeInitVal(node,'@attr')
	nodeInitVal(node,'@css')
	nodeInitVal(node,'@event')
}

@baro.parsePages(parent, &s, template) {
	isProp = func(&s) { 
	c=s.ch()
		return c.eq('{') 
	};
	page = null
	while(s.valid()) {
		left = s.findPos('##>')
		if(page) {
			@baro.initPage(parent,page,left,template)
			if( Cf.error() ) return;
		}
		c=s.ch()
		not(c) break
		if( c.eq('*')) {
			page=null
			line = s.findPos("\n")
			print("@@ skip page : $line")
			continue;
		}
		note = null
		if( lineCheck(s,'[')) {
			path = s.findPos('[',0,1).trim()
			pageCode = @baro.varName(path)
			note = s.match(1).trim()
			bprop = isProp(s)
		} 
		else if( lineCheck(s,'{')) {
			path = s.findPos('{',0,1).trim()
			pageCode= @baro.varName(path)
			bprop = true
		}
		else {
			path = s.findPos("\n").trim()
			pageCode = @baro.varName(path)
			bprop = isProp(s)
		}
		page = nodeReuse(parent.addNode(pageCode))
		if( bprop ) {
			s.findPos('{',0,1)
			data = s.match(1) if(typeof(data,'bool')) return print("$pageCode 페이지 설정정보 매칭오류")
			@baro.parseProps(parent,page,page,data)
			arr=page.get('@keyArray') not(typeof(arr,'array')) return print("page props 키배열 오류");
			while(k, arr) {
				v=page.get(k)
				if(v.find('@[')) {
					src=@baro.parseSource(parent,page,page,v,'value')
					page.set(k,src)
				}
			}
		}
		page.pageCode = pageCode
		page.path = "${path}.js"
		if(note) page.note = note
	}	
}

@baro.makePageScript(parent, page, savePath) {	
	layout = page.get('@layout')
	if(layout.tag) {
		root = layout.child(0)
	} else {
		root = layout
	}
	root.set('varName', 'root')	
	pn=root.parentNode()
	pn.set('varName', 'pageLayoutElement')
	not(typeof(root,'node')) {
		return print("@@ make page script 오류 페이지 레이아웃 노드미정의", page, layout)
	}
	ss=@baro.pageScript(parent, page, root)
	nodeAppendText(page, '@pageInit', ss);
	print("page script == $ss")
}

@baro.pageScript(parent, page, node) {
	nl=conf("cf.newline")
	ss=''
	parentVar = node.parentNode().get('varName')
	node.inject(tag,key,varName)
	not(tag) tag='div'
	if( node.childCount() || node.isset('@event') ) {
		not(varName) {
			print("varName not define node==>$node")
			return;
		}
		ss.add("const ${varName}=")
	}
	ss.add('$',"('<${tag}")	
	if(node.get('@attr')) {
		ss.add(' ', node.get('@attr'))
	}
	if(node.get('@styValue')) {
		ss.add(' style="',node.get('@styValue'),'"')
	}
	ss.add("/>')")
	if(node.get('@sty')) {
		css=node.get('@sty')
		ss.add(".css({$css})")
	}
	if(node.isset('@html')) {
		html=node.get('@html')
		ss.add(".html(`${html}`)")
	}
	ss.add(".appendTo($parentVar)",nl)
	if(node.get('@event')) {
		ss.add(node.get('@event'),nl)
	}
	if(node.isset('render')) {
		baseElement = null
		fnm = node.get('render')
		if(typeof(fnm,'bool')) {
			renderFunc = 'renderItem(item)'
		}
		else if(lineCheck(fnm,'(')) {
			// renderList(item) or renderList(base, item)
			baseElement = _baseElement(fnm)
			renderFunc = fnm
		} else {
			renderFunc = "${fnm}(item)"
		}
		
		if(baseElement) {
			src=@baro.pageRenderScript(parent,page,node,baseElement)
			fsrc = #[
function ${renderFunc} {
	${src}
}]
			nodeAppendText(page,'@funcs',fsrc,nl)
			ss=''
		} else {
			src=@baro.pageRenderScript(parent,page,node)
			fsrc = #[
function ${renderFunc} {
	const baseElement = getRenderElement('$varName')
	${src}
}]
			ss.add("setRenderElement('$varName', $varName)",nl)
			nodeAppendText(page,'@funcs',fsrc,nl)
		}
		return ss;
	}
	while(cur,node) {
		src=@baro.pageScript(parent,page,cur)
		if(src) ss.add(src)
	}
	return ss;
	
	_baseElement = func(&s) {
		name = s.findPos('(')
		left = s.findPos(')')
		a = left.findPos(',').trim()
		return when(left.ch(),a)
	};
}

@baro.pageRenderScript(parent, page, node, baseElement) {
	_renderHtml = func(&s) {
		ss=''
		not(s.find('@[')) return s;
		while(s.valid()) {
			left = s.findPos('@[',0,1)		
			ss.add(left) not(s.ch()) break;
			s.incr()
			param = s.match(1)
			if(typeof(param,'bool')) continue;
			if(param.ch()) {
				ss.add('${',param,'}')
			}
		}
		return ss;
	};
	addCheck = false
	nl = conf("cf.newline")
	ss=''
	if(baseElement) {
		parentVar=baseElement
		addCheck = true
	} else {
		p=node.parentNode()
		parentVar=when(p.isset('render'),'baseElement',p.varName) 
		not(node.isset('render')) addCheck = true
	}
	if(addCheck ) {		
		not(node.tag) node.tag='div'
		ss.add("const ${node.varName}=",'$',"(`<${node.tag}")	
		if(node.get('@attr')) {
			attr=_renderHtml(node.ref('@attr'))
			if(attr) ss.add(' ', attr)
		}
		if(node.get('@styValue')) {
			ss.add(' style="',node.get('@styValue'),'"')
		}
		ss.add("/>`)")
		if(node.get('@sty')) {
			css=node.get('@sty')
			ss.add(".css({$css})")
		}
		if(node.isset('@html')) {
			html=_renderHtml(node.ref('@html'))
			if(html) ss.add(".html(`${html}`)")
		}
		ss.add(".appendTo($parentVar)",nl)
		if(node.get('@event')) {
			ss.add(node.get('@event'),nl)
		}		
	}
	while(cur, node) {
		ss.add(@baro.pageRenderScript(parent,page,cur))
	}	  
	return ss;
}

@baro.parseArray(parent, page, node, arr, &s ) {
	not(typeof(arr,'array')) return print('@baro.parseArray 배열 미정의');
	if(typeof(s,'bool')) return print('@baro.parseArray 매치오류');
	arr.reuse()
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',')) {
			s.incr()
			continue;
		}
		if(c.eq()) {
			v=s.match()
			arr.add(v)
			continue;
		}
		if(c.eq('{')) {
			cur= node.addNode()
			@baro.parseProps(parent,page,cur,s.match())
			arr.add(cur)
		} else if(c.eq('[')) {
			a = node.addArray()
			@baro.parseArray(parent,page,node, a, s.match())
		} else if(lineCheck(s,',')) {
			v=s.findPos(',').trim()
			arr.add(v)
		} else {
			v=s.findPos("\n").trim()
			arr.add(v)
		}
	}
}
@baro.parseProps(parent,page, node, &s, ignoreProp) {
	if(typeof(s,'bool')) return print('@baro.parseProps 매치오류');
	arr = node.addArray('@keyArray').reuse()
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',',';')) {
			s.incr()
			continue;
		}		
		if(c.eq()) {
			k=s.match()
		} 
		else {
			sp=s.cur()
			if(c.eq('@')) {
				s.incr()
			} else if(s.start('--')) {
				s.incr(2)
			}
			c=s.next().ch()
			while(c.eq('-')) {
				c=s.incr().next().ch()
			}
			k=s.trim(sp,s.cur(),true)
		}
		if(c.eq('(')) {
			fnm = k
			fparam=s.match(1).trim(), fsrc=''
			c=s.ch()
			if(c.eq('{')) {
				key = "&$fnm"
				fsrc=s.match(1)
				node.set(key, "${fparam}=>${fsrc}")
				not(arr.find(key)) arr.add(key)
			} else {
				key = "^$fnm"
				node.appendText(key, "{$fparam}")
				not(arr.find(key)) arr.add(key)
			}
			c=s.ch()
			if(c.eq(';')) s.incr().ch()			
			continue
		}
		
		bprop=false, bsty=false
		if(c.eq(':','=')) {			
			if(ignoreProp) {
				bsty=true
			} else {
				if(c.eq(':')) {
					bsty=true
				} else{
					bprop=true
				}
			}
			c=s.incr().ch()
		}
		key='', v=''
		if(@baro.isFunc(s)) {
			key=k
			sp=s.cur()
			while(@baro.isFunc(s)) {
				s.findPos('(',0,1)
				s.match()
				c=s.ch() 
				if(c.eq(';')) s.incr().ch()
			}
			v = s.trim(sp,s.cur(),true)
			node.set(key,v)
			not(arr.find(key)) arr.add(key)
		}
		else if(c.eq()) {			
			v=s.match()
			if(bsty) {
				key=k
			} else if(bprop) {
				key="#$k"
			} 
			node.set(key, v)
			not(arr.find(key)) arr.add(key)
		} else if(c.eq('{')) {
			cur = node.addNode(k)
			@baro.parseProps(parent,page,cur,s.match(),ignoreProp)
		} else if(c.eq('[')) {
			arr = node.addArray(k)
			@baro.parseArray(parent,page,node,arr,s.match())
		} else if(c.eq('<')) {
			sp=s.cur()
			html=''
			if(s.start('<>')) {
				html.add(s.match('<>','</>'))
			} else {
				c=s.incr().next().ch()
				while(c.eq('-')) {
					c=s.incr().next().ch()
				}
				tag = s.trim(sp+1,s.cur(),true)
				s.pos(sp)
				src=s.match("<$tag","</$tag>",8) if(typeof(src,'bool')) return print("$tag 매칭오류", page.pageCode)				
				p=src.findPos('>')
				html.add("<${tag}") if(p.ch()) html.add(" $p>") else html.add(">");
				html.add(@baro.parseSource(parent,page,node,src,'value'))
				html.add("</$tag>")
			}
			node.set("@$k", html)
		} else {			
			if(bsty || bprop) {
				if(lineCheck(s,',')) {
					v=s.findPos(',').trim()
				} else {
					v=s.findPos("\n").trim()
				}
			}
			if(bsty) {			
				key = k
			} else if(bprop) {
				key = "#$k"
			} else {
				key = k
				v=true
			}
			node.set(key,v)
			not(arr.find(key)) arr.add(key)
		}
	}
}
@baro.makeHtmlLayout(parent, page, &s) { 
	layout = page.addNode('@layout').removeAll(true)
	cur = null	
	parentArray=[]
	indentArray=[]
	parentArray.add(layout)	
	while(s.valid()) {
		if(lineBlankCheck(s)) {
			s.findPos("\n")
			continue;
		}
		a = indentText(s)
		c = s.ch()
		not(c) return;
		if( s.start('end') ) {
			not(cur) continue;
			if( cur && lineCheck(s,'<')) {
				s.findPos('<',0,1)
				_tagValue()
			}
			s.findPos("\n")
			_tagValue()
			continue;
		}
		if( indentArray.size()) {
			if(a) {
				idx=indentArray.find(a)
			} else {
				idx=0
			}
			if(idx==-1) {
				idx=indentArray.size()
				indentArray.add(a)
			}
		} else {
			idx=0
			indentArray.add(a)
		}
		root = parentArray.get(idx)
		not(root ) return print("@@ 레이아웃 분석 부모노드 찾기오류 idx:$idx");
		if(c.eq('.','#')) s.incr()
		sp = s.cur()
		c=s.next().ch(1)
		while(c.eq('-','.')) c=s.incr().next().ch(1)
		key = s.trim(sp, s.cur(), true)
		cur = root.addNode()
		@baro.initLayoutVar(page,cur,key)
		if( lineCheck(s,'{') ) {
			left = s.findPos('{',1,1)
			body = s.match(1)
			if(typeof(body,'bool')) return print("레이아웃 속성 매핑오류", left);
			if( left.ch()) {				
				cur.appendText('@html', left.trim())
			}
			@baro.parseProps(parent,page,cur, body)
		} else if(_checkProp(s)) {
			s.ch()
			body = s.match(1)
			@baro.parseProps(parent,page,cur, body)
		}
		line = @baro.getLine(s);
		// print("======> $key ", line)
		not(_tagValue()) {
			left = s.findPos("\n")
			if( left.ch()) {
				cur.appendText('@html', left.trim())
				if(_checkProp(s)) {
					s.ch()
					body = s.match(1)
					@baro.parseProps(parent,page,cur, body)
				}
			}
		}
		if( Cf.error() ) return;
		render = cur.get('render')
		if(render) {
			if(typeof(render,'bool')) render='render'
			@baro.pageRenderScript(parent,page,cur,render)
		}
		if(root==layout) {
			cur.varName = @baro.varName("${page.pageCode}-${cur.key}")
		} else if( cur.id ) {
			cur.varName = @baro.varName(cur.id)
		} else {
			cur.varName = @baro.varName(cur.key)
		}
		setArray(parentArray, idx+1, cur)
	}
	_checkProp = func(&s) {
		if(lineBlankCheck(s) ) {
			c=s.ch()
			return c.eq('{');
		}
		return false;
	};
	_checkTag = func(&s) {
		c=s.ch()
		return c.eq('<')
	};
	_tagValue = func() {
		not(_checkTag(s) ) return false;
		while(_checkTag(s)) {
			c=s.ch() not(c) break;
			cc=s.ch(1)
			if(cc.eq('>')) {
				body=s.match('<>','<>',1)
				print("@@ <><> html : $body")
				cur.appendText('@html',body)
				continue;
			}			
			sp=s.cur()
			c=s.incr().next().ch(1)
			if(c.eq('-',':')) c=s.incr().next().ch(1)
			tag=s.trim(sp+1, s.cur(), true)
			print("_checkTag", page.pageCode, tag, line)
			s.pos(sp)
			body=s.match("<$tag", "</$tag>",8)
			if(typeof(body,'bool')) {
				return print("매칭되는 태그를 찾을수 없습니다", left, tag);
			}
			if(tag.eq('css','style')) {
				body.findPos('>')
				src=@baro.parseSource(parent,page,cur,body,'css')
				if(tag.eq('css')) {
					page.appendText('@css', src) 
				} else {
					parent.appendText('@style', src)
				}
			} 
			else if(tag.eq('js','init')) {
				body.findPos('>')
				src=@baro.parseSource(parent,page,cur,body,'js')
				if(tag.eq('js')) {
					nodeAppendText(page,'@funcs', src, conf("cf.newline"))
				} else {
					nodeAppendText(page,'@funcsInit', src, conf("cf.newline"))
				}
			} 
			else {
				props=body.findPos('>')
				src=@baro.parseSource(parent,page,cur,body,'value')
				cur.appendText('@html', "<$tag")
				if(props.ch()) {
					cur.appendText('@html'," $props>")
				} else {
					cur.appendText('@html', ">")
				}
				cur.appendText('@html', src,"</$tag>")
			}
		}
		return true;
	};
	
}
@baro.applyStyleFunc(parent, page, node, fnm, &s, cssCheck) { 
	if(map.isset(fnm)) {
		fparam=s.match(1)
		not(typeof(fparam,'bool')) {
			if( cssCheck ) {
				key = @baro.styleKey(map.get(fnm))
				sty = "${key}:${fparam};"
			} else {
				key = map.get(fnm)
				val =@baro.styleValue(fparam)
				sty ="${key}:${val}"
				nodeAppendText(node,'@sty',sty,",","$key:")
			}
			return sty;
		}
	} else {
		while(s.valid(),idx) {
			not(s.ch()) break;
			fparam=s.match(1)
			@baro.funcVal(parent,page,node,fnm,fparam)
		}
	}
	return;
}
@baro.colorMap(page, param) {
	map=page.addNode('@colorMap')
	idx = 1
	if(param ) {
		a=param.get(0)
		if(typeof(a,'bool') && a) {
			map.removeAll()
		}
		if(typeof(a,'num')) idx=a
	}
	not(map.childCount()) {
		db=Baro.db('data_map')
		not(db.open()) db.open('data_map.db')
		sp=Math.random(0,10).toInt();
		sp*=4;
		sql="select idx, color from color_map order by like_num desc limit $sp, 4"
		root = db.fetchAll(sql)
		while(cur, root) {
			cur.inject(idx, color)
			map.addNode().with(idx,color)
		}
	}
	n=idx-1;
	cur= map.child(idx)
	if(cur) {
		c=cur.color
	} else {
		c=randomColor()
	}
	return "$c"
}
@baro.funcVal(parent, page, node, fnm, &param, prev) {
	nl = conf('cf.newline')
	if(fnm.eq('css')) return _css(param);
	if(fnm.eq('focus')) return _css(param,'focus');
	if(fnm.eq('hover')) return _css(param,'hover');
	if(fnm.eq('valid')) return _css(param,'valid');
	if(fnm.eq('before')) return _css(param,'before');
	if(fnm.eq('after')) return _css(param,'after');
	if(fnm.eq('kf','keyframes')) return _keyframes(param);
	if(fnm.eq('style')) {
		ss="#${page.pageId} .${node.class}"
		src=@baro.parseSource(parent,page,node,param,'value')
		ss.add(" {$src}")
		nodeAppendText(page,'@css',ss,conf('cf.newline'))
		return ss;
	}
	map=Cf.getObject('baro','styleMap')
	if(map.isset(fnm.lower())) {
		print("@@ funcVal styleMap set $fnm $param")
		return param.trim();
	}
	arr=null
	if( param.ch()) {
		arr=_arr()
		node.set("@endpos", 0)
		sp=0
		while(param.valid()) {
			c=param.ch() not(c) break;
			arr.add( @baro.parseSource(parent, page, node, param, 'param') )
			ep = node.set("@endpos") not(ep) break;
			if(ep<=sp) break;
			param.pos(ep)
			sp=ep
		}
	}
	print("@@ funcVal param arr=>$arr")
	ss = ''
	if(fnm.eq('find')) {
	} else if(fnm.eq('random','randomInt','randomFloat')) {
		a=arr.get(0), b=arr.get(1) not(b) b=a+10
		if(fnm.eq('randomFloat')) {
			a=Math.random(a,b)
			aa=Math.round(c,2)
			ss=trimChar("$aa")
		} else {
			aa=Math.random(a,b).toInt()
			ss="$aa"
		}
	}
	else if(fnm.eq('randomColor')) {
		ss=randomColor()
	}
	else if(fnm.eq('colorMap')) {
		ss=@baro.colorMap(page, arr)
	}
	else if(fnm.eq('color')) {
		if(arr.size()>2) {
			ss=color(arr)
		} else {
			cc=color(arr.get(0))
		}	
	} else if(fnm.eq('border')) {
		
	}
	else if(fnm.eq('light','dark','mix')) {
		c=color(prev) not(typeof(c,'color')) c=randomColor()
		num=arr.get(0) not(typeof(num,'num')) num=20
		if(fnm.eq('light')) ss=c.lightColor(num)
		else if(fnm.eq('dark')) ss=c.darkColor(num)
	}
	return ss;
	
	_keyframes = func(&str) {
		ss='@keyframes '
		name = str.findPos('=>').trim()
		ss.add(name,'{',nl)
		c=str.ch()
		if(c.eq('{')) {
			str=str.match(1)
		}
		while(str.valid()) {
			c=str.ch() not(c) break;
			if(c.eq(',',';')) {
				str.incr()
				continue;
			}
			a=str.findPos('{',0,1).trim()
			b=str.match(1)
			result =@baro.getStyleText(parent,page,b)
			if(result) ss.add(a,'{',result,'}',nl)
		}
		ss.add('}')
		nodeAppendText(page,'@cssVar', ss, nl)
	};
	_css = func(&str, type) {		
		cur=_node()
		expr=''
		if(type.eq('focus','valid','hover','before','after')) {
			expr=":$type"
			if(str.find('=>')) {
				key = str.findPos('=>').trim()
				left = @baro.parseSource(parent,page,node,key,'css')
				if(left.ch()) {
					expr.add(" $left")
				} else {
					expr.add(" $key")
				}
				print("@@ funcVal _css $key, $left");
			}
		}
		src=@baro.parseSource(parent,page,node,str,'value')
		ss = @baro.getStyleText(parent,page,src)
		className = @baro.getClassName(page, node.class)
		css=#[
${className}${expr} {
	${ss}
}]
		nodeAppendText(page,'@css', css, nl)
		return true;
	};
}

@baro.getClassName(pageNode, &s) {
	ss="#${pageNode.pageId} "
	if( s.find(' ') ) {
		while(s.valid(),n) {
			left = s.findPos(' ').trim() not(left) break;
			ss.add('.',left)
		}		
	} else {
		ss.add(".$s")
	}
	return ss;
}
@baro.parseSource(parent, page, node, &s, type) {
	ss=''
	if(s.find('@[')) {
		while(s.valid()) {
			left = s.findPos('@[',0,1)		
			ss.add(left) not(s.ch()) break;
			s.incr()
			param = s.match(1)
			ss.add(_paramVal())
		}
	} else {
		if(type.eq('value','js','css')) {
			return s.trim();
		}
		param = s
		ss.add(_paramVal())
	}	
	_paramVal = func() {
		not(param.ch()) return;
		val='', prev=null
		pageNode=page
		while(param.valid(),n) {
			c=param.ch() not(c) break;
			if(c.eq(',')) {
				param.incr()
				if(type.eq('endpos')) {
					node.set("@endpos", param.cur())					
				} else {
					val.add(param)
				}
				break;
			}
			if( c.eq('#') ) {
				v=param.incr().move()
				val=color("#$v")
				continue;
			}
			if( c.eq() ) {
				val=param.match()
				continue;
			}
			if(c.eq('(') ) {
				val = param.match(1)
				continue;
			}
			if(c.eq('.')) {
				prev = val
				param.incr()
				continue;
			}
			if(@baro.isFunc(param)) {
				fnm = param.move()
				fparam=param.match()
				val = @baro.funcVal(parent,page,node,fnm,fparam,prev)
				continue;
			}
			sp=param.cur()
			if(c.eq('@')) {
				param.incr()
			}
			c=param.next().ch()
			while(c.eq('-')) c=param.incr().next().ch()
			key=param.trim(sp,param.cur(),true)
			if(typeof(key,'num') || key.eq('true','false')) {
				if(typeof(key,'num')) {
					if(c.eq('.')) {
						a=param.incr().move()
						return "${key}.${a}";
					}
				}
				return key;
			}
			if(key.eq('this','page','global')) {
				if(key.eq('parent')) {
					val = node.parentNode()
				} else if(key.eq('page')) {
					val = page
				} else if(key.eq('global')) {
					val = parent
				} else {
					val = node
				}				
				continue;
			}
			if( typeof(prev,'node') ) {
				if( prev.isset(key)) {
					val = prev.get(key)
				} else {
					if( type.eq('html')) {
						sub = when(typeof(val,'node'),val,page)
						if(sub.isset(key)) {
							val=sub.get(key)
						} else {
							val=sub.get("@$key")
						}
					} else {
						if( type.eq('css')) {
							name = Cf.val(prev.get('key'),' ',key)
							val =@baro.findFieldValue(page.get('@layout'),'class',name)
							not(param.ch()) {
								return @baro.getClassName(pageNode,name);	
							}
						}
						if( prev==parent) {
							val =@baro.findFieldValue(parent,'pageCode',key)
							pgaeNode = val
						} else if(prev==pageNode) {
							val = @baro.findLayoutChild(pageNode,key)
						} else {
							val = @baro.findLayoutChild(prev,key)
						}
						not(typeof(val,'node')) return print("$key 요소찾기 오류", prev);
					}
				}
				continue;
			}
			if(prev) {
				print("@@ == parseSource == key not valid prev alread set ", prev)
			}
			if(page.isset(key)) {
				val=page.get(key)
			} else {
				val=page.get("@$key")
			}
			not(val) {
				val = @baro.findLayoutChild(pageNode,key)
			}
		}
		if( typeof(val,'node') ) {
			node = val
			not(typeof(pageNode,'node')) return print("페이지 노드 찾기 오류", val);
			not(pageNode.pageId) return print("페이지 아이디 미정의", val, pageNode);
			if(type.eq('css')) {
				val = @baro.getClassName(pageNode, node.class)
			} else if(type.eq('js')) {
				if(pageNode==page) {
					val=#[getPageEl('.${node.class}')]
				} else {
					val=#[getPageEl('${pageNode.pageCode}','.${node.class}')]
				}
			} else {
				val = @baro.getClassName(pageNode, node.class)
			}	
		} else {
			not(typeof(val,'string')) return "$val";
		}
		return val;
	};
	return ss;
}

@baro.findFieldValue(node,field,val) {
	if(node.cmp(field,val)) return node;
	while(cur, node) {
		find=@baro.findFieldValue(cur,field,val)
		if(find) return find;
	}
	return;
}
@baro.findLayoutChild(node,key) {
	layout = node.get('@layout')
	not(typeof(layout,'node')) return print("$key 레이아웃 요소를 찾을수 없습니다")
	cur =layout.child(0)
	not(typeof(layout,'node')) return print("$key 레이아웃 요소가 없습니다")
	return @baro.findFieldValue(cur,'key',key)
}

@baro.styleMap() {
	map=Cf.getObject('baro','styleMap') if(map && map.startTick) return map;
	map=Cf.getObject('baro','styleMap',true)
	map.parseJson(#[
		w:width,h:height,p:padding,m:margin,
		mt:marginTop, mb:marginBottom, ml:marginLeft, mr:marginRight,
		pt:paddingTop, pb:paddingBottom, pl:paddingLeft, pr:paddingRight,
		bg:background,
		bd:border,
		b:border,
		bt:borderTop, bb:borderBottom, bl:borderLeft, br:borderRight,
		rad:borderRadius,
		c:color,
		x:top, y:left,
		t:top, l:left,
		tform: transform,
		tf: transform,
		tr:transition, 
		trdelay:transitionDelay,
		fwrap:flexWrap,
		ani:animation
		anidelay: animationDelay,
		bgpos:backgroundPosition,
		bgsize:backgroundSize,
		fit:object-fit,
		minh:minHeight,
		maxh:maxHeight,
		minw:minWidth,
		maxw:maxWidth,
		rel:relative,
		abs:absolute,
		space:letterSpace,
		ls:letterSpace,
		lh:lineHeight,
		fs:fontSize,
		fw:fontWeight,
		pe:pointerEvent,
		ai:alignItems,
		jc:justifyContent,
		shadow: boxShadow,
		ctt: content,
		hint: placeholder
	])
	map.startTick = System.tick()
	return map;	
}
@baro.isEmpty(s) {
	type=typeof(s)
	if(type.eq('string')) {
		not(s) return true;
	} else if(type.eq('bool')) {
		not(s) return true;
	} else if(type.eq('null')) {
		return true;
	}
	return false;
}
@baro.styleValue(&s) {
	val=s.trim()
	if(typeof(val,'num') || val.eq('true','false','null')) return val;		
	return Cf.val("'",val,"'");
}
@baro.addHtmlStyle(parent, page, node, k, val,bcls) {
	result=''
	_addSty = func(key, &s) {
		ss=''
		if(@baro.isFunc(s) || s.find('@[') ) {
			src=@baro.parseSource(parent,page,node,s,'value')
			if(src) {
				ss.add(src)
			} else {
				ss.add(s)
			}
		} else {
			ss.add(s)
		}
		if(bcls) {
			k=@baro.styleKey(key), v=ss
			if(@baro.isEmpty(v)) v="''"
			sty="${k}:${v};"			
			result.add(sty)
		} else {
			c=key.ch()
			if(c.eq('-')) {
				v=ss
				sty="${key}:${v};"
				node.appendText('@styValue',sty)
			} else if(ss) {
				v =@baro.styleValue(ss)
				sty ="${key}:${v}"
				nodeAppendText(node,'@sty',sty,",","$key:")
			}
		}
	};
	map = @baro.styleMap()
	key = map.get(k.lower()) not(key) key=k	
	if(key.eq('col')) key='column'
	else if(key.eq('c')) key='color'
	if( typeof(val,'bool') && val ) {
		if(key.eq('flexWrap','pointerEvent','listStyle')) {
			_addSty(key,'none')
		}
		else if(key.eq('alignItems','justifyContent')) {
			_addSty(key,'center')
		}
		else if(key.eq('pointer','row','column','vbox','hbox')) {
			switch(key) {
			case row: 
				_addSty('display','flex')
				_addSty('flexDirection','row')
			case column: 
				_addSty('display','flex')
				_addSty('flexDirection','column')
			case vbox: 
				_addSty('display','flex')
				_addSty('flexDirection','column')
				_addSty('height','100%') 
			case hbox: 
				_addSty('display','flex')
				_addSty('flexDirection','row')
				_addSty('width','100%')
			default:
			}
		}
		else if(key.eq('content')) {
			_addSty('content', '')
		}
		else if(key.eq('shadow')) {
			_addSty('boxShadow', '0 10px 30px rgba(0,0,0,0.4)')
		}
		else if(key.eq('hidden','grid','block','flex')) {
			_addSty('display', key);
		}
		else if(key.eq('absolute','relative','fixed')) {
			_addSty('position', key);
		}
		else if(key.eq('required','text','password')) {
			if(key.eq('required')) {
				nodeAppendText(node,'@attr','required',' ')
			} else {
				val = _s('type="${key}"')
				nodeAppendText(node,'@attr',val,' ')
			}
		}
		else if(key.eq('flexCenter')) {	
			if(bcls) {
				result.add('display:flex;align-items:center;justify-content:center') 
			} else {
				nodeAppendText(node,'@sty',"display:'flex', alignItems:'center', justifyContent:'center'",',')
			}
		}
		else if(key.eq('stretch')) {
			_addSty('flex','1')
		}
		else if(key.eq('full')) {
			if(bcls) {
				result.add('width:100%;height:100%') 
			} else {
				nodeAppendText(node,'@sty',"width:'100%',height:'100%'",',')
			}
		} else {
			print("@@ addHtmlStyle $key 미정의")
		}
	}
	else if(key && val ) {
		_addSty(key,val)
	}
	return result;
}

@baro.makeHtmlProps(parent, page, node) {
	not(node) { 
		layout = page.get('@layout')
		cnt = layout.childCount()
		not(cnt) return print("페이지 레이아웃 미정의")
		if( cnt > 1 ) {
			node = layout
			node.set('@sty',"display:'flex',flexDirection:'column', alignItems:'center',justifyContent:'center', width:'100%',height:'100%'")
		} else {
			layout.tag='layout'
			node = layout.child(0)
		}
		not( node.isset('id') ) {
			node.id = page.pageId
		}
		print("makeHtmlProps node=>$node", layout)
	}
	map = @baro.styleMap()	
	_addProp=func(k,v) {
		key = map.get(k.lower()) not(key) key=k 
		src=@baro.parseSource(parent,page,node,v,'value')
		if(key.eq('class')) {
			nodeAppendText(node,'class',src, ' ')
		} else {
			attr = _s('${key}="${src}"')
			nodeAppendText(node,'@attr',attr, ' ')
		}
	};
	_addFunc = func(k,&s) {
		print("@@ addFunc $k, $s");
		fparam=s.findPos('=>')
		fsrc=@baro.parseSource(parent,page,node,s,'js')
		ss=''
		if(k.start('on')) {
			fnm = k.trim(2)
			ss.add(node.varName, ".on('$fnm',($fparam)=>")
			if(fsrc.find("\n")) {
				ss.add("{$fsrc}")
			} else {
				ss.add(fsrc)
			}
			ss.add(')')
			nodeAppendText(node,'@event', ss, conf("cf.newline"))
		} else if(k.eq('if')) {
		} else if(k.eq('init')) {
			nodeAppendText(page,'@funcsInit', fsrc, conf("cf.newline"))
		} else {
			ss.add("const $k = ($fparam)=>")
			if(fsrc.find("\n")) {
				ss.add("{$fsrc}")
			} else {
				ss.add(fsrc)
			}
			nodeAppendText(page,'@funcs', ss, conf("cf.newline"))
		}
	};
	arr=node.get('@keyArray') 
	not(typeof(arr,'array')) {
		arr=node.keys()
		print("html props 키배열 오류", node, arr)
	}
	while(k, arr) {
		if(@baro.ignoreStyleProp(k)) continue;		
		val=node.get(k)
		if(typeof(val,'node','array')) continue;
		c=k.ch()
		if(c.eq('#','&','%','^')) {
			// #props, %:tag, &:function
			if(c.eq('#')) {
				_addProp(k.value(1),val)
			} else if(c.eq('&')) {
				_addFunc(k.value(1),val)
			} else if(c.eq('^')) {
				fnm = k.value(1)
				@baro.applyStyleFunc(parent,page,node,fnm,val)
			}
			continue;
		}
		if(typeof(val,'bool')) {
			src=val
		} else {
			src=@baro.parseSource(parent,page,node,val,'value')
		}
		@baro.addHtmlStyle(parent,page,node,k,src)
	}
	if(node.isset('hbox') || node.isset('vbox')) {
		while(cur, node) {
			if(cur.isset('^css')) continue;
			if(node.isset('hbox')) {				
				not(cur.isset('height')) {
					addNodeProp(cur, 'h','100%')
				}
				not(cur.isset('width')) {
					not(cur.isset('w')) addNodeProp(cur, 'flex','1')
				}
			} else {
				not(cur.isset('width')) {
					addNodeProp(cur, 'w','100%')
				}
				not(cur.isset('height')) {
					not(cur.isset('h')) addNodeProp(cur, 'flex','1')
				}
			}
		}
	}
	
	if(node.id) {
		val = Cf.val("id=",Cf.jsValue(node.id))
		nodeAppendText(node,'@attr', val, ' ')
	}
	if(node.class) {
		val = Cf.val("class=",Cf.jsValue(node.class))
		nodeAppendText(node,'@attr', val, ' ') 
	}
	while(cur, node) {
		@baro.makeHtmlProps(parent, page, cur)
	}
}
@baro.getStyleText(parent, page, str) {
	node=_node()
	map=Cf.getObject('baro','styleMap')
	data=@baro.parseSource(parent, page,node,str,'value')
	@baro.parseProps(parent,page,node,data)
	ss=''
	arr=node.get('@keyArray') not(typeof(arr,'array')) return print("스타일 텍스트 키배열 오류");
	while(k, arr) {
		val=node.get(k)
		if(typeof(val,'node','array')) continue;
		c=k.ch()
		if(c.eq('#','&','%','^')) {
			if(c.eq('^')) {
				fnm=k.value(1)
				result = @baro.applyStyleFunc(parent,page,node,fnm,val,true)
				if(result) ss.add(result)
			}
			continue
		}
		key=map.get(k) not(key) key=k
		result = @baro.addHtmlStyle(parent,page,node,k,val,true)
		not(result) continue;
		replaceVal = replaceFindText(ss,"${key}:",result,';')
		if(replaceVal) {
			ss=replaceVal
		} else {
			c=result.ch(-1)
			ss.add(result)
			not(c.eq(';')) ss.add(';')
		}
	}
	return ss;
}
@baro.isFunc(&s) {
	s.ch() not(c) return;
	if(c.eq('@')) s.incr()
	c=s.next().ch()
	while(c.eq('-','.')) c=s.incr().next().ch()
	return when(c.eq('('), true)
}
@baro.lastIndent(&s) {
	if(s.find("\n")) {
		left =s.findLast("\n")
		a=left.right()
		return indentText(a);
	} else if(s) {
		return indentText(s);
	}
	return;
}
@baro.jsVal(s) {
	ss=''
	if(typeof(s,'string')) {
		if(typeof(s,'num')) {
			return s;
		} else {
			return Cf.jsValue(s)
		}
	} else if(typeof(s,'bool','number')) {
		return s
	}
	return Cf.jsValue("$s")
}
@baro.varName(&s) {
	not(typeof(s,'string')) return;
	ss='', upper=false
	while(n=0,s.size()) {
		c=s.ch(n) not(c) break;
		if(c.eq('-','_')) {
			upper=true
			continue;
		}
		if(c.eq('/')) {
			c='_'
		} 
		if(c.eq(' ') || c.is('oper')) {
			break;
		}
		if(upper){
			ss.add(c.upper())
			upper=false
		} else {
			ss.add(c)
		}
	}
	return ss;
}
@baro.styleKey(&s) {
	not(typeof(s,'string')) return;
	ss='', upper=false
	while(n=0,s.size()) {
		c=s.ch(n)
		if(c.is('upper')) {
			if(n) ss.add('-',c.lower())
		} else {
			ss.add(c)
		}
	}
	return ss;
}
@baro.changePageScript(parent, page, src) {
	not(page.path) return print("changePageScript => 페이지 소스 저장 실패 저장경로 미정의",page);
	not(src) return print("changePageScript => 페이지 소스 내용이 없습니다",page);	
	pageCode = page.get('pageCode')
	not(page.pageCode) return print("changePageScript => 페이지 코드 미정의",page);
	savePath = _s('${@web.rootPath}/assets/pages/${page.path}')
	fileWrite(savePath, src)
	server=Baro.server('websocket')
	while(client, server) {
		not(client) continue;
		if(client.cmp('mode','dev')) {
			@baro.websocketSendMessage(client, 'changePageScript',"pageCode:$pageCode")
			
		}
	}
}


nodeAppendText(node,key,value,sep,replace) {
	ss=node.get(key)
	if(replace && ss ) {
		result = replaceFindText(ss,replace,value,sep)
		if(result) {
			node.set(key,result)
			return;
		}
	}
	if(sep) {
		if(ss) node.appendText(key,sep)
	}
	node.appendText(key,value)
}
trimChar(s,ch) {
	not(ch) ch='0'
	last=s.size()-1;
	while(n=pos, 0) {
		c=s.ch(n)
		if(c.ne(ch)) {
			if(n.eq(last)) return s;
			last=n+1
			break;
		}
	}
	return s.trim(0,last,true)
}
nodeInitVal(node, key, val) {
	if(node.isset(key)) {
		not(val) val=null
		node.set(key,val)
	}
}
nodeReuse(node) {
	not(typeof(node,'node')) return _node();
	while(k,node.keys(true)) {
		v=node.get(k)
		if(typeof(v,'node')) {
			nodeReuse(v)
		} else if(typeof(v,'array')) {
			v.reuse()
		} else {
			node.set(k,null)
		}
	}
	return node;
}
addNodeProp(node,k,v) {
	a=node.get('@keyArray')
	if(a) {
		if(a.find(k)) return;
		a.add(k)
	}
	print('@@ addNodeProp ', k, v, a)
	node.set(k,v)
}
catchError() {
	err=Cf.error() not(err) return false;
	print("## catch error : $err")
	return true;
}
@baro.startWatcher(id, path, callback) {
	not(id) return print('@@ start watcher 아이디 미정의')
	not(isFolder(path)) return print('@@ start watcher $path 경로 미정의')
	if(path.find('/')) {
		path=path.replace('/','\')
	}
	not(typeof(callback,'func')) {
		callback = @baro.watcherFileProc
	}
	watcher = System.watcherFile(id, callback)
	watcher.start(path)
	config = Cf.rootNode().get('@watcherFiles').get(id)
	config.addArray('watcherNames')
	return config;
}

@baro.watcherFileProc() {
	args(type, name)
	fn = Cf.funcNode()
	tick= fn.get('prevTick')	
	if(type.eq(3)) {
		if(tick) {
			dist = System.tick() - tick			
			if(dist<100) {
				return
			}
		}
		path = this.target
		print("watcher file name == $name")
		while(curName,this.watcherNames) {
			if(name.find(curName) ) {
				fullpath = Cf.val(path,'/',curName) 
				@baro.loadPage(fullpath)
			}	
		}
		
	}
	fn.set('prevTick', System.tick())
}
@baro.conf(name, def, overwrite) {
	k="baro.$name"
	if(def && overwrite) {
		conf(k,def,true)
		return def;
	}
	v=conf(k)	
	not(v) {
		v=def
		if(v) conf(k,v,true)
	}
	return v;
}

@baro.jsonDataArray(&s,parentType) {
		nm=null
		while(s.valid()) {
			c=s.ch() not(c) break;
			if(c.eq('{')) {
				a=s.match()
				ss=@baro.jsonDataNode(a)
				not(nm){
					nm=parentType
					types=object('baro.types')
					types.appendText('@arrayTypes', "$parentType {$ss}")
				}
			} else if(c.eq()) {
				a=s.match()
				not(nm) nm='string'
			} else {
				v=s.findPos(',').trim()
				not(nm) {					
					if(typeof(v,'num')) nm='number'
					else if(v.eq('true','false')) nm='boolean'
					else nm='null'
				}
			}
		}
		return nm
	}
	@baro.jsonDataNode(&s,parentType) {
		nl=conf('cf.newline')
		ss=''
		while(s.valid()) {
			c=s.ch()not(c) break;
			if(c.eq(',')) {
				s.incr()
				continue
			}
			not(c.eq()) break;
			if(ss) {
				ss.add(',',nl)
			}
			k=s.match()
			c=s.ch()
			not(c.eq(':')) break;
			c=s.incr().ch()
			 if(c.eq('[','{')) {
				a=s.match(1)
				tynm=typeName(k)
				if(c.eq('{')) {
					ty='node'
					@baro.jsonDataNode(a,tyNm)
					ss.add("$k:",tynm)
				} else {					
					ty='array'
					nm=@baro.jsonDataArray(a,tynm)
					ss.add("$k:$nm[]")
				}
				continue;
			}
			if(c.eq()) {
				v=s.match()
				ty='string'
			} else {
				v=s.findPos(',').trim()
				if(typeof(v,'num')) {
					ty='number'
				} else {
					ty='boolean'
				}
			}
			ss.add("$k:$ty")
		}
		if(parentType) {
			types=object('baro.types')
			types.appendText('@nodeTypes', "$parentType {$ss}")
		}
		return ss;
	}