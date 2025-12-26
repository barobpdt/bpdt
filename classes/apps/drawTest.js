class layout {
	<page id="main" margin="0" onInit() {class(this,'main',true)}>
		<context id="titleBar" height="40">
		<splitter id="sep">
	</page>
	<page id="canvasTest" onInit() {class(this, 'canvasTest', true)}>
		<canvas id="c">
	</page>

	<input id="filter">
		onFocusIn() { 
			this.timer(50, func() { this.select() })
		}
	</input>
	<combo id="cbType"></combo>
}

class main {
	initClass() {
		base=this.base()
		if(base) {
			apiClient.start(base)
			apiClient.setTarget(base, this)
		}
		@page.spacing(this, 2)
		print("=== func editor page init class ok ===", this)
	}
	onClose() {
		this.positionSave()
	}
}

class canvasTest {
	canvas=this.get('c') 
	inputFilter=this.getWidget('filter')
	cbType=this.getWidget('cbType')
	params=canvas.addNode('params')
	initClass() {
		inputFilter.flags('child')
		inputFilter.parentWidget(canvas)
		cbType.flags('child') 
		cbType.parentWidget(canvas)
		items=cbType.parseJson(#[[
			{type:hline,name:수평선}
			{type:vline,name:수직선}
		]])
		cbType.addItem(items,'type,name')
		class(canvas,'widget')
		canvas.setEvent('onDraw', this.draw, this)
		canvas.setEvent('onDraw', this.drawLines, this)
		canvas.setEvent('onMouseDown', this.mouseDown, this)
		canvas.setEvent('onMouseWheel', this.mouseWheel, this)
		inputFilter.setEvent('onKeyDown', this.keydown, this)
		params.rate=1
		@page.margin(this, 4)
		this.positionLoad()
		this.onResize()
	}
	onResize() {
		rc=canvas.rect()
		rcInput=rc.rightBottom(150,30,-10,-10)
		rcCombo=rc.leftBottom(120,30,4,-4)
		inputFilter.move(rcInput)
		cbType.move(rcCombo)
	}
	onClose() {
		this.positionSave()
	}
	draw() {
		args(dc,rc)
		rcCanvas=canvas.rect()
		not(rcCanvas.eq(rc)) return;
		dc.fill('#fff')
		name=canvas.var(imageName) 
		if(name) {
			params.inject(rate)
			img=mdc(name)
			if(img) {
				img.rect().inject(x,y,w,h)
				if(rate.gt(0)) {
					w*=rate;
					h*=rate;
				}
				dc.image(rc(x,y,w,h), img)
			}
		}
	}
	drawLines() {
		args(dc,rc)
		params.inject(rate)
		while(cur, params) {
			cur.inject(type, px, py, rect)
			switch(type) {
			case hline:
			case vline:
			default:
			}
			
		}
	}
	keydown() {
		args(k,a)
		if(k.eq(16777220, 16777221)) {
			this.keyEnter(target)
		}
	}
	keyEnter(input) { 
		val=input.value().trim()
		not(val) return;
		if(val.find(':')) {
			params.parseJson(val)
			if( params.name) {
				canvas.var(imageName, params.name)
			}
		} else {
			if(val.eq('width','height')) {
				name=canvas.var(imageName)
				if(name) {
					img=mdc(name)
					imgSize(img).inject(iw,ih)
					imgSize(canvas).inject(cw, ch)
					if(val.eq('width')) {
						rate=iw/cw;
					} else {
						rate=ih/ch;
					}
					params.rate=rate
				}
			} else if(val.eq('start','end')) {
				cbType.current().inject(type,name)
				imgSize(canvas).inject(pw,ph)
				px=0,py=0
				if(type.eq('vline')) {
					
				} else {
					if(val.eq('start')) {
						ph=4;
					}
				}
			} else {
				canvas.var(imageName, val)
			}
		}
		print("keyEnter params==$params")
		canvas.redraw()
	}
	mouseDown() {
		args(p,a)
		p.inject(px, py)
		cur=cbType.current()
		if(cur) {
			type=cur.type;
			params.addNode().with(type, px, py)
		}
	}
	mouseWheel() {
		args(p,a,b)
		if(b&KEY.ctrl) {
			if(a>0) {
				params.rate+=0.25;
				if(params.rate.gt(5)) params.rate=5;
			} else {
				params.rate-=0.25;
				if(params.rate.lt(0.25)) params.rate=0.25;
			}
			canvas.redraw()
		}
	}
}

class func {
	
}