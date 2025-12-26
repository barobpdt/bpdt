class func {
	openToolPopup(form, input, title, node) {
		chk=eq(this,form.parentWidget())
		not(chk) {
			form.parentWidget(this)
			form.flags('tool', true)			
		}
		if(title) form.title(title)
		if(node) form.set('workNode', node)
		form.open();
		form.active();
		input.value('')
		this.var(focusWidget, input)
	}
	defineWidgetTag(node) {
		if(typeof(node.space,'bool') && node.space) {
			node.tag='label'
			node.stretch=1
			return true;
		}
		return;
	}
	widgetLayoutChild(tag, parent) {
		while(cur, parent, idx) {
			scroll=cur.var(scroll)
			if(scroll) {
				parent.addChild(scroll)
			} else if(tag.eq('form')) {
				cur.inject(row, col, rowspan, colspan)
				parent.addChild(cur, row, col, rowspan, colspan)
			} else {
				parent.addChild(cur)
			}
		}
		return idx
	}
	newTagNode(tag, name, parent) {
		not(name) {
			idx=global().incrNum("$tag.index")
			name="$tag_$idx"
		}
		node=global().addNode("$tag.$name")
		if(typeof(node,'widget')) {
			while(n=0,32) {
				code="${name}_${n}"
				node=global().addNode("$tag.$code")
				if(typeof(node,'widget')) continue
				name=code
				break;
			}
		}
		node.with(tag, name)
		if( parent) {
			if(typeof(parent,'bool')) {
				Cf.createWidget(node)
			} else {
				Cf.createWidget(node,parent)
			}
		}
		/*
		btn.styleSheet(#[
		QPushButton {	
			border : none;
		}
		QPushButton:hover:!pressed
		{
			background-color: rgb(224, 255, 0);
		}])

		tab.styleSheet(#[
		QTabWidget::pane {
			border: 1px solid lightgray;
			top:-1px; 
			background: rgb(245, 245, 245);; 
		} 

		QTabBar::tab {
			background: rgb(230, 230, 230); 
			border: 1px solid lightgray; 
			padding: 4px;
		} 

		QTabBar::tab:selected { 
			background: rgb(245, 245, 245); 
			margin-bottom: -1px; 
		}	
		])
		*/
		return node
	}
	makeWidgetTag(node) {
		not(typeof(node,'node')) return print('make widget tag error', node);
		if( defineWidgetTag(node) ) return node;
		a=[page,dialog,main] 
		b=[hbox,vbox,form]
		c=[canvas,grid,tree,combo,input,label,
			check,radio,button,
			calendar,date,time,
			editor,video]
		d=[splitter,tab,group,div]
		tag=node.tag
		ty=''
		if(tag) {
			if(a.find(tag)) {
				ty='page'
			} else if(b.find(tag)) {
				ty='layout'
			} else if(c.find(tag)) {
				ty='widget'
			} else if(d.find(tag)) {
				ty='group'
			}
		} else {
			while(x,a) {
				v=node.get(x)
				if(typeof(v,'bool')) {
					ty='page'
					node.tag=x
					break;
				}
			}
			not(node.tag) {
				while(x,b) {
					v=node.get(x)
					if(typeof(v,'bool')) {
						ty='layout'
						node.tag=x
						break;
					}
				}
			}
			not(node.tag) {
				while(x,c) {
					v=node.get(x)
					if(typeof(v,'bool')) {
						ty='widget'
						node.tag=x
						break;
					}
				}
			}
			not(node.tag) {
				while(x,d) {
					v=node.get(x)
					if(typeof(v,'bool')) {
						ty='group'
						node.tag=x
						break;
					}
				}
			}
		}
		node.var(widgetType, ty)
		while(cur, node) {
			makeWidgetTag(cur)
		}
		return node;
	}
	addWidgetChild(box, node) {
		tag=node.tag
		not(tag) {
			if(node.childCount()) {
				print("$box 레이아웃 자식 태그 오류 (노드:$node)")
			}
			while(row, node) {
				if(typeof(row,'widget','layout')) continue;
				addWidgetChild(box,row)
			}
			return;
		}
		cur=box.addNode()
		cur.copyNode(node)
		if( box.var(useAddPage)) {
			Cf.createWidget(cur,box)
		} else if(box.cmp('tag','form')) {
			cur.inject(row, col, rowspan, colspan)
			box.addChild(cur, row, col, rowspan, colspan)
		} else {
			box.addChild(cur)
		}
		not(typeof(cur,'widget','layout')) {
			print("$box 레이아웃 자식 위젯생성 오류 (자식:$cur)")
			return;
		}
		ty=node.var(widgetType)
		if( ty.eq('page')) {
			layout=cur.child(0)
			while(row, node) {
				if(typeof(row,'widget','layout')) continue;
				addWidgetChild(layout,row)
			}
		} else if(ty.eq('layout')) {
			while(row, node) {
				if(typeof(row,'widget','layout')) continue;
				addWidgetChild(cur,row)
			}
		} else if(ty.eq('group')) {
			if( tag.eq('splitter','div','tab') ) {
				cur.var(useAddPage, true)
				while(row, node) {
					if(typeof(row,'widget','layout')) continue;
					sub=cur.addNode()
					addWidgetChild(cur,sub)
				}
			}
			while(row, node) {
				if(typeof(row,'widget','layout')) continue;
				addWidgetChild(cur,row)
			}
		}
		return cur;
	}
	formInfoApply(code, target, checkEvent) {
		src=conf("formInfo.$code") not(src) return print("폼적용 $code 소스 미정의")
		node=_node("formInfo.$code")
		node.parseJson(src)
		node.inject(name,draw,event)
		if(typeof(node.rows,'node')) return print("폼적용 $code 폼정보 미정의")
		src=''
		if(typeof(draw,'node')) {
			while(n=0,node.rows.size()) {
				fsrc=draw.get("row$n")
				not(fsrc) break;
				src.add("draw_${name}_${n}${fsrc}\r\n")
			}
		}
		if( checkEvent && typeof(event,'node')) {
			while(fcName, event.keys()) {
				fsrc=event.get(fcName)
				src.add("click_${fcName}${fsrc}\r\n")
			}
		}
		print("formInfoApply 소스: $src")
		if( target && src ) {
			target[$src]
			return true;
		}
		return false;
	}
	formInfoSource(node, target, code) {
		node.inject(name,draw,event,memberFunction,widget,size,flags)
		widgets=''
		sub=''
		if( size && size.find('x')) {
			size.split('x').inject(w,h)
			sub="${name}.size($w,$h)"
		}
		if(flags) {
			sub.add("\r\n${name}.parentWidget(null)\r\n${name}.flags('${flags}',true)")
		}
		if(typeof(widget,'node')) {
			while(varName, widget.keys()) {
				type=widget.get(varName)
				widgets.add("@${varName}=this.makeWidget('${type}','${name}_${varName}')\r\n")
			}
		}
		src=#[
			init_${name}(form) {
				@${name}=this.makeWidget()
				${widgets}
				${sub}
				${name}.setFormInfo(form, this)
				return ${name};
			}
		]
		if(typeof(draw,'node')) {
			while(n=0,100) {
				fsrc=draw.get("row$n")
				not(fsrc) break;
				src.add("draw_${name}_${n}${fsrc}\r\n")
			}
		}
		if(typeof(event,'node')) {
			while(fcName, event.keys()) {
				fsrc=event.get(fcName)
				if(target) {
					fc=target.get("click_${fcName}")
					not(typeof(fc,'func')) {
						src.add("click_${fcName}${fsrc}\r\n")
					}
				} else {
					src.add("click_${fcName}${fsrc}\r\n")
				}
			}
		}
		if(typeof(memberFunction,'node')) {
			while(fcName, memberFunction.keys()) {
				fsrc=memberFunction.get(fcName)
				if(target) {
					fc=target.get(fcName)
					not(typeof(fc,'func')) {
						src.add("${fcName}${fsrc}\r\n")
					}
				} else {
					src.add("${fcName}${fsrc}\r\n")
				}
			}
		}
		if(node.isVar('name') ) {
			name=node.name
			target[$src]
			fc=target.get("init_$name")
			if(typeof(fc,'func')) {
				call(fc, target, node)
				not(code) code=name
				node.formCode=code
			}
		}
		return src;
	}
	formInfoConf(code, target, reuse) {
		node=_node("formInfo.$code")
		if( node.rows ) {
			not(reuse) {
				formInfoSource(node,target,code)
				return node;
			}
			node.removeAll()
		}
		not(target) {
			target=this
		}
		not(typeof(target,'widget')) {
			return alert("폼정보 적용 대상 위젯 미정의");
		}
		Cf.funcNode().set("@targetPage", target)
		src=conf("formInfo.$code")
		ch=src.ch()
		if( ch.eq('{','[') ) {
			node.parseJson(src);
		}
		not(typeof(node.rows,'node')) {
			print("node==>", node)
			return print("formInfoConf 설정 $code 폼정보 미정의")
		}
		formInfoSource(node,target,code)
		return node;
	} 
	widget(tag, name, base) {
		not(name) {
			if(tag.eq('page')) {
				p=this.parentWidget()
				return when(p, p.pageNode(), this.pageNode());
			} else {
				return print("widget 함수 오류($tag 위젯 이름이 없습니다)")
			}		 
		}
		baseName="";
		if(base ) {
			baseName="$base:$name"
		} else {
			if(name.find(':')) {
				baseName=name
			} else { 
				base=classBaseName(this) not(base) base="baro"
				baseName="$base:$name"
			}
		}
		obj=Cf.getObject(tag, baseName) not(obj) return print("$baseName $tag 오류");
		if(typeof(obj,'widget')) {
			not(obj.var(useClass)) class(obj,'widget');
		} 
		return obj;
	}
	page(name, base) {
		return widget('page', name, base)
	}
	
	checkWidgetPosition(main, form) {
		list=main.member(updateWidgetList)
		not(list) return print("updateWidgetList 배열 미정의");
		not(form) form=this
		not(list.find(form)) list.add(form) 
		main.updateTick=System.tick()
		return true;
	}
	widgetMove(widget, param) {
		not(typeof(widget,"widget") ) return print("widgetMove widget error", args());
		switch(args().size()) {
		case 2:
			if( typeof(param,"widget")) {
				parent=param
				rect=widget.rcClient
			}
		case 3:
			args(1, rect, parent)
		default:
		}
		
		if(parent) { 
			if( parent != widget.parentWidget()) {
				print("@@@@@ widget move parent change @@@@@@", widget.id, rect)
				widget.flags("child"); 
				widget.parentWidget(parent);
			}
		}
		if(typeof(rect,"rect") ) {
			widget.move(rect);
			widget.open();
		}
		return widget;
	}
	splitterSize(s,a,n) {
		tot=s.sizes().sum()
		s.sizes(recalc(tot,a))
		if(typeof(n,'num')) s.stretchFactor(n,1)
	}
	@widget.find(base, id) {
		_find=func(&s) {
			s.findPos('.')
			if(s.start(base,true)) {
				if(s.ch(':')) {
					name=s.incr().trim()
					if(name.eq(id)) return true;
				}
			}
			return false;
		};
		root=Cf.getObject()
		while(name, root.keys()) {
			if(_find(name)) return root.get(name);
		}
		return;
	}
	@widget.onInitBase() {
		widgetStartTick=System.tick()
		this.firstCall=true
	}
	@widget.eventBase() {
		fn=Cf.funcNode()
		if( fn.eventFuncList()) {
			fn.callFuncSrc()
		}
	}
	@widget.idList(page) {
		arr=page.addArray('@idList')
		find=func(root) {
			while(cur, root) {
				if(cur.id) arr.add(cur)
				if(cur.childCount()) find(cur)
			}
		}
		find(page)
		return arr;
	}
	@widget.idFind(page, id) {
		arr=page.get('@idList')
		not(arr) arr=@widget.idList(page)
		while(cur,arr) {
			if(cur.cmp('id',id)) return cur;
		}
		return;
	}
	@widget.varValue(obj,param,nullCheck) {
		code="@$param"
		val=obj.get(code) 
		if(nullCheck) obj.set(code,null)
		return val;
	}
	@page.margin(page) {
		box=page.child(0)
		tag=box.tag
		not(tag.eq('vbox','hbox','form')) return print("page margin 오류 레이아웃 미정의");
		switch(args().size()) {
		case 2:
			args(1,a)
			box.margin(a)
		case 3:
			args(1,a,b)
			box.margin(a,b,a,b)
		case 5:
			args(1,a,b,c,d)
			box.margin(a,b,c,d)
		}
	}
	@page.spacing(page, num) {
		not(num) num=0
		box=page.child(0)
		tag=box.tag
		not(tag.eq('vbox','hbox','form')) return print("page margin 오류 레이아웃 미정의");
		box.spacing(num)
	}
	@page.makePage(id, base, props, subTag, checkClass) {
		not(id) {
			index=global().incrNum("@makePage_index")
			id="makePage_${index}"
		}
		not(base) base="baro"
		map=object("layoutMap#${base}")
		page=map.get(id) if(page) return page;
		page=object("page.$base:$id")
		page.id=id
		page.tag="page"
		if(props) page.parseJson(props)		
		checkForm=false
		if(subTag.eq('form')) {
			subTag='canvas'
			checkForm=true;
		}
		if(subTag) {
			vbox=page.addNode() vbox.tag='vbox'
			cur=vbox.addNode() 
			cur.tag=subTag
			cur.id=when(subTag.eq('canvas'),'form',subTag)
		}
		not( Cf.createWidget(page) ) return print("페이지 생성 실패 (베이스 맵아이디: $base:$id)")
		class(page, 'page')
		if( checkForm ) {
			class(cur,'widget')
			class(cur,'formInfo')
		}
		if( checkClass ) {
			class(page)
		}
		page.var(baseCode, "$base:$id")
		page.var(baseName, base)
		map.set(id,page)
		return page;
	}
} 

class widget {
	initClass() {
		@widgetList=null
	}
	base() {
		base=this.var(baseName)
		return nvl(base, classBaseName(this));
	}
	conf(name) {
		base=this.base()
		switch(args().size() ) {
		case 1:
			return conf("confSource.${base}:${name}");
		case 2:
			args(1,param)
			if(typeof(param,'bool')) {
				val=conf("confSource.${base}:${name}")
				if(param) {
					// ex) this.conf('xxx',true)
					if( this.member(targetPage)) {
						target=this.member(targetPage)
					} else {
						target=this
					}
					Cf.funcNode().set("@targetPage", target)
					ch=val.ch()
					if( ch.eq('{','[') ) {
						return _node("${base}:${name}").parseJson(val);
					} else {
						return val;
					}
				} else {
					val.ref()
					ch=val.ch() 
					if( ch.eq('{','[') ) {
						return val.match(1);
					} else {
						return val;
					}
				}
			} else {
				conf("confSource.${base}:${name}", param);
			}
		case 3:
			args(1,value, update)
			conf("confSource.${base}:${name}", value, update);
		}		
	}
	findWidget(id) {
		base=this.base()
		map=object("layoutMap#${base}")
		return map.get(id)
	}
	addWidget(param) {
		not( typeof(param,'widget')) {
			print("addWidget 오류 $param 위젯객체가 아닙니다")
			return;
		} 
		not( this.member(widgetList) ) {
			@widgetList=this.newArray()
		}
		not( widgetList.find(param) ) {
			widgetList.add(param)
		}
		return param;
	}
	getWidget(id, checkClass) { 
		obj=this.get(id)
		not(obj) {
			obj=this.findWidget(id)
			not(obj) { 
				arr=Cf.getObject().filter("$base:$id")
				while(cur, arr) {
					if(typeof(cur,'widget')) {
						obj=cur
						break;
					}
				}
			}
		}
		not( typeof(obj,'widget') ) {			
			return print("${this.id} 화면의 $id 위젯객체를 찾을수 없습니다");
		}
		not( obj.var(useClass)) {
			obj.var(baseCode, this.base())
			class(obj, 'widget')
		}
		if( typeof(checkClass,'bool') && checkClass ) {
			class(obj);
		}
		return obj;
	} 
	setEvent(param) {
		if(typeof(param,'string')) {
			switch(args().size()) {
			case 2:
				args(fnm, fc)
				thisNode=this
				reset=global('devMode')
			default:
				args(fnm, thisNode, fc, reset)
				not(reset) reset=global('devMode')
			}
			fn=this.get(fnm)
			if( fn && typeof(fn,'function')) {
				if(reset) {
					a=fn.eventFuncList()
					if(typeof(a,'array')) {
						a.reuse()
					}
				}
				if(fn.get('eventUse')) {
					if( typeof(thisNode,'node')) {
						fn.set("@this",thisNode)
						fn.set("sender",this)
					}
					not(fc) return;
				} else {
					not(typeof(fc,'func')) fc=fn
					fn=call(@widget.eventBase)
					this.set(fnm, fn)
					fn.set('eventUse', true)
					if( typeof(thisNode,'node')) {
						fn.set("@this",thisNode)
						fn.set("sender",this)
					}
				}
				if(typeof(fc,'func')) {
					fn.addFuncSrc(fc)
				}
			} else {
				fn=call(@widget.eventBase)
				if(typeof(thisNode,'node')) {
					fn.set("@this",thisNode)
					fn.set("sender",this)
				}
				fn.set('eventUse', true)
				if(typeof(fc,'func')) {
					fn.addFuncSrc(fc)
				}
				this.set(fnm, fn)
				print("@@@@@@@@@ set new event event $fnm @@@@@@@@@", fn.get() )
			}
			print("set event $fnm", fc)
			return;
		}
		if(typeof(param,'node')) {
			args(1,thisNode)
			not(thisNode) thisNode=this
			for(fnm, param.keys()) { 
				fc=param.get(fnm)
				if(typeof(fc,'func')) {
					this.setEvent(fnm, thisNode, fc)
				}
			}
		}
	}
	removeEvent(fnm, fc) {
		fn=this.get(fnm)
		not(typeof(fn,'func')) {
			print("$fnm 이벤트함수 정의되지 않음")
			return;
		}
		a=fn.eventFuncList()
		if(typeof(a,'array')) {
			if(fc) { 
				idx=a.find(fc)
				if(idx.ne(-1)) a.remove(fc)
			} else {
				a.reuse()
			}
		}
		print("$fnm remove event arr=", a)
		return a;
	}
	makePage(param) { 
		base=this.base() not(base) return print("페이지 생성실패 baseCode 미정의", args());
		prop='', subTag='';
		checkClass=false;
		switch(args().size()) {
		case 0:
			idx=global().incrNum("@pageIndex")
			id="page_$idx"
		case 1:
			args(id)
		case 2:
			args(id,props)
		default:
			args(id,props,subTag,checkClass)
		} 
		if( typeof(props,'bool')) {
			checkClass=props
			props=''
		}
		return @page.makePage(id,base,props,subTag,checkClass);
	}
	makeWidget() {
		checkClass=false, checkForm=false;
		props='', style='';
		switch(args().size()) {
		case 0:
			idx=global().incrNum("@formIndex")
			id="form_$idx"
			tag="canvas"
			checkForm=true
		case 1:
			args(id)
			tag='canvas'
			checkForm=true
			checkClass=true
		default:
			args(tag, id, props, style)
		} 
		if( typeof(props,'bool')) {
			checkClass=props
			props=''
		} else if( props) {
			not( props.findReg('[:=]+')) {
				rectid=props
				props="rectid:$rectid"
			}
		}  
		not(tag) return print("make widget error (태그가 정의되지 않았습니다)", args());
		base=this.base()
		map=object("layoutMap#${base}")
		obj=map.get(id)	if(obj) return obj;	
		obj=object("${tag}.${base}:${id}")
		
		if( props) obj.parseJson(props)
		obj.with(id, tag)
		this.createWidget(obj)
		obj.var(baseCode, "$base:$id")
		if( typeof(obj,'widget') ) {
			obj.hide()
			obj.flag(FLAG.set, true)
			if(style) obj.var(widgetStyle, style)
			class(obj, 'widget')
			if( checkForm ) {
				class(obj, 'formInfo')
				className="${id}Form" 
				if(Cf.getObject("class","$base:$className")) {
					class(obj,className,base)
				}
			}
			obj.var(baseName, base)
			this.addWidget(obj)
			map.set(id,obj)
		}		
		if( checkClass) {
			class(obj)
		}
		return obj; 
	}
	getNode(name) {
		code=this.id
		base=this.base() not(base) return print("data $code 노드 생성오류 위젯 base 미정의");
		not(name) name="temp"
		return _node("${base}.${code}:${name}");
	}
	getArray(param) {
		code=this.id
		base=this.base() not(base) return print("data $code 배열 생성오류 위젯 base 미정의")
		arr=_arr("${base}.${code}:${name}")
		if(param) {
			if(typeof(param,'num')) {
				args(2,info)
				return arr.recalc(param, info)
			}
			if(typeof(param,'array')) {
				arr.copy(param)
			} else if(typeof(param,'string')) {
				arr.copy(param.split(','))
			} else if(typeof(param,'bool')) {
				arr.reuse()
			}
		}
		return arr;
	}
	setFormRect() {
		fnParent=Cf.funcNode('pp')
		if(this.member(widgetList)) {
			while(widget, widgetList) {
				not(typeof(widget,'widget')) continue;
				if(typeof(widget.rect,'rect') ) {
					rc=widget.rect
				} else {
					not(widget.rectId) {
						widget.hide()
						continue;
					}
					rc=fnParent.get(widget.rectId)
					not(typeof(rc,'rect')) {
						widget.hide()
						// print("위젯 ${widget.tag}:${widget.id} 영역 ${widget.rectId} 이 설정되지 않았습니다");
						continue;
					}
				}
				widget.move(rc)
				if( widget.flag(FLAG.set) ) {
					widget.show()
				}
			}
		}
		if( typeof(this.updateButtons,'function') ) {
			this.updateButtons(fnParent)
		} 
	}
	
	initFormCheck(param) {
		not(this.member(widgetList)) return print("${this.id} 화면 자식위젯 미정의")
		if( typeof(param,"widget")) {
			parent=param
		} else {
			parent=this
		}	
		state='child'
		if( typeof(parent,'widget') ) {
			while(widget, widgetList) {
				style=widget.var(widgetStyle)
				if( style ) {
					widget.hide()
					if(style.eq('popup','splash','tool','tooltip')) {
						widget.parentWidget(null)
						widget.flags(style)
						widget.flag(FLAG.set, true)
					}
					continue;
				}
				if( eq(widget,parent) ) continue;
				if( eq(widget.parentWidget(),parent) ) continue;
				if( state ) {
					widget.flags(state)
					widget.flag(FLAG.set, true)
				}
				widget.parentWidget(parent)
				widget.hide()
			}
		}
	}
}

class page {
	initClass() {
		class(this, "widget");
	}
	positionSave() {
		page=this
		code=page.var(baseCode) not(code) return;
		page.geo().inject(x,y,w,h);
		y-=31;
		conf("pagePosition.${code}", "$x,$y,$w,$h", true)
	}
	positionLoad(w,h) {
		page=this
		code=page.var(baseCode) not(code) return;
		if(page.parentWidget()) {
			return;
		}
		s=conf("pagePosition.${code}")
		not(w) w=800
		not(h) h=600
		not(s) {
			return page.move( System.info("screenRect").center(w,h) );
		}
		s.ref();
		x=0, y=0;
		while(s.valid(),n) {
			not(s.ch()) break;
			v=s.findPos(",").trim()
			not(typeof(v,"num")) break;
			switch(n) {
			case 0: x=v.toInt();
			case 1: y=v.toInt();
			case 2: w=v.toInt();
			case 3: h=v.toInt();
			}
		} 
		rc=rc(x,y,w,h)
		while(n=0,System.info("screenCount")) {
			rcScreen=System.info("screenRect",n);
			if( rcScreen.contains(rc) ) {
				page.move(x,y).size(w,h);
				page.open();
				return;
			}
		}
		 
		page.move( System.info("screenRect").center(w,h) ); 
		page.open();
		page.active();
	}
	setLayout() {
		switch(args().size()) {
		case 1:
			args(param)
			box=this.child(0)
		case 2:
			args(box,param)
		default:
		}
		cur=null
		if(typeof(param,'string')) {
			if(param.ch('{')) {
				box.parseJson(param)
				while(cur,box) {
					if(typeof(cur,'widget','layout')) continue;
					box.addChild(cur)
				}
			} else {
				cur=box.addNode()
				if(param.find(':')) {
					cur.parseJson(param)
				} else {
					cur.tag=param
				}
				box.addChild(cur)
			}
		} else if(typeof(param,'widget','layout')) { 

		} else if(typeof(param,'node')) {
			if(param.tag) {
				cur=box.addNode()
				cur.copyNode(param,true)
				box.addChild(cur)
			} else if(param.childCount()) {
				while(row, param) {
					not(row.tag) continue;
					cur=box.addNode()
					cur.copyNode(row,true)
					box.addChild(cur)
				}
			}
		}
		return cur;
	}
}

class canvas {
	initClass() {
		class(this,'widget')
		this.setEvent('onDraw', this.draw)
		print("canvas class init")
	}
	draw(dc,rc) { 
		if( rc.eq(this.rect()) ) {
			not( rc.eq(this.rcBase) ) {
				this.rcBase=rc
				this.updateCanvas(dc, rc)
			} 
		}  
		this.drawCanvas(dc,rc) 
	}
	virtual updateCanvas(dc, rc) {
		print("update canvas 영역: $rc")
	}
	virtual drawCanvas(dc, rc) {
		dc.fill(randomColor())
		dc.text('draw canvas 함수 추가','center')
	}
}

class func:layout {
	@layout.add(box, param) {
		not(typeof(box,'node')) return;
		tag=box.tag
		if(tag.eq('page','dialog','main')) {
			box=box.child(0)
		}
		cur=null
		if(typeof(param,'string')) {
			cur=box.addNode()
			cur.tag=param
			box.addChild(cur)
		} else if(typeof(param,'node')) {
			node=param
			if(node.tag) {
				cur=box.addNode()
				cur.copyNode(node,true)
				box.addChild(cur)
			} else if(node.childCount()) {
				while(row, node) {
					not(row.tag) continue;
					cur=box.addNode()
					cur.copyNode(row,true)
					box.addChild(cur)
				}
			}
		}
		return cur;
	}
	@layout.insert(box, idx, param) {
		not(typeof(box,'node')) return;
		not(typeof(idx,'num')) return print('layout 추가오류 인덱스가 숫자가 아닙니다');
		cur=null
		if(typeof(param,'string')) {
			cur=box.insertNode(idx) not(cur) return;
			cur.tag=param
			box.addChild(cur,null,idx)
		} else if(typeof(param,'node')) {
			node=param
			if(node.tag) {
				cur=box.insertNode(idx) not(cur) return;
				cur.copyNode(node,true)
				box.addChild(cur,null,idx)
			} else if(node.childCount()) {
				while(row, node) {
					not(row.tag) continue;
					cur=box.insertNode(idx) not(cur) break;
					cur.copyNode(row,true)
					box.addChild(cur,null,idx)
					idx++;
				}
			}
		}
		
		return cur;
	}
	@layout.remove(box, idx) {
		not(typeof(box,'node')) return;
		size=args().size()
		if( size.eq(1)) {
			while(n=box.childCount()-1, 0) {
				cur=box.child(n)
				not(box.remove(cur)) break;
			}
			box.removeAll()
			return box;
		}
		cur=box.child(idx)
		not(typeof(cur,'node')) return;
		not(box.remove(cur)) return;
		return box.removeChild(cur)
	}
	@layout.filterBar(vbox) {
		node={
			tag:hbox, 
			{tag:input, id:filter}
			{tag:label, stretch:1}
			{tag:button, id:close, text:창닫기} 
		}
		return @layout.add(vbox, node)
	}
}
