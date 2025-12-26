/* 폼정보 처리 */
class formInfo {
	setFormInfo(formInfo, targetPage) {
		@targetPage=targetPage
		if(typeof(formInfo,'string')) {
			@formInfo=this.conf(formInfo, true)
		} else {
			@formInfo=formInfo
		}
		target=this
		not( target.var(widgetIndex) ) target.var(widgetIndex, 0)
		
		form=this.member('formInfo')
		not(typeof(form,'node')) {
			print("폼정보가 설정되지 않았습니다 (id=${target.id})")
			return;
		}
		this.setEvent('onDraw', this.drawLayout)
		this.setEvent('onMouseDown', this.formMouseDown)
		this.setEvent('onMouseUp', this.formMouseUp)
		this.setEvent('onMouseMove', this.formMouseMove) 
		this.setFormWidget(form)
		this.var(useForm, true)
		this.invalidate()
	} 
	getFormRows() {
		rows=this.var(formRows)
		not(rows) rows=formInfo.rows
		return rows;
	}
	virtual drawDetail(dc, rc) {
		dc.rectLine(rc, 0, )
	}
	applyTest(src) {
		ss="test: {$src}"		
		if(typeof(formInfo.test,'node')) {
			formInfo.test.removeAll(true)
		}
		formInfo.parseJson(ss)
		formInfo.inject(name, test)
		src=''
		target=this
		if(typeof(test,'node')) {
			while(fcName, test.keys()) {
				fsrc=test.get(fcName)
				src.add("${fcName}${fsrc}\r\n")
			}
		}
		not(src) return;
		target[$src]
		target.update()
		return target.test()
	}
	reloadForm(src, apply) { 
		checkFunc=func(&src) {
			not(src.ch()) return false;
			c=src.next().ch()
			if( c.eq('(')) {
				src.match()
				if(src.ch('{') ) return true;
			}
			return false;
		};
		checkRows=func(&src) {
			while(src.valid()) {
				src.findPos('rows') not(src.ch()) break;
				if(src.ch(':')) return true;
			}
			return false;
		};
		checkDraw=func(&src) {
			while(src.valid()) {
				src.findPos('draw') not(src.ch()) break;
				if(src.ch(':')) return true;
			}
			return false;
		};		 
		getTestFuncs=func(&src) {
			ss="testFunction: {"
			while(checkFunc(src)) {
				sp=src.cur()
				src.next().ch()
				src.match()
				if(src.ch('{')) {
					src.match(1)					
				}
				ep=src.cur()
				ss.add(src.value(sp,ep,true) )
				c=src.ch()
				not(c) break;
				if(c.eq(',',';')) src.incr()				
			}
			ss.add("}")
			if(src.ch() ) ss.add(src)
			return ss;
		};
		if( checkFunc(src)) {
			src=getTestFuncs(src)
		}
		drawUse=false
		if( checkDraw(src) ) drawUse=true
		if( checkRows(src)) {
			formInfo.rows.reuse()
		}
		target=this
		if( typeof(formInfo.testFunction,'node')) {
			formInfo.testFunction.reuse(true)
		}
		if( form.test ) form.test=''
		if( typeof(target.test,'func')) target.test=null
		formInfo.parseJson(src)
		formInfo.inject(name, draw, test, testFunction)
		if( this.var(formRows)) {
			rows=this.var(formRows)
			rows.reuse()
			rows.copyNode(formInfo.rows, true)
		}
		src=''
		if( drawUse && typeof(draw,'node')) {
			while(n=0,formInfo.rows.size()) {
				fsrc=draw.get("row$n")
				not(fsrc) continue;
				src.add("draw_${name}_${n}${fsrc}\r\n")
			}
		} 
		if( typeof(testFunction,'node')) {
			while(fcName, testFunction.keys()) {
				fsrc=testFunction.get(fcName)
				src.add("${fcName}${fsrc}\r\n")
			}
			if( test ) {
				src.add("test${test}\r\n")
			}
		}		 
		if(src) {
			formInfo.addArray("@testSouceList").add(src)
			target[$src]
			if( typeof(target.test,'func')) {
				target.test()
			}
		}
		this.hideAllWidget()
		this.setFormWidget(formInfo)
		this.invalidate()
	}
	setFormWidget() {
		target=this
		rows=this.getFormRows()
		while(row, rows) {
			if( row.widget ) {
				if(typeof(row.widget,'string')) {
					widget=target.get(row.widget)
				} else {
					widget=row.widget
				}
				if( typeof(widget,'widget')) {
					row.widgetObject=widget
					if(row.next) widget.var(nextFocus, row.next)
				}
			}
			while(cell, row) {
				not(cell.widget) continue;
				if( typeof(cell.widget,'string') ) {
					widget=target.get(cell.widget)
				} else {
					widget=cell.widget
				}
				if( typeof(widget,'widget')) {
					cell.widgetObject=widget
					if(cell.next) widget.var(nextFocus, cell.next)
				}
			}
		}
	}
	hideAllWidget() { 
		target=this
		idx=target.var(widgetIndex)
		rows=this.getFormRows()
		while(row, rows) {
			widget=row.widgetObject
			if(typeof(widget,'widget')) {
				widget.hide()
				row.widgetObject=null
			} 
			while(cell, row) {
				not(cell.widget) continue;
				widget=cell.widgetObject
				if(typeof(widget,'widget')) {
					widget.hide()
					cell.widgetObject=null
				}
			}
		}
	}
	drawLayout(dc,rc) { 
		if( rc.eq(sender.rect()) ) {
			not( rc.eq(this.rcBase) ) {
				this.rcBase=rc
				this.updateForm(dc, rc)
			} 
		}  
		this.drawForm(dc,rc)
		this.drawDetail(dc,rc)
	}
	updateForm(dc, rc) {
		not(formInfo) return;
		rows=this.getFormRows()
		target=this
		idx=target.var(widgetIndex)
		not(rows) {
			return print("formInfo 클래스 rows 정보 미정의");
		}
		not(rows.size()) { 
			return print("formInfo 클래스 rows 자식정보가 미등록");
		}
		rows.inject(rowInfo, margin, rowMargin, cellMargin, vbox)
		if(vbox) rowInfo=vbox not(rowInfo) rowInfo=formInfo.rowInfo
		not(margin) margin=formInfo.margin
		if( rowMargin) {
			while(row, rows) {
				if(isNull(row.margin)) row.margin=rowMargin;
			}
		}
      	rcForm=rc 
		if(isValid(margin)) rcForm.margin(margin)
		not(rowInfo) {
			while(row, rows, n) {
				if(n) rowInfo.add(',')
				if(row.height) rowInfo.add(row.height) else rowInfo.add('*');
			}
			not(rowInfo) rowInfo.add('*')
		}
		_setCell = func(cell, rcCell)  {
			cell.rect=rcCell
			margin=cell.margin
			if(cm && isNull(margin) ) margin=cm
			if(isValid(margin)) rcCell.margin(margin);
			widget=cell.widgetObject
			if(typeof(widget,'widget')) {
				return this.moveWidget(widget, rcCell)
			}
			cell.rcCell=rcCell
		};
		while(rcRow, vbox(rcForm, rowInfo), n) {
			row=rows.get(n)
			row.inject(margin, cellInfo, width, height, hbox) 
			if(hbox) cellInfo=hbox
			row.rect=rcRow 
			if(isValid(margin)) rcRow.margin(margin)
			if(width || height) {
				row.rcOrigin=rcRow
				rcRow.size().inject(rw,rh)
				if(width) rw=width
				if(height) rh=height
				rcRow.center(rw, rh)	
			}
			widget=row.widgetObject
			row.rcRow=rcRow
			if(typeof(widget,'widget')) {
				this.moveWidget(widget, rcRow)
				continue;
			}
			
			not(row.size()) continue;
			not(cellInfo) {
				dc.save()
				cellInfo=''
				while(cell, row, c) {
					cell.inject(tag, width, text, icon, style, margin)
					if(c) cellInfo.add(',')
					if(width) {
						cellInfo.add(width)
						continue;
					}
					if(style) dc.font(style)
					tw=0
					if(tag.eq('btn','lable','check')) {							 
						if(text) {
							dc.textSize(text).inject(tw)
							if( icon || tag.eq('check')) tw+=25; 
						} else {
							tw=30
						} 
					} else if(text) {
						dc.textSize(text).inject(tw)
					} else {
						cellInfo.add('*')
					}
					if(tw) {
						if( cellMargin && isNull(margin) ) margin=cellMargin
						gap=0
						if(typeof(margin,'array')) {
							size=margin.size()
							if(size<3) {
								gap=margin.get(0)*2;
							} else {
								gap=margin.get(0) + margin.get(3);
							} 
						} else if(typeof(margin,'num')) {
							gap=margin*2;
						}
						gap+=10;
						cellInfo.add(tw+gap)
					}
				}
				not(cellInfo.find(',')) cellInfo='*'
				row.cellInfo=cellInfo
				dc.restore()
			}
			cm=row.cellMargin
			not(cm) cm=cellMargin
			not(row.hbox) row.hbox=cellInfo
			while(rcCell, hbox(rcRow, cellInfo), c) { 
				_setCell(row.child(c), rcCell)
			}
		}
	}
	drawForm(dc,rc) { 
		not(formInfo) {
			dc.text('폼정보가 설정되지 않았습니다', 'center')
			return; 
		}
		formInfo.inject(name, bgColor)
		rows=this.getFormRows()
		not(bgColor) {
			bgColor=this.bgColor
			not(bgColor) bgColor=color('#D4D4B7A0')
			formInfo.set('bgColor', bgColor)
		}
		not(name) name="row"
		target=this
		dc.fill(bgColor)
		dc.mode()
		while(row, rows, r) {
			if(row.cmp('type','draw')) {
				fc=target.get("draw_${name}_${r}")
				if(typeof(fc,'func')) {
					dc.save()
					fc(dc, row.rect, row, bgColor)
					dc.restore()
					continue;
				}
			}
			while(cell, row, c) {				
				not(cell.tag) continue; 
				if(cell.flag(NODE.hide)) continue;
				switch(cell.tag) {
				case btn:
					this.drawBtn(dc, cell, bgColor)
				case label:
					this.drawLabel(dc, cell, bgColor)
				case check:
					this.drawCheck(dc, cell, bgColor)
				default:
				}
			}
		}
	}

	drawCheck(dc, cell, bg) {
		cell.inject(rcCell, icon, text, style, bgColor)
		rect=rcCell
		dc.save()
		if(bgColor) bg=bgColor
		if( cell==this.mousedownItem ) {
			dc.fill(rect, bg.darkColor(50) )
		} else if( cell==this.mouseoverItem ) {
			dc.fill(rect, bg.lightColor(150) )
		}
		if(text) {
			rcIcon=rect.leftCenter(16,16, 4)
			rect.incrX(24)
			dc.pen(bg.lightColor(200)).text(rect, text)
			dc.pen(bg.darkColor(200)).text(rect.incrXY(1,1), text)
		} else {
			rcIcon=rect.center(16,16)
		} 
		if(cell.checked ) {				
			dc.image(rcIcon, 'icons:check1')
		} else {
			dc.rectLine(rcIcon, 0, '#888', 2)
		} 
		dc.restore()
	} 
	drawLabel(dc, cell, bg) {
		cell.inject(rcCell, icon, text, style, bgColor)
		rect=rcCell
		dc.save()
		if(bgColor) {
			dc.fill(rect, bgColor)
			dc.rectLine(rect, 0, bgColor.darkColor(180)) 
		}
		if(icon) {
			if(text) {
				rcIcon=rect.leftCenter(16,16,4)
			} else {
				rcIcon=rect.center(16,16)
			}
			if(style) dc.font(style)
			dc.image(rcIcon, icon)
			if(text) { 
				rect.incrX(24)
				dc.text(rect, text)
			}
		} else if(text) {
			dc.text(rect, text)
		}
		dc.restore()
	}
	drawBtn(dc, cell, bg) {
		cell.inject(rcCell, icon, text, style, bgColor) 
		dc.save()		
		if(cell.flag(NODE.disable)) {
			bg=nvl(bgColor,bg.darkColor(120)) 
			lc=bg.darkColor(80)
			p1=bg.lightColor(200)
			p2=bg.darkColor(50)
		} else {
			bg=nvl(bgColor,bg.darkColor(50)) 
			if( cell==this.mousedownItem ) {
				bg=bg.darkColor(160)
			} else if( cell==this.mouseoverItem ) {
				bg=bg.lightColor(100)
			}
			lc=bg.darkColor(120)
			p1=bg.lightColor(200)
			p2=bg.darkColor(200)
		}
		dc.fill(rcCell, bg)
		dc.rectLine(rcCell, 0, lc)
		if(icon) {
			if(text) {
				rcIcon=rcCell.leftCenter(16,16,4)
			} else {
				rcIcon=rcCell.center(16,16)
			}
			dc.image(rcIcon, icon)
			if(text) {
				rcCell.incrX(24)
				dc.pen(p1).text(rcCell, text)
				dc.pen(p2).text(rcCell.incrXY(1,1), text)
			}
		} else {
			dc.pen(p1).text(rcCell, text,'center')
			dc.pen(p2).text(rcCell.incrXY(1,1), text,'center')
		}
		dc.restore()
	}
	isCheck(id) {
		not(formInfo) return;
		node=findId(formInfo.rows, id);
		if(node && node.checked ) return true;
		return false;
	}
	formMouseDown(pos) {
		not(formInfo) return;  
		_check = func(cell) {
			cell.inject(tag, rect);
			not(tag.eq('btn','check')) continue;
			not(rect) continue; 
			if(rect.contains(pos)) {
				this.mousedownItem=cell;
				this.update()
				return true;
			}
			return;
		};
		rows=this.getFormRows()
		while(row, rows, r) {
			not( row.size()) {
				if(_check(row)) return true;
				continue;
			}
			while(cell, row, c) {
				if(cell.flag(NODE.disable|NODE.hide)) continue;
				if(_check(cell)) return true;
			}
		} 
	}
	formClickButton(cur) {
		id=cur.id;
		target=null
		if(targetPage) {
			if(targetPage!=this) {
				target=targetPage
			}
		}
		if(target) {
			fc=target.get("click_$id")
			if(typeof(fc,'func')) {
				call(fc,target,cur)
			}
		} else {
			fc=this.get("click_$id")
			if(typeof(fc,'func')) {
				return fc(cur, target)
			} 
		}
	}
	formMouseUp(pos) {
		cur=this.mousedownItem
		not(cur) return;
		if( cur.rect.contains(pos)) {
			if( cur.cmp('tag','check') ) {
				cur.toggle('checked')
			}
			this.formClickButton(cur)
		}
		this.mousedownItem=null
		this.update()
	}
	formMouseMove(pos) {
		if(this.mousedownItem) return;
		not(formInfo) return; 
		_check=func(cell) {
			cell.inject(tag, rect, tip );
			not(tag.eq('btn','check')) continue;
			not(rect) continue; 
			if(rect.contains(pos)) {
				if(tip) {
					not(cell.useTooltip) this.tooltip(tip, true)
					cell.useTooltip=true
				}
				if(cell==this.mouseoverItem ) return true;
				// mainPage.var(mouseOverTick, System.tick())
				this.mouseoverItem = cell
				this.overRect=rect
				this.cursor(CURSOR.PointingHandCursor)
				this.update()
				return true;
			} else if(cell.useTooltip) {
				this.tooltip('',false)
				cell.useTooltip=false
			}
			return;
		};
		rows=this.getFormRows()
		while(row, rows, r) {
			not( row.size()) {
				if(_check(row)) return true;
				continue;
			}
			while(cell, row, c) {
				if(cell.flag(NODE.disable|NODE.hide)) continue;
				if(_check(cell)) return true;
			} 
		} 
		if( this.mouseoverItem ) {
			this.mouseoverItem = null
			this.cursor(0)
			this.update()
		} 
	}
	formMouseOverCheck(pos) {
		not(pos) pos=this.cursorPos()
		rc=this.mapGlobal(this.rect()) 
		not( rc.contains(pos) ) { 
			this.mouseoverItem = null
			this.cursor(0)
			this.update()
			// mainPage.var(mouseOverTick, null)
		}
	} 
	moveWidget(widget, rect ) {
		not(typeof(widget,'widget')) return;
		not( widget.parentWidget() ) {
			widget.flag(FLAG.set, true)
			widget.hide()
			widget.parentWidget(this)
		}
		widget.move(rect)
		if( widget.flag(FLAG.set) ) {
			widget.show()
		}
		widget.rcClient=rect
	}
	cellProp() {
		asize=args().size()
		rows=this.getFormRows()
		switch(asize) {
		case 1:
			args(id)
			return findId(rows, id)
		case 2:
			args(id, text)
			obj=findId(rows,id)
			if(obj) obj.text=text
			this.update()
			return obj;
		default:
		}
		args(row,col,type,value)
		rowNode=rows.get(row)
		not(col) col=0
		cur=null
		size=rowNode.size()
		if(size==0) {
			if(col==0) cur=rowNode
		} else {
			if(col<0) {
				tmp=col
				col=size-tmp;
			} else if(col>=size ) {
				col=size-1;
			}
			cur=rowNode.get(col)
		}
		not(typeof(cur,'node')) return;
		 
		if( typeof(type,'widget')) {
			idx=this.var(widgetIndex)
			if(asize.eq(3)) return cur.get("widget_$idx")
		} else {
			if(asize.eq(3)) return cur.get(type) else cur.set(type,value);
		}
		if(asize.eq(5)) this.invalidate() else this.update();
		return cur;
	}
	setNextFocus() {
		callback=this.inputNextFocus 
		not(typeof(callback,'func')) return print("setNextFocus 이벤트 콜백함수 미정의", this.inputNextFocus)
		arr=args()
		target=this
		idx=target.var(widgetIndex)
		not(arr.size()) {
			rows=this.getFormRows()
			while(row, rows) {
				widget=row.widgetObject
				if(typeof(widget,'widget')) {
					 if(widget.var(nextFocus)) arr.add(widget)
				} 
				while(cell, row) {
					not(cell.widget) continue;
					widget=cell.widgetObject
					if(typeof(widget,'widget')) {
						 if(widget.var(nextFocus)) arr.add(widget)
					}
				}
			}
		}
		while(input, arr) {
			if(typeof(input,'widget')) {
				input.setEvent('onKeyDown', this, callback)
			}
		} 
	}
	inputNextFocus(k,a) {
		if( typeof(this.keydownCallback,'func')) {
			if( this.keydownCallback(k,a)) return true;
		}
		not(k.eq(KEY.Enter, KEY.Return, KEY.Tab)  ) return;
		input=sender
		name=input.var(nextFocus)
		not(name) return;
		next=this.get(name) 
		if( typeof(next,'widget')) {
			if( this.var(focusDelay)) {
				this.var(focusWidget, next)
				this.timer(250, func() { this.var(focusWidget).focus() })
			} else {
				next.focus()
			}
			return true
		}
	}
	openTool(parent, title) {
		// this.member(targetPage, parent)
		target=this
		target.parentWidget(parent)
		target.flags('tool', true)
		if(title) target.title(title)
		target.open()
		target.active()
		return target;
	}
	invalidate(tickUse) {
		if(tickUse && this.var(updateTick) ) {
			dist=System.tick() - this.var(updateTick);
			if(dist<500 ) return;
		}
		this.rcBase=null
		this.update()
		this.var(updateTick, System.tick())
	}
	setCurrentNode(cur) {
		node=this.addNode('@currentNode')
		node.flag(0,true,true)
		if(typeof(cur,'node')) node.copyNode(cur, true)
		return node;
	}
	getCurrentNode() {
		return this.addNode('@currentNode');
	}
	closePrevCheck(closeFlag) {
		prev=this.var(parentForm) 
		if( prev && typeof(prev.popupClosed,'func') ) {
			prev.popupClosed(this)
		}
		if(closeFlag) {
			this.close()
		}
		return true;
	}
	parentTag(tag) {
		p=this.parentWidget()
		if(p) {
			if(tag) return p.cmp('tag',tag);
			return p.tag;
		}
		return;
	}
	activeForm(input) {
		this.active()
		if( typeof(input,'widget')) {
			input.focus()
		}
	}
}

class func {
	@form.target(code, idx) {
		formCode="formInfo.$code"
		root=Cf.rootNode("@newNode")
		not(root.isVar(formCode)) return;
		formNode=root.get(formCode)
		not(formNode.isVar("@targetList")) return;
		list=formNode.addArray("@targetList") not(list.size()) return;
		if(args().size()==1) {
			idx=list.size()-1;
		} else {
			not(typeof(idx,'num')) idx=0;
			if( idx>list.size() ) idx=list.size()-1;
			if( idx<0 ) idx=0;
		}
		return list.get(idx)
	}
	@form.makeWidget(tag, varName, target) {
		not(target) target=this
		idx=target.var(widgetIndex)
		form=target.formInfo
		not(typeof(form,'node')) return print("form makeWidget 오류 폼정보 미정의")
		name=form.name
		id="${name}_${varName}"
		if(typeof(idx,'num')) {
			id.add("_${idx}")
		} 
		return target.makeWidget(tag, id)	
	}
	@form.get(code, target) {
		formNode=_node("formInfo.$code") 
		list=formNode.addArray("@targetList")		
		if( typeof(list,'array') && list.size()) {
			last=list.size()-1;
			form=list.get(last)
		} else {
			not(target) target=this
			form=@form.load(code, target)
		}
		return form
	}
	@form.getVisible(code, targetPage) {
		formNode=_node("formInfo.$code") 
		list=formNode.addArray("@targetList")		
		form=null
		while(cur, list) {
			not(typeof(cur,'widget')) continue;
			if(cur.is('visible')) continue;
			form=cur;
		}
		not(form) form=@form.load(code, targetPage)
		return form;
	}
	@form.open(code, targetPage, title) {
		form=@form.get(code)
		if(form ) form.openTool(targetPage, title)
		return form;
	}
		
	@form.load(code, targetPage) {
		not(typeof(targetPage,'widget')) return print("$code 폼추가 대상위젯 미정의")
		form=targetPage.makeWidget()
		not(typeof(form,'widget')) return print("$code 폼추가 오류 위젯 생성실패")
		formNode=_node("formInfo.$code") 
		list=formNode.addArray("@targetList")
		idx=list.size()
		print("form load $code index==$idx")
		if( idx ) {
			prev=list.get(0)
			form.copyNode(prev, 'class');
			form.initFormInfo=null
			src=@form.source(formNode, code, true)
			rows=form.addNode("@formRows", true)
			rows.copyNode(formNode.rows, 'form');
			form.var(formSource, src)
			form.var(widgetIndex, idx)
			form[$src]
		} else {			
			src=stripJsComment(conf("formInfo.$code"))
			formNode.parseJson(src);
			not( typeof(formNode.rows,'node') ) {
				return print("폼레코드 정보가 없습니다 (폼:$formNode)")
			}
			src=@form.source(formNode, code)
			form.var(formSource, src)
			form.var(widgetIndex,0)
			if(src) {
				form[$src]
				formNode.var(useForm, true)
			}
		} 
		list.add(form)
		name=formNode.name
		fc=form.get("init_${name}");
		not( typeof(fc,'func')) {
			src=#[
				init_${name}() { this.alert("폼정보 생성오류 init_${name} 함수를 등록하세요") }
			]
			target[$src]
		}
		form.var(formCode, code)
		form.initFormInfo(formNode)
		return form;
	}
		
	@form.loadSource(code, targetPage, src, idx) { 
		not(typeof(targetPage,'widget')) return print("$code 폼추가 대상위젯 미정의")
		not(src) return print("$code 폼소스가 없습니다") 
		not(idx) idx=1
		form=targetPage.makeWidget() not(typeof(form,'widget')) return print("$code 폼추가 오류 위젯 생성실패")
		form.var(formCode, code)
		form.var(widgetIndex, idx)
		formNode=_node("formInfo.${code}_${idx}")  
		formNode.parseJson(stripJsComment(src));
		not( typeof(formNode.rows,'node') ) {
			return print("폼레코드 정보가 없습니다 (폼:$formNode)")
		}
		src=@form.source(formNode, code, false, idx)
		if(src) {
			form[$src]
			formNode.var(useForm, true)
		} 
		name=formNode.name
		fc=form.get("init_${name}");
		not( typeof(fc,'func')) return print("폼정보 생성오류 init_${name} 함수를 등록하세요")
		form.initFormInfo(formNode)
		return form;
	}
	@form.makeWidget(target, tag, varName) {
		idx=target.var(widgetIndex)
		form=target.formInfo
		not(typeof(form,'node')) return print("form makeWidget 오류 폼정보 미정의")
		name=form.name
		id="${name}_${varName}"
		if(typeof(idx,'num')) {
			id.add("_${idx}")
		}
		return this.makeWidget(tag, id)
	}
	@form.source(formNode, code, checkInit, idx) {
		not(idx) {
			idx=formNode.addArray("@targetList").size()
		}
		formNode.inject(name,widget,size,flags)
		not(name) {
			name=formNode.set('name',code)
		}
		widgets=''
		sub=''
		if( size && size.find('x')) {
			size.split('x').inject(w,h)
			sub="form.size($w,$h)"
		}
		if(flags) {
			sub.add("\r\nform.parentWidget(null)\r\nform.flags('${flags}',true)")
		}
		if(typeof(widget,'node')) {
			while(varName, widget.keys()) {
				tag=widget.get(varName)
				switch(tag) {
				case div:
					widgets.add("@${varName}=this.makePage('${name}_${varName}_${idx}','margin:0,spacing:0','div')\r\n")
				case splitter:
					widgets.add("@${varName}=this.makePage('${name}_${varName}_${idx}','margin:0,spacing:0','splitter')\r\n")
				case tabs:
					widgets.add("@${varName}=this.makePage('${name}_${varName}_${idx}','margin:0,spacing:0','div')\r\n")
				case form:
					widgets.add("@${varName}=@form.load('${varName}',this)\r\n")
				default:
					widgets.add("@${varName}=this.makeWidget('${tag}','${name}_${varName}_${idx}')\r\n")
				}
			}
		}
		src=#[
			initFormInfo(formNode) {
				form=this
				${widgets}
				${sub}
				form.setFormInfo(formNode)
				form.init_${name}()
			}
		]
		if(checkInit) return src;
		formNode.inject(draw, memberFunction)
		if(typeof(draw,'node')) {
			while(n=0,formNode.rows.size()) {
				fsrc=draw.get("row$n")
				not(fsrc) continue;
				src.add("draw_${name}_${n}${fsrc}\r\n")
			}
		}
		if(typeof(memberFunction,'node')) {
			while(fcName, memberFunction.keys()) {
				fsrc=memberFunction.get(fcName)
				src.add("${fcName}${fsrc}\r\n")
			}
		} 
		return src;
	}
}

class divForm {
	initClass() {
		this.setEvent('onResize', this.resizeDiv)
	}
	resizeDiv() {
		cur=this.current()
		if( cur && cur.var(useForm) ) {
			cur.invalidate(true)
		}
	}
}