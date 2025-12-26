class editor {	
	initClass() {
		@editor=this;
	}
	setKeyMap(targetId) {
		not(targetId) targetId='default'
		map=object("map.editorKey");
		node=map.addNode(targetId);
		not(node.childCount()) {
			db=Baro.db("config");
			db.fetchAll("select key, data, note from keymap_info where targetId='${targetId}'", node);
			while(sub, node) {
				sub.inject(key,data);
				node.set(key,data,note);
				if(note) node.set('#$key',note);
			}
		}
		editor.var(keyMap, node);
	}
	onKeyDown(key, mode) {
		if( typeof(this.keydownCallback,'func') ) {
			rst=this.keydownCallback(key,mode);
			if( typeof(rst,'bool') && rst ) return true;
		}
		if( key.eq(KEY.Tab) ) {
			if( this.is('select') ) return;
			if( this.tabPositionsMove() ) return true;
			return this.editorSourceKeyMap(true);
		}
		if( key.eq(KEY.Return, KEY.Enter) ) {
			return this.editorKeyEnter(mode);
		}
		switch( key ) {
		case KEY.F3:		return this.editorSearch(mode);
		case KEY.QuoteDbl:	return this.editorKeyString('"');
		case KEY.Apostrophe:	return this.editorKeyString("'");
		case KEY.ParenRight:	return this.startBlockMark("(");
		case 123:	return this.braketStart();
		/*
		case 125:	return this.braketEnd();
		*/
		default:
		}
		
		if( mode&KEY.ctrl ) {
			if(key.eq(KEY.D) ) {
				return this.editorCopyLine(mode&KEY.shift);  
			} 
			if(key.eq(KEY.B) ) {
				if( editor.is('select') ) {
					editorBindString(editor, mode&KEY.shift)
					return true;
				} else {
					if( editorFunctionSelect(editor) ) return true;
					if( editorBlaketMove(editor, mode&KEY.shift) ) return true;
					if( editorTagSelect(editor, mode&KEY.shift) ) return true;
				}
			}
			if(key.eq(KEY.J) ) {
				if( editor.is('select') ) {
					node=_node('JsonParse').removeAll(true)
					node.parseJson(editor.text('select') )
					str=do(node)
					this.insertIndent(str)
					if(mode&KEY.shift) {
						System.copyText(str)
					}
					return true;
				}
			}
			if(key.eq(KEY.E) ) {
				if( this.is('wrapUse')) {
					this.is('wrapUse',false)
				} else {
					this.is('wrapUse',true)
				}
				return true;
			} 
			if(key.eq(KEY.Up)) {
				return this.editorUpPress(editor);
			} 
			if(key.eq(KEY.Down)) {
				return this.editorDownPress(editor);
			} 			
		}
		this.prevKey=key;		
	}
	onCursorChange() {
		if( editor.is('select') || editor.is('wrapUse')) return;
		if( editor.var(changeCursorTick) ) {
			dist=System.tick() - editor.var(changeCursorTick);
			if(dist<500) return;	
		}
		editor.var(changeCursorTick, System.tick());
		editor.findAll();
		text=editor.text('word') 
		if(text.is('oper')) return;
		if(text.size()<3) return;
		editor.findAll(text);
	}
	onMouseWheel(pos, delta, mode) {
		not( mode&KEY.ctrl ) return;
		num=delta/80;
		this.zoomIn(num.toInt() );
		return true;
	}
	onContentChange(pos, add, remove) {
		tabArray=this.var(tapPositions) not( tabArray.size() ) return;
		if( add.eq(remove) ) return;
		
		if( add ) {
			n1=add-remove;
			if( n1.eq(1) ) {
				pos=this.pos()-1;
				add=1, remove=0;
			} else if( n1.lt(0) && add.gt(1) ) {
				pos=this.pos();
				remove-=add;
				add=0;
			}
		}

		if( remove ) {
			arr=_arr();
			end=this.pos('end');
			end+=remove;
			while(cur, tabArray, idx) {
				cur.inject(sp,ep);
				if( sp.gt(end) ) {
					arr.add(cur);
					break;
				}
				if( pos.ge(ep) ) {
					continue;
				}
				chk=pos.gt(sp) && pos.lt(ep);

				if( chk ) {
					size=ep-sp;
					if( remove.ge(size) ) {
						arr.add(cur);
					} else {
						ep-=remove;
					}
				} else {
					epos=pos+remove;
					if( epos.lt(sp) ) {
						sp-=remove, ep-=remove;
					} else if( epos.gt(ep) ) {
						arr.add(cur);
					} else {
						ep-=remove;
					}
				}
				tabArray.set( idx, Baro.point(sp, ep) );
			}
			while( cur, arr ) {
				tabArray.remove(cur);
			}
		}
		if( add ) {
			while(cur, tabArray, idx) {
				cur.inject(sp,ep)
				if( pos.gt(ep) ) continue;
				if( sp.eq(ep) ) {
					chk=sp.eq(pos);
				} else {
					chk=pos.ge(sp) && pos.le(ep);
				}
				if( chk ) {
					ep+=add;
				} else {
					sp+=add, ep+=add;
				}
				tabArray.set(idx, Baro.point(sp,ep) );
			}
		}
	}
	
	tabPositionsMove( back ) {
		tabArray=this.var(tapPositions) not(tabArray&&tabArray.size()) return false;
		_finish=func() {
			this.tabPositionsIndex=0;
			tabArray.reuse();
			return true;
		}
		pos=this.pos();
		idx=this[tabPositionsIndex++];
		end=this.pos('end');
		pt=tabArray.get(idx);
		print("#### pt ####", pt, idx );
		not( pt ) {
			return _finish();
		}
		pt.inject(sp,ep);
		if( sp>=end ) {
			if( sp.eq(end) ) {
				this.movePos(sp);
			}
			return _finish();
		}
		size=ep-sp;
		if( size>1 ) {
			this.select(sp,ep);
		} else {
			this.movePos(sp);
		}
		return true;
	}
	isSyntaxString() {
		key=this.prevKey;
		if( key && key.eq(16777219, 16777223) ) {
			return true;
		}
		sty=this.syntaxAt();
		if( sty ) {
			clr=right(sty,',');
			if( clr.eq('#FF00FF') ) return true;
		}
		return false;
	}
	editorSourceKeyMap(tab) { 
		if(this.is("select")) return false;
		map=this.var(keyMap) not(map ) return false;
		e=this
		start=e.pos()
		c=e.text('prevWord', 'prevChar', true)
		while(c.eq('-','.')) {
			c=e.text('prevWord', 'prevChar', true)
		}
		sp=e.pos()
		k=e.text(sp, start).trim()
		str=map.get(k)
		not( str ) {
			e.pos(start)
			return false;
		}
		sz=k.size()
		e.select(start-sz, start);
		if(str.find("\n")) {
			line=e.text("lineStart");
			indent=indentText(line);
			e.insertIndent(str, indent);
		} else {
			e.insert(str, true);
		}
		return true
	}
	public insertIndent( &str, indent, pos ) {
		if( pos ) this.movePos(pos);
		not( indent ) {
			line=this.text('lineStart')
			indent=indentText(line);
		}
		rst='';
		while( str.valid(), num ) {
			left=str.findPos("\n");
			if(num) rst.add("\n", indent);
			rst.add(left);
		}
		this.insert( rst, true);
		return true;
	}
	public editorKeyEnter() {
		pos=this.pos();
		if( pos==this.pos('lineStart') ) {
			return false;
		}
		line=this.text('lineStart');
		indent=indentText(line);
		line.ref();
		size=line.length();
		ch=line.prevChar();
		size-=line.length();
		if( ch.eq("{", ":", "[") ) {
			val=line.trim();
			c=this.text('nextChar');
			if( ch.eq('{') && c.eq('}') ) {
				this.insert("\n$indent\t^|\n$indent", true);
				return true;
			}
			if( ch.eq('[') && c.eq(']') ) {
				this.insert("\n$indent\t^|\n$indent", true);
				return true;
			}
			not( val.start('switch') ) {
				indent.add("\t");
			}
		}
		/* 커서 앞에 공백제거 */
		size-=1;
		if( size>0 ) {
			sp=pos-size;
			this.movePos(sp,true);
			this.insert("");
		}
		/* 커서뒤 공백제거 */
		remain=this.sp().spText('lineEnd');
		if( remain ) {
			blank=indentText(remain);
			pos=this.pos() + blank.size();
			this.movePos(pos, true);
		}
		this.insert("\n$indent");
		return true;
	}
	 
	public editorSearch(mode) {	
		if( mode&KEY.ctrl ) {
			str = this.text('select');
			not( str ) {
				str=this.text('word');			
			}
			if(str) {
				global("prevSearchValue",str);
			} else {
				str=global("prevSearchValue");
			}
		} else {
			str=global("prevSearchValue");
		}
		not( str ) {
			System.beep();
			return;
		}
		this.var(prevSearchValue, str);
		if( mode&KEY.shift ) {
			this.searchPrev(str);
		} else {
			this.searchNext(str);
		}
		return true;
	}
	editorKeyString( c ) {
		if( this.is('select') ) return false;
		ch = this.text('nextChar');
		if( ch.eq(c) ) {
			cur=this.pos();
			this.pos(cur+1);
			return true;
		}
		str=this.text('prevWord');
		ch=str.prevChar();
		if( ch.eq('=',',','(') ) {
			this.insert("$c^|$c", true);
			return true;
		}
		return false;
	} 
	braketStart() {
		if( this.isSyntaxString()) return false;
		str=this.text('prevWord');
		ch=str.prevChar();
		if( str.start('else') || ch.eq(')') ) {
			line=this.text('lineEnd').trim()
			if(line) return;
			line=this.text('lineStart').trim()
			if( line.start('switch') ) {
				this.insertIndent("{\ncase ^|:\ndefault:\n}");
			} else {
				this.insertIndent("{\n\t^|\n}");
			}
			return true;
		}
		return false;
	}
	braketEnd() {
		if( this.isSyntaxString()) return false;
		str=this.text('start');
		str.add("}");
		indent = this.editorStartIndent( str );
		if( indent ) {
			this.insert(indent);
		}
		return false;
	}
	public startBlockMark(ch) {
		flag = 0x100 | 1;
		if( ch.eq('(')) {
			start="(", end=")";
		} else {
			start="[", end="]";
		}
		str=this.sp().spText(-128);
		str.add(end);
		str.ref();
		ep=str.cur(-1);
		in=str.match(start,end,flag);
		if(typeof(in,'bool')) return false;
		if(in.size() ) {
			sp=str.cur(-1);
			sp+= 1;
			len=str.pos(sp,ep).length();
			if(len>2) {
				pos = this.pos() - len;
				this.insert(end);
				this.mark( pos, pos+1 );
				return true;
			}
		}
		return false;
	}
	public editorStartIndent( &str) {
		flag = 0x100 | 1;
		in = str.match('{','}',flag);
		if(typeof(in,'bool')) return false;
		not( in.find("\n") ) return false;	// 매칭문자열이 같은줄이라면 무시한다
		indent = str.findLast("\n").right();
		// #2  에디터 현재 위치 내용에 따른 처리
		line = this.sp().spText('lineStart');
		val = '';
		if( line.check(" \t") ) {
			pos = this.pos();
			start = pos-line.size();
			this.movePos(start).movePos(pos,true);
		} else { 
			val.add("\n");
		}
		// #3. indent 를 넣어준다.
		if( indent ) {
			val.add( indentText(indent) );
		} else {
			val.add( indentText(str) );
		}
		return val;
	}
	editorCopyLine( mode) {
		e=this
		sp=0, ep=0;
		if( e.is('select') ) {
			ss=e.pos('selectStart'), se=e.pos('selectEnd')
			sp=e.pos(ss,'lineStart') 
			ep=e.pos(se,'lineStart')
			if(sp.eq(ep) || se.ne(ep) ) {
				ep=e.pos(se,'down','lineStart')
			}
		} else {
			sp=e.pos('lineStart'), ep=e.pos('down', 'lineStart');
		}
		str=e.text(sp,ep)
		e.movePos(ep)
		e.insert(str)
		pos=e.pos()
		if(mode) {
			e.select(ep,pos)
		} else {
			e.movePos(pos-1)
		}
		return true;
	}
	public editorUpPress() {
		sp=0,ep=0;
		ok=false;
		if( this.is('select') ) {
			sp=this.pos('selectStart', 'lineStart'), ep=this.pos('selectEnd','lineEnd');
			if( this.pos('selectEnd')==this.pos('selectEnd','lineStart') ) {
				ep=this.pos('selectEnd')-1;
			} else {
				ep=this.pos('selectEnd','lineEnd');
			}
			str=this.movePos(sp).movePos(ep,true).text('select');
			if( str.find("\n") ) {
				ok=true;
			}
		}
		not( ok ) {
			sp=this.pos('lineStart'), ep=this.pos('lineEnd');
			str=this.movePos(sp).movePos(ep,true).text('select');
		}
		not(sp,ep) return;
		ep+=1;
		end=this.pos('end');
		if( ep.gt(end) ) ep=end;
		this.movePos(sp).movePos(ep, true);
		this.insert('');
		usp=this.pos('up','lineStart'), uep=this.pos('up','lineEnd');
		upLine=this.movePos(usp).movePos(uep,true).text('select');
		indent=indentText( upLine );
		len=str.length();
		sp=usp, ep=usp+len;
		this.insert("$str\n$upLine");
		this.movePos(sp).movePos(ep,true);
		return true;
	}
	public editorDownPress() {
		sp=0, ep=0;
		ok=false;
		if( this.is('select') ) {
			sp=this.pos('selectStart', 'lineStart'), ep=this.pos('selectEnd','lineEnd');
			if( this.pos('selectEnd')==this.pos('selectEnd','lineStart') ) {
				ep=this.pos('selectEnd') - 1;
			} else {
				ep=this.pos('selectEnd','lineEnd');
			}
			str=this.movePos(sp).movePos(ep,true).text('select');
			if( str.find("\n") ) {
				ok=true;
			}
		}
		not( ok ) {
			sp=this.pos('lineStart'), ep=this.pos('lineEnd');
			str=this.movePos(sp).movePos(ep,true).text('select');
		}
		not(sp, ep) return;
		end=this.pos('end');
		dsp=this.pos('down','lineStart'), dep=this.pos('down','lineEnd');
		if( dep >= end ) return;
		if( dsp < dep ) {
			downLine=this.movePos(dsp).movePos(dep,true).text("select");
			len=str.length(), dlen=downLine.length();
			dep+=1;
			this.movePos(sp).movePos(dep,true);
			this.insert("$downLine\n$str\n");
			dlen+=1;
			sp+=dlen, ep=sp+len;
			this.movePos(sp).movePos(ep,true);
		}
		return true;
	}
	setEditorSyntax(type) {
		node=Cf.getObject("editor","syntax:${type}");
		not(node) {
			node=Cf.getObject("editor","syntax:${type}",true);
			node.parseJson(conf("editor.syntax:${type}"));
		}
		editor.syntax(node);
	}
}
 
class editorSql {
	initClass() {
		not(isClass('editor')) class(this,'editor')
		this.setEditorSyntax('sql')
		this.setKeyMap('template')
	}
	getQueryBlock() {
		pos=editor.pos()
		val=editor.text(pos-1, pos)
		if( val.eq(';')) {
			editor.pos(pos-1)
		}
		if( editor.searchPrev(';',0) ) {
			sp=editor.pos()
		} else {
			sp=0
		}
		editor.pos(sp+1)
		if( editor.searchNext(';',0) ) {
			ep=editor.pos() - 1;
		} else {
			ep=editor.pos('end')
		}
		arr=_arr()
		arr.add(sp,ep)
		return arr;
	}
	insertQuery(&s) {
		ln="\r\n"
		line=editor.text('lineStart')
		c=line.ch()
		indent=indentText(line)
		// s.findPos("\n")
		ss='', firstSize=0, idx=0
		if(c) {
			editor.movePos('lineEnd')
			ss.add(ln,indent)
		}
		_endCheck = func(&s) { return when(s.ch(), false, true) }
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
			if(_endCheck(s)) break;
			ss.add(ln)
			idx++;
		}
		editor.insert("${ss};\r\n", true)
	}
}

class func {
	@editor.insertKeymap(&s) {
		map=object("map.editorKey").addNode('template')
		keyCode=''
		db=Baro.db('config')
		tm=System.localtime()
		while(s.valid() ) {
			left=s.findPos('##')
			if(keyCode) {
				map.set(keyCode, left)
				db.exec("insert into keymap_info (key,targetId,data,note,tm) values ('${keyCode}','template','${left}','','${tm}')")
			}
			not(s.ch()) break;
			keyCode=s.findPos("\n").trim()
			print("keyCode=$keyCode")
		}
	
	}
	editorError(e, sp, msg, popup) {
		line=e.lineNumber(sp)
		not(popup) return print("라인:$line 오류 $msg")
		this.alert("라인:$line 오류 $msg")
		return false
	}
	editorBlaketMove(e, mode) {
		if(e.is('select')) return false;		
		pos=e.pos()
		sp=pos
		c=e.text('nextChar')
		ok=c.eq('{','[')
		prevChar=false;
		not(ok) {
			c=e.text('prevChar')
			ok=c.eq('{','[')
			if(ok) {
				sp-=1;
				prevChar=true;
			}
		}
		if(ok) {			
			e._source_=e.text(sp,'end')
			src=e.ref(_source_)
			in=src.match(1)
			if(typeof(in,'bool')) return editorError(e, sp, "$c 매칭오류", true)
			matchSize = in.length()
			if(prevChar) {
				sp+=1;
				ep=sp+matchSize;
			} else {
				matchSize+=2;
				ep=sp+matchSize;
			}
			if(mode) {
				e.select(sp,ep)
			} else {
				e.movePos(ep)
			}
			return true;
		}
		c=e.text('prevChar')
		ok=c.eq('}',']') 
		not(ok) {
			c=e.text('nextChar')
			ok=c.eq('}',']')
			sp+=1;
			not(ok) return false;
		}
		e._source_=e.text(sp,'start')
		src=e.ref(_source_)
		if(c.eq('}')) sc='{' else sc='[';

		in=src.match(sc,c,0x101)
		if(typeof(in,'bool')) return editorError(e, sp, "$c 매칭오류", true)
		matchSize = in.length() + 2;
		ep=sp-matchSize;
		if(mode) {
			e.select(sp,ep)
		} else {
			e.movePos(ep)
		}
		return true;
	}
	editorFunctionSelect(e, mode) {
		if(e.is('select')) return false;
		pos=e.pos()
		s=e.text('nextWord')
		c=s.ch() 
		not(c.eq('(','=',':')) {
			c=e.text('nextWord','nextChar')
			if(c.eq('(','=',':')) e.pos('nextWord', true)
		} 
		not(c.eq('(','=',':')) return false;
		sp=e.pos('prevWord', true)
		e._source_=e.text('end')
		src=e.ref(_source_)
		ps=0
		c=src.next().ch()
		select = func(sp,ep) {
			if(mode) {
				sp=e.pos(sp,'lineStart')
				ep=e.pos(ep,'down','lineStart')
			}
			e.select(sp,ep)
		};
		if(c.eq('=',':')) {
			c=src.incr().ch()
			if(c.eq()) {
				// ex) aaa='xxx'  xxx 선택
				ps=src.cur()
				not(mode) ps+=1;
				line=src.value(0, ps, true)
				sp+=line.length();
				val=src.match()
				ep=sp+val.length();
				if(mode) ep+=1;
				e.select(sp, ep)
				return true;
			}
			if(c.eq('[','{')) {
				// ex) aaa=[123] 
				in=src.match(1)
				if(typeof(in,'bool')) return editorError(e, sp, "함수 매칭오류", true)
				all=src.trim(ps, src.cur(), true)
				select(sp, sp+all.length())
				return true;
			}
			if(src.start('func',true)) {
				c=src.ch()
			} else if(src.start('function',true)) {
				c=src.ch()
			}
			not(c.eq('(')) return false
		} 
		if(c.eq('(')) {
			// ex) aaa=function () or aaa() {...} or aaa=()=>
			src.match()
			c=src.ch() 
			if( src.start('=>', true) ) {
				// ()=>{} or ()=> line
				c=src.ch()
				if(c.eq('{')) {
					in=src.match(1)
					if(typeof(in,'bool')) return editorError(e, sp, "함수 매칭오류", true)
					all=src.trim(ps, src.cur(), true)
					select(sp, sp+all.length())
					return true;
				} else {
					src.findPos("\n")
					line=src.trim(ps, src.cur(), true)
					ep=sp+line.length()
					select(sp, ep)
					return true;
				}
			}
			if(c.eq('{')) {
				in=src.match(1)
				if(typeof(in,'bool')) return editorError(e, sp, "함수 매칭오류", true)
				all=src.trim(ps, src.cur(), true)
				select(sp, sp+all.length())
				return true;
			}
		}
		e.mosePos(pos)
		return false;
	}
	
	editorTagSelect(e, move) {
		pos=e.pos()
		c=e.text('prevChar')
		not(c.eq('<','/')) {
			if(c.eq('-')) {
				e.pos('prevChar',true)
			}
			while(n=0,10) {
				c=e.text('prevWord','prevChar')
				if(c.eq('-')) {
					e.pos('prevWord','prevChar',true)
					continue;
				}
				e.pos('prevWord',true)
				break;
			}
		}
		ch=e.text('prevChar')
		if(ch.eq('<','/')) {
			sp=e.pos()
			c=e.text('nextWord','nextChar')
			while(n=0,10) {
				c=e.text('nextWord','nextChar')
				if(c.eq('-') ) {					
					e.pos('nextWord','nextChar',true)
					continue;
				}
				e.pos('nextWord',true)
				break;
			}
			ep=e.pos()
			tag=e.text(sp,ep).trim()
			tagLen=tag.length()
			totLen=tagLen*2;
			totLen+=4;
			if( ch.eq('<') ) {
				sp-=1;
				e.pos(sp,true)
				src=e.text('end')
				in=src.match("<$tag","</$tag>")
				totLen+=in.length()
				ep=sp+totLen
				if(move) {
					e.movePos(ep)
				} else {
					e.select(sp, ep)
				}
			} else {
				sp-=2;
				ep+=1;
				e.pos(ep,true)
				src=e.text('start')
				in=src.match("<$tag","</$tag>", 0x100)
				dist=tagLen+2;
				len=in.length() - dist;
				totLen+=len 
				sp=e.pos(ep-totLen,'lineStart',true)
				if(move) {
					e.movePos(sp)
				} else {
					e.select(sp, ep)
				}
			}
			return true;
		}
		e.pos(pos,true)
		return false;
	}
	editorBindString(e,mode) {
		not(e.is('select')) return false;
		s=e.text('select')
		s.ref()
		ss='';
		while(s.valid(), n) {
			left=s.findPos(',').trim()
			if(n) ss.add(', ')
			if(mode) {
				ss.add("#{$left}")
			} else {
				ss.add("'$left'")
			}
		}
		e.insert(ss)
	}
}


## html 추가방법
    QTextCursor cursor = te.textCursor();
    cursor.insertHtml("<a href='http://www.w3schools.com/'>Link!</a>oops<br>");
    cursor.insertHtml("<a href='http://www.w3schools.com/'>Link!</a>");
    cursor.insertHtml("something");
    cursor.insertHtml("<br>");
    cursor.insertHtml("<a href='http://www.w3schools.com/'>Link!</a>");
    cursor.insertText("something");
}



m=Class.model('CommandModel');
root=m.rootNode().addNode({tag:root, text:커멘드 데이터모델});


cur=root.addNode({tag:current, text: 현재명령 모듈});
cur.addNode({tag:run, text:실행모듈});
cur.addNode({tag:page, text:페이지});
cur.addNode({tag:class, text:클래스});
cur.addNode({tag:func, text:함수});
cur.addNode({tag:subfunc, text:서브함수});
cur.addNode({tag:parh, text:경로});
cur.addNode({tag:define, text:정의값});
cur.addNode({tag:conf, text:설정값});
cur.addNode({tag:job, text:작업});
cur=root.addNode({tag:runtime, text: 실시간 정보});
cur.addNode({tag:page, text:페이지});
cur.addNode({tag:class, text:클래스});
cur.addNode({tag:func, text:함수});
cur.addNode({tag:subfunc, text:서브함수});
cur=root.addNode({tag:history, text: 커멘드실행 이력});
cur=root.addNode({tag:save, text: 커멘드저장 내용});
cur=root.addNode({tag:test, text: 예제정보});




p=this.sourceNotePage();

p=this.iconSelectPage();


p=this.screenCapturePage();
p.flags('noTitle');
_class('current', p.c);
_class('call', 'screenChange');


p=this.screenCapturePage();
c=p.c;
_class('add', c, @s[

]);


		return _call('super', 'editorKeyDown', args());


_subfunc('src', '_command', '', true);

_subfunc('exec', '_command', '',  @s[
])

##> call
p=this.commandEditorPage();
p.open();
Cf[commandEditor]=p;


p=Cf[commandEditor]
e=p.c.editor
c=e.text('nextChar');
not( c.eq('(') ) {
	e.move('nextWord');
}
left=e.sp().spText('lineStart');
right=e.sp().spText('lineEnd');

left.str();
fnm=left.prevWord();
if( left.prevChar('.') ) {
	onm=left.prevWord();
}
print( left, right,  fnm, onm)


c=p.c
call(&_command('insertNewCommand'), c)

setSkipChange, insertCaptureImage, editorDrop, editorDrag, editorDragMove, addClip, editorDoubleClick


web download http://www.codeproject.com/KB/scripting/HowToWritePluginInJQuery/Thumb-291290.png
c.webDownloadCallback=callback(type, data, total ) {
	switch(type) {
	case progress:
		not( isset(totalSize) ) totalSize=0;
		downloadSize=data, totalSize=total;
	case finish:
		status='finish';
	case error: 
		status='error', error=data;
	}
}
System.openExplore('c:/temp', 'a.jpg, error.log')


setThisNodeFunction(fnCur, fn);
fnCur=setThisNodeAndParentFunction(fsrc, fn);

##> call
p=this.textEditorPage();
p.open()

_class('funcRun', @s[
drawImpl(d,rc) {
	d.rectLine(canvasRect, 0 , color, 4);
	if( this.isset(desktopCanvas) ) {
		d.drawImage(canvasRect, desktopCanvas);
	}
	d.fill('#d0d0d050');
	if( mouseDownPos && mouseMovePos ) {
		rcCapture=Class.rect(mouseDownPos, mouseMovePos);
		rcCapture.inject(x,y,w,h);
		d.drawImage( rcCapture, desktopCanvas, x,y,w,h);
		d.rectLine( rcCapture, 0, color.lightColor(80), 2, 'dash' );
	}
}
])
_class('call', 'test');


_subfunc('src', '_class', 'funcRun', true)


## gadget
w=Class.widget("tag:gadget");
w.show()
Cf[xx]=w;

## tooltip
void QTextControlPrivate::showToolTip(const QPoint &globalPos, const QPointF &pos, QWidget *contextWidget)
{
    const QString toolTip = q_func()->cursorForPosition(pos).charFormat().toolTip();
    if (toolTip.isEmpty())
        return;
    QToolTip::showText(globalPos, toolTip, contextWidget);
}


## resizeImage
 
void AdvancedTextEdit::resizeImage()
{

    QTextBlock currentBlock = m_textEdit->textCursor().block();
    QTextBlock::iterator it;

    for (it = currentBlock.begin(); !(it.atEnd()); ++it)
    {

             QTextFragment fragment = it.fragment();
             if (fragment.isValid()) {
                 if( fragment.charFormat().isImageFormat () ) {
                      QTextImageFormat newImageFormat = fragment.charFormat().toImageFormat();
                      QPair<double, double> size = ResizeImageDialog::getNewSize(this, newImageFormat.width(), newImageFormat.height());

                      newImageFormat.setWidth(size.first);
                      newImageFormat.setHeight(size.second);

                      if (newImageFormat.isValid())
                      {
                          //QMessageBox::about(this, "Fragment", fragment.text());
                          //newImageFormat.setName(":/icons/text_bold.png");
                          QTextCursor helper = m_textEdit->textCursor();

                          helper.setPosition(fragment.position());
                          helper.setPosition(fragment.position() + fragment.length(),
                                              QTextCursor::KeepAnchor);
                          helper.setCharFormat(newImageFormat);
                      }
                  }
              }
       }
}

## desktop capture
     originalPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
     updateScreenshotLabel();

     newScreenshotButton->setDisabled(false);
     if (hideThisWindowCheckBox->isChecked())
         show();

setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
setParent(0); // Create TopLevel-Widget
setAttribute(Qt::WA_NoSystemBackground, true);
setAttribute(Qt::WA_TranslucentBackground, true);  
setAttribute(Qt::WA_PaintOnScreen); // not needed in Qt 5.2 and up



##> layout
<page>
	<textedit id="e">
	<hbox>
		<button id="ok" text="ok"><space>
	</hbox>
</page>
onInit() {
	e=this.e;
	e.syntax(conf('syntax.html') );
}
e.onDraw() {
	@draw.fill('#c0c0c0');
}
e.onCursorChange() {
	fmt=@me.fragment().format();
	print("fragment: $fmt");
}
ok.onClick() {
	e.fragmentInsert('test', func(cursor, data) {
		while( n, 10 ) {
			color=randomColor();
			c=color.toString();
			switch(n) {
			case 0: cursor.insert("color\n", "style name=test$n color=$c");
			case 1: cursor.insert("bold\n", "style name=test$n weight=bold");
			case 2: cursor.insert("link\n", "link name=test$n href=http://www.daum.net");
			case 3: cursor.insert("background\n", "style name=test$n background=$c");
			case 4: cursor.insert("tip\n", "style name=test$n tip=xxxxx");
			case 5: cursor.insert("size\t", "style name=test$n size=20");
			case 6: cursor.insert("underline\t", "style name=test$n underline=wave underlineColor=$c");
			case 7: cursor.insert("italic\t", "style name=test$n ltalic=true");
			case 8: cursor.insert("overline ", "style name=test$n overline=true");
			case 9: cursor.insert("stricke ", "style name=test$n stricke=true");
			}
		}
	});
}

	@me.blocks(func(block) {
		while(true) {
			block.fragments( func(fragment) {
				while( true) {
					print("x", fragment.text() );
					not( fragment.next() ) break;
				}
			});
			not( block.next() ) break;
		}
	});


## svg format
     QTextCharFormat svgCharFormat;
     svgCharFormat.setObjectType(SvgTextFormat);
     QSvgRenderer renderer(svgData);

     QImage svgBufferImage(renderer.defaultSize(), QImage::Format_ARGB32);
     QPainter painter(&svgBufferImage);
     renderer.render(&painter, svgBufferImage.rect());

     svgCharFormat.setProperty(SvgData, svgBufferImage);

     QTextCursor cursor = textEdit->textCursor();
     cursor.insertText(QString(QChar::ObjectReplacementCharacter), svgCharFormat);
     textEdit->setTextCursor(cursor);

## list format
void TextEdit::textStyle(int styleIndex) {
    QTextCursor cursor = textEdit->textCursor();
    if (styleIndex != 0) {
        QTextListFormat::Style style = QTextListFormat::ListDisc;
        switch (styleIndex) {
            default:
            case 1:
                style = QTextListFormat::ListDisc;
                break;
            case 2:
                style = QTextListFormat::ListCircle;
                break;
            case 3:
                style = QTextListFormat::ListSquare;
                break;
            case 4:
                style = QTextListFormat::ListDecimal;
                break;
            case 5:
                style = QTextListFormat::ListLowerAlpha;
                break;
            case 6:
                style = QTextListFormat::ListUpperAlpha;
                break;
            case 7:
                style = QTextListFormat::ListLowerRoman;
                break;
            case 8:
                style = QTextListFormat::ListUpperRoman;
                break;
        }
        cursor.beginEditBlock();
        QTextBlockFormat blockFmt = cursor.blockFormat();
        QTextListFormat listFmt;
        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }
        listFmt.setStyle(style);
        cursor.createList(listFmt);
        cursor.endEditBlock();
    } else {
        // ####
        QTextBlockFormat bfmt;
        bfmt.setObjectIndex(-1);
        cursor.mergeBlockFormat(bfmt);
    }
}

void TextEdit::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textEdit->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    textEdit->mergeCurrentCharFormat(format);
}
void TextEdit::textAlign(QAction *a)
{
    if (a == actionAlignLeft)
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if (a == actionAlignCenter)
        textEdit->setAlignment(Qt::AlignHCenter);
    else if (a == actionAlignRight)
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if (a == actionAlignJustify)
        textEdit->setAlignment(Qt::AlignJustify);
}



##> layout
<page>
	<editor id="e">
	<hbox>
		<button id="ok" text="ok"><space>
	</hbox>
</page>
onInit() {
	e=this.e;
}
e.onDraw() {
	@draw.fill('#c0c0c0');
}
e.onCursorChange() {
	@me.blocks(func(block) {
		while(true) {
			print("xxx", block.text() );
			not( block.next() ) break;
		}
	});
}



a=Cf.map('ssrc');
while( c,  a ) print("c-$c")

Cf.debug(true, true)
p=this.sourceNotePage();
c=p.c;
c.addEditor('aaa1')


treeMouseUp(pos, mode, button) {
	if( this.var('movePage#tick') ) {
		dist=System.tick() - this.var('movePage#tick');
		if( dist<500 ) {
			this.var(contextUse, true);
		} else {
			this.var(contextUse, false);
		}
		 _mouseTick( TICK.up, 'movePage', pos);
	}
}

treeMouseDown(pos,mode,button) {
	if( button.eq('right') ) {
		print('mouseDown', pos);
		_mouseTick( TICK.down, 'movePage', pos );
	}
}
treeMouseMove(pos) {
	if( _mouseTick( TICK.move, 'movePage', pos) ) {
		return 'ignore';
	} 
	offset=this.var(layout.tree).lt();
	pos.incr(offset);
	if( rectFormStatus.contains(pos) ) { 
		formStatus.move(rectFormStatus);
		formStatus.show();
	} else if( formStatus.is('visible') ) {
		formStatus.hide();
	}
}


treeDragStart(nodes) {
	print('dragStart',nodes);
}
treeDrag(type,data) {
	if( data.hasType('application/nodeData') ) {
		arr=data.get();
		print( 'treeDrag', type, arr);
	}
	return 'accept';
}
treeDragMove(pos, data) {
	node=tree.at(pos);
	if( node ) {
		if( node!=dragOverNode ) {
			this.var(dragOverNode, node);
			tree.update();
		}
	} else if(dragOverNode) {
		this.var(dragOverNode, null);
		tree.update();
	}
	return 'copy';
}
treeDrop(pos, data) {
	this.var(dragOverNode, null);
	tree.update();
}



##> layout
<page>
	<editor id="e">
	<hbox>
		<button id="add" text="add">
		<button id="save" text="save">
		<space>
	</hbox>
</page>
onInit() {
	e=this.e;
}
add.onClick() {
	e.insertHtml("<a href='http://www.daum.net'>test</a>");
	e.insertImage('vicon.add_default');
	
}
save.onClick() {
	str=e.linkText();
	e.append(str);
}



db.exec("drop table note_tree")
db.exec("CREATE TABLE note_tree (
	idx integer not null,
	pidx integer not null,
	tag varchar(64),
	value text ,
	node_dest text ,
	note_src text,
	note_text text,
	note_type char(1) DEFAULT 'A',
	note_status char(1) DEFAULT '0',
	depth integer DEFAULT 1,
	sort integer  DEFAULT 0,
	ref varchar(64) ,
	ref1 varchar(64) ,
	ref2 varchar(64) ,
	ref3 varchar(64) ,
	ref4 varchar(64) ,
	use_yn char(1) DEFAULT 'Y' ,
	tm integer DEFAULT 0,
	reg_dt datetime DEFAULT (datetime('now','localtime')),
	CONSTRAINT PK_note_tree PRIMARY KEY (idx)  
);
")

##> page
pageFuncTest() {
	&page=this.widget({
		layout: 	<page>
			<editor id="e">
			<hbox>
				<button id="ok" text="ok"><space>
			</hbox>
		</page>
		onInit() {
			editor=this.e;
			this.ok.eventMap(onClick, eventMap.okClick);
		}
		e.onMouseMove() {
			a=@me.anchorAt(@pos);
			if( a ) {
				@me.setCursor(CURSOR.PointingHandCursor);
				if( this.linkOver ) return;
				idx=strRight(a, '_');
				arr=_arr(this, 'localSrcArray');
				src=arr.get(idx).str();
				src.ch();
				line=src.findPos("\n").trim();
				print( "line===========$line");
				editor.tooltip(line);
				this.linkOver=true;
			} else if( this.linkOver ) {
				this.linkOver=false;
				@me.setCursor(CURSOR.IBeamCursor);
			}
		}
		e.onMouseDown() {
			a=@me.anchorAt(@pos);
			if( a ) {
				print(1, a);
				this.var(linkText, a);
			}
		}
		e.onMouseUp() {
			a=@me.anchorAt(@pos);
			if( a && a.eq(linkText) ) {
				this.alert( linkText);
			}
		}
	});
	return page;
}
	 

##> eventMap
okClick() {
	arr=_arr(this, 'localSrcArray');
	_make('localFuncLink', editor, '_make', arr.reuse() );
}

##> call
p=this.pageFuncTest();
call( p, @s[
	e.onMouseMove() {

	} 
])



@@ canvas 이용한 함수링크
##> page
pageFuncLink() {
	&page=this.widget({
		layout: <page>
			<canvas id="c">
		</page>
		onInit() {
			this.size(800,600);
			classLoad(this, 'CPage');
		}
	});
	return page;
}

##> conf
src/pageFuncLinkImpl={
	Start {
		editor<editor>
		grid<CGrid name=funcList impl=[]>
	} Conf {
		vbox/grid,splitter,editor/300px,8px,*
	} Draw(d) {
		d.fill('#c0c0c0');
	} Timeout {
		
	}
}

src/pageFuncLinkEvent={
	editor.mouseMove(pos) { 
		a=editor.anchorAt(pos);
		if( a ) {
			if( a.eq(this.linkOver) ) return;
			this.linkOver=a;
			idx=strRight(a,'_');
			arrSrc=_arr(this, 'staticSrcArray');
			src=arrSrc.get(idx);
			editor.tooltip(src);
			editor.setCursor(CURSOR.PointingHandCursor);
		} else if( this.linkOver ) {
			editor.setCursor(CURSOR.IBeamCursor);
			this.linkOver=null;
		}		
	} 
	editor.mouseDown(pos, mode, button) {
		a=editor.anchorAt(pos);
		if( a ) {
			print(1, a);
			this.var(linkText, a);
		}
	}
	editor.mouseUp(pos, mode, button) {
		a=editor.anchorAt(pos);
		if( a && a.eq(linkText) ) {
			page.alert( linkText);
		}
	}
}