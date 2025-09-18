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