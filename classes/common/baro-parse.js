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
		key = Cf.val(key,'-',page.incrNum('@varIndex'))
	} else {
		node.set('class',key)
		if(varMap.isset(key)) {
			key = Cf.val(key,'-',page.incrNum('@varIndex'))
		} else {
			varMap.set(key,true)
		}
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
	pageSrc = @baro.parseSource(parent, page, page, template,'html')
	print("## pageSrc ==> $pageSrc")
	@baro.changePageScript(parent, page)
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
		not(s.ch()) break		
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
		cur.path = path
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
	page.set('@funcsInit', ss)
	print("page script == $ss")
}

@baro.pageScript(parent, page, node ) {
	ss=''
	parentVar = node.parentNode().get('varName')
	node.inject(tag,class,key,varName)
	not(tag) tag='div'
	
	ss.add("const ${varName}=",'$',"('<${tag}")	
	if(node.get('@attr')) {
		ss.add(' ', node.get('@attr'))
	}
	if(node.get('@styValue')) {
		ss.add(' style="',node.get('@styValue'),'"')
	}
	ss.add("/>)")
	if(node.get('@sty')) {
		css=node.get('@sty')
		ss.add(".css({$css})")
	}
	if(node.isset('@html')) {
		css=node.html('@html')
		ss.add(".html(`$html`)")
	}
	ss.add(".appentTo($parentVar)",conf("cf.newline"))
	while(cur,node) {
		src=@baro.pageScript(parent,page,cur)
		ss.add(src)
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
			fparam=s.match(), fsrc=''
			c=s.ch()
			if(c.eq('{')) {
				fsrc=s.match(1)	
			}
			node.set("&$fnm", "${fparam}=>${fsrc}")
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
		if(c.eq()) {			
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
			html.add(@baro.parseSource(parent,page,node,src,'html'))
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
				cur.appendText('html', left.trim())
			}
			@baro.parseProps(parent,page,cur, body)
		}
		line = @baro.getLine(s);
		print("======> $key ", line)
		not(_tagValue()) {
			left = s.findPos("\n")
			if( left.ch()) {
				cur.appendText('html', left.trim())
			}
		}
		if( Cf.error()) {
			print(">> page parse error ", Cf.error(), s.size())
			return;
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
			} else if(tag.eq('js','init')) {
				body.findPos('>')
				src=@baro.parseSource(parent,page,cur,body,'js')
				if(tag.eq('js')) {
					nodeAppendText(page,'@func', src, conf("cf.newline"))
				} else {
					nodeAppendText(page,'@funcInit', src, conf("cf.newline"))
				}
			} else {
				props=body.findPos('>')
				src=@baro.parseSource(parent,page,cur,body,'html')
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
	arr=null
	if( param.ch()) {
		arr=_arr()
		node.set("@endpos", 0)
		sp=0
		while(param.valid()) {
			c=param.ch() not(c) break;
			arr.add( @baro.parseSource(parent, page, node, param, type) )
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
		if(type.eq('html','js','css')) {
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
				continue;
			} else {
				if( type.eq('html')) {
					if(page.isset(key)) {
						val=node.get(key)
					} else {
						val=page.get("@$key")
					}
				} else {
					val = @baro.findLayoutChild(pageNode,key)
				}
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
		print(">> $key = $val")
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
		bt:borderTop, bb:borderBottom, bl:borderLeft, br:borderRight,
		br:borderRadius,
		tr:transition,
		fwrap:flex-wrap,
		ani:animation
		anidelay: animation-delay,
		bgpos:background-position,
		bgsize:background-size,
		fit:object-fit,
		minh:minHeight,
		maxh:maxHeight,
		minw:minWidth,
		maxw:maxWidth,
		rel:relative,
		abs:absolute,
		space:letter-space,
		lh:line-height,
	])
	map.startTick = System.tick()
	return map;	
}

@baro.addHtmlStyle(parent, page, node, k, val) {
	_styleValue = func(&s) {
		val=s.trim()
		if(typeof(val,'num') || val.eq('true','false','null')) return val;		
		return Cf.val("'",val,"'");
	}
	_addSty = func(key, &s) {
		ss=''
		if(@baro.isFunc(s) || s.find('@[') ) {
			src=@baro.parseSource(parent,page,node,&s,'css')
			ss.add(src)
		} else {
			ss.add(s)
		}
		c=key.ch()
		if(c.eq('-')) {
			val=ss
			nodeAppendText(node,'@styValue',"$key:$val",";","$key:")
		} else if(ss) {
			val =_styleValue(ss)
			nodeAppendText(node,'@sty',"$key:$val",",","$key:")
		}
	};	
	map = @baro.styleMap()
	key = map.get(k.lower()) not(key) key=k
	if(key.eq('required','text','password')) {
		if(key.eq('required')) {
			nodeAppendText(node,'@attr','required',' ')
		} else {
			val = _s('type="${key}"')
			nodeAppendText(node,'@attr',val,' ')
		}
		return;
	}
	if(key.eq('flex')) {
		@baro.checkFlexStyle(node.parentNode(), node)
		return;
	}
	
	if(key.eq('absolute','relative','fixed')) {
		_addSty('position', val)
	}
	else if(key.eq('full')) {
		nodeAppendText(node,'@sty',"width:'100%',height:'100%'",',')
	}
	else if(key.eq('hidden','pointer','row','col')) {
		switch(key) {
		case hidden: 
			_addSty('display','hidden')
		case row: 
			_addSty('display','flex')
			_addSty('flexDirection','row')
		case col: 
			_addSty('display','flex')
			_addSty('flexDirection','column')
		default:
		}
	}
	else if(key && val ) {
		_addSty(key,val)
	}
}
@baro.makeHtmlProps(parent, page, node) {
	not(node) { 
		layout = page.get('@layout')
		node = layout.child(0)
	}
	map = @baro.styleMap()	
	_addProp=func(k,v) {
		key = map.get(k.lower()) not(key) key=k 
		attr = _s('${key}="${v}"')
		nodeAppendText(node,'@attr',attr, ' ')
	};
	_addFunc = func(k,&s) {
		fparam=s.findPos('=>')
		fsrc=@baro.parseSource(parent,page,node,s,'js')
		ss=''
		if(k.start('on')) {
			fnm = k.trim(2)
			ss.add(node.varName, ".off('$fnm').on('$fnm',($fparam)=>")
			if(fsrc.findPos("\n")) {
				ss.add("{$fsrc}")
			} else {
				ss.add(fsrc)
			}
			nodeAppendText(node,'@event', ss, conf("cf.newline"))
		} else if(k.eq('init')) {
			nodeAppendText(page,'@funcsInit', fsrc, conf("cf.newline"))
		} else {
			ss.add("const $k = ($fparam)=>")
			if(fsrc.findPos("\n")) {
				ss.add("{$fsrc}")
			} else {
				ss.add(fsrc)
			}
			nodeAppendText(page,'@funcs', ss, conf("cf.newline"))
		}
	};
	if(node.id) {
		val = Cf.val("id=",Cf.jsValue(node.id))
		nodeAppendText(node,'@attr', val, ' ')
	}
	if(node.class) {
		val = Cf.val("class=",Cf.jsValue(node.class))
		nodeAppendText(node,'@attr', val, ' ') 
	}
	while(k, node.keys()) {
		if(k.eq('id','key','class','varName','html')) continue;		
		val=node.get(k)
		if(typeof(val,'node','array')) continue;
		c=k.ch()
		print("##### makeHtmlProps $k = $val [$c]", _addProp)
		if(c.eq('#','&','%')) {
			// #props, %:tag, &:function
			if(c.eq('#')) {
				_addProp(k.value(1),val)
			} else if(c.eq('&')) {
				_addFunc(k.value(1),val)
			}
			print("## skip k==$k")
			continue;
		}
		@baro.addHtmlStyle(parent,page,node,k,val)
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

@baro.checkFlexStyle(parent, flexNode) {
	
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
	savePath = _s('${@web.rootPath}/assets/js/pages/${page.path}')
	fileWrite(savePath, src)
	ws = wss().get('server')
	while(client, ws) {
		not(client) continue;
		if(client.cmp('mode','dev')) {
			client.sendWs("changePageScript\r\n\r\n${page.pageCode}")
		}
	}
}

nodeAppendText(node,key,value,sep,replace) {
	ss=node.get(key)
	if(replace) {
		pos = _find(ss)
		if(typeof(pos,'num')) {
			return _replace(ss,pos);
		}
	}
	if(sep) {
		if(ss) node.appendText(key,sep)
	}
	node.appendText(key,value)
	
	_replace = func(&str, pos) {
		ss=str.value(0,pos,true)
		size = replace.size()
		str.pos(pos+size)
		str.findPos(sep,1,1)
		ss.add(value)
		if(str.ch()) ss.add(str)
		node.set(key,ss)
	};
	_find = func(&str) {
		while(str.valid()) {
			left = str.findPos(replace,1,1)
			c=left.ch(-1)
			if( c.eq(sep) || ~(c) ) {
				return str.cur()
			}
		}
		return false;
	}
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
