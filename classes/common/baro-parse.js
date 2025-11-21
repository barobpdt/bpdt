@baro.makePageScript(parent, page, savePath) {
	parseLayout(page.get('@layout'))
	
	parseLayout = func(node) {
		while(cur,node) {
			parseLayout(cur)
		}
	};
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
		} else {
			sp=s.cur()
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
			node.set(k, "\f${fparam}=>$fsrc")
			continue
		}
		bprop=false
		if(c.eq(':')) {
			bprop=true
			c=s.incr().ch()
		}
		if(c.eq()) {
			v=s.match()
			node.set(k,v)
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
			src=s.match("<$tag","</$tag>",4)
			node.set(k, "<${tag}${src}</${tag}>")
		} else if(bprop) {
			if(lineCheck(s,',')) {
				v=s.findPos(',').trim()
			} else {
				v=s.findPos("\n").trim()
			}
			node.set(k,v)
		} else {
			node.set(k, true)
		}
	}
}

@baro.parsePages(parent, &s) {
	cur = null
	while(s.valid()) {				
		left = s.findPos('##>')
		if(cur) {
			prev = conf("pageSrc.${cur.pageCode}")
			if(prev.eq(left)) {
				print("######## @baro.parsePage not chanage", cur)
			} else {
				conf("pageSrc.${cur.pageCode}", left, true)
			}
			cur.src = left
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

@baro.parseHtmlLayout(page, &s) { 
	parentArray=[]
	layout = page.addNode('@layout').removeAll(true)
	indentArray=page.addArray('@indentArray').reuse()
	parentArray.add(layout)
	while(s.valid()) {
		if(lineBlankCheck(s)) {
			s.findPos("\n")
			continue;
		}
		a = indentText(s)
		if( indentArray.size()) {			
			idx=indentArray.find(a)
			endCheck = idx.eq(0) || ~(a)
			if(idx==-1) {
				print("@@ end check $idx $endCheck")
				idx=indentArray.size()
				indentArray.add(a)
			}
		} else {
			idx=0
			indentArray.add(a)
		}
		parent = parentArray.get(idx)
		print("xxxxx $idx $parent $line")
		not(parent ) return print("@@ 레이아웃 분석 부모노드 찾기오류 idx:$idx $line");
		cur = _findEnd(parent)
		not(cur) continue
		setArray(parentArray, idx+1, cur)
	}
	
	_findEnd = func(parent) {
		c=s.ch() not(c) return;
		startPos = s.cur()
		if( s.start('end') ) {
			s.findPos("\n")
			return;
		}
		if(c.eq('.','#')) s.incr()
		sp = s.cur()
		c=s.next().ch(1)
		while(c.eq('-','.')) c=s.incr().next().ch(1)
		key = s.trim(sp, s.cur(), true)
		cur = parent.addNode()
		cur.set('key', key)
		cur.set('html','')
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
		if(_checkTag(s) ) {
			while(_checkTag(s)) {
				c=s.ch() not(c) break;
				sp=s.cur()
				c=s.incr().next().ch(1)
				if(c.eq('-',':')) c=s.incr().next().ch(1)
				tag=s.trim(sp+1, s.cur, true)
				body=s.match("<$tag", "</$tag>",8)
				if(typeof(body,'bool')) {
					return print("매칭되는 태그를 찾을수 없습니다", left, tag);
				}
				cur.appendText('html', "<$tag",body,"</$tag>")
			}
		}
		else {
			left = s.findPos("\n")
			if( left.ch()) {
				cur.appendText('html', left.trim())
			}
		}
		return cur;
	};
	_checkTag = func(s) {
		c=s.ch()
		return c.eq('<')
	};
}
@baro.styleMap() {
	map=Cf.getObject('baro','styleMap') if(map) return map;
	map=Cf.getObject('baro','styleMap',true)
	map.parseJson(#[
		w:width,h:height,p:padding,m:margin,
		mt:marginTop, mb:marginBottom, ml:marginLeft, mr:marginRight,
		pt:paddingTop, pb:paddingBottom, pl:paddingLeft, pr:paddingRight,
		bt:borderTop, bb:borderBottom, bl:borderLeft, br:borderRight,
		minH:minHeight,
		maxH:maxHeight,
		minW:minWidth,
		maxW:maxWidth,
		rel:relative,
		abs:absolute
	])
	return map;	
}
@baro.setHtmlStyle(node) {
	sty = ''
	map = @baro.styleMap()
	while(k, node.keys()) {
		if(k.eq('key','class')) continue;
		val=node.get(k)
		key = map.get(k) not(key) key=k
		if(key.eq('absolute','relative','fixed')) {
			_add_sty('position', val)
		}
		else if(key.eq('hidden','pointer','flex','row','col')) {
			switch(key) {
			case hidden: _add_sty('display','hidden',true)
			case flex: 
				@baro.checkFlexStyle(node.parent(), node)
			case row: 
				_add_sty('display','flex',true)
				_add_sty('flexDirection','row',true)
			case col: 
				_add_sty('display','flex',true)
				_add_sty('flexDirection','column',true)
			default:
			}
		} 
		else if(key && key.ne(k)) {
			_add_sty(key,val)
		}
	}
	node.set('@style',sty)
	while( cur, node) {
		@baro.setHtmlStyle(cur)
	}
	_add_sty = func(k,&s,skip) {
		if(sty) sty.add(';')
		if(skip) return sty.add(k,':',s);
		ss=''
		if(_isFunc(s)) {
			
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
		sty.add(k,':',ss)
	};
	_isFunc = func(&s) {
		c=s.next().ch()
		return when(c.eq('('), true)
	}
}
@baro.checkFlexStyle(parent, flexNode) {
	
}
@baro.loadPage(path) {
	parent = object('baro.pages')
	src = fileRead(path)
	@baro.parsePages(parent, src)
}