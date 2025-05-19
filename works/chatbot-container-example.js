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
		ss = @websrc.parseTemplate(s,fn,param)
		fileWrite('c:/temp/a_conv.html', ss)
	}
</func>

~~
<func>
	@websrc.parseTemplate(&s, fn, param) {
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
	 		ss.add( @websrc.parseTemplateVar(src,fn,param) )
	 	}
	 	return ss; 
	}
	@websrc.isTemplateVar(&s) {
		c=s.next().ch()
		return c.eq('[')
	}
	@websrc.parseTemplateVar(&s,fn,param) {
		not(@websrc.isTemplateVar(s)) return "#{$s}";
		type=s.findPos('[',0,1).trim()
		a=s.match(1).trim()
		arr=@websrc.splitParam(a,[])
		fc=call("websrc.tp_$type") not(typeof(fc,'func'))  return "tp_$type 함수 미정의"
		return call(fc,param,s,arr,fn, param)
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
		node.set(k,@websrc.parseHtml(src,fn,param))
		return;
	}
	@websrc.tp_innerHTML(&src, arr, fn,param) {
		id=arr.get(0)
		node = param.addNode('@setEl')
		node.set(k,@websrc.parseHtml(src,fn,param))
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
			@websrc.parseInclude(fileRead("$path/$line"),fn,param)
		}
		return;
	}
	@websrc.tp_script(&src, arr, fn,param) {
		arr.inject(&s)
		not(s.ch()) s=src
		return;
	}	
</func> 
~~
<func>
	@websrc.parseInclude(&s, fn, param,name) {
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
	 		if( typeof(src,'bool')) {
	 			s.findPos('}')
	 			continue;
	 		}
	 		// @websrc.parseTemplateVar(src,fn,param)
	 	}
	 	if( ss) param.set('@script', ss)
	}
	@websrc.splitParam(&s,arr) {
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
	@websrc.parseHtml(&s,fn,param, prop, depth) {		
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
					src = @websrc.parseHtml(ss,fn,param,prop,depth)
					if(tag) {
						result.add("<$tag $prop>", src, "</$tag>")
					} else {
						result.add(src)
					}
				} else {
					if( @websrc.isTemplateVar(ss)) {
						printLine(">> parse html prop:$prop $tag ", ss)
						result.add(@websrc.parseHtmlSrc(ss,fn,param,tag,prop))						
					} else {
						result.add("<$tag $prop>", ss, "</$tag>")
					}
					break;
				}
			} else {
				printLine("parse html 시작오류:: ", ss)
			}
		}
		return result;
	}
	@websrc.parseHtmlSrc(&s,fn,param, tag, prop, depth) {
		result=''
		type = s.move()
		if(s.ch('[')) {
			ss = s.match(1)
		}
		if(type.eq('echo')) {
			result.add('#{',ss.trim(),'}')
		} else if(type.eq('effect')) {
		} else if(type.eq('eval')) {
			@websrc.parseHtml_eval(s,fn,param,tag,prop)
		}
		return result;
	}
	@websrc.parseHtml_eval(&s,fn,param,tag,prop) {
		
	}
	
</func>

<script>
const clog=window.console.log
const randomKey = () => (new Date%9e64).toString(36)
const isNull = a => a===null || typeof a == 'undefined'
const isEmpty = a => isNull(a) || (typeof a=='string' && a=='' )
const isObj = a => !isNull(a) && typeof a=='object';
const isNum = a => typeof a=='number' ? true: typeof a=='string' ? /^[0-9]+$/.test(a): false
const jqCheck= a => typeof JQuery=='object' && a instanceof jQuery;
const constructorName = val => val && val.constructor ? val.constructor.name: ''
const isValid= a => Array.isArray(a)? a.length>0: isObj(a)? Object.keys(a).length : !isEmpty(a)
const isEl = o => 
	typeof HTMLElement === "object" ? o instanceof HTMLElement :
	o && typeof o === "object" && o.nodeType === 1 && typeof o.nodeName==="string"
const getEl = el => isEl(el) ? el : 
	jqCheck(el)? el[0]: 
	typeof el=='string'? (('#'==el.charAt(0)|| el.indexOf('.')!=-1)? $(el)[0]: document.getElementById(el)): null;
	
const cf={stateMap:{}, effectMap:{}}

function setState(id, data) {
	const map = cf.effectMap
	stateMap[id]=data
	for(const cur of getStateNodes(id)) {
		if(typeof(cur.target)=='function') {
			cur.target(data, cur)
		} else {
			const el=getEl(cur.target)
			if(el) {
				$(el).html(data)
			}
		}
	}	
}
function setEffect(incCode, names, target, data) {
	if(!Array.isArray(names)) {
		name = names
		names = [name]
	}
	const alen = arguments.length
	names.forEach(name=> {
		const id = incCode+'--'+name
		if(cf.effectMap[id]) {
			if(alen>2 ) cf.effectMap[id].target = target
			if(alen>3 ) cf.effectMap[id].data = data
		} else {
			cf.effectMap[id] = {incCode,id,names,target,data}
		}
	})	
}
function changeState(id) {
	if(cf.effectMap.hasOwnProperty(id)) {
		const {incCode,id,names,target,data} = cf.effectMap[id]
	}
}

function getStateNodes(id) {
	const arr=[]
	const map = cf.effectMap
	stateMap[id]=data	
	for(const k in map) {
		const cur = map[k]
		if(!Array.isArray(cur.names)) continue;
		for(const a of cur.names) {
			if( a.name==nm && (pc==null||pc==cur.pageCode)) {
				arr.push(cur)
			}
		}
	}
	return arr;
}
</script>
<func>
	getFileName(path) {
		if(path.find("\\")) path = path.replace("\\","/")
		a=path.findLast('/')
		if(a) {
			return left(a.right(),'.')
		} else {
			return left(path,'.')
		}
	}
	printLine(n,&s) {
		if(s.ch()) {
			a=s.findPos("\n").trim()
		} else {
			a='no data'
		}
		print("$n>>$a")
		return;
	}

</func>
