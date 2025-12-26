class grid { 
	setDefault(target, callback) {  
		this.setEvent('onDraw', this.drawDefault )
	}
	setInput(skipTimer) {
		if(this.member(input) ) return input;
		input=this.makeWidget('input',"input_${this.id}")
		input.parentWidget(null)
		if( checkTimer) {
		} 
		input.flags('child', true)
		input.parentWidget(this)
		if(skipTimer) {
			return input;
		}
		this.setEvent('onTimer', func() {
			if( this.var(editStartTick)) {
				this.var(editStartTick,0)
				this.inputFocus()
			}
		})
		this.timer(1200)
		this.member(input, input)
		return input;
	}
	edit(node, field) {
		not(this.member(input)) this.setInput()
		hh=this.headerHeight()+2;
		rc=this.nodeRect(node,field).incrY(hh, true)
		input.value(node.get(field))
		input.move(rc.margin(4,3,4,1))
		input.show()
		this.var(currentEditField, field)
		this.var(editNode, node)
		this.var(editStartTick, System.tick())		
	}
	inputValue() {
		return input.value()
	}
	inputHide(update) {
		if(update) {
			field=this.var(currentEditField)
			node=this.var(editNode)
			prev=node.get(field), val=input.value()
			not(eq(prev,val) ) {
				node.set(field,val)
				node.flag(NODE.modify, true)
				this.update()
			}
		}
		input.hide()
	}
	inputFocus() {
		input.focus()
	}
	virtual drawGrid(dc, rc, node, field) {
		dc.text(rc, node.get(field));
	}
	virtual drawDefault(dc, node, index, state) {
		field=this.field(index)
		rc=this.drawState(dc, node, state, index, field );
		this.drawGrid(dc, rc, node, field)
	}
	virtual drawState(dc, node, state, index, field) {
		last=this.columnCount()-1;
		rc=dc.rect();
		clr=this.var(bgColor)
		not(clr) clr=color("#c96");
		ty=when(last.eq(idx), 24, 234);
		if( state & STYLE.Selected ) {
			if(field.eq('chk','status')) {
				dc.rectLine(rc,3, clr.lightColor(120), 1 );
			} else {
				dc.fill(rc, clr.darkColor(150) );
				dc.rectLine(rc,3, clr.lightColor(120), 1 );
				dc.pen(clr.lightColor(220));
			}
		} else if( state & STYLE.MouseOver ) {
			dc.fill(rc,'#def' );
			dc.pen(clr.darkColor(150));
			dc.rectLine(rc,ty);
		} else {
			dc.fill(rc,'#ffffff');
			dc.rectLine(rc,ty,'#ddd');
			dc.pen(clr.darkColor(200));
		}
		return rc.incrX(4);
	}
	virtual drawHeader(dc, text, index, order) {
		rc=dc.rect(), fields=this.fields();
		last=fields.childCount()-1;
		if(last.eq(index)) {
			dc.rectLine(rc, 4,'#a0a0a0');
		} else {
			dc.rectLine(rc, 34,'#a0a0a0');
		}
		if( index.eq(sortIndex) ) {
			if(order) {
				icon="vicon.bullet_arrow_up";
			} else {
				icon="vicon.bullet_arrow_down";
			}
			rcIcon=rc.rightCenter(16,16,-5);
			dc.text(rc.incrX(10), text );
			dc.image(rcIcon, icon);
		} else {
			dc.text(rc, text, 'center');
		}
	} 
	fullWidth(param) {		
		grid=this 
		grid.size().inject(gw)
		if( grid.scrollValue('v') ) gw-=28;
		else gw-=2;
		num=grid.fields().childCount() not(num) return;
		if( typeof(param,'bool') ) {
			if(param) {
				while(w,recalc(gw,num),c) {
					grid.headerWidth(c, w, 'interactive')
				}
			} else {
				grid.headerWidth(recalc(gw,num) )
			}
		} else if( param.eq('resizeToContent') ) {
			while(n=0,num) {
				grid.headerWidth(n, 'resize')
			}
		} else if( param) {				
			grid.headerWidth(recalc(gw,param))
		} else {
			ss=''
			while(cur,grid.fields(),n) {
				if(n) ss.add(',')
				if(cur.width) {
					ss.add(cur.width)
				} else {
					ss.add('*');
				} 
			}
			a=recalc(gw, ss)
			grid.headerWidth(a)
		}
	}
}
 
 