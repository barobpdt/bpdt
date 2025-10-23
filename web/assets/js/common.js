/* 
페이지 공통설정 
*/
const clog=window.console.log
const randomKey = () => (new Date%9e64).toString(36)
const isNull = a => a===null || typeof a == 'undefined'
const isEmpty = a => isNull(a) || (typeof a=='string' && a=='' )
const isObj = a => !isNull(a) && typeof a=='object';
const isNum = a => typeof a=='number' ? true: typeof a=='string' ? /^[0-9]+$/.test(a): false
const jqCheck= a => a instanceof jQuery;
const constructorName = val => val && val.constructor ? val.constructor.name: ''
const isValid= a => Array.isArray(a)? a.length>0: isObj(a)? Object.keys(a).length : !isEmpty(a)
const isEl = o => 
	typeof HTMLElement === "object" ? o instanceof HTMLElement :
	o && typeof o === "object" && o.nodeType === 1 && typeof o.nodeName==="string"
const getEl = el => isEl(el) ? el : 
	jqCheck(el)? el[0]: 
	typeof el=='string'? (('#'==el.charAt(0)|| el.indexOf('.')!=-1)? $(el)[0]: document.getElementById(el)): null;
const getJq = el => isEl(el) ? $(el) : 
	jqCheck(el)? el: 
	typeof el=='string'? (('#'==el.charAt(0)|| el.indexOf('.')!=-1)? $(el): $(document.getElementById(el))): null;

Object.prototype.update = function(...args) { return Object.assign(this,...args) }
Object.prototype.copy = function(...args) { return Object.assign({},this,...args) }
Object.prototype.isset = function(name) { return this.hasOwnProperty(name) }

String.prototype.lpad = function(padLength, padString) {
    let arrTxt = this;
	if(!padString) padString='0';
    while (arrTxt.length < padLength)
        arrTxt = padString + arrTxt;
    return arrTxt;
}
String.prototype.rpad = function(padLength, padString) {
    let arrTxt = this;
	if(!padString) padString='0';
    while (arrTxt.length < padLength)
        arrTxt += padString;
    return arrTxt;
}
/*
String.prototype.trim = function() { return this.replace(/^\s+|\s+$/g,"") }
String.prototype.ltrim = function() { return this.replace(/^\s+/,"") }
String.prototype.rtrim = function() { return this.replace(/\s+$/,"") }
*/

function getRandomColor() {
	var letters = '0123456789ABCDEF';
	var color = '#';
	for (var i = 0; i < 6; i++) {
		color += letters[Math.floor(Math.random() * 16)];
	}
	return color;
}
function getElRect(el) {
	const target = getEl(el)
	if(target) {
		return target.getBoundingClientRect()
	}
}
const setScrollTop = (parent, target) => {
	const a = getJq(parent), b=getJq(target)
	const yp = $(b).offset().top - $(a).offset().top
	const sp = yp + a.scrollTop()
	a.scrollTop(sp)
}
const getOffsetParent = target => {
	const a=getJq(target)
	if( !a[0] ) return {top:0,left:0}
	const tag = a.offsetParent().prop('tagName')
	if( tag=='HTML' ) {
		return a.offset()
	}
	let {top,left} = a.offset()
	let p=a.parent()
	while( p.prop('tagName')!='BODY' ) {
		const o=p.offset()
		top+=o.top
		left+=o.left
		p=p.parent()
	}
	return {top,left}
}
const getElOffset = (el, checkRect) => {
	const target=getEl(el)
	if( !target ) return;
	if( target.getBoundingClientRect ) {
		var m = target.getBoundingClientRect();
		var n = document.body;
		var c = document.documentElement;
		var a = window.pageYOffset || c.scrollTop || n.scrollTop;
		var h = window.pageXOffset || c.scrollLeft || n.scrollLeft;
		var l = c.clientTop || n.clientTop || 0;
		var o = c.clientLeft || n.clientLeft || 0;
		var r = m.top + a - l;
		var e = m.left + h - o;
		return checkRect ? 
			{ top: Math.round(r), left: Math.round(e), width: m.width, height: m.height } : 
			{ top: Math.round(r), left: Math.round(e) }
	}
	let aa = target;
	let left=0, top=0, width=aa.offsetWidth, height=aa.offsetHeight;
	while (aa) {
		left = c + parseInt(aa.offsetLeft);
		top = e + parseInt(aa.offsetTop);
		aa = aa.offsetParent
	}
	return checkRect? {top,left,width,height} : { top, left }
}

const screenSize = () => ({ width:$(window).width(), height:$(window).height() })

class WebsocketManager {
	constructor(url) {
		this.websocket = null
		this.url = url
		this.maxRetries = 3
		this.currentRetry = 0
		this.retryInterval = 10000
	}
	closeWebsocket() {
		if( this.websocket ) {
			this.websocket.close()
			this.websocket = null
		}
	}
	connect() {
		if( this.websocket ) {
			this.closeWebsocket()
		}
		const me = this
		this.currentRetry = 0
		this.websocket = new WebSocket(this.url);
		this.websocket.onopen = function() {
			console.log('웹소켓 연결 성공!')
		}
		this.websocket.onmessage = function(event) {
			try {
				const pos = event.data.indexOf('\r\n\r\n')
				if(pos!=-1) {
					const header = event.data.substr(0,pos)
					const message = event.data.substr(pos+4)
					// const node = JSON.parse(message);
					// 메시지 타입에 따른 처리
					me.recvData(header, message);
				}
			} catch (error) {
				console.error('메시지 파싱 오류:', error);
			}
		}
		this.websocket.onclose = function(event) {
			console.log('웹소켓 연결 종료:', event.code, event.reason);
			if (!event.wasClean && me.maxRetries && me.currentRetry < me.maxRetries) {
				me.currentRetry++;
				console.log(`${me.retryInterval/1000} 초 후 재연결 시도...`);
				setTimeout(me.connect, me.retryInterval);
			} else if (me.currentRetry >= me.maxRetries) {
				console.error('최대 재시도 횟수에 도달했습니다. 연결을 포기합니다.');
			}
		}
		this.websocket.onerror = function(error) {
			me.closeWebsocket()
		}
	}
	isConnect() {
		return this.websocket
	}
	recvData(header, data) {
		
	}
	sendData(type, header, param) {
        let message='', contentType=''
		if( param instanceof FileReader ) {
			const ab = param.result
			console.log('@@ websocket send readyState == ', param.readyState, ab)
			message = btoa(String.fromCharCode.apply(null, new Uint8Array(ab)));
			contentType='base64'
		} else if( param && typeof(param)=='object' ) {
            contentType= 'json'
            message = JSON.stringify(param)
        } else {
            contentType = 'text'
            message = param
        }
        if(ws==null) {
            updateConnectionStatus('전송오류 웹소켓 미정의', 'error')
            return
        }
        const size = stringByteLength(message)
        const data = '@'+type+'::'+header+'\r\n'+size+'::'+contentType+'::'+cf.wsVersion+'\r\n\r\n'+message
        console.log('@@ send\r\n@'+type+'::'+header+'\r\n'+size+'::'+contentType+'::'+cf.wsVersion)
        ws.send(data)
    } 
}

function websocketUploadFiles(ws) {
	const info={ws, uploadFiles:null, fileIndex:1, currentFile:null, chunkSize: 64 * 1024 }	
	// 파일아이디 생성 
	function generateFileId() {
		return 'file_' + Date.now() + '_' + Math.random().toString(36).substr(2, 9);
	}

	// 파일 크기 포맷 함수
	function formatFileSize(bytes) {
		if (bytes === 0) return '0 Bytes';
		const k = 1024;
		const sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
		const i = Math.floor(Math.log(bytes) / Math.log(k));
		return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
	}


	function start(files) {
		info.fileIndex=0
		info.currentFile
		info.uploadFiles=[]
		if( Array.isArray(files) && files.length>0 ) {
			files.map(c=>info.uploadFiles.push({
				file:c,
				name:c.name,
				size:c.size,
				type:c.type,
				lastModified:c.lastModified,
				fieldId:'',
				progress: 0,
				currentIndex: 0,
				currentChunk: 0,
				currentSendSize: 0,
				totalChunks:0
			}))
			startUpload()
		}
	}
	function startUpload() {
		if( info.uploadFiles.length==0 ) {
			return updateUploadStatus(`업로드 파일이 없습니다`);
		}
		uploadFile(info.uploadFiles.splice(0,1)[0]).then(e=>start).catch(e=>start)
	}
	function uploadFile(cur) {
		info.currentFile = cur
		return new Promise((resolve, reject) => {
			const reader = new FileReader();
			const file = cur.file
			cur.fieldId = generateFileId();
			cur.totalChunks = Math.ceil(cur.size / info.chunkSize);
			// 파일 정보 표시
			updateUploadStatus(`파일 업로드 시작: ${cur.name} (${formatFileSize(cur.size)})`);
			
			// 파일 청크 읽기
			reader.onload = function(e) {
				const {fieldId, name, size, currentIndex, currentChunk, totalChunks, lastModified, type } = cur
				console.log('reader onload ', reader, ws, appendParam(fieldId, name, size, currentIndex, currentChunk, totalChunks, lastModified, type))
				ws.sendData('req_chunkFileUpload', appendParam(fieldId, name, size, currentIndex, currentChunk, totalChunks, lastModified, type), reader)
				
				cur.currentChunk += cur.currentSendSize
				// 다음 청크 읽기 또는 완료
				if (cur.currentChunk < cur.size ) {
					cur.progress = Math.round((cur.currentIndex / cur.totalChunks) * 100);
					
					cur.currentIndex++
					updateUploadStatus(`파일 업로드 중: ${cur.name} (${cur.progress}%)`);
					readNextChunk();
				} else {
					resolve()
				} 
			};
			
			// 다음 청크 읽기 함수
			function readNextChunk() {
				const start = cur.currentChunk;
				const end = Math.min(start + info.chunkSize, cur.size);
				cur.currentSendSize = end - start
				console.log('@@ readNextChunk ', start, end, cur.currentSendSize)
				reader.readAsArrayBuffer(file.slice(start, end));
			}
			
			// 첫 번째 청크 읽기 시작
			readNextChunk();
		});
		return {}
	} 

	// 업로드 상태 표시 함수
	function updateUploadStatus(message, type = 'info') {
		console.log('@@ upload status >> '+ message)
	}
	return {info, start}
}
const getLocalId = (prefix, arr) => {
	const idx = arr.length+''
	return prefix+'_'+idx.lpad(2,'0')
}
const loadCss = (src) => {
	var link = document.createElement("link");
	link.href = src;
	link.async = false;  
	link.rel = "stylesheet";
	link.type = "text/css";    
	document.head.appendChild(link);  
}
const loadStyle = (src) => {
	const el = document.createElement('style');
	el.textContent = src;
	document.head.appendChild(el);
}

class Apps {
	constructor(target) {	
		this.apps = []
		this.targetEl = getJq(target)
		this.currentApp = null
		this.menus = null
		this.tabs = null
		this.currentAddCode = null
	}
	getApp(code) {
		return this.apps.find(cur=>cur.code==code)
	}
	makeApp(code, appInfo) {
		const sty = cf.styles
		if( !jqCheck(this.targetEl) ) return clog(`@@ Apps 타겟객체 미정의`)
		if( this.getApp(code) ) return clog(`@@ Apps ${code} 앱이 이미 추가됨`)
		const appStyle = {}
		if( isObj(appInfo) ) {
			if(appInfo.isset('color')) appStyle.color=appInfo.color
			if(appInfo.isset('bg')||appInfo.isset('background')) appStyle.background=appInfo.background||appInfo.bg
			// background: getRandomColor()
		} else {
			appInfo={}
		}
		clog('appStyle===>', appStyle, appInfo)
		const container = $('<div/>').css(appStyle.update(sty.full, sty.flexcenter)).appendTo(this.targetEl)
		const app = new App(container, code, appInfo )
		this.apps.push(app)
		if( this.currentAddCode==null ) {
			const me = this
			this.currentAddCode = code
			setTimeout(()=> {
				me.setCurrentApp(me.currentAddCode)
				me.currentAddCode = null
			}, 100)
		} else {
			this.currentAddCode = code
		}
		return app
	}
	setCurrentApp(code) {
		const app = this.getApp(code)
		if( app ) {
			this.apps.map(cur=>cur.hideApp())
			this.currentApp = app
			app.showApp()
		} else {
			clog(`@@ Apps.setCurrentApp ${code} 앱오류`)
		}
	}
	reload() {
		if( !this.currentApp ) return
		this.currentApp.reload()
	}
}

class App {
	constructor(container, code, appInfo) {
		this.pages=[]
		this.containerEl = container
		this.contentEl = null
		this.code = code
		this.name = appInfo.name||''
		this.appInfo = appInfo
		this.currentAddPageId = null
		this.currentPage = null
		this.currentPopup = null
		this.layout = appInfo.isset('layout') ? this.makeLayout(appInfo.layout): null
	}
	loadPage(url) {
		if( !jqCheck(this.contentEl) ) return clog(`@@loadPage 부모 content 미정의  ${url} 페이지 로드오류`)
		const me = this
		apiGet(cf.devHost+url, res => me.makePage(res.pageId, this.contentEl, res) )
	}
	deleteLayout() {
		
	}
	makeLayout(layoutInfo) {
		if(this.layout ) {
			this.deleteLayout()
		}
		const layout = new PageLayout(this.containerEl, layoutInfo, null, target)
		if( layout.kind=='content' ) {
			this.contentEl = layout.el
		}
		return layout
	}
	getPage(pageId) {
		return this.pages.find(cur=>cur.id==pageId)
	}
	makePage(pageId, targetEl, pageInfo) {
		if( this.getPage(pageId) ) return clog(`@@makePage ${pageId} 페이지 이미 추가됨`)
		const page = new Page(pageId, targetEl, pageInfo)
		this.pages.push(page)
		if( this.currentAddPageId==null ) {
			const me = this
			this.currentAddPageId = pageId
			setTimeout(()=> {
				me.setCurrentPage(me.currentAddPageId)
				me.currentAddPageId = null
			}, 100)
		} else {
			this.currentAddPageId = pageId
		}
	}
	setCurrentPage(pageId, reload) {
		const page = this.pages.find(cur=>cur.id==pageId)
		if( page ) {
			this.pages.map(cur=>cur.hidePage())
			this.currentPage = page
			page.showPage()
			if(reload) {
				page.rendor()
			}
		} else {
			clog(`@@ Apps.setCurrentPage ${pageId} page 오류`)
		}
	}
	hideApp() {
		if(!jqCheck(this.contentEl)) return clog('@@ app hideApp 대상오류', this.dump())
		this.contentEl.hide()
	}
	showApp() {
		if(!jqCheck(this.contentEl)) return clog('@@ app showApp 대상오류', this.dump())
		this.contentEl.show()
	}
	reload() {
		if( this.currentPopup) {
			this.currentPopup.reload()
		}
		if( this.currentPage ) {
			this.currentPage.reload()
		}
	}
	
 
	dump() {
	
	}
}
class Page {
	constructor(pageId, targetEl, pageInfo) { 
		this.id = pageId
		this.info = pageInfo
		this.targetEl = targetEl
		const css = pageInf.css || {}
		this.pageEl = $('<div/>').css(css.update(cf.styles.pageBox)).appendTo(targetEl)
		this.layout = this.makeLayout(pageInfo.layout)
	}
	deleteLayout() {
		
	}
	makeLayout(layoutInfo) {
		if(this.layout ) {
			this.deleteLayout()
		}
		const layout = new PageLayout(this.pageEl, layoutInfo, null, target)
		if( layout.kind=='content' ) {
			this.contentEl = layout.el
		}
		return layout
	}
	showPage() {
		this.pageEl.show()
	}
	hidePage() {
		this.pageEl.hide()
	}
	reload() {
		
	}
	findEl(selector) {
		return this.layout.findEl(selector)
	}
}
	
class PageLayout {
	constructor(parentEl, layoutInfo, parentLayout, target) {
		this.target = target
		this.parentLayout = parentLayout
		this.parentEl = parentEl
		this.layoutInfo = layoutInfo
		this.el = null
		this.kind = layoutInfo.kind||''
		this.childLayout = []
		this.createLayout(layoutInfo)
	} 
	createLayout(layout) {
		const tag = layout.tag || 'div'
		const sty = layout.style || {}
		this.el = $('<'+tag+'/>').css(sty).appendTo(this.parentEl)
		if( layout.class) this.el.attr('class', layout.class)
		if( Array.isArray(layout.children) ) {
			for( cur of layout.children ) {
				const obj = new PageLayout(this.el, cur, this, this.target)
				this.childLayout.push(obj)
			}
		}
	}
	findEl(selector) {
		return this.el.find(selector)
	}	
}

const cf = {
	apps: null
	, devMode: false 		// 개발자모드
    , websocket: new WebsocketManager('ws://localhost:8092/chat') // 개발자모드 실시간 메시지 처리
	, styles:{				// 공통스타일
		full:{width:'100%',height:'100%'},
		flexcenter: {display:'flex', alignItems:'center', justifyContent:'center' },
		pageBox: {display:'flex', alignItems:'center', justifyContent:'center', flexDirection:'column', width:'100%',height:'100%' }
	}
	, devHost: 'http://localhost'
	, apiHost: 'http://localhost:8000'
}