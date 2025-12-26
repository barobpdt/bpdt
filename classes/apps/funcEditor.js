class conf {
editorSrc: 
	<page id="#{0}"><editor id="editor"></page>
tabStyle: 
	<text>
		QTabWidget::pane {
		  border: 1px solid lightgray;
		  top:-1px; 
		  background: rgb(245, 245, 245);; 
		} 

		QTabBar::tab {
			background: rgb(245, 245, 245); 
		  border: 1px solid lightgray; 
		  padding: 5px;
		} 

		QTabBar::tab:selected { 
		  background: rgb(90, 90, 120);
		  color: #fff;
		  margin-bottom: -1px; 
		}
	</text>	
}

class layout {
	<page id="main" margin="0" onInit() {
		this.put(sep, titleBar)
		leftPanel=page('leftPanel')
		contentPanel=page('contentPanel')
		sep.addPage(leftPanel)
		sep.addPage(contentPanel)
		apiClient=class('ProxyClient')
		class(this,'main', true)
		class(leftPanel,'leftPanel', true)
		class(contentPanel,'contentPanel', true)
		class(titleBar, 'titleBar', true)
		this.positionLoad()
		this.timer(200, this.setSepSize)		
	}>
		<context id="titleBar" height="40">
		<splitter id="sep">
	</page>
	
	<page id="leftPanel">
		<tab id="leftTab">
		<context id="leftBar" height=30>
	</page>
	<page id="funcTree"><tree id="tree"></page>
	<page id="workTree"><tree id="tree"></page>
	
	<page id="contentPanel">
		<div id="content">
		<div id="contentBar" height=30>
	</page>
	<input id="filter">
		onKeyDown(k,a,b) {
			print("keydown ", k,a,b)
		}
	</input>
}

class main {
	initClass() {
		base=this.base()
		if(base) {
			apiClient.start(base)
			apiClient.setTarget(base, this)
		}
		@page.spacing(this, 2)
		print("=== func editor page init class ok ===", this)
	}
	onClose() {
		this.positionSave()
	}
	setSepSize() {
		total=arraySum(sep.sizes());
		if(total>600) {
			arr=arrayDivid("225,*",total);
		} else {
			arr=arrayDivid("3,7",total)
		}
		sep.sizes(arr);
	}
	apiCall(url) {
		base=this.base() not(base) return print("api call error 페이지 base 미정의")
		socket=apiClient.socket(base)
		not(socket) {
			apiClient.var(apiUrl, url)
			apiClient.start(base)
			return;
		}
		apiClient.send(socket,'api',url)
	}
	apiResult(uri, data, param) {
		print("api result $url, $data", param)
	}
}
class titleBar {
	onDraw(dc, rc) {
		dc.fill('#eee')
		dc.rectLine(rc.incrH(-1), 4, '#ccc', 2)
	}
}
class leftPanel {
	this.put(leftTab, leftBar)
	funcTree=page('funcTree')
	workTree=page('workTree')
	class(leftBar, 'leftBar', true)
	class(findId(funcTree,'tree'), 'funcTree', true)
	class(findId(workTree,'tree'), 'workTree', true)
	initClass() { 
		@page.spacing(this,0)
		@page.margin(this,4,0,2,0)
		@page.margin(funcTree, 0,2,2,0)
		@page.margin(workTree, 0,2,2,0)
		leftTab.styleSheet(this.conf('tabStyle'));
		leftTab.addPage(funcTree,'함수정보')
		leftTab.addPage(workTree,'작업정보')
		leftTab.current(0)
	}
	setTabChange(code) {
		if(code=='funcTree') leftTab.current(funcTree)
		else leftTab.current(workTree)
	}
}
class contentPanel {
	this.put(content, contentBar)
	class(content, 'content', true)
	class(contentBar, 'contentBar', true)
	initClass() {
		pageId="contentDefaultEditor"
		page=this.makePage("editorSrc",pageId)
		this.addEditor(page)
		print("contentPanel init class default page ==>", page)
	}
	addEditor(page) {
		content.addPage(page, true)
	}
}

class leftBar {
	onDraw(dc, rc) {
		dc.fill('#eee')
		dc.rectLine(rc.incrH(-1), 4, '#ccc', 2)
		dc.text('funcEditor leftBar','center')
	}
} 
class contentBar {
	onDraw(dc, rc) {
		dc.fill('#eee')
		dc.rectLine(rc.incrH(-1), 4, '#ccc', 2)
		dc.text('funcEditor contentBar','center')
	}
}

class funcTree {
	tree=this
	onMouseDown(p,a) {
		print("funcEditorTree mouse down", p,a)
	}
	setTreeData() {
		
	}
}
class workTree {	 
	tree=this
	setTreeData() {
		
		
	}
}