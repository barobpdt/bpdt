_arr(code, reuse) {
	not( code ) {
		return Cf.array();
	}
	arr=Cf.rootNode().addArray("_arr.$code");
	if( typeof(reuse,"bool") && reuse ) arr.reuse();
	return arr;
}
_node(code, reuse) {
	not( code ) {
		return Cf.node();
	}
	node=Cf.rootNode("_node.$code", true)
	not(node.var(tag)) node.var(tag, code)
	if( typeof(reuse,"bool") && reuse ) node.removeAll(true)
	return node;
}
findServiceNode(cur) {
	p=cur.parentNode()
	while(n=0,100) {
		if(p.isset('serviceName')) {
			return p;
		}
		p=p.parentNode()
		not(p) return;
	}
	return;
}
event(obj, eventName, fc, target, reset) {		
	not(typeof(obj,'node')) return print('@@ job event 객체 오류', obj, fc) 
	fn = obj.get(eventName)
	if( typeof(fn,'func')) {
		print("xxxxxxxx", args())
		not(reset) {
			print("＠＠ $eventName 함수가 이미등록되었습니다")
			return fn;
		}
	}
	fcType = typeof(fc)
	not( fcType.eq('funcRef') ) {		
		if(fc) print("@@ job event  함수타입 오류 (타입:$fcType)")
		return;
	}
	fn=Cf.funcNode(fc, obj)
	if( typeof(target,'node')) {
		fn.set('@sender', obj);
		fn.set('@this',target)
	}
	obj.set(eventName, fn) 
	return fn;		
}
isValid(s) {
	not(s) return false;
	if(typeof(s,'array') && ~(s.size()) ) return false;
	return true;
}
varValue(k, fn, node) {
	if(node && node.isVar(k)) return node.get(k);
	not(fn) fn=Cf.funcNode('parent') if(fn.isset(k)) return fn.get(k);
	not(node) {
		node=fn.get('@this')
	}
	if(typeof(node,'node')) {
		fn=Cf.funcNode(node)
		if(fn && fn.isset(k)) return fn.get(k);
	}
	if(k.eq('nl')) v="\r\n";
	else if(k.eq('tab')) v="\t";
	else v="[$k 미정의]"
	return v;
}
getVarValue(&s,fn,node) {
	not(fn) fn=Cf.funcNode('parent')
	isVar = func(s) {
		c=s.next().ch() not(c) return true;
		while(c.eq('.')) c=s.next().ch()
		return when(c,false,true);
	};
	userVal=func(&s) {
		c=s.ch() not(c) return ' ';
		if(c.eq('@')) {
			k=s.incr().trim()
			v=conf(k) not(v) v="[conf $k 미정의]";
			return v;
		}
		if(c.eq('#')) {
			ss=''
			k=s.incr().trim()
			v=getVarValue(k,fn)
			v.ref()
			while(v.valid()) {
				k=v.findPos(',').trim()
				not(k) break;
				if(ss) ss.add(', ')
				ss.add("#{$k}")
			}
			return ss;
		}
		return;
	};
	not(isVar(s)) return eval(s, fn);
	v=userVal(s,fn) if(isValid(v)) return v;
	c=s.ch()
	if(c.eq(':')) {
		k='int'
	} else {
		k=s.move()
		c=s.ch()
	}
	if(c.eq(':')) {
		type=k
		k=s.incr().move()
		v=varValue(k,fn,node)
		if(type.eq('int')) {
			if(typeof(v,'num')) {
				v=v.toInt()
			} else {
				v=0
			}
		}
		return v;
	}
	ss=varValue(k, fn, node)
	c=s.ch() not(c) return ss;
	while(c.eq('.')) {
		not(typeof(ss,'node')) return '';
		k=s.incr().move()
		c=s.ch()
		not(c) {
			v=ss.get(k)
			not(isValid(v)) v=ss.member("$k")
			return v;
		}
		ss=ss.get(k)
	}
	return ss;
}

str(&s, fn, node) {
	if(typeof(fn,'node')) {
		node=fn
		fn=null
	} 
	not(fn) {
		fn=Cf.funcNode('parent')
	}
	ss=''
	while(s.valid()) {
		left = s.findPos('$')
		ss.add(left)
		c=s.ch() not(c) break;
		if(c.eq('{')) {
			src=s.match(1)
			if(typeof(src,'bool')) break;
			ss.add(getVarValue(src,fn,node))
			continue;
		}
		k=s.move()
		c=s.ch(0)
		if(c.eq(':')) {
			type=k
			k=s.incr().move()
			v=varValue(k,fn,node)
			if(type.eq('int')) {
				if(typeof(v,'num')) {
					v=v.toInt()
				} else {
					v=0
				}
			}
		} else if(c.eq('[')) {
			cur=fn.get(k) if(node && typeof(node,'node')) cur=node.get(k)
			if(typeof(cur,'node')) {
				k=s.match().trim()
				v=varValue(k,fn,cur)
			}
		} else {
			v=varValue(k,fn,node)
		}
		if(isValid(v)) ss.add(v)
	}
	return ss;
} 
confSearch(&s) {
	not(s.ch()) return;
	sp=s.cur()
	c=s.next().ch()
	while(c.eq('#')) {
		c=s.incr().next().ch()
	}
	a=s.trim(sp, s.cur(), true)
	db=Baro.db('config')
	filter=''
	if(a.eq('*') ) {
		c=s.ch()
		not(c.eq('.')) return print("@@ error conf list $a 하위 정보 오류");
		b=s.incr().trim()
		if(b.find('%')) {
			filter = "and cd like '$b'"
		} else {
			filter = "and cd='$b' "
		}
	} else {
		if(c.eq('.')) {
			b=s.incr().trim()
			if(b.eq('*')) {
				filter = "and grp='$a'"
			} else {
				filter = "and grp='$a' and cd like '$b'"
			}
		} else {
			filter = "and grp='$a'"
		}
	}
	node=db.fetchAll("select grp, cd, data from conf_info where 1=1 $filter")
	ss=''
	while(cur, node) {
		cur.inject(grp, cd, data)
		info = getLine(data)
		line=str('$grp.$cd $info $nl')
		ss.add(line)
	}
	return ss;
}
global(code) { return Cf.rootNode() }
object(code, newCheck) {
	not( code.find('.') ) return Cf.rootNode(code,true)
	code.split('.').inject(a,b);
	return Cf.getObject(a,b,true);
}
checkError(msg) {
	err=Cf.error()
	if(err) {
		print("@@ $msg [error]: $err");
		return true;
	}
	return;
}
isFunc(&s) {
	s.ch() not(c) return;
	if(c.eq('@')) s.incr()
	c=s.next().ch()
	while(c.eq('-','.')) c=s.incr().next().ch()
	return when(c.eq('('), true)
}
getLine(&s) {
	not(s) return '[line blank]';
	not(typeof(s,'string')) return "$s";
	s.ch()
	line = s.findPos("\n").trim()
	return line;
}
lastLine(&s) {
	if( s.find("\n")) {
		left=s.findLast("\n")
		return left.right();
	} else if(s) {
		return s;
	}
}
jsValue(s) {
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
getVarName(&s) {
	not(typeof(s,'string')) return;
	ss='', upper=false
	while(n=0,s.size()) {
		c=s.ch(n) not(c) break;
		if(c.eq('_','-')) {
			upper=true
			continue;
		}
		if(c.eq('/')) {
			c='_'
			continue;
		} 
		if(c.eq(' ') || c.is('oper')) {
			break;
		}
		if(upper){
			ss.add(c.upper())
			upper=false
		} else {
			ss.add(c)
		}
	}
	return ss;
}
getStyleKeyName(&s) {
	not(typeof(s,'string')) return;
	ss='', upper=false
	while(n=0,s.size()) {
		c=s.ch(n)
		if(c.is('upper')) {
			if(n) ss.add('-',c.lower())
		} else {
			ss.add(c)
		}
	}
	return ss;
}
getDbFieldName(&s) {
	not(typeof(s,'string')) return;
	ss='', upper=false
	while(n=0,s.size()) {
		c=s.ch(n)
		if(c.is('upper')) {
			if(n) ss.add('_',c.upper())
		} else {
			ss.add(c.upper())
		}
	}
	return ss;
}
propValue(&s, propName) {
	while(s.valid()) {
		left=s.findPos(propName)
		c=s.ch() not(c) break;
		if(c.eq('=',':')) {
			c=left.ch(-1)
			if(c.is('alphanum')) continue;
			c=s.incr().ch()
			if(c.eq()) {
				val=s.match()
			} else {
				val=s.findPos(", \t\n",4)
			}
			return val;
		}
	}
	return;
}

stripComment(&s, mode) {
	not(mode) mode=1;
	rst='';
	while(s.valid()) {
		if(mode.eq(1)) {
			left=s.findPos("/*",1,1);
			s.match();
		} else if(mode.eq(2)) {
			left=s.findPos("//",1,1);
			s.findPos("\n");
		} else if(mode.eq(3)) {
			left=s.findPos("<!--",1,1);
			s.match("<!--","-->");
		} else if(mode.eq(4)) {
			left=s.findPos("--",1,1);
			s.findPos("\n");
		}
		rst.add(left);
		not(s.valid()) break;
	}
	return rst;
}
stripJsComment(&s) {
	rst=stripComment(s,1);
	return stripComment(rst,2);
}

filePathInfo(path) {
	a=_arr()
	b=path.replace('\','/')
	s=b.ref()
	path=s.findLast('/')
	if(path) {
		filename=path.right()
	} else {
		path=b, filename=b
	}
	a.add(path,filename)
	name=filename.findLast('.')
	a.add(name)
	return a;
}
fileTime(fullpath) { return Baro.file().modifyDate(fullpath) }
isFile(fileName) { return Baro.file().isFile(fileName) }
isFolder(fullPath, makeCheck) {
	fo=Baro.file(); 
	folder=fo.isFolder(fullPath);
	not(folder) {
		if(makeCheck) {
			fo.mkdir(fullPath, true);
			folder=fo.isFolder(fullPath);
		}
	}
	return folder;
}
pathJoin() {
	ss=''
	while(a,args(), n) {
		not(typeof(a,'string')) return print("pathJoin 오류 메게변수 오류 ",args())
		if(a.find('\')) a=a.replace('\','/')
		c=a.ch(-1)
		if(c.eq('/')) a=a.value(0,-1)
		if(n) {
			ss.add('/')
		}
		c=a.ch()
		if(c.eq('/')) {
			ss.add(a.value(1))
		} else {
			ss.add(a)
		}
		print("a>>$a")
	}
	return ss;
}
fileRead(path) {
	fo=Baro.file('read'); // 파일객체 생성
	not(fo.open(path,'read')) {
		return print("readFile open error (경로 $path)");
	}
	src = fo.read();
	fo.close()
	return src;
}
fileWrite(path, buf) {
	fo=Baro.file('save');
	if(path.find('/')) {
		str=path.findLast('/').trim();
	} else {
		str=path;
	}
	not(fo.isFolder(str)) {
		fo.mkdir(str, true);
	}
	not(fo.open(path,"write")) return print("writeFile open error (경로 $path)");
	fo.write(buf);
	fo.close();
} 

/*
페이지처리 함수
*/
include(name, checkRealod) {
	not(name.find('.')) {
		name.add('.js')
	}
	path = conf('include.path')
	if(path) {
		filenm = "$path/$name"
	} else {
		filenm = name
	}
	
	not( isFile(filenm) ) return print("include 오류 ($filenm 파일이 없습니다)")
	map=object('map.include') 
	modify=Baro.file().modifyDate(filenm)
	not(checkRealod) {
		tm=map.get(name)
		if(tm==modify) {
			print("include 경로 $name 이미 등록", tm, prevName)
			Cf.rootNode('@funcInfo').set('includeFileName', prevName)
			return;
		}
	}
	prevName = Cf.rootNode('@funcInfo').get('includeFileName') not(prevName) prevName=''
	Cf.rootNode('@funcInfo').set('includeFileName', name)
	filePathInfo(filenm).inject(folder,filename,fname)
	subName = null
	if( fname.find('#')) {
		split(fname,'#').inject(fname, subName)
	}
	map.set(name, modify)
	src=fileRead(filenm)
	parseSource(stripJsComment(src), fname, subName)
	Cf.rootNode('@funcInfo').set('includeFileName', prevName)
}
 
page(name, param) {
	asize=args().size()
	if(asize.eq(0)) {
		not(this) return print("page 함수 호출오류 (this 미정의)")
		p=this.parentWidget()
		return when(p, p.pageNode(), this.pageNode());
	}
	moduleAdd = false
	moduleCode = ''
	if( asize.eq(1) ) {		
		target=this
	} else if(asize.eq(2)) { 
		if(typeof(param,'bool')) {
			args(name, moduleAdd)
			target = this
		} else if(typeof(param,'widget')) {
			args(name, target)
		}
		not(typeof(target,'widget')) return print("page 대상이 위젯이 아닙니다 (이름:$name)")
	} else { 
		args(name, moduleAdd, target, moduleCode)
	}
	if( name.find(':') ) return Cf.getObject('page', name)
	base = left(target.get('@baseCode'),':') not(base) return print("page 함수 호출오류 (페이지 base 코드오류)")
	baseCode = "$base:$name"
	page = Cf.getObject('page', baseCode) not(page) return print("page 함수 호출오류 ($baseCode 페이지를 찾을수 없습니다)")
	if( page.var(initUse) ) {
		return page;
	}
	addModule(page, '@page')
	if( moduleCode ) baseCode = moduleCode
	if( moduleAdd && baseCode ) {
		addModule(page, baseCode)
	}
	if(typeof(page.initPage,'func')) {
		page.initPage()
	}
	page.var(initUse, true)
	return page;
}
dialog(name, param) {
	moduleAdd = false
	moduleCode = ''
	asize=args().size()
	if( asize.eq(1) ) {		
		target=this
	} else if(asize.eq(2)) {
		if(typeof(param,'bool')) {
			args(name, moduleAdd)
			target = this
		} else if(typeof(param,'widget')) {
			args(name, target)
		}
		not(typeof(target,'widget')) return print("dialog 대상이 위젯이 아닙니다 (이름:$name)")
	} else { 
		args(name, moduleAdd, target, moduleCode, reload)
		if(reload) {
			path = object('@inc.userFunc').get("${moduleCode}#initDialog")
			if(path ) include(path)
		}
	}	
	if( name.find(':') ) return Cf.getObject('dialog', name)
	base = left(target.get('@baseCode'),':') not(base) return print("dialog 함수 호출오류 (페이지 base 코드오류)")
	baseCode = "$base:$name"
	dialog = Cf.getObject('dialog', baseCode) not(dialog) return print("dialog 함수 호출오류 ($baseCode 페이지를 찾을수 없습니다)")
	
	if( dialog.var(initUse) ) {
		return dialog
	}
	if( moduleCode ) baseCode = moduleCode
	if( moduleAdd && baseCode ) {
		addModule(dialog, baseCode)
	}
	if(typeof(dialog.initDialog,'func')) {
		dialog.initDialog()
	}
	dialog.var(initUse, true)
	return dialog;
}
widget(name) {
	asize = args().size()
	if(asize.eq(0)) {
		return allWidget(this)
	}
	if(asize.eq(1)) {
		widget = this.member("$name")
		not(typeof(widget,'widget')) widget = this.findWidget(name)
		not(typeof(widget,'widget')) return print("widget 위젯 찾기오류 (이름:$name)")
		return widget;
	}
	moduleName = ''
	if(asize.eq(2)) {
		if(typeof(name,'widget')) {
			args(target, name)
		} else {
			args(name, moduleName) 
		}
	} else if(asize.eq(3)) {
		args(target, name, moduleName)
	}	
	not(typeof(target,'widget')) return print("widget 참조 대상이 위젯이 아닙니다 (이름:$name)")
	widget = target.member("$name")
	not(typeof(widget,'widget')) widget = target.findWidget(name)
	not(typeof(widget,'widget')) return print("widget 위젯 찾기오류 (이름:$name)")
	if( widget.var(initUse) ) {
		return widget
	}
	if( moduleName ) {
		addModule(widget, baseCode)
	}
	if(typeof(widget.initWidget,'func')) {
		widget.initWidget()
	}
	widget.var(initUse, true)
	return widget;	
}

allWidget(parent, arr) {
	not(arr) arr=_arr()
	while(cur, parent) {
		not(cur.tag) continue;
		arr.add(cur)
		if(cur.childCount()) {
			allWidget(cur,arr)
		}
	}
	return arr;
}

applyFunc(src, module) {
	if( module ) {
		if(module.ch('@')) {
			module=module.value(1)
		}
		Cf.rootNode('@funcInfo').set('refName', module)
		Cf.sourceApply("<func>$src</func>")
		Cf.rootNode('@funcInfo').set('refName', '')
	} else {
		call(src)
	}
} 
parseSource(&s, base, subName) {
	map = null
	pageBase = ''
	widgetSource = ''
	while(s.valid() ) {
		c=s.ch() not(c) break;		
		if(c.eq(',',';')) {
			s.incr()
			continue;
		}
		not(c.eq('<')) {
			if( map ) break;
			if( base && subName ) {
				applyFunc(s, "${base}:${subName}")
			} else {
				applyFunc(s)
			}
			return 
		}
		if( s.start('<!--')) {
			s.match('<!--', '-->')
			continue;
		}
		sp=s.cur()
		tag = s.incr().move() s.pos(sp)
		ss=s.match("<$tag","</$tag>")
		if(typeof(ss,'bool')) return print("parseSource 함수오류 ($tab 태그 매칭오류)")
		prop=ss.findPos('>')
		if( tag.eq('widgets','pages') ) {
			not(base) {
				base = propVal(prop,'base') not(base) base ='test'
			}
			widgetSource.add(str('<widgets base="$base">$ss</widgets>'))
		} else if( tag.eq('script') ) {
			module=propVal(prop, 'module')
			if( module ) { 
				if( module.ch('@') ) {
					module=module.value(1)
				} else {
					subName=module
					not(module.find(':')) {
						module = "${base}:${subName}"
					}
				}	
			} 
			applyFunc(ss, module)		
		} else {
			print("parseSource 오류 태그 $tag 가 정의되지 않았습니다")
		}
	}
	if( widgetSource ) {
		Cf.rootNode('@funcInfo').set('pageBase', base)
		Cf.sourceApply(widgetSource)
		Cf.rootNode('@funcInfo').set('pageBase', '')
	}
}
addObjectArrayVar(obj, name, val) {
	a=obj.var("$name")
	not(typeof(a,'array')) {
		a = obj.newArray()
		obj.var("$name", a)
	}
	not(a.find(val)) a.add(val)
	return a
}
findObjectArrayVar(obj, name, val) {
	a=obj.var("$name")
	not(typeof(a,'array')) return;
	idx=a.find(val);
	return idx.ne(-1);
}

addModule(obj, moduleName) {
	asize=args().size()
	if(asize.eq(0)) {
		obj=this
		moduleName = obj.var(baseCode)
	} else if(asize.eq(1)) {
		obj=this
		args(moduleName)
	}
	if( moduleName.ch('@')) moduleName = moduleName.value(1)
	print("addModule => ", moduleName)
	if( findObjectArrayVar(obj,'moduleList',moduleName) ) return obj;
	// ex) editor#myedit
	subName = ''
	if( moduleName.find(':')) {
		split(moduleName,':').inject(subFuncName, subName)
	} else {
		subFuncName = moduleName
	}
	funcInfo = object('user.subfuncMap').get(subFuncName)
	result = addModuleFunc(obj, subFuncName, funcInfo, subName) 
	if( result ) {
		if( subName ) {
			fcInit = obj.get("init_$subName")
		} else {
			fcInit = obj.get("init")
		}
		if( typeof(fcInit,'func') ) {
			if(asize.eq(0)) {
				params=args()
			} else if(asize.eq(0)) {
				params=args(1)
			} else {
				params=args(2)
			}
			call(fcInit, obj, params)
		}
	}
	obj.var(useModule, true)
	addObjectArrayVar(obj,'moduleList',moduleName)
	return obj
}
isEventName(&s) {
	if(s.start('on',true)) {
		c=s.ch()
		if(c.is('upper')) return true;
	}
	return false;
}

addModuleFunc(obj, subFuncName, &funcs, subName) {
	cnt = 0
	not(funcs) return cnt;
	while(funcs.valid()) {
		a=funcs.findPos(',')
		fnm = a.trim() not(fnm) break;
		fc=call("${subFuncName}.${fnm}") not(typeof(fc,'func')) continue;
		if(subName) {
			not(fnm.find('#')) continue;
			aa = left(fnm,'#') not(subName.eq(aa)) continue;
			bb = right(fnm,'#')
			if(bb.eq('init')) {
				bb.add("_$subName")
				cnt++
			}
			fnm = bb
		} else {
			if(fnm.find('#')) continue;
			if(fnm.eq('init')) {
				cnt++
			}
		}
		if(isEventName(fnm)) {
			fn=Cf.funcNode(fc,obj)
			fn.setPersist(true)
			obj.set(fnm, fn)
		} else {
			obj.set(fnm, fc)
		}
		print("모듈함수 ${subFuncName}.${fnm} 등록")
	}
	return cnt;
}
