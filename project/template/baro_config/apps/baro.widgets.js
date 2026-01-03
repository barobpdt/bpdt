##> func { note=공통 위젯함수 }
	initWidget() {
		sty=object('font.bold')
		sty.weight='bold'
	}
	@widget.create(tag, id, props, target) {
		not(id) {
			idx=global().incrNum('@widgetCreateIndex')
			id="${tag}_${idx}"
		}
		not(target) target=Cf.funcNode().get('@this')
		base=''
		if(typeof(target,'node')) {
			splitSep(target.var(baseCode),':').inject(base,pid)
		}
		not(base) base='common'
		widget=object("${tag}.${base}:${id}")
		if(typeof(props,'string')) {
			widget.parseJson(props)
		}
		widget.var(baseCode,"${base}:${id}")
		Cf.createWidget(widget)
		return widget;
	}
	@widget.eventSender(tag) {
		fn=Cf.funcNode('parent')
		sender=fn.get('@sender')
		not(typeof(sender,'node')) sender=fn.get('@this')
		if(tag) 
			return when(tagCheck(sender,tag), sender);
		else
			return sender;
	}
	@widget.setParent(widget, parent) {
		if( parent != widget.parentWidget()) {
			print("@@@@@ widget move parent change @@@@@@", widget.id, rect)
			widget.flags("child"); 
			widget.parentWidget(parent);
		}
	}
	
	@tree.drawDefault(dc, node, index, state, over) { 
		rc=@tree.drawSelect(dc, dc.rect(), state, over)
		fa=this.fields()
		field=fa.child(0).get('field')
		node.rcIcon=rc.moveLeft(18,18,-2,0,true)
		text=node.get(field)
		// dc.textSize(text).inject(tw, th)
		dc.text(rc, text)
	}
	@tree.drawSelect(dc, rc, state, over) {
		if( state & STYLE.Selected ) {
			rcBk=rc.x(0,true)
			dc.fill( rcBk, '#c0c0a090' );
			return rc;
		}
		dc.fill( rc.x(0,true) );
		if( state & STYLE.MouseOver ) {
			dc.rectLine(rc, 0, '#aaa', 1, 'dot')
		} 
		return rc;
	}
	@tree.drawTreeIcon(dc, rcIcon, state ) {
		if( state & STYLE.Open ) {
			dc.image( rcIcon.center(14,16).incrY(2), 'tree:plus' );
		} else {
			dc.image( rcIcon.center(14,16).incrY(2), 'tree:minus' );			
		}
	}
	
	@grid.drawState(dc, node, state, index, field) {
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
	
	@grid.drawHeader(dc, text, index, order) {
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
	@grid.fullWidth(param) {		
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