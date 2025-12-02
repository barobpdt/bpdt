@baro.ignoreStyleProp(k) { return k.eq('id','key','class','varName','render') }
@baro.loadPage(path) {
	not(path) path='c:/temp/page-test.js'
	not(conf("cf.newline")) conf("cf.newline","\r\n",true)
	parent = object('baro.pages')
	parent.set('@style','')
	src = fileRead(path)
	@baro.parsePages(parent, src)
}
@baro.initLayoutVar(page, node, key) {
	not(key) return print("레이아웃 노드 추가오류 키미정의", node);
	varMap = page.get('@varMap')
	if(@baro.isHtmlTag(key)) {
		node.set('tag',key)
	}
	node.set('class',key)
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
@baro.initPage(parent,page,src,template) {
	pageCode=page.pageCode
	prev = conf("pageSrc.${pageCode}")
	if(prev.eq(src)) {
		print("######## @baro.parsePage not chanage", page)
		// conf('baro.debugMode', 'true', true)
		not(conf('baro.debugMode')) return;
	}
	not(template) template = fileRead('C:/temp/page-template.txt')
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
	if( parent.get('@style')) {
		parent.set('@style','')
		nodeAppendText(page, '@pageCss', parent.get('@style'))
	}
	if( page.get('@css')) {
		nodeAppendText(page, '@pageCss', page.get('@css'))
	}
	nodeAppendText(page, '@pageFuncs', page.get('@funcs'))
	nodeAppendText(page, '@pageInit', page.get('@funcsInit'))
	nodeInitVal(page,'@css')
	nodeInitVal(page,'@funcs')
	nodeInitVal(page,'@funcsInit')

	conf("pageSrc.${pageCode}", src, true)
	pageSrc = @baro.parseSource(parent, page, page, template,'value')
	print("## pageSrc ==> $pageSrc")
	@baro.changePageScript(parent, page, pageSrc)
}

@baro.parsePages(parent, &s, template) {
	isProp = func(&s) { 
	c=s.ch()
		return c.eq('{') 
	};
	cur = null
	while(s.valid()) {
		left = s.findPos('##>')
		if(cur) {
			@baro.initPage(parent,cur,left,template)
		}
		c=s.ch()
		not(c) break
		if( c.eq('*')) {
			cur=null
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
		cur = nodeReuse(parent.addNode(pageCode))
		if( bprop ) {
			s.findPos('{',0,1)
			data = s.match(1) if(typeof(data,'bool')) return print("$pageCode 페이지 설정정보 매칭오류")
			@baro.parseProps(parent,cur,cur,data)
		}
		cur.pageCode = pageCode
		cur.path = "${path}.js"
		if(note) cur.note = note
	}	
}

@baro.makePageScript(parent, page, savePath) {
	layout = page.get('@layout')
	layout.tag='layout'
	layout.varName = "content"
	
	root = layout.child(0)
	not(typeof(root,'node')) return print("@@ make page script 오류 페이지 레이아웃 노드미정의", page, layout)
	root.id = page.pageId
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
	
	ss.add("const ${varName}=",'$',"('<${tag}")	
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
		render = node.get('render')
		if(typeof(render,'bool')) {
			renderFunc = 'renderItem(item)'
		}
		else if(lineCheck(render,'(')) {
			renderFunc = render
		} else {
			renderFunc = "${render}(item)"
		}
		src=@baro.pageRenderScript(parent,page,node)
		nodeAppendText(page,'@funcs', #[
function ${renderFunc} {
	const content = getRenderElement('$varName')
	${src}
}],nl)
		ss.add("setRenderElement($varName)",nl)
		return ss;
	}
	while(cur,node) {
		src=@baro.pageScript(parent,page,cur)
		ss.add(src)
	}
	return ss;
}

@baro.pageRenderScript(parent, page, node) {
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
	nl = conf("cf.newline")
	ss=''
	not(node.isset('render')) {
		p=node.parentNode()
		parentVar=when(p.isset('render'),'content',p.varName) 
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
@baro.parseProps(parent,page, node, &s) {
	if(typeof(s,'bool')) return print('@baro.parseProps 매치오류');
	while(s.valid()) {
		c=s.ch() not(c) break;
		if(c.eq(',')) {
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
			fparam=s.match(1), fsrc=''
			c=s.ch()
			if(c.eq('{')) {
				fsrc=s.match(1)
				node.set("&$fnm", "${fparam}=>${fsrc}")
			} else {
				result = @baro.funcVal(parent,page,node,fnm,fparam)
				print("@@ parseProps $fnm funcval result==$result")
			}
			c=s.ch()
			if(c.eq(';')) s.incr().ch()			
			continue
		}
		
		bprop=false, bsty=false
		if(c.eq(':','=')) {
			if(c.eq(':')) {
				bsty=true
			} else{
				bprop=true
			}
			c=s.incr().ch()
		}
		v=''
		if(@baro.isFunc(s)) {
			sp=s.cur()
			while(@baro.isFunc(s)) {
				s.next().ch()
				s.match()
				c=s.ch() 
				if(c.eq(';')) s.incr().ch()
			}
			v = s.trim(sp,s.cur(),true)
			src = @baro.parseSource(parent,page,node,v,'props')
			if(src) {
				node.set(k,src)
			} else {
				node.set(k,v)
			}
			print("@@ parseProps function : $k == $v")
			
		}
		else if(c.eq()) {			
			v=s.match()
			if(bsty) {			
				node.set(k,v)
			} else if(bprop) {
				node.set("#$k", v)
			} 
		} else if(c.eq('{')) {
			cur = node.addNode(k)
			@baro.parseProps(parent,page,cur,s.match())
		} else if(c.eq('[')) {
			arr = node.addArray(k)
			@baro.parseArray(parent,page,node,arr,s.match())
		} else if(c.eq('<')) {
			sp=s.cur()
			c=s.incr().next().ch()
			while(c.eq('-')) {
				c=s.incr().next().ch()
			}
			tag = s.trim(sp+1,s.cur(),true)
			s.pos(sp)
			src=s.match("<$tag","</$tag>",8) if(typeof(src,'bool')) return print("$tag 매칭오류", page.pageCode)
			html=''
			p=src.findPos('>')
			html.add("<${tag}") if(p.ch()) html.add(" $p>") else html.add(">");
			html.add(@baro.parseSource(parent,page,node,src,'value'))
			html.add("</$tag>")
			node.set("%$k", html)
		} else {			
			if(bsty || bprop) {
				if(lineCheck(s,',')) {
					v=s.findPos(',').trim()
				} else {
					v=s.findPos("\n").trim()
				}
			}
			if(bsty) {			
				node.set(k,v)
			} else if(bprop) {
				node.set("#$k", v)
			} else {
				node.set(k, true)
			}
		}		
	}
}
@baro.getLine(&s) {
	s.ch()
	line = s.findPos("\n").trim()
	return line;
}
@baro.makeHtmlLayout(parent, page, &s) { 
	layout = page.addNode('@layout').removeAll(true)
	cur = null	
	parentArray=[]
	indentArray=[]
	parentArray.add(layout)	
	pageIdCheck = false
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
			idx=indentArray.find(a)
			endCheck = idx.eq(0) || ~(a)
			if(idx==-1) {
				// print("@@ end check $idx $endCheck")
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
		if( root==layout ) {
			not(pageIdCheck) cur.id=page.pageId
			pageIdCheck = true
		}		
		@baro.initLayoutVar(page,cur,key)
		endCheck = false
		if( lineCheck(s,'{') ) {
			left = s.findPos('{',1,1)
			body = s.match(1)
			if(typeof(body,'bool')) return print("레이아웃 속성 매핑오류", left);
			if( left.ch()) {				
				cur.appendText('@html', left.trim())
			}
			@baro.parseProps(parent,page,cur, body)
		}
		line = @baro.getLine(s);
		// print("======> $key ", line)
		not(_tagValue()) {
			left = s.findPos("\n")
			if( left.ch()) {
				cur.appendText('@html', left.trim())
			}
		}
		if( Cf.error()) {
			print(">> page parse error ", Cf.error(), s.size())
			return;
		}
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
	
	_checkTag = func(s) {
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
				cur.appendText('html',body)
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
@baro.funcVal(parent, page, node, fnm, &param, prev) {
	if(fnm.eq('css')) return _css(param)
	if(fnm.eq('focus')) return _css(param,'focus')
	if(fnm.eq('hover')) return _css(param,'hover')
	if(fnm.eq('valid')) return _css(param,'valid')
	if(fnm.eq('before')) return _css(param,'before')
	if(fnm.eq('after')) return _css(param,'after')
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
	
	_css = func(&str, type) {		
		cur=_node()
		expr=''
		if(type.eq('focus','valid','hover','before','after')) {
			expr=":$type"
			if(str.find('=>')) {
				left = str.findPos('=>')
				if(left.ch()) expr.add(" $left")
			}
		}
		src=@baro.parseSource(parent,page,cur,str,'value')
		@baro.parseProps(parent,page,cur,src)
		ss='', nl=conf('cf.newline')
		while(k, cur.keys()) {
			if(@baro.ignoreStyleProp(k)) continue;		
			val=cur.get(k)
			if(typeof(val,'node','array')) continue;
			c=k.ch()
			if(c.eq('#','&','%')) {
				continue;
			}		
			rst=@baro.addHtmlStyle(parent,page,cur,k,val,true)
			if(rst) {
				replaceVal = replaceFindText(ss,"$k:",rst,';')
				if(replaceVal) {
					ss=replaceVal
				} else {
					ss.add(rst,';')
				}
			}
		}
		css=#[
#${page.pageId} .${node.class}${expr} {
	${ss}
}]
		nodeAppendText(page,'@css', css, ln)
		return true;
	};
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
			return s;
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
			if(type.eq('endpos')) {
				if(c.eq(',')) {
					param.incr()
					node.set("@endpos", param.cur())
					break;
				}
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
						node = when(typeof(val,'node'),val,page)
						if(node.isset(key)) {
							val=node.get(key)
						} else {
							val=node.get("@$key")
						}
					} else {
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
			if( type.eq('value')) {
				if(page.isset(key)) {
					val=node.get(key)
				} else {
					val=page.get("@$key")
				}
			} else {
				val = @baro.findLayoutChild(pageNode,key)
			}
		}
		print(">> parse source ########", type, key)		
		if( typeof(val,'node') ) {
			node = val
			not(typeof(pageNode,'node')) return print("페이지 노드 찾기 오류", val);
			not(pageNode.pageId) return print("페이지 아이디 미정의", val, pageNode);
			if(type.eq('css')) {
				val=#[#${pageNode.pageId} .${node.class}]
			} else if(type.eq('js')) {
				if(pageNode==page) {
					val=#[getPageEl('.${node.class}')]
				} else {
					val=#[getPageEl('${pageNode.pageCode}','.${node.class}')]
				}
			} else {
				val=#[${pageNode.pageId} : ${node.class}]
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
		bg:background
		bd:border,
		b:border,
		bt:borderTop, bb:borderBottom, bl:borderLeft, br:borderRight,
		rad:borderRadius,
		c:color,
		x:top, y:left,
		t:top, l:left,
		tr:transition, trs:transition,
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
		jc:justifyContent
	])
	map.startTick = System.tick()
	return map;	
}

@baro.addHtmlStyle(parent, page, node, k, val,bcls) {
	result=''
	_styleValue = func(&s) {
		val=s.trim()
		if(typeof(val,'num') || val.eq('true','false','null')) return val;		
		return Cf.val("'",val,"'");
	};
	_addSty = func(key, &s) {
		ss=''
		if(@baro.isFunc(s) || s.find('@[') ) {
			src=@baro.parseSource(parent,page,node,s,'css')
			ss.add(src)
		} else {
			ss.add(s)
		}
		if(bcls) {
			kk=@baro.styleValue(key)
			sty="${kk}:${val}"			
			result.add(sty)
		} else {
			c=key.ch()
			if(c.eq('-')||key.find('-')) {
				val=ss
				sty="${key}:${val}"
				nodeAppendText(node,'@styValue',sty,";","$key:")
			} else if(ss) {
				val =_styleValue(ss)
				sty ="${key}:${val}"
				nodeAppendText(node,'@sty',sty,",","$key:")
			}
		}
	};	
	map = @baro.styleMap()
	key = map.get(k.lower()) not(key) key=k	
	if( typeof(val,'bool') && val ) {	
		if(key.eq('flexWrap','pointerEvent','listStyle')) {
			_addSty(key,'none')
		}
		else if(key.eq('pointer','row','col','vbox','hbox')) {
			switch(key) {
			case row: 
				_addSty('display','flex')
				_addSty('flexDirection','row')
			case col: 
				_addSty('display','flex')
				_addSty('flexDirection','column')
			case vbox: 
				_addSty('display','flex')
				_addSty('flexDirection','row')
				_addSty('height','100%') 
			case hbox: 
				_addSty('display','flex')
				_addSty('flexDirection','column')
				_addSty('width','100%')
			default:
			}
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
				result.add('width:100%;height:100%;display:flex;align-items:center; justify_content:center;') 
			} else {
				nodeAppendText(node,'@sty',"width:'100%',height:'100%',display:'flex', alignItems:'center', justifyContent:'center'",',')
			}
		}
		else if(key.eq('stretch')) {
			_addSty('flex','1')
		}
		else if(key.eq('full')) {
			if(bcls) {
				result.add('width:100%;height:100%;') 
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
		node = layout.child(0)
	}
	map = @baro.styleMap()	
	_addProp=func(k,v) {
		key = map.get(k.lower()) not(key) key=k 
		if(key.eq('class')) {
			nodeAppendText(node,'class',v, ' ')
		} else {
			attr = _s('${key}="${v}"')
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
			if(fsrc.findPos("\n")) {
				ss.add("{$fsrc}")
			} else {
				ss.add(fsrc)
			}
			ss.add(')')
			nodeAppendText(node,'@event', ss, conf("cf.newline"))
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
	while(k, node.keys()) {
		if(@baro.ignoreStyleProp(k)) continue;		
		val=node.get(k)
		if(typeof(val,'node','array')) continue;
		c=k.ch()
		if(c.eq('#','&','%')) {
			// #props, %:tag, &:function
			if(c.eq('#')) {
				_addProp(k.value(1),val)
			} else if(c.eq('&')) {
				_addFunc(k.value(1),val)
			}
			continue;
		}		
		@baro.addHtmlStyle(parent,page,node,k,val)
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
@baro.isFunc(&s) {
	c=s.next().ch()
	return when(c.eq('('), true)
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

@baro.isHtmlTag(key) {
	tag=key.lower()
	if(tag.start('h')) {
		c=key.value(1)
		if(typeof(c,'num')) return true;
	} else if(tag.eq('div','span','p','img','button','input','video','a','nav','header')) {
		return true;
	}
	return false;
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
@baro.styleValue(&s) {
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
	ws = wss().get('server')
	while(client, ws) {
		not(client) continue;
		if(client.cmp('mode','dev')) {
			client.sendWs("changePageScript\r\n\r\n${page.pageCode}")
		}
	}
}

replaceFindText(str, replace, value, sep) {
	pos = _find(str)
	if(typeof(pos,'num')) {
		return _replace(str,pos)
	}
	return false;
	
	_replace = func(&str, pos) {		
		if(pos>0) {
			ss=str.value(0,pos,true)
		} else {
			ss=''
		}
		size = replace.size()
		str.pos(pos+size)
		str.findPos(sep,1,1)
		ss.add(value)
		if(str.ch()) ss.add(str)
		return ss;
	};
	_find = func(&str) { 
		while(str.valid()) {
			left = str.findPos(replace,1,1) not(str.valid()) return false;
			not(left.ch()) return str.cur();
			c=left.ch(-1) not(c) return str.cur();
			if( c.eq(sep) ) {
				return str.cur()
			}
		}
		return false;
	};
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
	pos=s.size()-1;
	while(n=pos, 0) {
		c=s.ch(n)
		if(c.ne(ch)) {
			if(n.eq(pos)) return s;
			pos=n+1
			break;
		}
	}
	return s.trim(0,pos,true)
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
catchError() {
	err=Cf.error() not(err) return false;
	print("## catch error : $err")
	return true;
}
/* temp/page-test.js
##> leftNavbar [메뉴바 에니메이션] {
	bgColor:#223f4d
}
container
	navbar {
		flexCenter,fixed,x:40,w:80,p:20
		bg:@[bgColor.light(50)]
		rad:50
	}
		ul {render:renderNavItems(item), col,gap:10,width:100}
			li {rel,listStyle,w:100%,h:60, p:0 10px}
				a {href="#", flexCenter,ai:center,jc:flex-start, onclick(){linkClick('link click')}}
					span {class="icon", rel,block,minw:65,h:65,rad:65,bg:@[bgColor],c:#fff,fs:1.75em}
						icon {class="vicon @[item.icon]", abs,w:24,h:24}
		end
		<init>
			clog('init page this==>', @[this])
			alert('init')
		</init>
	end
<js>
	const aaa = () => {
		clog('aaa function ', @[this])
		@[ul].html('')		
	}
	const linkClick = () => {
		clog('link click')
	}
</js>	

#page-template.js
(function() {
	loadStyle(`
@[pageCss]
	`)
	@[pageFuncs]
	function initPage(page, content) {
		@[pageInit]
	}
	makePage('@[pageCode]', initPage)
})()
*/