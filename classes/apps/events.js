class eventSet {
	isEventFunc(fnm) {
		fn=this.get(fnm) ty=typeof(fn)
		if(ty.eq('func')) {
			return when(fn.isPersist(), true, false);
		}
		src=#[
			${fnm}=event() {
				fn=Cf.funcNode()
				if(fn.eventFuncList()) {
					fn.callFuncSrc()
				}
			}
		]
		this[$src]
		fn=this.get(fnm)
		return when(fn.isPersist(), true, false);
	}
	setEvent(e,fc) {
		if(typeof(e,'string')) {
			fnm=e
			if(this.isEventFunc(fnm) ) {
				fn.addFuncSrc(e.get(fnm))
			} 
		} else if(typeof(e,'node')) {
			for(fnm, e.keys()) { 
				if( this.isEventFunc(fnm) ) {
					fn.addFuncSrc(e.get(fnm))
				}
			}
		}
	}
}

class eventScroll {
	onMouseDown(p,a) {
		if(a.eq(KEY.ctrl) ) {
			not(this.mouseDownTick) {
				p.inject(px, py)
				this.member(mx,px)
				this.member(my,py)
				print("mouse down ", px, py)
				this.mouseDownTick=System.tick()
				return
			}
		}
	}
	onMouseUp(p,a) {
		this.mouseDownTick=0;
		print("mouse up", this.mouseDownTick)
	}
	onMouseMove(p,a) {	
		if(this.mouseDownTick ) {
			p.inject(px, py)
			this.member(sx, mx-px)
			this.member(sy, my-py)
			this.redraw()
		}
	}
}
