<!DOCTYPE html>
<html lang="ko">
<head>
	<meta charset="UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1.0">
	<title>${title}</title>
	<link rel="stylesheet" href="/css/styles.css">
	<link rel="stylesheet" href="/css/emoji-api.css">
	<script src="/js/common.js"></script>
	<script src="/js/jquery.js"></script>
	<!-- Ace Editor CDN -->
</head>
<body>
	<div class="chat-container"></div>
</body>
</html>

#{var[title: 채팅 메시지]}
#{include[inc/chat-container.js]}
#{include[inc/upload-container.js]}

#{script[ready]
	fetch() [result=>changeState('chat-list', result.children)]
	fetch() [result=>changeState('emoji-list', result.chidren)]
}
#{innerHTML[chat-container]
	<div class="chat-header">echo[chat_header]</div>
	<div class="chat-messages">
		effect[chat-list] 
		while(item) item.messageType=='recv' ? echo[chat_recv] : echo[chat_send]	
	</div>
	<div class="chat-input-container">
		<div class="chat-toolbar">echo[chat-toolbar]</div>	
		<div class="emoji-picker">
			effect[emoji-list]
			while(item) echo[chat_emoji]
		</div>	
		<div class="chat-input">echo[chat_input]</div>	
		<div class="file-preview"></div>
	</div>
}


#{set[chat-header]
	<h1>채팅 메시지</h1>
	<div class="user-info">
		<span class="user-name">사용자</span>
		<span class="status online"></span>
	</div>
}

#{html[chat-recv]
	<div class="message received">
		<div class="message-content">
			<p>${item.msg}</p>
			<span class="message-time">${item.time}</span>
		</div>
	</div>
}

#{html[chat-send]
	<div class="message sent">
		<div class="message-content">
			<p>${item.msg}</p>
			<span class="message-time">${item.time}</span>
		</div>
	</div>
}

/* 이모지추가 아이콘 추가 */
#{set[chat-toolbar]
	<button class="toolbar-button">
		<i class="far fa-smile"></i>
	</button>
	<label class="toolbar-button upload-clip">
		<i class="fas fa-paperclip"></i>
	</label>
}

#{html[chat-emoji]
	<div class="emoji-category">
		<h4>${item.name}</h4>
		<div class="emoji-list">
			while(cur, item.chidren) 
			<span class="emoji" data-emoji="${cur.emoji}">${cur.emoji}</span>
		</div>
	</div>
}
 

#{set[chat-input]
	<textarea id="message-input" placeholder="메시지를 입력하세요..." rows="1"></textarea>
	<button id="send-button">
		<i class="fas fa-paper-plane"></i>
	</button>
}


#{css[chat]
	/* 기본 스타일 */
	* {
		margin: 0;
		padding: 0;
		box-sizing: border-box;
		font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
	}
	body {
		background-color: #f5f5f5;
		height: 100vh;
		display: flex;
		justify-content: center;
		align-items: center;
	}

}

##### source #####
include('classes/test/web_test')
test()


x=object('@inc.userFunc').get('printLine')


~~
<func>
	test() {
		fn=Cf.funcNode()
		s=fileRead('c:/temp/a.html')
		param = _node('param')
		param.val('webpageFileName','c:/bpdt/web/common/a.html')
		ss = @web.parseTemplate(s,fn,param)
		fileWrite('c:/temp/a_conv.html', ss)
	}
</func>

~~
<func>
	printLine(n,&s) {
		if(s.ch()) {
			a=s.findPos("\n").trim()
		} else {
			a='no data'
		}
		print("$n>>$a")
		return;
	}
	@web.parseTemplate(&s, fn, param) {
		not(s.find('#{')) return s;
		checkStart = false
	 	not(typeof(fn,'func') ) fn=Cf.funcNode('parent')
	 	not(param) {
			checkStart = true
			param=fn.get('param')
		}
	 	not(typeof(param,'node')) param = null
	 	ln ="\r\n", ss=''
	 	while(s.valid()) {
	 		left = s.findPos('#{')
	 		ss.add(left)
	 		not(s.ch()) break;
	 		sp = s.cur() - 1;
	 		s.pos(sp) not(s.ch('{')) continue;
	 		src=s.match(1) if(typeof(src,'bool')) continue;
	 		ss.add( @web.parseTemplateVar(src,fn,param) )
	 	}
	 	return ss; 
	}
	@web.isTemplateVar(&s) {
		c=s.next().ch()
		return c.eq('[')
	}
	@web.parseTemplateVar(&s,fn,param) {
		not(@web.isTemplateVar(s)) return "#{$s}";
		type=s.findPos('[',0,1).trim()
		a=s.match(1).trim()
		arr=@web.splitParam(a,[])
		fc=call("websrc.tp_$type")
		if(typeof(fc,'func')) {
			return call(fc,param,s,arr,fn, param)
		}
		return "tp_$type 함수 미정의"
	}	 
	@websrc.tp_var(&src, arr, fn,param) {
		arr.inject(&s)
		not(s.ch()) s=src
		param.parseJson(s)
		a=s.move()
		print("s->$s a:$a", src, fn, arr, param)
		return;
	}
	@websrc.tp_set(&src, arr, fn,param) {
		k=arr.get(0)
		node = param.addNode('@set')
		node.set(k,src)
		return;
	}
	@websrc.tp_html(&src, arr, fn,param) {
		id=arr.get(0)
		node = param.addNode('@set')
		node.set(k,@web.parseHtml(src,fn,param))
		return;
	}
	@websrc.tp_include(&src, arr, fn,param) {
		arr.inject(&s)
		not(s.ch()) s=src
		path = param.val('webpageFileName').findLast('/').trim()
		while(src.valid()) {
			line = src.findPos("\n").trim()
			not(line) continue;
			param.set('@includeName', getFileName(line))
			@web.parseInclude(fileRead("$path/$line"),fn,param)
		}
	}
	@websrc.tp_script(&src, arr, fn,param) {
		arr.inject(&s)
		not(s.ch()) s=src
		
	}	
</func> 
~~
<func>
	@web.parseInclude(&s, fn, param,name) {
		not(s.find('#{')) return s;
		not(name) name = param.val('@includeName')
	 	ln ="\r\n", ss=''
	 	while(s.valid()) {
	 		left = s.findPos('#{')
	 		ss.add(left,ln)
	 		not(s.ch()) break;
	 		sp = s.cur() - 1;
	 		s.pos(sp) not(s.ch('{')) continue;
	 		src=s.match()
	 		if(typeof(src,'bool')) {
	 			s.findPos('}')
	 			continue;
	 		}
	 		@web.parseTemplateVar(src,fn,param)
	 	}
	 	if( ss) param.set('@script', ss)
	}
	@web.splitParam(&s,arr) {
		arr.reuse()
		while(s.valid()) {
			c=s.ch()
			not(c) break;
			if(c.eq()) {
				v=s.match().trim()
				c=s.ch() if(c.eq(',')) s.incr()
			} else {
				v=s.findPos(',').trim()
			}
			arr.add(v)
		}
		return arr;
	}
	@web.parseHtmlSrc(&s,fn,param, prop, depth) {
		print("parse html src => $s", prop)
	}
	@web.parseHtml(&s,fn,param, prop, depth) {		
		not(depth) depth=0
		result=''
		while(s.valid()) {
			c=s.ch() not(c) break;
			if(c.eq('<')) {
				sp=s.cur()
				tag = ''
				if( s.start('<>')) {
					ss=s.match('<>','</>')
					if(typeof(ss,'bool')) ss=s.findPos('<>')
					prop=''
				} else {
					c=s.incr().next().ch()
					if(c.eq('.','-')) c=s.incr().next().ch()
					tag = s.trim(sp+1,s.cur())
					s.pos(sp)
					ss=s.match("<$tag","</$tag>")
					if(typeof(ss,'bool')) ss=s.findPos("</$tag>")
					prop=ss.findPos('>')
				}
				if(typeof(ss,'bool')) break;
				if(ss.ch('<')) {
					src = @web.parseHtml(ss,fn,param,prop,depth)
					if(tag) {
						result.add("<$tag $prop>", src, "</$tag>")
					} else {
						result.add(src)
					}
				} 
			} else {
				result.add(@web.parseHtmlSrc(s,fn,param,prop))
				break;
			}
		}
		return result;
	}
</func>
