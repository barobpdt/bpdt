/* 
공통 객체처리 함수
*/
<script>
	setCallback(obj, fc, target) {
		if(typeof(obj,'func')) {
			if(typeof(fc,'node')) {
				temp = obj
				obj = fc
				fc = temp
			} else {
				fc = obj
				obj = this
			}
		}
		not(obj) obj = this
		not(typeof(obj,'node')) return print('setCallback 객체 오류', obj, fc) 
		not(typeof(target,'node')) {
			target = obj
		}
		arr = null
		fn = obj.get('@callback')
		if( typeof(fn,'func')) {
			arr=fn.eventFuncList()
		}
		fcType=typeof(fc)
		if( fcType.eq('bool') ) {
			if( fcType && arr ) {
				print("@@ setEvent $eventName 함수를 초기화 했습니다")
				obj.set('@callback', null)
				arr.reuse()
			}
			return arr;
		}
		not( fcType.eq('funcRef') ) {		
			if(fc) print("setCallback 함수타입 오류 (타입:$fcType)")
			return;
		}
		not( typeof(arr,'array') ) {
			fn=Cf.funcNode(@event.eventChildFunc, target)
			obj.set('@callback', fn)
			if( obj!=target ) {
				fn.set('@this', target)
				fn.set("sender", obj)
			}
		}
		not(typeof(fn,'func')) {
			return print('setCallback 등록오류 ')
		}
		
		if(arr.find(fc)) {
			print("@@ setEvent ${target.tag} 이미 추가된 콜백함수")
		} else {
			fn.addFuncSrc(fc)
			print("@@ setEvent ${target.tag} 콜백함수 추가")
		}
		return fn;		
	}
	getFuncKeys(fn) {
		a=_arr()
		s=fn.get() s.ref()
		while(s.valid()) {
			line = s.findPos("\n")
			not(line.ch()) break
			key = line.findPos('=').trim()
			a.add(key)
		}
		return a;
	}
	getAsyncFunc(fc, param) {
		fn=Cf.funcNode()
		if(typeof(param,'node')) {
			param.set('@checkSend', true)
			fnParent = Cf.funcNode('parent')
			fn.setParent(fnParent)
			/*
			while(key, getFuncKeys(fnParent)) {
				fn.set(key, fnParent.get(key))
			}
			*/
		}		
		fn.setPersist()
		fn.addFuncSrc(fc)
		return fn;
	}
	callAsyncFunc(fn, data) {
		fn.callFuncParams(data)
		fn.callFuncSrc()
		arr=fn.get('@params') if(typeof(arr,'array')) arr.remove(true)
		fn.delete()
	}
	setEventClear(obj, eventName) {
		not(typeof(obj,'node')) return print('setEventClear 객체 오류', obj, eventName)
		fn = obj.get(eventName)
		ftype = typeof(fn)
		not(ftype.eq('func')) return print('setEventClear $eventName 이벤트함수 미정의', obj)
		arr = fn.eventFuncList()
		not(typeof(arr,'array')) return print('setEventClear $eventName 이벤트함수 미등록', obj)
		arr.reuse()
		return arr;
	}
	setEvent() {
		arr=null
		obj = null
		target = null
		switch(args().size()) {
		case 2: args(eventName, fc)
		case 3: args(obj,eventName,fc)
		case 4: args(obj,eventName,fc,target)
		}
		not(obj) obj = this
		not(typeof(obj,'node')) return print('setEvent 객체 오류', obj, fc) 
		if(typeof(target,'node')) {
			targetCheck = true
		} else {
			targetCheck = false
			target = obj
		}
		fn=obj.get(eventName)
		if( typeof(fn,'func')) {
			arr=fn.eventFuncList()
		}
		fcType=typeof(fc)
		if( fcType.eq('bool') ) {
			if( fcType && arr ) {
				print("@@ setEvent $eventName 함수를 초기화 했습니다")
				obj.set(eventName, null)
				arr.reuse()
			}
			return arr;
		}
		not( fcType.eq('func','funcRef') ) {		
			if(fc) print("setEvent 함수타입 오류 (타입:$fcType)")
			return;
		}
		if( typeof(arr,'array') ) {
			if( targetCheck ) {
				fn.set('@this', target)
			}
		} else {
			fn=Cf.funcNode(@event.eventChildFunc, target)
			obj.set(eventName, fn)
			if(obj!=target) {
				fn.set('@this', target)
				fn.set("sender",obj)
			}
		}
		not(typeof(fn,'func')) {
			return print('setEvent 등록오류 ', eventName)
		}
		
		if( arr && arr.find(fc)) {
			print("@@ setEvent ${obj.tag} ${eventName} 이미 추가된 함수")
		} else {
			print("@@ setEvent ${obj.tag} ${eventName} 이벤트함수 추가")
			fn.addFuncSrc(fc)
		}
		return fn;
	}
	
	@event.eventChildFunc() {
		fn=Cf.funcNode()
		if( fn.eventFuncList()) {
			fn.callFuncSrc()
		}
	}
	setEventFirst(target, eventName, fc) {
		fn = target.get(eventName)
		a=null
		if(typeof(fn,'func')) {
			a=fn.eventFuncList()
		}
		if(isValid(a)) {
			a.insert(0, fc)
		} else {
			setEvent(target, eventName, fc)
		}
	}
	arrayDeleteChild(arr) {
		not(typeof(arr,'array')) return;
		while(cur, arr) {
			if(typeof(cur,'node')) {
				cur.remove(true)
			}
			if(typeof(cur,'array')) {
				cur.remove(true)
			}
		}
		arr.reuse()
	}
</script>

/* 
공통 유틸리티함수
*/
<script>
	isNull(a) {
		if(typeof(a,'num')) return false;
		if(typeof(a,'null')) return true;
		not(a) return true;
		return false;
	}
	isValid(a) {
		if(typeof(a,'null')) return false;
		if(typeof(a,'num')) return true;
		not(a) return false;
		if(typeof(a,'array')) {
			if(a.size()==0 ) return false;
			return true;
		}
		if(typeof(a,'node')) {
			a=a.keys()
			if(a.size()==0 ) return false;
			return true;
		}
		return true;
	}
	recalc(total, info) {
		return _arr().recalc(total, info)
	}
	stripTag(&s) {
		not(s.find('<')) return s;
		result=''
		while(s.valid(), n) {
			ss=s.findPos('<')
			if(n) result.add(' ')
			result.add(ss.trim())
			not(s.ch()) break;
			s.findPos('>')
		}
		return result;
	}
	marginRect(rc, param) {
		not(typeof(param,'array')) {
			param=args(1)
		}
		switch(param.size()) {
		case 1:
			param.inject(margin)
			return rc.incr(margin);
		case 3:
			param.inject( xw, yh) 
			rc.inject(x,y,w,h)
			x+=xw, w-=xw;
			y+=yh, h-=yh;
			return rc(x,y,w,h);
		case 4:
			param.inject( cx, cw, yh) 
			rc.inject(x,y,w,h)
			x+=cx;
			w-=cw;
			y+=yh;
			h-=yh;
			return rc(x,y,w,h);
		case 5:
			param.inject( cx, cy, cw, ch) 
			rc.inject(x,y,w,h)
			x+=cx;
			y+=cy;
			w-=cw;
			h-=ch;
			return rc(x,y,w,h);
		default:
		}
		return rc;
	}
	confNote(grp, data, note) {
		tm=System.localtime()
		node=_node()
		node.with(grp, data, note, tm)
		db=Baro.db('config')
		if( db.count("select count(1) from conf_info where grp=#{grp} and note=#{note}",node) ) {
			db.exec("update conf_info set data=#{data} where grp=#{grp} and note=#{note}",node)
		} else {
			num=db.count("select count(1) from conf_info where grp=#{grp}", node) + 1
			node.cd=lpad(num,3)
			db.exec(#[
				insert into conf_info (grp, cd, data, note, tm) values (
					#{grp}, #{cd}, #{data}, #{note}, #{tm}
				)
			], node)
		}
	}
	
	checkMember(a) {
		fn=Cf.funcNode(this)
		not(fn) return false;
		return fn.isset(a);
	}
	toLong(s) {
		a=when(typeof(s,'number'),"$s",s)
		return a.toLong()
	}
	toDouble(s) {
		a=when(typeof(s,'number'),"$s",s)
		return a.toDouble()		
	}
	lineStr(&s) {
		s.ch()
		return s.findPos("\n").trim()
	}
	arrayFind(arr, key) {
		not(typeof(arr,'array')) return false;
		idx=arr.find(key);
		return idx.ne(-1);
	}
</script>


/*
콜백처리 함수
*/ 
<script>
	@callback.watcher(act, name) { 
		print("파일감시 콜백함수를 등록하세요 변경 경로=${this.target}")
	}
	@callback.web(type,data) {
		print("웹요청 결과 ", type, data)
	}
	@callback.process(type,data) {
		print("웹요청 결과 ", type, data)
	}
</script>

/*
영역처리 함수
*/ 
<script>
	randomColor() {
		hue=System.rand(360).toInt(); 
		return Baro.color('hsl', hue, 100, 100);
	}
	randomIcon() {
		num=System.rand(360).toInt();
		Baro.db('icons').fetch("select type, id from icons where type='vicon' limit $num,1"  ).inject( type, id);
		return "$type.$id";
	}
	/* 주위진 위치에 폭높이만큼 가운데정렬 영역 생성 */
	centerRect(pt, w, h, mode, gab ) {
		pt.inject(x, y);
		w0=w/2.0, h0=h/2.0;
		not( mode ) mode="center";
		switch(mode) {
		case center:
			x-=w0, y-=h0;
		case top:
			x-=w0, y-=h;
			if( gab ) y-=gab;
		case bottom:
			x-=w0, y+=h;
			if( gab ) y+=gab;
		case left:
			x-=w, y-=h0;
			if( gab ) x-=gab;
		case right:
			x+=w, y-=h0;
			if( gab )x+=gab;
		default:
		}
		return Baro.rect(x,y,w,h);
	}
</script>

/* 
배열/노드처리 공통함수
*/
<script>
	copyNode(src, dest) {
		dest.reuse()
		while(k,src.keys()) {
			dest.set(k,src.get(k))
		}
	}
	copyArray(src, dest) {
		dest.reuse()
		while(c,src) {
			dest.add(c)
		}
	}
	copyNodeField(src, dest, field) {
		a=src.get(field)
		if(typeof(a,'node')) { 
			copyNode(a,dest.addNode(field))
		} else if(typeof(a,'array')) {
			copyArray(a,dest.addArray(field))
		} else {
			dest.set(field,a)
		}
	}
	findField(root, field, val) {
		while( cur, root ) {
			if( cur.cmp(field, val) ) return cur;
			if( cur.childCount() ) {
				find=findField(cur, field, val);
				if( find ) return find;
			}
		}
		return null;
	}
	findTag(root, tag) {
		while( cur, root ) {
			if( cur.cmp("tag", tag) ) return cur;
			if( cur.childCount() ) {
				find=findTag(cur,tag);
				if( find ) return find;
			}
		}
		return null;
	}
	findId( root, id) {
		while(cur, root) {
			if(cur.cmp("id",id))return cur;
			if( cur.childCount() ) {
				find=findId(cur,id);
				if( find ) return find;
			}
		}
		return;
	} 
	setArray(arr, idx, node) {
		not(typeof(idx,"num")) return arr;
		if(idx.lt(arr.size()) ) {
			arr.set(idx, node);
		} else {
			arr.add(node);
		} 
		return arr;
	}	
	arrayFind(arr, key) {
		not(typeof(arr,'array')) return false;
		idx=arr.find(key);
		return idx.ne(-1);
	}
	arr() {
		a=_arr()
		while(c,args()) {
			if(typeof(c,'array')) {
				while(d,c) a.add(d)
			} else {
				a.add(c)
			}
		}
		return a;
	}
</script>

/* 
기초객체 관리함수
*/
<script>

	rectTypeArray(type, checkObject) { 
		arr=_arr()
		while(sub, map) {
			if(sub.cmp('type',type)) {
				if(checkObject) {
					if( typeof(checkObject,'bool')) arr.add(sub) else arr.add(sub.get(checkObject));
				} else {
					arr.add(sub.rect)
				}
			}
		}
		return arr;
	}
	rectMap(id) {
		not(id) return;
		map=this.get('@rectMap')
		not(map) map=this.addNode('@rectMap')
		cur=when(map, map.get(id))
		asize=args().size()	
		if(asize.eq(1)) {
			return when(cur, cur.rect, rc(0,0,0,0));
		}
		not(cur) cur=map.addNode(id)
		rcDraw = null
		if(asize.eq(2)) {
			args(id,param)
			if(typeof(param,'string')) {
				fn=Cf.funcNode('parent')
				type=param
				rect=fn.get(id)
			} else {
				type='common'
				rect=param
			}
			not(typeof(rect,'rect')) return print("rectMap 영역찾기 오류 [ID:$id]", rect);
		} else {
			args(id, rect, type, rcDraw)
		}
		cur.with(id,type,rect,rcDraw)
		return cur
	}
	rectInject(&s, type) {
		map=this.var(rectMap)
		if(type) {
			arr=_arr() not(map) return arr;
			while(s.valid()) {
				k=s.findPos(',').trim()
				cur=map.get(k)
				arr.add(cur.rect)
			}
			return arr;
		}
		not(map) return;
		fn=Cf.funcNode('parent')
		while(s.valid()) {
			k=s.findPos(',').trim()
			cur=map.get(k)
			fn.set(k, cur.rect)
		}
	}
</script>

/* 
파일시스템 관리 함수
*/ 
<script>
getFolderCount(path, type) {
	not(isFolder(path)) return 0
	fo=Baro.file("fs");
	fo.var(filter,'folders')
	cnt=0;
	fo.list(path,function(info) {
		while(info.next()) {
			info.inject(type, name)
			if(name.eq('.','..')) continue
			cnt.incr()
		}
	})
	return cnt;
}
copyFile(src, dest) {
	fo=Baro.file('util');
	path=leftVal(dest,'/');
	not(fo.isFolder(path)) return print("$dest 복사 경로가 폴더가 아닙니다");
	not(fo.isFile(src)) return print("복사대상파일이 없습니다 (파일:$src)");
	not(fo.copy(src, dest)) return print("$src 에서 $dest 복사실패");
}
deleteFiles(path) {
	fp=Baro.file()
	fp.list(path, func(info) {
		while(info.next()) {
			info.inject(type,fullPath)
			if(type=='file') fp.delete(fullPath)
		}
	})
}
localPath(name) {
	c=name.ch(1) 
	if(c.eq(':')) {
		if(name.find('/')) {
			path=name.replace("/","\\")
		} else {
			path=name
		}			
	} else {
		path=System.path()
		aa=name.replace("/","\\")
		path.add(when(aa.ch("\\"), aa, "\\$aa"))
	}
	arr=args(), size=arr.size() 
	if(size>1) {
		while(n=1,size) {
			name=arr.get(n)
			aa=name.replace("/","\\")
			path.add(when(aa.ch("\\"), aa, "\\$aa")) 
		}
	}
	return path;
}
relativePath(base, path) {
	if(base ) {
		base=base.trim();
	} else {
		base=System.path();
	}
	not(path ) return base;
	while( path.ch('.') ) {
		ch=path.ch(1);
		if( ch.eq('/') ) {
		// 경로 ./ 처리
		path=path.value(2);
		} else if( ch.eq('.') ) {
		// 경로 ../../ 처리
		ch=path.ch(2);
		if( ch.eq("/") ) {
			path=path.value(3);
			not( base.find("/") ) return print("[relativePath] 기준경로 오류 (base:$base)");
			base=base.findLast("/").trim();
		} else {
			return print("[relativePath] 경로오류 (path:$path)");
		}
		}
	}
	return "$base/$path";
} 
	
fileAppend(path, data) {
	fo=Baro.file('append');
	if(fo.open(path,'append')) {
		fo.append(data);
		fo.close();
	}
}
fileDelete(path) {
	fo=Baro.file();
	if(isFile(path)) {
		result=fo.delete(path);
	} else if(isFolder(path)) {
		result=fo.rmDir(path);
	}
	return result;
}
fileFind(path, flag, val, check, arr) {
	not(arr) arr=_arr();
	num=0
	if(typeof(flag,'string')) {
		switch(flag) {
		case eq: flag=1;
		case index: flag=2;
		case ext: flag=3;
		case modify: flag=4;
			if(typeof(val,'string') ) {
				val=System.localtime(val)
			}
		case all: flag=9;
		default:
		}
	}
	not(flag) {
		if(val) {
			flag=1;
		} else {
			flag=9;
		}
	}
	Baro.file().list(path, func(info) {
		while(info.next()) {
			info.inject(type, name, fullPath, ext, modifyDt);
			if(type=='folder') {
				if(check) {
					fileFind(fullPath, flag, val, check, arr)
					if(flag.eq(1,2)) {
						if(arr.size()) return arr;
					}
					
				}
				continue;
			}
			switch(flag) {
			case 1: 
				if(name==val) return arr.add(fullPath);
			case 2: 
				if(num==val) return arr.add(fullPath);
			case 3:
				if(typeof(val,'array')) {
					while(c,val) {
						if(c==ext) {
							arr.add(fullPath);
							break;
						}
					}
				} else {
					if(val==ext) {
						arr.add(fullPath);
					}
				}
			case 4: 
				if(modifyDt>val) arr.add(fullPath);
			case 9:
				arr.add(fullPath);
			default:
			}
			num.incr();
		}
	});
	return arr;
}
</script>
	
