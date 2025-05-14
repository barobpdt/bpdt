s=#[<><div class="chat-messages">
		effect[chat-list] 
		while(item) item.messageType=='recv' ? echo[chat_recv] : echo[chat_send]	
	</div>
	<>
]
parse(s)

~~
<func>
	parse(&s) {
		c=s.ch('<')
		if(c.eq('<')) {
			sp=s.cur()
			if( s.start('<>')) {
				ss=s.match('<>','<>')
				print("=======>",s,ss)
				prop=''
			} else {
				c=s.incr().next().ch()
				if(c.eq('.','-')) c=s.incr().next().ch()
				tag = s.trim(sp+1,s.cur())
				s.pos(sp)
				ss=s.match("<$tag","</$tag>")
				print("xxxx", tag, c,ss)
				prop=ss.findPos('>')
			}
			print("ss==$ss")
			parse(ss)
		} else {
			a=isPageVar(s)
			print(">>",a,s)
		}
		isPageVar = func(&s) {
			c=s.next().ch()
			return c.eq('[')
		}
	}
</func>
