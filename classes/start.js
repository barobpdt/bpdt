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
	node=Cf.rootNode().addNode("_node.$code")
	not(node.var(tag)) node.var(tag, code)
	if( typeof(reuse,"bool") && reuse ) node.removeAll(true)
	return node;
}
global(code) {
	root=Cf.rootNode().addNode("@global");
	result = ''
	switch(args().size()) {
	case 0:
		return root;
	case 1: 
		return root.get(code)
	case 2:
		args(code,value)
		if( typeof(value,'null')) {
			result = root.get(code)
			root.set(code, '')
		} else {
			result = value
			root.set(code, value)
		}
	default:
	}
	return result;
}
object(code, newCheck) {
	not( code.find('.') ) return Cf.rootNode().addNode(code);
	code.split('.').inject(a,b);
	return Cf.getObject(a,b,true);
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
stripFileName(name) {
	if(name.find('/')) name=right(name,'/')
	if(name.find('.')) name=left(name,'.')
	return name
}
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
	base = stripFileName(filenm)
	subName = null
	if( base.find('#')) {
		split(base,'#').inject(base, subName)
	}
	map.set(name, modify)
	src=fileRead(filenm)
	parseSource(stripJsComment(src), base, subName)
	Cf.rootNode('@funcInfo').set('includeFileName', prevName)
}
tag(tag, id) {
	node = Cf.getObject().addNode('@newObject')
	not(id) {
		idx = node.incrNum(tag)
		id = "${tag}_${idx}"
	}
	cur = node.addNode(id)
	not(cur.id) {
		cur.id=id
		cur.tag=tag
	}
	return cur;
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
parseConf(name, &s) {
	while(s.valid() ) {
		c=s.ch() not(c) break;
		not(c.eq('<')) break;
		sp=s.cur()
		tag = s.incr().move() s.pos(sp)
		ss=s.match("<$tag","</$tag>")
		if(typeof(ss,'bool')) return print("parseConf 설정등록 오류 ($tab 태그 매칭오류)")
		prop=ss.findPos('>')
		id=propVal(prop,'id')
		if(id) {
			if(tag.eq('json')) {
				node=_node()
				node.parseJson(ss)
				data=json().nodeStr(node)
			} else {
				data=ss
			}
			conf("${name}.${id}", data, true)
		}
	}		
}
parseSource(&s, base, subName) {
	map = null
	pageBase = ''
	widgetSource = ''
	while(s.valid() ) {
		c=s.ch() not(c) break;
		if(c.eq('/')) {
			c=s.ch(1)
			if(c.eq('/')) s.findPos("\n") else s.match()
			continue
		}
		if(c.eq(',',';')) {
			s.incr()
			continue;
		}
		not(c.eq('<')) {
			if( map ) break;
			if( base && subName ) {
				module = "${base}:${subName}"
			} else {
				module = base
			}
			return applyFunc(s, module)
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
			if( base ) {
				prop = Cf.val('base=',Cf.jsValue(base) )
				pageBase = propVal(prop,'base')
			} else {
				base = propVal(prop,'base') not(base) base ='test'
				pageBase = base
			}
			widgetSource.add("<widgets $prop>$ss</widgets>")
		} else if( tag.eq('script') ) {
			module=propVal(prop, 'module')
			if( module ) {
				if( module.ch('@') ) {
					module=module.value(1)
				} else {
					if( module.find(':') ) {
						split(module,':').inject(base,subName)
					} else if( module.find('#') ) {
						split(module,'#').inject(base,subName)
					} else {
						subName=module
					}
					module = "${base}:${subName}"
				}			
			} else if( base && subName ) {
				module = "${base}:${subName}"
			}
			applyFunc(ss, module)		
		} else if( tag.eq('conf') ) {
			name=propVal(prop, 'name')
			not(name) name=propVal(prop, 'id')
			not(name) name='common'
			parseConf(name, ss)
		} else if( tag.eq('sql','text') ) {
			id=propVal(prop, 'id')
			if(id) {
				not(pageBase) {
					pageBase=base not(pageBase) pageBase='common'
				}
				conf("${tag}.${pageBase}:${id}", ss, true)
			}
		} else if( tag.eq('example') ) {
			print("[사용예제]", ss)
		} else {
			print("parseSource 오류 태그 $tag 가 정의되지 않았습니다")
		}
	}
	if( widgetSource ) {
		Cf.rootNode('@funcInfo').set('pageBase', pageBase)
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
	print("addModule => ", moduleName, obj.id)
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
