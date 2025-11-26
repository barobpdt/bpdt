@baro.loadPage(path) {
	not(path) path='c:/temp/page-test.js'
	parent = object('baro.pages')
	parent.set('@style','')
	src = fileRead(path)
	@baro.parsePages(parent, src)
}
@baro.initLayoutVar(page, node, key) {
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
	node.set('html','')
	node.set('@js','')
	node.set('@css','')
	node.set('@js','')
}
@baro.initPage(parent,page,src,template) {
	not(template) template = fileRead('C:/temp/page-template.txt')
	pageCode=page.pageCode
	page.set('@css','')
	page.set('@sty','')
	page.set('@funcs','')
	page.set('@funcsInit', '')
	page.set('@varIndex',1)
	page.addNode("@varMap").reuse()

	@baro.makeHtmlLayout(parent, page, src)
	@baro.makeHtmlProps(parent, page)
	@baro.makePageScript(parent, page)
	page.set('pageCss','')
	if( parent.get('@style')) {
		parent.set('@style','')
		nodeAppendText(page, 'pageCss', parent.get('@style'))
	}
	if( page.get('@css')) {
		nodeAppendText(page, 'pageCss', page.get('@css'))
	}
	page.set('pageFuncs',page.get('@funcs'))
	page.set('pageInit',page.get('@funcsInit'))
	
	pageSrc = @baro.parseSource(parent, page, page, template,'html')
	prev = conf("pageSrc.${pageCode}")
	if(prev.eq(src)) {
		print("######## @baro.parsePage not chanage", page)
	} else {
		conf("pageSrc.${pageCode}", src, true)
	}
}

@baro.parsePages(parent, &s, template) {
	cur = null
	while(s.valid()) {				
		left = s.findPos('##>')
		if(cur) {
			@baro.initPage(parent,cur,left,template)
		}
		not(s.ch()) break
		if( lineCheck(s,'{')) {
			pageCode= s.findPos('{',0,1).trim()
			cur = parent.addNode(pageCode)
			data = s.match() if(typeof(data,'bool')) return print("$pageCode 페이지 설정정보 매칭오류")
			cur.parseJson(data)
		}
		else if( lineCheck(s,'[')) {
			pageCode= s.findPos('[',0,1).trim()
			cur= parent.addNode(pageCode)
			cur.title = s.match() if(typeof(data,'bool')) return print("$pageCode 페이지 타이틀 매칭오류")
			if( lineCheck(s,'{')) {
				data = s.match() if(typeof(data,'bool')) return print("$pageCode 페이지 설정정보 매칭오류")
				cur.parseJson(data)
			}
		} else {
			pageCode = s.findPos("\n").trim()
			cur = parent.addNode(pageCode)
		}
		cur.pageCode = pageCode
	}		
}

@baro.makePageScript(pages, page, savePath) {
	layout = page.get('@layout')
	cur = layout.child(0)
	not(typeof(cur,'node')) return print("@@ make page script 오류 페이지 레이아웃 노드미정의", page, layout)
	@baro.pageScript(pages, cur)	
}

@baro.pageScript(pages, node, parent) {
	nl = "\r\n"
	ss=''
	parentVar = when(parent, parent.varName, 'content')
	node.inject(tag,class,key,varName)
	not(tag) tag='div'
	
	ss.add(#[const ${varName}=$('<${tag}/>')])
	while(cur,node) {
		ss.add(@baro.pageScript(pages,cur,node))
	}
	return ss;
}


@baro.parseArray(arr, &s, node ) {
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
			@baro.parseProps(cur, s.match())
			arr.add(cur)
		} else if(c.eq('[')) {
			a = node.addArray()
			@baro.parseArray(a, s.match(), node)
		} else if(lineCheck(s,',')) {
			v=s.findPos(',').trim()
			arr.add(v)
		} else {
			v=s.findPos("\n").trim()
			arr.add(v)
		}
	}
}
@baro.parseProps(node, &s) {
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
			if(s.start('--')) {
				s.incr(2)
			}
			c=s.next().ch()
			while(c.eq('-')) {
				c=s.incr().next().ch()
			}
			k=s.trim(sp,s.cur(),true)
		}
		if(c.eq('(')) {
			fparam=s.match(), fsrc=''
			c=s.ch()
			if(c.eq('{')) {
				fsrc=s.match(1)	
			}
			node.set("&$key", "${fparam}=>${fsrc}")
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
			@baro.parseProps(cur, s.match())
		} else if(c.eq('[')) {
			arr = node.addArray(k)
			@baro.parseArray(arr, s.match(), node)
		} else if(c.eq('<')) {
			sp=s.cur()
			c=s.incr().next().ch()
			while(c.eq('-')) {
				c=s.incr().next().ch()
			}
			tag = s.trim(sp+1,s.cur(),true)
			s.pos(sp)
			src=s.match("<$tag","</$tag>",8)			
			node.set("%$k", "<${tag}${src}</${tag}>")
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
		not(s.ch()) return;
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
		not(root ) return print("@@ 레이아웃 분석 부모노드 찾기오류 idx:$idx $line");
		
		if(c.eq('.','#')) s.incr()
		sp = s.cur()
		c=s.next().ch(1)
		while(c.eq('-','.')) c=s.incr().next().ch(1)
		key = s.trim(sp, s.cur(), true)
		cur = root.addNode()
		
		@baro.initLayoutVar(page,cur,key)
		endCheck = false
		if( lineCheck(s,'{') ) {
			left = s.findPos('{',1,1)
			body = s.match(1)
			if(typeof(body,'bool')) return print("레이아웃 속성 매핑오류", left);
			if( left.ch()) {				
				cur.appendText('html', left.trim())
			}
			@baro.parseProps(cur, body)
		}
		not(_tagValue()) {
			left = s.findPos("\n")
			if( left.ch()) {
				cur.appendText('html', left.trim())
			}
		}
		if(root==layout) {
			cur.varName = _varName("${page.pageCode}-${cur.key}")
		} else if( cur.id ) {
			cur.varName = _varName(cur.id)
		} else {
			cur.varName = _varName(cur.key)
		}
		setArray(parentArray, idx+1, cur)
	}
	_varName = func(&s) {
		ss='', upper=false
		while(n=0,s.size()) {
			c=s.ch(n)
			if(c.eq('-','_')) {
				upper=true
				continue;
			}
			if(upper){
				ss.add(c.upper())
				upper=false
			} else {
				ss.add(c)
			}
		}
		return ss;
	};
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
			print("_checkTag", page.pageCode, tag)
			s.pos(sp)
			body=s.match("<$tag", "</$tag>",8)
			if(typeof(body,'bool')) {
				return print("매칭되는 태그를 찾을수 없습니다", left, tag);
			}
			if(tag.eq('css','style')) {
				body.findPos('>')
				src=@baro.parseSource(parent,page,cur,body,'css')
				print("$tag ######## $body ##############")
				if(tag.eq('css')) {
					page.appendText('@css', src) 
				} else {
					parent.appendText('@style', src)
				}
			} else if(tag.eq('js','init')) {
				body.findPos('>')
				src=@baro.parseSource(parent,page,cur,body,'js')
				if(tag.eq('js')) {
					page.appendText('@js', src)
				} else {
					page.appendText('@init', src)
				}
			} else {
				props=body.findPos('>')
				src=@baro.parseSource(parent,page,cur,body,'html')
				cur.appendText('html', "<$tag")
				if(props.ch()) {
					cur.appendText('html'," $props>")
				} else {
					cur.appendText('html', ">")
				}
				cur.appendText('html', @baro.parseHtml(page,cur,body),"</$tag>")
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
	if(fnm.eq('random','randomInt','randomFloat')) {
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
		
	else if(fnm.eq('border')) {
		
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
	print("@@ parseSource $type ", s)
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
		param = s
		ss.add(_paramVal())
	}	
	_paramVal = func() {
		not(param.ch()) return;
		val='', prev=''
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
			bref=false			
			if(c.eq('*')) {
				bref=true
				param.incr()
			}
			
			sp=param.cur()
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
			if(key.eq('this')) {
				
			}
			else if( typeof(prev,'node') && prev.isset(key) ) {
				val=prev.get(key)
			} 
			else if(node.isset(key)) {
				val=node.get(key)
			} 
			else if(page.isset(key)) {
				val=page.get(key)
			} 
			else {
				val=parent.get(key)
			}
			not(val) {
				if(c.eq(':')) {
					param.incr()
					val=param.trim()
				}
			}
		}
		not(typeof(val,'string')) return "$val";
		return val;
	};
	return ss;
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
@baro.styleFuncValue(parent,page,node,key,&s) {
	name=s.move()
	c=s.ch() not(c.eq('(')) return;
	
	return "$key:$ss"
}
@baro.addHtmlStyle(parent, page, node, k, val) {
	_styleValue = func(&s) {
		val=s.trim()
		if(typeof(val,'num') || val.eq('true','false','null')) return val;		
		return Cf.val("'",val,"'");
	}
	_addStyle =func(key, &s, skip) {
		ss=''
		if(skip) {
			ss.add(@baro.jsVal(s));
		} else if(@baro.isFunc(s) || s.find('@[') ) {
			ss.add(@baro.parseSource(parent,page,node,&s))
		} else {
			while(s.valid(), n) {
				val=s.findPos(" \t\n",4) not(val.ch()) break;
				if(n) ss.add(' ')
				if(typeof(val,'num')) {
					ss.add(val,'px')
				} else {
					ss.add(val)
				}
			}
		}
		if(ss) {
			val =_styleValue(ss)
			nodeAppendText(node,'@sty',"$key:$val",';')
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
	}
	else if(key.eq('absolute','relative','fixed')) {
		_addSty('position', val)
	}
	else if(key.eq('hidden','pointer','flex','row','col')) {
		switch(key) {
		case hidden: _addSty('display','hidden',true)
		case flex: 
			@baro.checkFlexStyle(node.parent(), node)
		case row: 
			_addSty('display','flex',true)
			_addSty('flexDirection','row',true)
		case col: 
			_addSty('display','flex',true)
			_addSty('flexDirection','column',true)
		default:
		}
	} 
	else if(key && key.ne(k)) {
		_addSty(key,val)
	}
	
}
@baro.makeHtmlProps(parent, page, node) {
	not(node) node = page
	map = @baro.styleMap()	
	_addProp=func(k,v) {
		key = map.get(k.lower()) not(key) key=k
		val = _s('key="${v}"')
		nodeAppendText(node,'@attr', val, ' ')
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
			nodeAppendText(node,'@js', ss, "\r\n")
		} else if(k.eq('init')) {
			nodeAppendText(page,'@funcsInit', fsrc, "\r\n")
		} else {
			ss.add("const $k = ($fparam)=>")
			if(fsrc.findPos("\n")) {
				ss.add("{$fsrc}")
			} else {
				ss.add(fsrc)
			}
			nodeAppendText(page,'@funcs', ss, "\r\n")
		}
	};	
	while(k, node.keys()) {
		if(k.eq('key','class')) continue;		
		val=node.get(k)
		if(typeof(val,'node','array')) continue;
		if(k.start('#','%','&')) {
			// #props, %:tag, &:function
			if(k.start('#')) {
				_addProp(k.value(1),val)
			} else if(k.start('&')) {
				_addFunc(k.value(1),val)
			}
			continue;
		}
		@baro.addHtmlStyle(parent,page,node,k,val)
	}	
	while( cur, node) {
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
nodeAppendText(node,key,value,sep) {
	if(sep) {
		if(node.get(key)) node.appendText(key,sep)
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

