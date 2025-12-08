/* 
페이지 공통설정 
*/
const clog=window.console.log
const tm = ()=> new Date().getTime()
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
Object.prototype.cmp = function(name, value) { return this.isset(name) && this[name]===value }

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
String.prototype.splitComma = function() {
	return this.split(',').map(v=>v.trim())
}
/*
String.prototype.trim = function() { return this.replace(/^\s+|\s+$/g,"") }
String.prototype.ltrim = function() { return this.replace(/^\s+/,"") }
String.prototype.rtrim = function() { return this.replace(/\s+$/,"") }
*/
function getRenderElement(name) {
	const app = cf.apps.currentApp
	const page = app.currentPage
	return page? page.elementMap[name]: null
}
function setRenderElement(name, el) {
	const app = cf.apps.currentApp
	const page = app.currentPage
	if(page) {
		page.elementMap[name]=el
	}
}

function getPageEl() {
	const app = cf.apps.currentApp
	if( arguments.length<2 ) {
		const page = app.currentPage;
		if(!page) {
			clog('@@ getPageEl page 미정의', arguments)
			return null
		}
		if(arguments.length==0) {
			return page
		}
		if(arguments.length==1) {
			const selector = arguments[0];
			return page.contentEl ? page.contentEl.find(selector): null
		}
	}
	else if(arguments.length==2) {
		const pageId = arguments[0]
		const page = app.getPage(pageId)
		return page && page.containerEl ? page.containerEl.find(selector): null
	}
	clog('@@ getPageEl 요소찾기 실패', arguments)
	return null
}
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

function WebsocketManager(serverUrl ) {
	if(!serverUrl) serverUrl = 'ws://localhost:8092/chat'
	const WS_READY = 0
	const WS_START = 1
	const WS_CONNECT = 3
	const WS_JOIN = 4
	const WS_DISCONNECT = 5
	const WS_ERROR = 9
	
	let ws = null
	let callbackFunc = null
	let reconnectTimeout = 0
	let wsStatus=WS_READY

	// 채팅 입장
	function getStatus() {
		return wsStatus;
	}

	// WebSocket 연결
	function connectWebSocket() {
		updateConnectionStatus(WS_START, false);
		
		// WebSocket 연결 (ws:// 프로토콜 사용)
		ws = new WebSocket(serverUrl);

		// 연결 성공
		ws.onopen = () => {
			console.log('✅ WebSocket 연결 성공');
			updateConnectionStatus(WS_CONNECT, true);
			// 방 입장
			sendWsMessage('join', { mode:'dev' });
		};

		// 메시지 수신
		ws.onmessage = (event) => {
			try {
				clog('@@ websocket onMessage ==> '+event.data)
				const { type, data } = JSON.parse(event.data)
				handleMessage(type, data)
			} catch (error) {
				console.error('메시지 파싱 오류:', error);
			}
		};

		// 연결 종료
		ws.onclose = () => {
			updateConnectionStatus(WS_DISCONNECT, false);
			reconnectTimeout = setTimeout(() => {
				console.log('🔄 재연결 시도...');
				connectWebSocket();
			}, 5000);
		};

		// 에러 처리
		ws.onerror = (error) => {
			console.error('WebSocket 오류:', error);
			updateConnectionStatus(WS_ERROR, false);
		};
	}
	function isConnect() {
		return ws && ws.readyState === WebSocket.OPEN
	}
	function isJoin() {
		return wsStatus === WS_JOIN
	}
	// WebSocket 메시지 전송
	function sendWsMessage(type, data) {
		if( isConnect() ) {
			ws.send(JSON.stringify({ type, data }));
		} else {
			clog('@@ sendWsMessage error: websocket not connect')
		}
	}

	// 메시지 핸들러
	function handleMessage(type, data) {
		if( typeof callbackFunc =='function' ) {
			result = callbackFunc(type,data)
			if(result) return
		}
		switch (type) {
		case 'joined':
			updateConnectionStatus(WS_JOIN, true)
			break;	
		case 'error':
			clog('서버 오류:', data.message);
			break;
		}
	}

	// 연결 상태 업데이트
	function updateConnectionStatus(status, connected) {
		wsStatus = status;
	}
	 
	// HTML 이스케이프
	function escapeHtml(text) {
		const div = document.createElement('div');
		div.textContent = text;
		return div.innerHTML;
	}
	function setCallbackFunc(fc) {
		if(typeof fc=='function' ) {
			callbackFunc = fc
		} else {
			callbackFunc = null
		}
	}
	function closeSocket() {
		if (ws) {
			ws.close();
			wsStatus = WS_READY
			ws = null
		}
	}
	// 페이지 종료 시 WebSocket 닫기
	window.addEventListener('beforeunload', () => {
		if (ws) {
			ws.close();
		}
	});
	return {getStatus, isConnect, isJoin, connectWebSocket, handleMessage, sendWsMessage, closeSocket, setCallbackFunc}
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
const apiGet = async (url, param) => {
	if( isObj(param) ) {
		const query = Object.keys(param).map(k=>encodeURIComponent(k)+'='+encodeURIComponent(param[k]) ).join('&')
		url += (url.indexOf('?')==-1 ? '?':'&') + query
	}
	const res = await fetch(url)
	return res.text()
}
const apiCall = async (method, url, param, callback) => {
	if(typeof callback !== 'function') {
		callback = data => clog("apiCall 콜백함수 미정의 ", url, method, data)
	}
	const urlCall = url.startsWith('http')? url: cf.apiHost+url
	if(method=='GET') {
		if( isObj(param) ) {
			const query = Object.keys(param).map(k=>encodeURIComponent(k)+'='+encodeURIComponent(param[k]) ).join('&')
			url += (url.indexOf('?')==-1 ? '?':'&') + query
		}
		fetch(url).then(res=>res.json()).then(data=>callback(data)).catch(err=>apiCallError(url,err))
	} else {
		const params = {
			method,
			mode:'cors',
			credentials: 'include',
			headers: {"Content-Type": "application/json"},
			body: JSON.stringify(param)
		}
		fetch(url,params).then(res=>res.json()).then(data=>callback(data)).catch(err=>apiCallError(url,err))
	}
}
const apiCallError = (url, err) => {
	clog(url + "API 호출오류 =>", err)
}
const tagBtn3d = (target, text, style) => {
	if( style ) loadStyle(style)
	const el = $('<button type="button" class="btn3d"/>').css({marginLeft:10}).appendTo(getJq(target))
	$('<div class="top">').html(text).appendTo(el)
	$('<div class="bottom">').appendTo(el)
	return el
}


async function isUserSessionValid(token, redirectPage) {
    const options = {
        method: "POST",
        headers: {
            "Content-Type": "application/json",
            "Authorization": token
        },
        body: token
    };
    try {
        const response = await fetch(cf.apiHost+"/session/isValidToken", options);
        if(response.ok) {
            sessionStorage.setItem("user", JSON.stringify(await response.json()));
            return true;
        }
    } catch(e) {
    }
	localStorage.clear();
    sessionStorage.clear();
    if( redirectPage) location.replace("../login/login.html");
    return false;
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
	createApp(code, appInfo, callback) {
		if( !jqCheck(this.targetEl) ) return clog(`@@ Apps 타겟객체 미정의`)
		if( this.getApp(code) ) return clog(`@@ Apps ${code} 앱이 이미 추가됨`)
		const appStyle = getCss('vbox',{height:'100%'})
		if( isObj(appInfo) ) {
			if(appInfo.isset('color')) appStyle.color=appInfo.color
			if(appInfo.isset('bg')||appInfo.isset('background')) appStyle.background=appInfo.background||appInfo.bg
			// background: getRandomColor()
		} else {
			appInfo={}
		} 
		const container = $('<div class="app-'+code+'"/>').css(appStyle).appendTo(this.targetEl)
		const app = new App(container, code, appInfo )
		if( typeof(callback)=='function' ) {
			app.appStartCallback = callback
		}
		this.apps.push(app)
		if( this.currentAddCode==null ) {
			const apps = this
			this.currentAddCode = code
			setTimeout(()=> {
				apps.setCurrentApp(apps.currentAddCode)
				apps.currentAddCode = null
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
			if( app.appStartTime==0 ) {
				if( typeof(app.appStartCallback)=='function' ) {
					app.appStartCallback(app)
				}
				app.appStartTime=new Date().getTime()
			}
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
		this.appStartTime = 0
		this.appStartCallback = null
		this.currentAddPageId = null
		this.currentPage = null
		this.currentPopup = null
		this.layout = this.makeLayout(appInfo.layout)
	} 
	loadPage(name) {
		const path = cf.devHost+cf.pagesBase
		$.getScript(path+name+'.js', ()=>clog('>> app loadPage NAME: '+name+' 페이지 준비완료'))
	}
	deleteLayout() {
		this.layout = null
	}
	makeLayout(layoutInfo) {
		if( this.layout ) {
			this.deleteLayout()
		}
		if( !isObj(layoutInfo)) {
			this.contentEl = this.containerEl
			return null
		}
		const layout = new LayoutTree(this.containerEl, layoutInfo, null, this)
		const content = layout.findContent()
		if( !isObj(layoutInfo.style) ) {
			setCss(layout.el, 'pageContent', {overflow:'auto'})
		}
		if( content ) {
			this.contentEl = content
		} else {
			this.contentEl = layout.el
		}
		return layout
	}	
	deletePage(page) {
		if( jqCheck(page.pageEl)) {
			if( this.currentPage==page) {
				this.currentPage = null
			}
			page.layout = null
			page.pageEl.remove()
			this.pages = this.pages.filter(cur=>cur!==page)
			return true
		}
		return false
	}
	getPage(pageId) {
		return this.pages.find(cur=>cur.id==pageId)
	}
	createPage(pageId, pageInfo, pageImpl) {
		const prev = this.getPage(pageId)
		if( prev ) {
			clog('@@ createPage 이전페이지가 존재합니다 이전페이지 삭제처리')
			if(!this.deletePage(prev)) return clog('>> createPage 이전 페이지 삭제오류 [pageId]=='+pageId)
		}
		clog('@@ createPage id:'+pageId+' [this.currentAddPageId]=='+this.currentAddPageId)
		const page = new Page(pageId, pageInfo, this, pageImpl)
		this.pages.push(page)
		if( this.currentAddPageId ) {
			this.currentAddPageId = pageId
		} else {
			const app = this
			this.currentAddPageId = pageId
			setTimeout(()=> app.startPage(), 100)
		}
		// clog('>>> create page ', pageImpl)
	}
	startPage() {
		const pageId = this.currentAddPageId
		if( pageId ) {
			this.currentAddPageId = null
			this.setCurrentPage(pageId)
		} else {
			clog('>> startPage 호출오류 페이지아이디 미설정')
		}
	}
	setCurrentPage(pageId) {
		if( !pageId ) return clog('>> setCurrentPage pageId not defined !!!')
		const page = this.pages.find(cur=>cur.id==pageId)		
		if( page ) {
			if( page === this.currentPage ) return clog('>> setCurrentPage same page [pageId]=='+pageId)
			this.pages.map(cur=>cur.hidePage())
			this.currentPage = page
			page.showPage()
		} else {
			clog('>> setCurrentPage 페이지 찾기오류 [pageId]=='+pageId)
		} 
		if( isObj(page) && page.pageStartTime==0 ) {
			if( typeof(page.initPage) =='function') {
				page.initPage()
			}
			page.pageStartTime = new Date().getTime()
		}
		return page;
	}
	hideApp() {
		if(!jqCheck(this.containerEl)) return clog('@@ app hideApp 대상오류', this.dump())
		this.containerEl.hide()
	}
	showApp() {
		if(!jqCheck(this.containerEl)) return clog('@@ app showApp 대상오류', this.dump())
		this.containerEl.show()
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
	constructor(pageId, pageInfo, parentApp, pageImpl) {
		const css = getCss('pageContent')
		if(pageInfo.style ) css.update(pageInfo.style) 
		this.id = pageId
		this.info = pageInfo
		this.app = parentApp
		this.contentEl = null
		this.pageStartTime = 0
		this.elementMap = {}
		this.pageEl = $('<div/>').css(css).appendTo(parentApp.contentEl)
		this.pageEl.addClass('page-content')
		this.layout = this.makeLayout(pageInfo.layout)
		if( isObj(pageImpl) ) {
			// clog('page init ==>', pageId, Object.keys(pageImpl))
			for(let key of Object.keys(pageImpl) ) {
				if( pageImpl.hasOwnProperty(key)) {
					const fc = pageImpl[key]
					// clog('>> page::constructor', key, fc)
					if(typeof(fc)=='function' && !this.hasOwnProperty(key) ) {
						this[key] = fc
					}
				}
			}
		}
	}
	
	makeLayout(layoutInfo) {		
		const layout = new LayoutTree(this.pageEl, layoutInfo, null, this)
		this.contentEl = layout.findContent()
		return layout
	}
	findEl(selector) {
		return this.layout.findEl(selector)
	}
	showPage() {
		this.pageEl.show()
	}
	hidePage() {
		this.pageEl.hide()
	}
	reload() {
		
	}
}
	
class LayoutTree {
	constructor(parentEl, layoutInfo, parentLayout, target) {
		this.target = target
		this.parentLayout = parentLayout
		this.parentEl = parentEl
		this.el = null
		this.layoutInfo = layoutInfo
		this.contentUse = false 
		this.childLayout = []
		if( isObj(layoutInfo) ) {
			this.contentUse = layoutInfo.content===true
			this.createLayout(layoutInfo)
		}
	}
	createLayout(layout) {
		const tag = layout.tag || 'div'
		const sty = layout.style || {}
		this.el = $('<'+tag+'/>').css(sty).data('layout-node',this).appendTo(this.parentEl)
		if( layout.className) this.el.attr('class', layout.className)
		if( this.contentUse) this.el.css(getCss('pageContent',{overflow:'auto'}))
		if( Array.isArray(layout.children) ) {
			for( let cur of layout.children ) {
				const obj = new LayoutTree(this.el, cur, this, this.target)
				this.childLayout.push(obj)
			}
		}
	}
	findEl(selector) {
		return this.el.find(selector)
	}
	findContent() {
		if( this.contentUse ) return this.el
		for(let cur of this.childLayout ) {
			const el = cur.findContent()
			if( el ) return el
		}
		return null
	}
}

function getCss() {
	const sty = cf.styles
	const css = {}
	for(v of arguments) {
		if( typeof v=='string') {
			if(sty.isset(v)) {
				Object.assign(css,sty[v])
			} 
		} else if( isObj(v)) {
			Object.assign(css,v)
		}
	}
	return css
}

function setCss(el) {
	if(!jqCheck(el)) return clog('>> setCss HTML Element 오류 ')
	const sty = cf.styles
	const css = {}
	let n=0
	for(v of arguments) {
		if( n++ > 0 ) {
			if( typeof v=='string') {
				if(sty.isset(v)) {
					Object.assign(css,sty[v])
				} 
			} else if( isObj(v)) {
				Object.assign(css,v)
			}
		}
	}
	el.css(css)
}
function makePage(pageId, initFunc) {
	if(typeof initFunc!=='function') {
		initFunc=function(page,content) {
			clog(`페이지 생성오류 초기화 함수 미정의 페이지아이디:${pageId}`, page, content)
		}
	}
	const pageImpl = {
		initPage: function() { initFunc(this, this.contentEl) }
	}
	const layout = {
		tag:'div'
		, style: getCss('pageContent')
		, content: true
	}
	const pageInfo = {id:pageId, layout}
	const app = cf.apps.currentApp
	app.createPage(pageId, pageInfo, pageImpl)
}
const mapAt = (map,idx) => { 
    let n=0
    for(const a of map) {
        if(n==idx) return a[1]
    }
    return null
}
const cf = {
	apps: null
	, devMode: false 		// 개발자모드
    , websocket: new WebsocketManager('ws://localhost:8092/chat') // 개발자모드 실시간 메시지 처리
	, styles:{				// 공통스타일
		full:{width:'100%',height:'100%'},
		itemCenter: {display:'flex', alignItems:'center', width:'100%',height:'100%'},
		flexCenter: {display:'flex', alignItems:'center', justifyContent:'center', width:'100%',height:'100%' },
		pageContent: {display:'flex', flexDirection:'column', flex:1, position:'relative', width:'100%' },
		row: {display:'flex', flexDirection:'row'}, 
		col: {display:'flex', flexDirection:'column'}, 
		hbox: {display:'flex', flexDirection:'row', height:'100%' },
		vbox: {display:'flex', flexDirection:'column', width:'100%' }
	}
	, devHost: 'http://localhost'
	, apiHost: 'http://localhost:8000'
	, pagesBase: '/assets/pages/'
}

function createApp(appId, name, pageId) {
	cf.apps = new Apps('apps')
	const layout = {
		tag:'div'
		, children:[
			{tag:'div',style:getCss('full',{height:30,background:getRandomColor()}), className:'appTop'},
			{tag:'div',style:{flex:1}, className:'appContents', content:true},
			{tag:'div',style:{height:30,background:getRandomColor()}, className:'appFooter'},
		]
	}
	if(!pageId) pageId='main'
	return cf.apps.createApp(appId, {name, layout}, app=>app.loadPage(pageId))
	
}