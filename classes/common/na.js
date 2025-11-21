isFuncSrc(s) {
	not(s.ch()) return false;
	c=s.next().ch()
	while(c.eq('.')) c=s.incr().next().ch();
	not(c.eq('(')) return false;
	s.match()
	c=s.ch()
	return c.eq('{');
} 
nextStepPos(s,indent) {
	indentCur = indentCount(s)
	if( indent>=indentCur ) return;
	if( lineCheck(s,'{')) {
		s.findPos('{',0,1)
		ss=s.match()
		if(typeof(ss,'bool')) {
			line = s.findPos("\n")
			return print("nextStepPos match error LINE:$line")
		}
	}
	s.findPos("\n")
	return s.cur();
}
isValid(&s) {
	not(typeof(s,'string')) return false;
	not(s.ch()) return false;
	return true;
}
@na.parsePage(&s, node) {
	not(node) node = _node('pages').addNode()
	prev=-1;
	while(s.valid()) {
		if( lineBlankCheck(s)) {
			s.findPos("\n")
			continue;
		}
		not(isValid(s)) break;
		indent=indentCount(s)
		if( prev.eq(-1)) {
			prev = indent
		}
		if(prev.eq(indent)) {
			cur = node.addNode()
			cur.line = s.findPos("\n").trim()
			continue;
		}
		if(isFuncSrc(s)) {
			type = 'func'
			fnm = s.findPos('(',0,1)
			fparam = s.match()
			c=s.ch()
			not(c.eq('{')) return print("$fnm 함수 시작오류")
			fsrc = s.match(1)
			cur.addNode().with(type, fnm, fparam, fsrc)
		} else {
			type = 'props'
			line = s.findPos("\n")
			print("line ========> $line indent:$indent")
			if(lineCheck(line,':')) {
				name = line.findPos(':').trim()
				value = line.trim()
			} else {
				name = line.trim()
				value = ''
			}
			data = ''
			sp = s.cur()
			while(n=0,1000) {
				end = nextStepPos(s,indent) not(end) break;
				if(sp>=end ) {
					return print("## parse page props end check error: $s");
				}
				s.pos(end)
			}
			ep = s.cur()
			if(sp<ep) {
				data = s.value(sp,ep,true)
				print("==============", sp,ep,data, name)
			}
			cur.addNode().with(type, name, value, data)
		}
	}
	return node;
}