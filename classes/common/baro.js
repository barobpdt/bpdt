@baro.tableName(&s) {
	sz=s.size()
	while(n=0,sz ){
		c=s.ch(n)
		if(c.is('upper')) {
			if(n) ss.add('_')
		}
		ss.add(c.upper())
	}
	return ss;
}
/*
https://jason.today/falling-sand
s=#[
	contents {@vbox}
		title {@rows, height:40}
			img {src:}
			h3 asasas
			img {src:/icon.png, click:@toggleLeft}
		box {@hbox}
			leftPanel {@vbox, flex:30, padding:2}
				codeGroupTitle {@hbox}
				codeGroupList {@vbox}
			centerPanel {@vbox, flex:70, padding:2}
				codeListTitle {@render, @hbox}
				codeListHeader {@hbox}
				codeListBody {@vbox}
		end box
	end contents
]
node=_node("page")
@baro.parseLayout(node, s)
ss=toString(node.get('@layout'), 2)
System.copyText(ss)
layout = node.get('@layout')
@baro.parseLayoutLine(layout)
*/
@baro.parseLayout(page, &s) { 
	prev = false
	if(lineBlankCheck(s)) {
		prev = true
		s.findPos("\n")
	} 
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
		line = findEnd() not(line) break; 
		if( prev ) {
			prev = false
			root = true
		} else {
			root = ~(indentArray.size())
			not(root) {
				idx=indentArray.find(a)
				endCheck = idx.eq(0) || ~(a)
				print("@@ end check $idx $endCheck")
				if(idx==-1) {
					idx=indentArray.size()
					indentArray.add(a)
				}
			}
		}
		if( root ) {
			idx=0
			indentArray.add('')
		}
		parent = parentArray.get(idx)
		print("xxxxx $idx $parent $line")
		not(parent ) return print("@@ 레이아웃 분석 부모노드 찾기오류 idx:$idx $line");
		cur = parent.addNode()
		cur.set('@line', line)
		setArray(parentArray, idx+1, cur)
	}
	findEnd = func() {
		ss=''
		c=s.ch() not(c) return;
		startPos = s.cur()
		if( s.start('end') ) {
			s.findPos("\n")
			return 'end';
		}
		if( lineCheck(s,'<') ) {
			left = s.findPos('<',1,1)
			sp=s.cur()
			c=s.incr().next().ch()
			while(c.eq('-',':')) c=s.incr().next().ch()
			tag=s.trim(sp+1, s.cur(), true)
			body=s.match("<$tag", "</$tag>", 4)
			if(typeof(body,'bool')) {
				return print("매칭되는 태그를 찾을수 없습니다", left, tag);
			}
			ss.add(left)
			ss.add('<#tag#>'
		} else if( lineCheck(s,'{') ) {
			left = s.findPos('{',1,1)
			body = s.match()
			if(typeof(body,'bool')) {
				return print("레이아웃 속성 매핑오류", left);
			}
			ss.add(left)
			ss.add('<#json#>')
		} else {
			left = s.findPos("\n")
			body = ''
			ss.add(left)
		}
		if(body) {
			s.findPos("\n")
		}
		return ss;
	};
	checkTag = func(s) {
		c=s.ch()
		return c.eq('<')
	}
}

@baro.parseLayoutLine(root) {
	while(cur, root, n) {
		s=cur.ref('@line')
		print(">>$s")
		@baro.parseLayoutLine(cur)
	}		
}


@baro.getAppNode(appId) {
	root =f.getObject("apps.$appId") if(root) return root;
	root=object("apps.$appId")
	root.addNode("@pages")
	root.addNode("@apis")
	root.addNode("@tables")
	return root;
}
@baro.getPageNode(apps, pageId) {
	if( typeof(apps,'string')) {
		apps = @baro.getAppNode(apps)
	}
	not(apps) apps=_node('apps')
	pages = apps.get("@pages")
	page = pages.get(pageId) 
	if(page) return page;
	page = page.addNode(pageId)
	page.set('pageId', pageId)
	page.addNode('@renders')
	page.addNode('@layout')
	page.addNode('@funcs')
	page.addNode('@configData')
	return page;
}
@baro.loadPage(param) {
	switch(args().size()) {
	case 2:
		args( pageId, fileName)
		appId = 'baroApp')
	case 3:
		args(appId, pageId, fileName)
	case 4:
		args(appId, pageId, fileName, reload)
	}
	root = @baro.getAppNode(appId)
	page = @baro.getPageNode(root, pageId)
	modifyDt = Baro.file().modifyDate(fileName)
	not(modifyDt) {
		return print("@loadPage error", pageId, fileName);
	}
	not(reload) {
		if( modifyDt.ne(page.get('@modifyDate')) ) return page;
	}
	page.set('@pageId', pageId)
	src = fileRead(fileName) not(src) print("@@ 페이지 소스가 없습니다", pageId, fileName)
	@baro.parsePage(page, src)
	return page;
}
@baro.isAppSrc(&s) {
	not(s.ch()) return false;
	c=s.next().ch()
	while(c.eq('.',':','-')) c=s.incr().next().ch();
	if(c.eq('(')) 
}

@baro.isFuncSrc(&s) {
	c=s.ch()
	not(c) return false;
	s.start('const',true)
	s.start('var',true)
	s.start('function',true)
	if(c.eq('@')) s.incr().ch()
	c=s.next().ch()
	while(c.eq('.')) c=s.incr().next().ch();
	if(c.eq('=')) {
		c=s.incr().ch()
		s.start('function',true)
		if(c.eq('(')) {
			s.match()
			c=s.ch()
			if( s.start('=>') ) return true;
		} else if( lineEndCheck(s,'=>') ) {
			return true;
		} else {
			return false;
		}
	} else if(c.eq('(')) {
		s.match()
	} else {
		return false;
	}
	c=s.ch()
	if( lineEndCheck(s,'=>') ) {
		return true;
	}
	return c.eq('{')
}
@baro.tagEndPos(&s) {
	sp=s.cur() 
	c=s.incr().next().ch()
	while(c.eq('.','-')) c=s.incr().next().ch();
	tag = s.trim(sp+1, s.cur())
	body = s.match("<$tag","</$tag>", 4)
	if(typeof(body,'bool')) return;
	return s.cur()
}
@baro.funcEndPos(&s) {
	not(s.ch()) return;
	c=s.next()
	while(s.valid()) {
		c=s.ch() not(c) break;
		if( c.eq('(') ) {
			s.match()
		} else if( c.eq('<') ) (
			ep = @baro.tagEndPos(s)
			not(typeof(ep,'num')) return;
			s.pos(ep)
		} else {
			if(c.eq('@')) c=s.incr()
			c=s.next().ch()
			while(c.eq('.')) c=s.incr().next().ch()
			if( c.eq('(') ) {
				s.match()
			}
		}
		c=s.ch()
		if(c.eq('?',':',';')) {
			s.incr()
			if(c.eq(';')) return s.cur();
			continue;
		}
		break;
	}
	return s.cur();
}
@baro.openNodeInfo(node, fileName) {
	not(fileName) fileName='c:/temp/node_info.json'
	src=toString(node, true, false)
	not(src) return print('@@saveNodeInfo 노드데이터가 없습니다')
	fileWrite(fileName, src)
	node=_node()
	node.set('@saveFileName', fileName)
	@job.addPost('openUrl', node)
}
@baro.parseApp(appId, &s) {
	app = @baro.getAppNode(appId)
	while(s.valid()) {
		
	}
	return app;
}

@proc.print(type,data) {
	print("$type>> $data")
}
@parse.youtubeSource(&s) {
	url = s.findPos('=>')
	print("YOUTUBE source URL:$url 분석시작")
	not(s.ch()) return;
	node = this
	node.fullpath='c:/temp/youtubeSource.html'
	fileWrite(node.fullpath, s)
	print(">> job.addPost start", node.fullpath)
	// @job.addPost('openSource', this)
	while(s.valid()) {
		s.findPos('<a data-testid="next-link"') not(s.ch()) break;
		s.findPos('<img src=')
		c=s.ch()
		if(c.eq()) {
			img=s.match()
			print("image URL:$img")
		}
	}
}
@parse.echo(&s) {
	print("echo >> $s")
}

/*
base: baro
table {
}
route {
}
*/
log(&s) {
	tm = System.date('hh:mm:dd')
	if(s.ch()) line=s.findPos("\n").trim() else line='[null]'
	a=args()
	asize=a.size()
	if(asize>1) {
		while(n=1,asize) line.add(", ",a.get(n))
		print("#log $tm>>($line)")
	} else {
		print("#log $tm>>$line")
	}
}
@baro.lineVal() {
	c=s.ch()
	isComma = func(s) {
		c=s.next().ch()
		while(c.eq('.')) c=s.incr().next().ch()
		return c.eq(',')
	};
	if(c.eq()) {
		v=s.match()
		c=s.ch()
		if(c.eq(',',';')) s.incr()
	} else if(isComma(s)) {
		v=s.findPos(',').trim()
	} else if(lineCheck(s,'//')) {
		v=s.findPos('//').trim()
		s.findPos("\n")
	} else {
		v=s.findPos("\n").trim()
	}
	return v;
}
@baro.isFunc(&s) {
	c=s.next().ch()
	while(c.eq('.')) c=s.incr().next().ch();
	return c.eq('(');
}
@baro.isInfo(&s) {
	c=s.next().ch()
	while(c.eq('.')) c=s.incr().next().ch();
	return c.eq('{');
}
@fapi.parseInfo(node, &s) {
	node.addNode('@table')
	node.addNode('@route')
	while(s.valid()) {
		if(lineBlankCheck(s)) {
			s.findPos("\n")
			continue;
		}
		c=s.ch()
		if(c.eq('/')) {
			c=s.ch(1)
			if(c.eq('/')) s.findPos("\n") else s.match()
			continue;
		}
		if(@baro.isInfo(s)) {
			k=s.findPos('{',0,1).trim()
			v=s.match(1)
			if(k.eq('endpoint')) k='route'
			fnm=_s('@fapi.$k#info')
			fc=call(fnm)
			not(typeof(fc,'func'))  {
				print("@@ [FastAPI 분석정보 오류] $fnm 함수를 등록하세요 ")
				continue;
			}
			fc(node,v)
		} else {
			log("parse => $s")
			if( lineCheck(s,':')) {
				k=s.findPos(':').trim()
				v=@baro.lineVal()
			} else {
				k=@baro.lineVal()
				v=true
			}
			log(k,v)
			node.set(k,v)
			not(lineBlankCheck(s)) {
				c=s.ch()
				if(c.eq(',',';')) s.incr()
			}
		}
	}
}
/*
	#comment
	class:table
		*field : int, pk, uuid, unique, index, text, now, dtm, str(n), fk(), list(model), rel(), def()
*/
@fapi.table#info(root, &s) {
	if( lineBlankCheck(s)) {
		s.findPos("\n")
	}
	node=root.get('@table')
	not(typeof(node,'node')) node=root.addNode('@table')
	node.removeAll(true)
	indent = indentCount(s)
	comment = ''
	cur=null
	while(s.valid()) {
		if(lineBlankCheck(s)) {
			s.findPos("\n")
			continue;
		}
		cnt = indentCount(s)		
		c=s.ch()
		if(c.eq('#')) {
			while(c.eq('#')) c=s.incr().ch();
			comment = s.findPos("\n").trim()
			continue;
		}
		if(cnt<indent ) {
			continue
		}
		print(">> $indent $cnt ")
		if( cnt.eq(indent) ) {
			if(lineCheck(s,':')) {
				tableClass = s.findPos(':').trim()
				tableName = @baro.lineVal()
			} else {
				tableClass = @baro.lineVal()
				tableName = tableClass
			}
			log('table class:', tableClass, tableName)
			cur=node.addNode(tableClass)
			cur.set('tableComment', comment)
			comment=''
		} else if(cur) {
			notnull = false
			if(c.eq('*')) {
				notnull = true
				c=s.incr().ch()
			}
			if(lineCheck(s,':')) {
				field=s.findPos(':').trim()
			} else {
				field=@baro.lineVal()
			}
			sub=cur.addNode(field)
			sub.set('notnull', notnull)
			sub.set('field', field)
			parseColumn(sub)
		} else {
			log("table info error $s", cnt, indent)
			break;
		}
	}
	parseColumn = func(sub) {
		while(s.valid()) {
			if(@baro.isFunc(s)) {
				k=s.findPos('(',0,1).trim()
				v=s.match()
			} else {
				k=s.move()
				v=true
			}
			sub.set(k,v)
			if(lineBlankCheck(s)) {
				s.findPos("\n")
				break;
			}			
			c=s.ch()
			if(c.eq('/')) {
				c=s.ch(1)
				if(c.eq('/')) cmt = s.findPos("\n") else cmt=s.match();
				sub.set('comment', cmt.trim())
				s.findPos("\n")
				break;
			}
			if(c.eq(',')) s.incr()
		}
	};
}
@fapi.route$info(node, &s) {

}

@fpai.parseContent(node, &s) {
	while(s.valid()) {
		left=s.findPos('@base')
		ss.add(left)
		not(s.ch()) break;
		sp=s.cur()
		c=s.ch()
		if(c.eq('.')) {
			name=s.incr().move()
			sp=s.cur()
			c=s.ch()
		}
		if(c.eq('?','|','{')) {
			if(c.eq('?','|')) {
				c=s.incr().ch()
				if(c.eq('[')) {
					v=s.match()
				} else if(c.eq()) {
					v=s.match()
				} else {
					v=s.move()
				}
			}
		} else {
			s.pos(sp)
		}
	}
	return ss;
}
@python.isStartComment(&s, move) {
	c=s.ch()
	if(s.start("'''",'"""')) {
		if(s.start("'''")) {
			s.match("'''","'''")
		} else {
			s.match('"""','"""')
		}
		return s.cur()
	} 
	if(c.eq('#')) {
		s.findPos("\n")
		return s.cur()
	}
	return;
}
@python.removeIndent() {
	lines=_arr()
	aa=_arr()
	ss=''
	ln="\r\n"
	while(s.valid()) {
		if(lineBlankCheck(s)) {
			s.findPos("\n")
			continue;
		}
		ep=@python.isStartComment(s)
		if(typeof(ep,'num')) {
			s.pos(ep)
			continue;
		}
		line=s.findPos("\n").trim('right')
		lines.add(line)
	}
	while(line, lines, n) {
		a=indentText(line)
		idx = aa.find(a)
		if(aa.size() && idx.eq(0)) {
			print("ss=>$ss")
			aa.reuse()
			ss=''
			idx=-1;
		}
		if(idx<0) {
			not(aa.size()) {
				firstIndent = a.size()
			}
			aa.add(a)
		}
		if( firstIndent) {
			ss.add(line.value(firstIndent),ln)
		} else {
			ss.add(line, ln)
		}
	}
}