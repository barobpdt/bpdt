##> config {name=css}
	STYLE_MAP = @eval( return object('user.styleMap').parseJson(this.styleMap) )
	styleMap {
		w:width,h:height,p:padding,m:margin,
		mt:marginTop, mb:marginBottom, ml:marginLeft, mr:marginRight,
		pt:paddingTop, pb:paddingBottom, pl:paddingLeft, pr:paddingRight,
		bg:background,
		bd:border,
		b:border,
		bt:borderTop, bb:borderBottom, bl:borderLeft, br:borderRight,
		rad:borderRadius,
		c:color,
		x:top, y:left,
		t:top, l:left,
		tform: transform,
		tf: transform,
		tr:transition, 
		trdelay:transitionDelay,
		fwrap:flexWrap,
		ani:animation
		anidelay: animationDelay,
		bgpos:backgroundPosition,
		bgsize:backgroundSize,
		fit:object-fit,
		minh:minHeight,
		maxh:maxHeight,
		minw:minWidth,
		maxw:maxWidth,
		rel:relative,
		abs:absolute,
		space:letterSpace,
		ls:letterSpace,
		lh:lineHeight,
		fs:fontSize,
		fw:fontWeight,
		pe:pointerEvent,
		ai:alignItems,
		jc:justifyContent,
		shadow: boxShadow,
		ctt: content,
		hint: placeholder
	}
##> func {name=frontend}
	renderLayout(page,node) {
		result=''
		not(node) {
			node=page.get('@layout') 
		}
		map=object('user.styleMap')
		while(cur, node) {
			cur.inject(tag,css,style)
			if(css) {
				nodeCss=parseConfigProps(css)
				while(key, nodeCss.get('@keyArray')) {
					val=nodeCass.get(key)
					name=map.get(key) not(name) name=key;
					
				}
			}
		}
		return result;
	}
	makeLayout(page, &s) { 
		layout = page.addNode('@layout').removeAll(true)
		parentArray=[]
		indentArray=[]
		parentArray.add(layout)	
		while(s.valid()) {
			// line 공백이면 무시
			if(lineBlankCheck(s)) {
				s.findPos("\n")
				continue;
			}
			a = indentText(s)
			c = s.ch()
			not(c) return;
			// line 'end'로 시작하면 현재 태그끝
			if( s.start('end') ) {
				not(cur) continue;
				if( cur && lineCheck(s,'<')) {
					s.findPos('<',0,1)
					_tagValue()
				}
				s.findPos("\n")
				_tagValue()
				continue;
			}
			// line 공백문자로 부모 인덱스 찾기
			if( indentArray.size()) {
				if(a) {
					idx=indentArray.find(a)
				} else {
					idx=0
				}
				if(idx==-1) {
					idx=indentArray.size()
					indentArray.add(a)
				}
			} else {
				idx=0
				indentArray.add(a)
			}
			/* 부모태그를 찾았다면 현재 라인 태그 생성 (태그는 -또는 . 문자포함 가능)
				예) tag [속성] <>html</> 형태
			   태그와 속성사이값이 있다면 하위 html로 추가 
				예) label 이름 [id:name]
			*/
			base = parentArray.get(idx)
			not(base ) return print("@@ 레이아웃 분석 부모노드 찾기오류 idx:$idx");
			sp = s.cur()
			c=s.next().ch(1)
			while(c.eq('-','.')) c=s.incr().next().ch(1)
			tag = s.trim(sp,s.cur(),true)
			cur = base.addNode()
			cur.tag=tag
			// tag 속성분석 
			if( lineCheck(s,'[') ) {
				left = s.findPos('[',1,1)
				body = s.match(1)
				if(typeof(body,'bool')) return print("태그 속성 매핑오류 태그:$tag", left);
				if( left.ch()) {				
					cur.appendText('@html', left.trim())
				}
				@baro.parseConfig(root,cur,body)
			} else if(_checkProp(s)) {				
				s.ch()
				body = s.match(1)
				@baro.parseConfig(root,cur, body)
			}
			// html 태그로 시작한다면 현재 tag 하위 요소로 추가
			not(_tagValue()) {
				left = s.findPos("\n")
				if( left.ch()) {
					cur.appendText('@html', left.trim())				
				}
			}
			if( checkError('태그분석오류') ) return;
			// 현재노드를 배열 다음인덱스에 추가
			setArray(parentArray, idx+1, cur)
		}
		return renderLayout(page);
		
		/* 속성 체크 */
		_checkProp = func(&s) {
			if(lineBlankCheck(s) ) {
				c=s.ch()
				return c.eq('[');
			}
			return false;
		};
		/* 태그체크 */
		_checkTag = func(&s) {
			c=s.ch()
			return c.eq('<')
		};
		/* 태그값 분석 (연속된 태그값도 허용 )*/
		_tagValue = func() {
			not(_checkTag(s) ) return false;
			html=''
			while(_checkTag(s)) {
				c=s.ch() not(c) break;
				cc=s.ch(1)
				if(cc.eq('>')) {
					body=s.match('<>','<>',1)
					print("@@ <><> html : $body")
					html.add(body)
					continue;
				}			
				sp=s.cur()
				c=s.incr().next().ch(1)
				if(c.eq('-',':')) c=s.incr().next().ch(1)
				tag=s.trim(sp+1, s.cur(), true)
				print("_checkTag", page.pageCode, tag, line)
				s.pos(sp)
				body=s.match("<$tag", "</$tag>",8)
				if(typeof(body,'bool')) {
					return print("매칭되는 태그를 찾을수 없습니다", left, tag);
				}				 
				props=body.findPos('>')
				src=@baro.parseSource(parent,page,cur,body,'value')
				html.add("<$tag")
				if(props.ch()) {
					html.add(" $props>")
				} else {
					html.add( ">")
				}
				html.add( src,"</$tag>")
			}
			node.appendText('@html', html)
			return true;
		};
	}

##> func {name=fileChangeCheck note=파일변경여부 체크해서 자동반영}
	/* 서비스별(프로트,백앤드,api 등) 파일감시 목록추가  */
	addWatchFile(serviceMode, fullpath) {
		not(isFile(fullpath)) return log('#{0} 파일을 찾을수 없습니다 (소스감시 등록오류)',fullpath);
		watchInfo=object('user.watchFileInfo')
		files=watchInfo.addArray('@fileList')
		if( files.find(fullpath) ) {
			return log("$fullpath 는 파일변경목록에 이미 추가되었습니다")
		}
		// 전체경로에서 폴더, 파일명, 확장자제외 이름 추출
		filePathInfo(fullpath).inject(folder,fileName,name)
		// 파일변경시간 추출
		modifyTime = fileTime(fullpath)
		// 파일명으로 소스감시 목록추가
		watchInfo.addNode(fileName).with(serviceMode,fullpath,folder,fileName,name,modifyTime)
		files.add(fullpath)
	}
	
	/* 전역 타이머 실행 */
	startGlobalTimer() {
		if( global().get('@timerDelay') ) {
			return log('global timer가 실행중입니다 #{0}', global().get('@timerDelay'))
		}
		watchInfo = object('baro.watchFileInfo')
		watchInfo.addArray('@watchFileList').reuse()
		watchInfo.addArray('@watchFileList').reuse()
		event(global(),'onTimeout', @user.timerProc)
		// 500ms 마다 파일변경체크
		System.globalTimer(500)
	}
	/* 전역 타이머 중지 */
	stopGlobalTimer() {
		global().set('@timerDelay',0)
		System.globalTimer(false)
	}
	/* 전역 타이머처리 콜백함수 */	
	@user.timerProc() {
		watchInfo = object('baro.watchFileInfo')
		jobList = watchInfo.get('@jobList')
		while(cur, watchInfo) {
			cur.inject(serviceMode,fullpath,fileName,name,modifyTime)
			// 현재파일 시간과 등록된 시간이 다르다면 파일 변경처리
			if(fileTime(fullpath) != modifyTime) {
				log("$fileName 변경됨 서비스등록 처리")
				@baro.loadService(serviceMode,fullpath)
				return;
			} 
		}		
		if(typeof(jobList,'array')) {
			job = jobList.pop() not(job) return;
			if(job.cmp('tag','precess') ) {
				cmd=job.get('@jobCommand')
				if(cmd) {
					job.set('@jobCommand', null)
					job.write(cmd)
				}
			} else if(job.cmp('tag','web') ) {
				url=job.get('@jobUrl')
				result=webResult(job, url)
			} else {
				log('job 태그 미정의 #{0}', job)
			}
			return;
		}
	}
	
##> func {name=jobs }
	/* 웹호출 결과 출력 (api 호출) */
	webResult(web, url, method, data, headerJson) {
		not(method) method='GET'
		if(method=='POST') {
			web.set('data',data)
		}		
		if(headerJson && typeof(headerJson,'string')) {
			header=web.addNode('@header').reuse()
			header.parseJson(headerJson)
		}
		web.call(url,method, @user.webCallback)
		return web.ref('@webResult')
	}
	@user.webCallback(type,data) {
		if(type=='error') return log('webResult 오류 객체:#{0} 메시지#{1}', this, data);
		if(type=='read') this.appendText('@webResult', data)
	}	