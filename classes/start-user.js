_node_user_func(source) {
	fn=Cf.funcNode('parent')
	funcIndex = _nodeUserfuncIndex_++;	
	not(lineCheck(source,'/')) {
		return _node_user_parse(source);
	}
	type=source.findPos('/').trim()		
	mode='set'
	switch(type) {
	case filter: 
		return _node_filter(source);
	case map: 
		return _node_map(source);
	case inject: 
		return _node_inject(source);
	case with: 
		return _node_with(source);
	default:
		return _node_user_default(source);
	}
}
_node_filter(&s) {
	Cf.funcNode('parent').inject(object, mode, fn, funcIndex)
	if(lineCheck(s,'=>')) {
		// (cur,idx)=>xxx또는 cur=>xxx 형태
		left=s.findPos('=>')
		if(left.ch('(')) {
			param=left.match()
		} else {
			param=left.trim()
		}
	} else {
		// 소스만 있는경우 item=>xxx 형태
		param='item'
	}
	funcName="nodefunc_$funcIndex"
	fc=call(funcName)
	not(typeof(fc,'func')) {		
		fc=_makeFunc(funcName, param, s)
	}
	arr=_arr()
	while(cur, object, idx) {
		if(fc(cur, idx)) arr.add(cur)
	}
	return arr;
}
_node_map(&s) {
	Cf.funcNode('parent').inject(object, mode, fn, funcIndex)
	if(lineCheck(s,'=>')) {		
		left=s.findPos('=>')
		if(left.ch('(')) {
			param=left.match()
		} else {
			param=left.trim()
		}
	} else {		
		param='item'
	}
	funcName="nodefunc_$funcIndex"
	fc=call(funcName)
	not(typeof(fc,'func')) {		
		fc=_makeFunc(funcName, param, s)
	}
	arr=_arr()
	while(cur, object, idx) {
		arr.add(fc(cur, idx))
	}
	return arr;
}

_node_inject(&s) {
	Cf.funcNode('parent').inject(object, mode, fn, funcIndex)	
	arr=[]	
	while(s.valid(), idx) {
		c=s.ch() not(c) break;
		if(c.eq(',',';')) {
			s.incr()
			continue;
		}
		name=s.move(), varName=name;
		if(s.start('->',true)) {
			varName=s.move()
		}
		if(varName.ch('@')) varName=varName.value(1)
		if(typeof(object,'node')) {
			if(object.isVar(name)) {
				val=object.get(name)
			} else {
				print("node_inject $name 요소가 객체에 미설정", object);
				val=null
			}
		} else {
			if(isValid(object,idx)) {
				val=object.get(idx)
			} else {
				val=null
				print("node_inject 배열범위를 벗어났습니다 (인덱스:$idx 변수명:$name)");
			}
		}
		switch(mode) {
		case arrray:
			arr.add(val)
		case set:
			fn.set(varName, val)
		case merge:
			not(fn.isset(varName)) fn.set(varName,val)
		}
	}
}
_node_with(&s) {
	// 향후 mode에 따라 overwirte 할지 skip 할지 처리 (현 무조건 overwrite)
	Cf.funcNode('parent').inject(object, mode, fn, funcIndex)	
	arr=[]
	while(s.valid(), idx) {
		c=s.ch() not(c) break;
		if(c.eq(',',';')) {
			s.incr()
			continue;
		}
		name=s.move(), val='', varName=''
		if(s.start('=',true)) {
			pos=_evalEndPos(s)
			if(pos) {
				src=s.value(sp,pos,true)
				val=eval(src,fn)
				s.pos(pos)
			} else {
				c=s.ch()
				if(c.eq()) {
					val=s.match()
					if(c.eq('"') && val.find('$') ) {
						val=fmt(val,fn)
					}
				}
				else if(lineCheck(s,',')) {
					varName=s.findPos(',').trim()
				} 
				else if(lineCheck(s,';')) {
					varName=s.findPos(';').trim()
				} 
				else {
					varName=s.findPos("\n").trim()
				}
			}
		} else {
			varName=val
		}
		if(varName) {
			if(varName.ch('@')) varName=varName.value(1)
			val=fn.get(varName)
		}
		if(typeof(object,'node')) {
			object.set(name,val)
		} else {
			object(val)
		}
	}
}
_makeFunc(fnm, param, source) {
	ss=''
	if(getLineCount(source)) {
		src=source.trim()
		if(src.start('return')) {
			ss.add(src)
		} else {
			ss.add("return $src")
		}
	} else {
		ss.add(src)
	}
	fsrc=str('$fnm($param){$ss}')
	call(fsrc)
	print("makeFunc == $fsrc")
	return call(fnm)
}
_evalEndPos(&s) {
	c=s.ch() not(c) return 0;
	if(c.eq()) return 0;
	sp=s.cur()
	while(s.valid(), idx) {
		c=s.ch() not(c) break;
		if(c.eq(',',';')) break;
		if(c.eq()) {
			s.match()
			continue;
		}
		if(c.eq('(','{','[')) {
			s.match(1)
			continue;
		}
		if(c.is('oper')) {
			s.incr()
			continue;
		}
		c=s.next().ch()
		while(c.eq('.')) {
			type='sub'
			c=s.next().ch()
		}
		if(c.eq('(','[')) {
			s.match()
			c=s.ch()
			if(c.eq('.')) {
				s.incr()
			}
		} else if(idx.eq(0) ) {
			if(c.eq(',',';') || lineBlankCheck(s) ) {
				return;
			}
		}
	}	
	return s.cur(); 
}