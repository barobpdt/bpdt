##
from PySide6.QtGui import QImage, QColor
    width = 200
    height = 150
    image = QImage(width, height, QImage.Format_ARGB32)
    image.fill(Qt.white) # 이미지를 흰색으로 채웁니다.	
		
	# Set the color of the pixel at (10, 20) to blue
	image.setPixelColor(10, 20, QColor(0, 0, 255)) # Blue color (RGB)
    painter = QPainter(image)
    label = QLabel()
    label.setPixmap(QPixmap.fromImage(image))
    label.show()

# Get the color of the pixel at (10, 20)
pixel_color = image.pixelColor(10, 20)
print(f"Pixel color at (10, 20): {pixel_color.red()}, {pixel_color.green()}, {pixel_color.blue()}")

    from PyQt5.QtGui import QPixmap, QColor, Qt
    # Load your image
    image = QPixmap("your_image.png")

    # Define the color to be made transparent (e.g., white)
    transparent_color = QColor(255, 255, 255) # RGB for white

    # Create a mask from the color
    # Qt.MaskInColor makes the specified color transparent
    mask = image.createMaskFromColor(transparent_color, Qt.MaskInColor)

    # Apply the mask to the image
    image.setMask(mask)

## 미디파일 목록
	https://songs.bardmusicplayer.com/ 
	
## 미디툴	
	https://github.com/ldrolez/free-midi-chords/releases

## 파이션 노래방 프로그
	https://github.com/giantdwarf17/KaraokeTube
## webRTC를 이용한 실시간 영상 처
	https://railly-linker.tistory.com/134

## 유사도 벡터추출
	https://wikidocs.net/blog/@TryOncePythonProject/880/
## 유튜브 영상 mp3로 저장하기
	https://expertpro.tistory.com/38#google_vignette	
## 유튜브 콘솔
	https://console.cloud.google.com/apis/dashboard?project=bpdt-de20b
	API 키: AIzaSyD8YZkO-B2Mu6SJQnuwqbAvPqNZV7d zmM
## 유튜브 다운로드
	https://github.com/yt-dlp/yt-dlp
## 자작 노래방 
	https://42morrow.tistory.com/entry/%EC%9E%90%EC%9E%91-%EB%85%B8%EB%9E%98%EB%B0%A9-%ED%94%84%EB%A1%9C%EA%B7%B8%EB%9E%A8-%EA%B0%80%EC%82%AC%EB%B3%B4%EB%A9%B0-%EB%85%B8%EB%9E%98-%EB%94%B0%EB%9D%BC-%EB%B6%80%EB%A5%B4%EA%B8%B0
## 반주와 음성분리하기
	https://blog.naver.com/kayoko79/223806280157
## moviepy 자막
	https://github.com/Anil-matcha/Free-Video-Tools/blob/main/Hardcode_subtitles_on_video.ipynb
	https://github.com/unconv/captacity
	
// job 호출예
	@job.webResult('sido_info', 'https://new.land.naver.com/api/regions/list?cortarNo=0000000000')
	@job.addJob('gunguInfo', node)

	@job.fc_gungoInfo(node) {
		url="https://new.land.naver.com/api/regions/list?cortarNo=${node.cortarNo}"
		web=@job.webObject()
		not(web) {
			while(n=0,10) {
				web=@job.webObject() if(web) break;
				System.sleep(500)
			}
		}
		@job.webResult(web,'gunguInfo', url, node)
	}
	@job.web_gunguInfo(s, target) {
		
	}
	
// cmd
	cmdNode(node, cmd, fc, target) {
		not(typeof(node,'node')) return print("@@ cmd add error [$node]");
		if(cmd) {
			node.set('cmd', cmd)
		}
		if(typeof(fc,'func') ) {
			node.set('callback', fc)			
		}
		cmdList.add(node)		
		if( status.eq('first','stay')) {
			this.run()
		}
	}
	cmdCallback(cmd, callback, target) {
		node=this.addNode().with(cmd, callback, target)
		cmdNode(node)
	}
<func>
	
	@job.event(obj, eventName, fc, reset) {		
		not(typeof(obj,'node')) return print('@@ job event 객체 오류', obj, fc) 
		fn = obj.get(eventName)
		if( typeof(fn,'func')) {
			print("xxxxxxxx", args())
			not(reset) {
				print("＠＠ $eventName 함수가 이미등록되었습니다")
				return fn;
			}
		}
		fcType = typeof(fc)
		not( fcType.eq('funcRef') ) {		
			if(fc) print("@@ job event  함수타입 오류 (타입:$fcType)")
			return;
		}
		fn=Cf.funcNode(fc, obj)
		obj.set(eventName, fn) 
		return fn;		
	}
	@job.addTimerJob(node) {
		not(System.globalTimer()) @job.timer();
		global=Cf.rootNode() 
		global.inject(@timerObjects)
		not(timerObjects) print("@@ timer job add error", global)
		timerObjects.add(node)
		print("xxxxxxxxxx timer job add xxxxxxxxxx", node, timerObjects)
	} 
	@job.timer() {
		if(System.globalTimer()) {
			print("timer 이미 설정됨")
			return;
		}
		global=Cf.rootNode()
		global.addArray('@timerObjects') 
		@job.event(global,'onTimeout', @job.timerProc)
		System.globalTimer(250)
	}
	@job.timerProc() {
		this.inject(@timerObjects)
		job = timerObjects.pop() not(job) return;
		print("timer job == $job")
		fc=call("@job.tm_${job.type}")
		if(typeof(fc,'function') ) {
			call(fc, this, job)
		} else {
			print("@@ timer job function not defined node==>$job")
		}
	} 
	@job.start() {
		conf('job.webMaxNum', 10)
		jobs=Baro.worker('jobs')
		not(jobs.is('start')) {			
			jobs.start(@job.proc)
		}
		return jobs;
	}
	@job.proc(node) {
		print("@@ 작업시작 => ", node)
		not(node) return print("@@ 작업시작오류 (작업노드 미정의)")
		not(node.jobType) node.jobType='test'
		fc = call("@job.fc_${node.jobType}")
		if(typeof(fc,'func')) {
			call(fc, this, node)
		}
	}
	@job.addJob(param) {
		if(typeof(param,'node')) {
			args(node)
		} else {
			args(jobType, node)
			node.jobType=jobType
		}
		jobs=Baro.worker('jobs')
		if(jobs.is('start')) {			
			jobs.push(node)
		} else {
			print("@@ 작업이 시작되지 않았습니다 job addNode 오류")
		}
	} 
	@job.timerProc() {
		this.inject(@timerObjects)
		job = timerObjects.pop() not(job) return;
		print("timer job == $job")
		fc=call("@job.tm_${job.type}")
		if(typeof(fc,'function') ) {
			call(fc, job, this)
		} else {
			print("@@ timer job function not defined node==>$job")
		}
	} 
	@job.webObject() {
		ws=object('baro.webObjectMap')
		arr = ws.get('@webObjects')
		cnt = 0
		if( typeof(arr,'array') ) cnt=arr.size()
		print("@@ web object size: ", cnt)
		not(cnt) {			
			arr=ws.addArray('@webObjects')
			while(n=1,5) {
				cur = arr.add(Baro.web("webObject_$n"))
				@job.event(cur, '@callback', @job.webTypeResult)
			}
		}
		obj=null
		while(cur, arr) {
			if(cur.is('run')) continue;
			obj = cur;
		}
		not( obj ) {
			idx = arr.size()+1;
			if( idx > conf('job.webMaxNum') ) {
				return null;
			}
			cur = arr.add(Baro.web("webObject_$idx"))
			@job.event(cur, '@callback', @job.webTypeResult)
			obj = cur
		}
		print("obj==>$obj")
		return obj;
	}
	
	@job.webTypeResult(type, data) {
		if(type=='read') this.appendText('result', data)
		if(type=='finish') {
			fc = call("@job.web_${this.resultType}")
			if(typeof(fc,'function')) {
				target = this.get('@target')
				call(fc, this, this.ref(result), target)
			} else {
				print("@@ webTypeResult 콜백 함수 미정의 : job.web_${this.resultType}")
			}
		}		
	}
	
	@job.webResult(param) {
		if( typeof(param,'node') ) {
			args(wo,resultType,url,target)
		} else {
			args(resultType, url,target)
			wo=@job.webObject()
			not(wo) return;
		}		
		not(resultType) resultType='default'
		not(url) url = wo.url
		wo.set('@target', target)
		wo.set('result','')
		wo.resultType=resultType
		wo.call(url)
		return true;
	}

	@job.web_default(&s, target) {
		print("## web result ==> $s")
	}
	@job.web_sido_info(s, target) {
		node=object('naver.sidoInfo')
		node.parseJson(s)
		while(cur,node.regionList) { 
			@job.addJob('gungoInfo', cur)
		}
	}

	// 작업추가
	// @job.addJob(node) 
	// 
	@job.fc_test(node) {
		@job.addTimerJob(node)		
	}
	@job.tm_test(node) {
		command=node.command
		not(command) command='dir'
		cmd().cmdNode(node, command, @job.test_callback )
		print("@@ tm_test call end", node)
	}
	@job.test_callback(&s) {
		print("test callback s==$s")
	}
</func>
	
<script>
	
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
	 		not(s.ch()) break;
	 		sp = s.cur() - 1;
	 		s.pos(sp) not(s.ch('{')) continue;
	 		src=s.match() if(typeof(src,'bool')) continue;
	 		@web.parseTemplateVar(src,fn,param,left)
	 	}
	 	return ss; 
	}
	@web.parseTemplateVar(&s,fn,param,depth) {
		not(depth) depth=0
		result=''
		while(s.valid()) {
			c=s.ch() not(c) break;
			if(c.eq('<')) {
				sp=s.cur()
				if( s.start('<>')) {
					ss=s.match('<>','<>')
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
				if(ss.ch('<')) {
					result.add(@web.parseTemplateVar(ss,fn,param,depth+1) )
				}
			} else {
				if(@web.isScriptPageVar(s)) {
					arr=[]
					a = s.findPos('[',0,1)
					arr=@web.splitParam(a,[])
					fc=call("websrc.sp_$type")
					if(typeof(fc,'func')) {
						not(param.var(funcNode)) param.var(funcNode,fn)
						call(fc,param,arr)
					}
				}
				print(">>",a,s)
			}
		}
		return result;
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
	@websrc.sp_effect(s) {
		arr=args()
		fn=this.var(funcNode)
		
	}
	@web.isScriptPageVar(&s) {
		c=s.next().ch()
		return c.eq('[')
	};
	@web.isTemplateVar(&s) {
		c=s.next().ch()
		return c.eq('[')
	}
	@web.parseTemplateVar(&s,fn,param,left) {
		not(Cf.funcNode('parent').isset('ss')) ss=''
		not(isTemplateVar(s)) return ss.add(left,"#{$s}");
		type=s.findPos('[',0,1).trim()
		a=s.match(1).trim() 
		switch(type) {
		case var: // #{var[a]b}
			not(a) a='json'
			if(a.eq('json')) param.parseJson(s)
		case set:
			cur = param.addNode('@setMap')
			cur.set(a,@web.parseTemplate(s,fn,param))
		case html:
			cur = param.addNode('@htmlMap')
			cur.set(a,@web.parseTemplateHtml(s,fn,param))
			
		}
		return ss;
	}
	
	@web.parseConfValue(&s,fn,param) {
		not(s.find('#{')) return;
		node = param.addNode('@confValue')
		jsinfo = param.addNode('@jsInfo')
	 	while(s.valid()) {
	 		left = s.findPos('#{')
			if(left.ch()) jsinfo.appendText('init', left)
	 		not(s.ch()) break;
	 		sp = s.cur() - 1;
	 		s.pos(sp) not(s.ch('{')) continue;
	 		ss=s.match() 
			if(typeof(ss,'bool')) continue;
			k=ss.findPos('[',0,1)
			v=ss.match()
			if(typeof(v,'bool')) continue;
			if(k.start('css-',true)) {
				code= k.trim()
				cur = param.addNode('@cssInfo')
				cur.appendText(code, v)
				continue;
			} 
			if(k.start('script-',true)) {
				code= k.trim()
				cur = param.addNode('@jsInfo')
				cur.appendText(code, v)
				continue;
			} 
			key = k.trim()
			if(key.eq('script')) {
				param.appendText('script', v)
			} else if(key.eq('css')) {
				param.appendText('css', v)
			} else {
				node.val(key, v) 
			}
	 	}
	}	 
	@web.parseControl(&s,fn,param) {
		ftype= s.findPos('(',1,1)
		fparam = s.match()
		not(s.ch('<')) return print("$ftype 시작문자 오류", s)
		sp= s.cur()
		c=s.incr().ch()
		if(c.eq('>')) {
			s.pos(sp)
			src = s.match('<>','<>')
			if(typeof(src,'bool')) return print("$s 시작태그 오류")
		} else {
			tag = s.move()
			s.pos(sp)
			match = s.match("<$tag","</$tag>")
			if(typeof(match,'bool')) return print("$tag 태그매치 오류")
			src = s.value(sp, s.cur())
		}
		result = ''
		if(ftype.eq('each')) {
			fparam.split(',').inject(a,b)
			node = fn.get(b) not(node) node=param.get(b)
			if(typeof(node,'node')) {
				while(cur, node) {
					fn.set(a,cur)
					result.add(@web.parseTemplate(src,fn,param))
				}
			} else {
				result = print("each( $a, $b ) 오브젝트 설정오류")
			}
		}
		return result;
	}
	 
	@web.parsePrint(&s,fn,param) {
		result = '' 
		line=s.findPos('[',0,1) not(line.ch()) return;
		src=s.match() if(typeof(src,'bool')) return;
		if(line.start('css-',true)) {
			code= line.trim()
			cur = param.addNode('@cssInfo')
			cur.appendText(code, src)
			return;
		}
		if(line.start('script-',true)) {
			code= line.trim()
			cur = param.addNode('@jsInfo')
			cur.appendText(code, src)
			return;
		}
		if(line.find('.')) {
			if(line.ch('@')) {
				line.incr()
				code = line.trim()
				conf(code, src, true)
			} else {					
				code = line.findPos('.').trim()
				if(code.eq('conf')) {
					node = param.val('@confValue', true)
					code = line.trim()
					node.val(code, src)
				}
			}
		} else {
			type = line.trim()
			if(type.eq('func','function')) {
				@src.addFuncSource(src)
			} else if(type.eq('eval')) {
				eval(src, fn, param)
			} else if(type.eq('set')) {
				param.parseJson(src)
			} else if(type.eq('script')) {
				param.appendText('script', src)
			} else if(type.eq('css')) {
				param.appendText('css', src)
			} else if(type.eq('include')) {
				path = param.val('webpageFileName').findLast('/').trim()
				while(src.valid()) {
					line = src.findPos("\n").trim()
					not(line) continue;
					@web.parseConfValue(fileRead("$path/$line"),fn,param)
				}
			} else if(type.eq('print')) {
				result.add(@web.parseTemplate(src,fn,param))
			} else {
				param.set(type, @web.parseTemplate(src,fn,param))
			}
		} 
		return result;
	}

	@web.parseCase(&s,fn,param,skipCheck){
		if(skipCheck) {
			c=s.ch()
		} else { 
			c=s.ch()
			sp = s.cur()
			if(c.eq('@')) c=s.incr()
			c=s.next().ch()
			if(c.eq('.')) c=s.incr().next().ch()
			ok=false
			if(c.eq('(')) {
				s.match()
				src = s.trim(sp, s.cur())
				if( eval(src) ) ok=true
			} else {
				name = s.trim(sp, s.cur())
				if(fn.isset(name)) {
					if(fn.get(name)) ok=true;	
				} else if(param.isVar(name)) {
					if(param.get(name)) ok=true;
				}
			}
			c=s.ch()
			not(c.eq('?')) return print("parseCase case 매칭오류", s.size())
			c=s.incr().ch()
		}
		result=''
		if(c.eq('<')) {
			sp=s.cur()
			if(s.start('<>')) {
				result = s.match('<>','<>')
			} else {
				tag=s.incr().move()
				s.pos(sp)
				print("s===$s")
				if( @src.isSingleTag(s) ) {
					s.findPos("/>")
				} else {
					match=s.match("<$tag","</$tag>")
					if(typeof(match,'bool')) return print("parseCase $tag 매칭오류 ")
				}
				ep = s.cur()
				result = s.value(sp,ep,true)
			}
		} else if(c.eq('(','[')) {
			result = s.match()
		} else if(c.eq()) {
			result = s.match()
		} else if(s.start('#{')) {
			s.incr()
			src=s.match()
			result = @web.parseVar(src,fn,param)
		} else {
			result = s.findPos(':',1,1).trim()
		}
		if(ok) {
			return @web.parseTemplate(result,fn,param);
		}
		c=s.ch()
		if(c.eq(':')) {
			s.incr()
			skipCheck = ~(@src.isCase(s))
			result = @web.parseCase(s,fn,param,skipCheck)
		}
		return result;
	}
	@src.isControl(&s) {
		c=s.next().ch()
		if(c.eq('(')) {
			s.match()
			c=s.ch()
			if(c.eq('<')) return true;
		}
		return false;
	}
	@src.isSingleTag(&s) {
		left= s.findPos('>')
		c=left.ch(-1)
		return c.eq('/')
	} 
	@src.isFunc(&s) {
		c=s.ch()
		if(c.eq('@')) c=s.incr()
		c=s.next().ch()
		if(c.eq('.')) c=s.incr().next().ch()
		if(c.eq('(')) {
			s.match()
			not( s.ch() ) return true;
		}
		return false;
	}
	@src.isCase(&s) {
		c=s.ch()
		if(c.eq('@')) c=s.incr()
		c=s.next().ch()	
		if(c.eq('.')) c=s.incr().next().ch()
		if(c.eq('(')) {
			s.match()
			c=s.ch()
		}
		return c.eq('?');
	}
	
	@src.isVar(&s) {
		c=s.next().ch()
		if(c.eq('#')) c=s.incr().next().ch()
		if(c.eq('.','-')) {
			c=s.incr().next().ch()
			if(c.eq('#')) c=s.incr().next().ch()
		}
		if(c.eq(':')) c=''
		not(c) return true;
		return false;
	}
	@src.isPrint(&s) {
		c=s.ch()
		if(c.eq('@')) c=s.incr()
		c=s.next().ch()
		if(c.eq('#','-')) c=s.incr().next().ch()
		if(c.eq('.')) {
			c=s.incr().next().ch()
			if(c.eq('#','-')) c=s.incr().ch()
		}
		if(c.eq('[') ) return true;
		return false;
	}
	@src.addFuncSource(&s) {
		ss=''
		while(s.valid()) {
			sp = s.cur()
			fnm = s.findPos('(',1,1).trim()
			fc=call(fnm)
			ok=true if(typeof(fc,'function')) ok=false;
			s.match()
			not(s.ch('{')) break;
			a=s.match(1) if(typeof(a,'bool')) return print("$fnm 함수 매칭오류");
			if(ok) {
				print("$fnm 웹사용 함수등록")
				fsrc=s.value(sp,s.cur())
				ss.add(fsrc)
			}
			c=s.ch() not(c) break;
			if(c.eq(',',';')) s.incr()
		}
		if(ss) call(ss)
	}
</script>

<script>
	@web.cssUiIcon(name, w, h) {
		not(w) w=24
		not(h) h=w
		src = conf("svg.$name")
		not(src) print("svg $name 로드오류 ")
		src.ref()
		prop = src.findPos("\n").trim()
		ss=''
		while(src.valid()) {
			d = src.findPos("\n").trim()
			ss.add( fv('<path d="#{d}"/>') )
		}
		// background-size: contain;
		return 
#[
	.ui_icon.${name} {
		background-image: url('data:image/svg+xml,<svg ${prop}>${ss}</svg>');
		width: ${w}px;
		height: ${h}px;
		background-position: center;
	}
]
	}
</script>


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
