class tree {
	setDefault() {
		this.setEvent('onDraw', this.drawDefault)
	}
	virtual drawDefault(dc, node, index, state, over) { 
		fa=sender.fields()
		rc=this.drawSelect(dc, dc.rect(), node, col, state, over)
		field=fa.child(0).get('field')
		node.rcIcon=rc.moveLeft(18,18,-2,0,true)
		text=node.get(field)
		dc.textSize(text).inject(tw, th)
		dc.text(rc, text)
	}
	drawSelect(dc, rc, node, col, state, over) {
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
	drawIcon(dc, rcIcon, state ) {
		if( state & STYLE.Open ) {
			dc.image( rcIcon.center(14,16).incrY(2), 'tree:plus' );
		} else {
			dc.image( rcIcon.center(14,16).incrY(2), 'tree:minus' );			
		}
	}
	treeDrag() {
		args(type, mime)
		return 'accept';
	}
	treeDrop() {
		args(mine, pos)
	}
	treeDragMove() {
		args(pos, mime)
	}
}
 
 