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
		while(n=1,asize) line.add("\r\n\t[$n]: ",a.get(n))
	}
	print("#log $tm>>$line")
}
@baro.val() {
	c=s.ch()
	if(c.eq()) {
		v=s.match()
		c=s.ch()
		if(c.eq(',',';')) s.incr()
	} else if(lineEndCheck(s,'//')) {
		v=s.findPos('//').trim()
		s.findPos("\n")
	} else if(lineEndCheck(s,',')) {
		v=s.findPos(',').trim()
	} else {
		v=s.findPos("\n").trim()
	}
	log("baro val => $v")
	return v;
}
@fapi.parseInfo(node, &s) {
	node.addNode('table')
	node.addNode('route')
	while(s.valid()) {
		c=s.ch()
		if(c.eq('/')) {
			c=s.ch(1)
			if(c.eq('/')) s.findPos("\n") else s.match()
			continue;
		}
		if(_isVar(s)) {
			if(lineEndCheck(s,':')) {
				k=s.findPos(':').trim()
			} else {
				k=s.findPos('=').trim()
			}
			v=@baro.val()
		} else if(_isNode(s)) {
			k=s.findPos('{',0,1).trim()
			v=s.match(1)
			if(k.eq('endpoint')) k='route'
			fnm=_s('@fapi.$key#info')
			fc=call(fnm)
			not(typeof(fc,'func'))  {
				print("@@ [FastAPI 분석정보 오류] $fnm 함수를 등록하세요 ")
				continue;
			}
			fc(node,v)
		} else {
			break;
		}
	}
	
	_isNode = func(s) {
		c=s.next().ch()
		return c.eq('{');
	};
	_isVar = func(s) {
		c=s.next().ch()
		return c.eq(':','=')
	};
}
/*
	class:table
		*field : int, pk, uuid, unique, index, text, now, dtm, str(n), fk(), list(model), rel(), def()
*/
@fapi.table#info(node, &s) {
	
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