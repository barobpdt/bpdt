<script>
left(&str, sep) {
	not(sep) sep=',';
	return str.findPos(sep).trim();
}
right(&str, sep) {
	not(sep) sep='.';
	if( str.find(sep)) str.findLast(sep)
	return str.trim();
}

nextLine(&s) {
	not(s.ch()) return;
	return s.findPos("\n").trim()
}

ff(&s) {
	fn=Cf.funcNode('parent')
	a=args(1)
	ss=''
	while(s.valid()) {
		left=s.findPos('{')
		ss.add(left)
		not(s.valid()) break;
		k=s.findPos('}').trim()
		if(typeof(k,'num')) ss.add(a.get(k))
		else ss.add(fn.get(k))
	}
	return ss
} 
fv(&s, map) {
	rst=''
	fn=Cf.funcNode('parent')
	arr=args()
	while(s.valid()) {
		left=s.findPos('#{')
		rst.add(left)
		not(s.valid()) break;
		key=s.findPos('}').trim()
		if(key.find('(')) {
			val=eval(key)
		} else {
			val=fn.get(key)
			not(val) {				
				if(map) val=map.get(val)
			}
		}
		rst.add(val)
	}
	return rst;
}

format(&s, map) {
	rst=''
	fn=Cf.rootNode("@callFuncNode")
	not(typeof(fn,"func")) fn=Cf.funcNode('parent')
	arr=args()
	while(s.valid()) {
		left=s.findPos('#{')
		rst.add(left)
		not(s.valid()) break;
		key=s.findPos('}').trim()
		if(typeof(key,'num')) {
			val=arr.get(key+1)
		} else {
			val = ''
			if(typeof(map,'node')) {
				val = map.get(key)
			}
			not(val) val=fn.get(key)
		}
		rst.add(val)
	}
	return rst;
}
escape(s) {
	if(s.find('\r')) s=s.schema.replace('\r',"")
	if(s.find('\n')) s=s.replace('\n',"\r\n")
	if(s.find('\t')) s=s.replace('\t',"\t")
	if(s.find('\"')) s=s.replace('\"','"')
	return s;
}
dateFormat(tm) {
	if(typeof(tm,'string') ) {
		if(tm.size().eq(8)) {
			yy=tm.value(0,4), mm=tm.value(4,6), dd=tm.value(6);
			return "${yy}-${mm}-${dd}";
		}  
	}
	if( typeof(tm,'num') ) {
		today=System.date('yyyy-MM-dd');
		date=System.date(tm,'yyyy-MM-dd');
		if( today.eq(date) ) {
			return System.date(tm,'hh시 mm분');
		} else {
			return date;
		}
	} 
	return '';
}
getTm(tm) {
	today=System.date('yyyy-MM-dd');
	not(tm) return today;
	date=System.date(tm,'yyyy-MM-dd');
	if( today.eq(date) ) {
		return System.date(tm,'hh시 mm분');
	} else {
		return date;
	}
}

parseXml(node, &data ) {
	while( data.valid() ) {
		ch=data.ch();
		not( ch.eq('<') ) {
			break;
		}
		if( data.ch(1).eq('!') ) {
			data.match('<!--','-->');
			continue;
		} 
		sp=data.cur();
		ch=data.incr().next().ch();
		while(ch.eq("-")) {
			ch=data.incr().next().ch();
		}
		ep=data.cur();
		tag=data.trim(sp+1,ep);
		sub = node.addNode();
		
		in=data.find('>');
		if( in.ch(-1).eq('/') ) {
			prop=data.findPos('/>');
			parseProp( sub, tag, prop);
		} else {
			data.pos(sp);
			in=data.match("<$tag","</$tag>",8);
			not( in ) {
				print("xml parse 오류 (태그:$tag 매칭되는 태그를 찾을수 없습니다)");
				in=data.findPos("</$tag>");
			}
			prop=in.findPos(">");
			parseProp( sub, tag, prop);
			if( tag.eq('html', 'text') ) {
				sub.data=in;
			} else {
				if( in.ch().eq('<') ) {
					parseXml(sub, in);
				} else {
					sub.data=in;
				}
			}
		}
	}
	return node;
}
parseProp(node, tag, &prop) {
	node.tag=tag;
	while( prop.valid() ) {
		k=prop.findPos('=').trim();
		not(k) break;
		ch=prop.ch();
		if( ch.eq() ) {
			node.set(k, prop.match() );
		} else {
			val=prop.findPos(" \t\n",4).trim();
			node.set(k, val);
		}
	}
}


splitSep(&s, sep, arr) {
	not(sep) sep=',';
	if(arr) arr.reuse() else arr=[];
	while(s.valid()) {
		val=s.findPos(sep).trim();
		arr.add(val);
	}
	return arr;
}
priceComma( val ) {
	if( typeof(val,'string')) {
		won=val.trim();
	}else if( typeof(val,'number') ) {
		won="$val";
	}
	not( won.is('number') ) {
		return won;
	}
	pv=null;
	if(won.find('.')) {
		pv=right(won,'.');
		won=won.findPos('.').trim();
	}
	s='', sign='';
	ch=won.ch();
	if( ch.eq('-','+') ) {
		sign=ch;
		w=won.value(1);
	} else {
		w=won;
	}
	size=w.size();
	sp= size % 3;
	if( sp ) {
		s.add( w.value(0,sp) );
		size-=sp;
	}
	while( n=0, n<8, n++ ) {
		if( size<=0 ) break;
		if( sp ) s.add(',');
		ep=sp+3;
		s.add( w.value(sp,ep) );
		sp=ep;
		size-=3;
	}
	if(pv) s.add('.', pv);
	return "${sign}${s}";
}
getColorStr(&s) {
	if(s.find(',')) {
		s.split(',').inject(r,g,b);
		c=color(r,g,b);
		return "$c";
	}
	return s;
}
tagBody(&s, tag) {
	s.findPos("<$tag",0,1);
	if(s.valid()) {
		ss=s.match("<$tag","</$tag>");
		if(typeof(ss,'bool')) return print("tag body match error tag:$tag");
		ss.findPos(">");
		return ss;
	}
	return;
}
propVal(&prop, key) {
	while(prop.valid()) {
		prop.findPos(key);
		ch=prop.ch();
		not(ch) break;
		if(ch.eq('=',':') ) {
			ch=prop.incr().ch();
			if(ch.eq()) {
				val=prop.match().value();
			} else {
				val=prop.findPos(' ').trim();
			}
			return val;
		}
	}
	return;
}
isEmpty(&s) {
	not(s) return true;
	not(s.ch()) return true;
	return false;
}
lineEndCheck(&s, sep) { 
	not(sep) sep=',';
	left=s.findPos("\n");
	if(left.find(sep,1)) return false;
	return true;
} 
lineBlankCheck(&s ) { 
	not(typeof(s,'string')) return false;
	c=s.ch(0);
	if(c.eq("\r","\n")) return true;
	sp=s.indexOf("\n") 
	s.ch();
	ep=s.cur();
	if(sp!=-1) {
		if(sp<ep) return true;
	}
	return false;
} 
lineCheck(&s, val) {
	line=s.findPos("\n");
	if(line.find(val)) return true;
	return false;
}
indentCount(&s) {
	sp=s.cur() s.ch();
	cp=s.cur();
	if(cp<sp) return 0;
	return cp-sp;
}
leftVal(&s,sep) {
	not(sep) sep='.';
	return s.findPos(sep).trim();
}
rightVal(&str, sep ) {
	not(sep) sep='.';
	a=str.findLast(sep);
	if( a.valid() ) {
		return a.right().trim();
	} else {
		return a;
	}
}
indentText( &s ) {
	not( s ) return '';
	sp=s.cur();
	s.ch();
	ep=s.cur();
	return s.value(sp,ep,true);
}

splitLine(&s, sep) {
	ss="";
	n=0;
	while(s.valid()) {
		line=s.findPos("\n");
		not(line.ch()) continue;
		if(n && sep) ss.add(sep);
		ss.add(line.trim());
	}
	return ss;
}
split(&str, sep, arr) {		
	not(arr) arr=when(sep,_arr(),[]);
	not(sep) sep=",";
	while(str.valid()) {
		val=str.findPos(sep).trim();
		if(val) arr.add(val)
	}
	return arr;
}
splitComma(&s) {
	return s.split(",");
}
	
findNumberPos(&s) {
	while(s.valid() ) {
	pos=s.cur();
	val=s.move()
	if( val.is('num') ) return pos;
	}
	return s.cur();
}
lpad(val, num) {
	not( num ) num=4;
	if( typeof(val,'number') ) {
		strVal="$val";
	} else {
		strVal=val.trim();
	}
	sz=strVal.size();
	if( num.gt(sz) ) {
		rst='';
		dist=num-sz;
		while( n=0, n<dist, n++ ) rst.add('0')
		rst.add(strVal);
		return rst;
	} else {
		return strVal;
	}
}
strStripIndent(&s) {
	ln="\r\n"
	ss='', idx=0;
	endCheck = func(&s) { return when(s.ch(), false, true) }
	while(s.valid()) {
		left=s.findPos("\n")
		in=indentText(left)
		inSize=in.size()
		line=left.trim()
		not(line) continue;
		not(idx) {
			firstSize=inSize
		} else {
			ss.add(indent)
		}
		if(firstSize<inSize) {
			ss.add(in.value(firstSize),line)
		} else {
			ss.add(line)
		}
		if(endCheck(s)) break;
		ss.add(ln)
		idx++;
	}
	return ss;
}
strLineCount(&s) {
	cnt=0
	while(s.valid()) {
		s.findPos("\n")
		cnt++
		not(s.ch()) break;
	}
	return cnt;
}

toString(obj, detail, skip, level) {
	not( level ) level=0;
	arrNode=_arr();
	rst=''
	if( typeof(obj, 'node') ) {
		arrNode.add(obj);
		parseNode(obj, level, skip );
	} else if( typeof(obj, 'array') ) {
		parseArray(obj, level );
	} else if( typeof(obj, 'string') ) {
		parseString(obj);
	} else if( typeof(obj, 'number') ) {
		return obj.toString();
	} else if( typeof(obj, 'null') ) {
		return "null";
	} else if( typeof(obj, 'bool') ) {
		return when(obj, 'true', 'false' );
	} else {
		rst=typeof(obj);
		if( level ) {
			rst.append("\n\t$obj\n");
		}
	}
	return rst;	
	parseMember=func(&s) {
		rst='';
		while(s.valid()) {
			line=s.findPos("\n").trim();
			not(line) continue;
			rst.add("\n\t",line);
		}
		return rst;
	};
	parseString=func( &str ) {
		str.ch();
		if( str.find("\n") ) {
			rst.append( str.findPos("\n").trim() );
		} else
		rst.append( str.trim() );
	};
	parseNode=func(node, depth, skip) {
		rst.append("{");
		cnt=node.childCount();	
		num=depth+1;
		fn=Cf.funcNode(node);
		if(fn) {
			rst.add("member [ ", parseMember(fn.get()), " ]\n");		
		}
		arr=node.keys(true).sort();
		while( key, arr ) {
			if( key.eq('@this', '@c', '@inlineNode', '@widget', '@timers', '@keys', '@,') ) continue;
			while(n=0, n<num, n++ ) rst.append(when(n.eq(0),"\n"), "\t");
			val=node[$key]
			if( typeof( val, 'num') ) {
				rst.append("$key: $val");
			} else if( typeof(val, 'bool') ) {
				rst.append("$key: ", when(val,'true','false') );
			} else if( typeof(val, 'pair') ) {
				rst.append("$key: $val");
			} else if( typeof(val, 'null') ) {
				rst.append("$key: null");
			} else if( typeof(val, 'string') ) {
				rst.append("$key: ");
				parseString(val);
			} else if( typeof(val, 'widget') ) {
				rst.append("$key: ", typeof(val) );
			} else if( typeof(val, 'node') ) {
				find=null;
				while( cur, arrNode ) {
					if( cur==val ) {
						find=cur;
						break;
					}
				}
				if( find ) {
						rst.append("$key: ", typeof(val) );
				} else {
					if( num<3 ) {
						arrNode.add(val);
						rst.append("$key: " );
						parseNode(val, num);
					} else {
						rst.append("$key: ", typeof(val) );
					}
				}
			} else if( typeof(val, 'array') ) {
				rst.append("$key: " );
				parseArray(val, num);
			} else {
				rst.append("$key: $val");
			}
		} 
		chk=cnt && ~(skip);
		if( detail && chk ) {
			while(n=0, n<num, n++ ) rst.append(when(n.eq(0),"\n"), "\t");
			num++;
			rst.append("children [ ");
			if( num<5 ) {
				while( sub, node, n, 0 ) {
					if( n ) rst.append(', ');
					rst.append( toString(sub, true, false, depth+1) );
				}
			} else {
				while( sub, node ) {
					while(n=0, n<num, n++ ) rst.append(when(n.eq(0),"\n"), "\t");
					rst.append("$sub");
				}
			}
			num--;
			while(n=0, n<num, n++ ) rst.append(when(n.eq(0),"\n"), "\t");
			rst.append("]");
		}
		if( depth.eq(0) ) {
			if( type ) {
				rst.append("\n");
			} else {
				rst.append("\n}" );
			}
		} else {
			while(n=0, n<depth, n++ ) rst.append(when(n.eq(0),"\n"), "\t");
			rst.append("}");
		}
	};
	parseArray=func(arr, depth) {
		rst.append("[");
		num=depth+1;
		chk=true;
		while( val, arr, idx, 0 ) {
			if( idx ) rst.append(",");
			if( chk ) {
				while(n=0, n<num, n++ ) rst.append(when(n.eq(0),"\n"), "\t");
			}
			if( typeof( val, 'num') ) {
				rst.append(val);
			} else if( typeof(val, 'bool') ) {
				rst.append(when(val,'true','false') );
			} else if( typeof(val, 'pair') ) {
				rst.append("$val");
			} else if( typeof(val, 'string') ) {
				parseString(val);
			} else if( typeof(val, 'widget') ) {
				rst.append(typeof(val) );
			} else if( typeof(val, 'node') ) {
				find=null;
				while( cur, arrNode ) {
					if( cur==val ) {
						find=cur;
						break;
					}
				}
				if( find ) {
						rst.append(typeof(val) );
				} else {
					if( num.lt(3) ) {
						arrNode.add(val);
						parseNode(val, num)
					} else {
						rst.append( typeof(val) );
					}
				}
			} else if( typeof(val, 'array') ) {
				parseArray(val, num);
			} else {
				rst.append(val);
			}
		}
		if( depth.eq(0) ) {
			rst.append("\n]" );
		} else {
			while(n=0, n<depth, n++ ) rst.append(when(n.eq(0),"\n"), "\t");
			rst.append("]");
		}
	};
}

typeVal(&s) {
	return when(typeof(s,'string'), s.typeValue(), s);
}	
jsValue(src) {
	if( typeof(src,"string")) {
		if( typeof(src,"num") || src.eq("true","false","null")) return src;
		return Cf.jsValue(src);
	}
	if( typeof(src,'number') ) {
		return src;
	}
	if( typeof(src,'bool') ) {
		return when(src, "true", "false");
	}
	if( typeof(src,'null') ) {
		return "null";
	}
	not( src ) return '""';

	return Cf.jsValue(src);
}

jsStringValue(&src, single) {
	not(typeof(src,'string')) {
		if(typeof(src,'bool'))return when(src, "true", "false");
		if(typeof(src,'function'))return 'function';
		if(typeof(src,'null'))return 'null';
		if(typeof(src,'num')) return src;
		return "$src";
	}
	not(src) return '""';
	c=src.ch();
	not( c ) {
		return '""';
	}
	if( c.eq() ) {
		return src;
	}
	rst='';
	rst.append('"');
	while( src.valid(), n, 0 ) {
		line=src.findPos("\n");
		ss=line;
		not( ss.ch() ) continue;
		if( line.find("\\") ) {
			ss='';
			while( line.valid() ) {
				left=line.findPos("\\", 0);
				ss.add(left);
				not( line.valid() ) break;
				ss.add('\\')
			}
			line=ss;
			print("-------xxx line=>$line");
		}
		if( line.find('"') ) {
			line=parseLine(line);
		}
		if( n ) rst.append('\n');
		if( line.find("\t") ) {
			str=line.replace("\t", '\t');
			rst.append( str.trim('right') );
		} else {
			rst.append( line.trim('right') );
		}
	}
	rst.add('"');
	return rst;

	parseLine=func(&src) {
		rst='';
		while( src.valid() ) {
			left=src.findPos('"');
			rst.append(left) not( src.valid() ) break;
			rst.append('\"');
		}
		return rst;
	};
}	
</script>