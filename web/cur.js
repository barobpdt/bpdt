parseTagProps(param, &s) {
	param.inject(@tagStack, @srcStack)
	node=tagStack.get(0)
	not(node) 
	while(s.valid()) {
		c=s.ch()
		not(c) break;
		if(c.eq('/','>')) {
			if(c.eq('>')) s.incr()
			break;
		}
		sp=s.cur()
		c=s.next().ch()
		while(c.eq('-')) c=s.incr().next().ch()
		k=s.trim(sp,s.cur())
		not(k) break;
		not(c.eq('=')) {
			node.set(k,true)
			continue;
		}
		c=s.incr().ch()
		if(c.eq()) {
			node.set(k,s.match())
		} else if(c.eq('{')) {
		} else if(isVarCheck(s)) {
			ty=s.move()
			pp=s.match(1)
		} else {
			v=s.findPos(" />\t\n",4)
			node.set(k,v)
		}
	}
	return s.cur()
	isVarCheck = func(&s) {
		c=s.nect().ch()
		if(c.eq('[')) return true;
	};
}
parseTag(param, &s) {
	param.inject(@tagStack, @tagMap)
	while(s.valid() ) {
		c=s.ch()
		if(c.eq('<')) {
			sp=s.cur()
			c=s.incr().ch()
			if(c.eq('!','/')) {
				if(c.eq('!')) s.findPos('-->') else s.findPos('>');
				continue;
			}
			c=s.next().ch()
			if(c.eq('.','#','-')) {
				c=s.incr().next.ch()
			}
			ep=s.cur()
			tag=s.trim(sp+1,ep)
			node=tagMap.addNode()
			node.set('tag',tag)
			pushArray(tagStack, node)
			pushArray(scriptStack, '')
			ep=@react.parseTagProps(param,s)
			not(@react.isEndCheck(param,s,sp,ep)) break;
			s.pos(ep)
			c=s.ch()
			if(c.eq('/')) {
				s.findPos('>')
				continue;
			}
			s.pos(sp), tlen=tag.size()
			ss=s.match("<$tag","</$tag>") if(typeof(ss,'bool')) break;
			@react.parseTag(param, ss.pos(ep))
			tagStack.pop()
			src = scriptStack.pop()
		} else if(c.eq('{')) {
			
		} else if(@react.isVar(s)) {
			ep=@react.parseVar(param, s)
		}
	}
}
