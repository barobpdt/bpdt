class form {
	initClass() {
		print("xxxxxxxxxxx form init xxxxxxxxxxxx", this, this.var(classNames))
		@buttons=this.getNode('buttons') 
		this.setEvent('onDraw', this.drawLayout)
	}
	drawLayout(dc,rc) {
		not(typeof(sender,'widget')) {
			print("draw layout 오류 sender 객체오류")
			return;
		}
		if( rc.eq(sender.rect()) ) {
			rcBase=this.member(rcBase)
			not( rc.eq(rcBase) ) {
				@rcBase=rc
				this.updateForm(rc)
			}
			this.drawForm(dc,rc)
		} else {
			this.drawSub(dc,rc)
			return 'ignore'
		}
	}	
	virtual updateForm(rc) {
		@rcBase=rc 
		this.setFormRect()
	}
	virtual drawForm(dc,rc) {
		dc.fill('#fff')
		dc.text("draw form implements")
	}
	virtual drawSub(dc,rc) {
		dc.fill('#fff')		
	}
	formFont(fontInfo) {
		not(fontInfo) fontInfo='size:10, weight:normal'
		this.font(fontInfo)
	}
	updateButtons(fn) {
		while(btn, buttons ) {
			not(btn.rectId) continue;
			btn.rectClient=fn.get(btn.rectId)
		}
	}
	getButtonWidth() {
		ss=''
		while(cur, buttons) {
			if(ss) ss.add(',')
			if(cur.isVar('space')) {
				ss.add('*')
			} else {
				tw=this.textSize(cur.text) + 20;
				ss.add(tw)
			}
		} 
		return ss;
	}
	setButtonEvent() {
		this.setEvent('onMouseDown', this.mouseDownButtons)
		this.setEvent('onMouseUp', this.mouseUpButtons)
		this.setEvent('onMouseMove', this.mouseMoveButtons)
		this.setEvent('onDraw', this.drawButtons)
	} 
	addButtons(params) {
		if(typeof(params,'array')) {
			args(arr, color)
			while(cur, arr) {
				if( typeof(cur,'node')) {
					buttons.addNode().copyNode(cur)
				}
			}
		} else {
			args(ids, names, color)
			texts=names.split() 
			while(id, ids.split(), n ) {
				if(id.eq('*')) {
					buttons.addNode().with(space:true)
					continue;
				}
				rectId="rc$id"
				text=texts.get(n)
				buttons.addNode().with(id, rectId, text) 
			}
		}
		not(buttons.childCount()) return;
		not(color) color=randomColor().lightColor(50)
		this.btnColor=color
		this.setButtonEvent()
	}
	drawButtons(dc,rc) { 
		while(btn, buttons) {
			btn.inject(id, text, rectClient)
			not(typeof(rectClient,'rect')) continue;
			c=this.btnColor
			if( rectClient.eq(this.buttonDownRect) ) {
				c=c.darkColor(50)
			} else if( rectClient.eq(this.buttonOverRect)) {
				c=c.lightColor(40)
			} 
			rcBtn=rectClient.incrXW(2,2)
			dc.font('size:10, weight:normal')
			dc.fill(rcBtn, c)
			dc.rectLine(rcBtn, 0, '#aaa')
			dc.font('color:#eee')
			dc.text(rcBtn, text, 'center')
		}
	}
	mouseDownButtons(pos) {
		while(btn, buttons) {
			btn.inject(id, rectClient)
			not(typeof(rectClient,'rect')) continue;
			if(rectClient.contains(pos)) {
				this.buttonDownRect=rectClient
				this.buttonDownId=id
				this.redraw()
				return true;
			}
		}
	}
	mouseUpButtons(pos) {
		not(this.buttonDownRect) return;
		if( this.buttonDownRect.contains(pos)) {
			id=this.buttonDownId
			if(typeof(this.buttonClick,'function')) {
				this.buttonClick(id)
			} else {
				fnm="click$id"
				fc=this.get(fnm)
				if(typeof(fc,'function')) {
					call(fc,this)
				} else {
					print("$id 버튼이벤트 미등록")
				}
			}
			this.buttonDownRect=null
			this.redraw()
		}
	}
	mouseMoveButtons(pos) {
		while(btn, buttons) {
			btn.inject(id, rectClient)
			not(typeof(rectClient,'rect')) continue;
			if( rectClient.contains(pos)) {
				eq=eq(rectClient,this.buttonOverRect)
				not(eq) {
					this.buttonOverRect=rectClient
					this.redraw()
				}
				this.cursor(CURSOR.PointingHandCursor)
				return true;
			}
		}
		if( this.buttonOverRect ) {
			this.buttonOverRect=null
			this.cursor(0)
			this.redraw()
		}
	}
}