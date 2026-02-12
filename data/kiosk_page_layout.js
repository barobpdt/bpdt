dev:main{
<page>
		<editor id="src">
		<hbox>
			<button id="run" text="실행"><space>
			<button id="cancel" text="취소">
		</hbox>
	</page>
}
common:image{
<page margin=4> 
	<hbox margin=0>
		<label text="ID : " width=45  height=24 align=right>
		<input id=idSearch width=150 height=24>
		<combo id=imageGroup height=24>
		<button id=search text=조회 height=26>
		<space>
		<button id=clipboardCapture text="클립보드 캡쳐" height=26>
		<button id=newImage text="이미지생성" height=26>
	</hbox>
	<tree id=grid>
	</page>
}
devTool:pages{
<page margin=0 spacing=0>
	<splitter id=splitter stretchFactor=content handleWidth=4>
		<vbox margin=0>
			<hbox margin="0,2,0,0">
				<button id=pageCreate text="페이지 생성" icon="vicon.brick_add"><space>
				<button id=allSource text="전체소스" icon="vicon.asterisk_yellow">
			</hbox>
			<tree id=tree stretch=1>
			<webview id=help>
		</vbox>
		<div id=content>
	</splitter>
	<hbox margin=0 spacing=0>
		<canvas id=status height=24 expand=true><canvas id=info width=420 height=24>
	</hbox>
</page>
}
DevFuncEdit:main{
<page>
		<hbox>
			<label text="페이지 그룹 :"><hbox spacing=0>
				<combo id="pageGroup" height=24>
				<toolbutton id="pageGroupButton" icon="vicon.add_default">
			</hbox>
			<label text="페이지 코드 :">
			<combo id="pageCode" width=160 height=24> 
			<combo id="pageSub" height=24>
			<button id="pageOpen" text="페이지 열기">
			<space>
			<toolbutton id="imageOpen" icon="ficon.photo-album" tip="이미지소스 보기">
			<toolbutton id="confManagerOpen" icon="ficon.report--pencil" tip="공통설정창 열기">
			<toolbutton id="devPageOpen" icon="ficon.table-money" tip="개발툴 열기">
			<button id="helpOpen" text="도움말">
			<button id="pageSourceLoad" text="페이지 소스 불러오기">
		</hbox>
 		<hbox>
			<label text="페이지 함수 : "><combo id="pageFuncCombo" width=165 height=24>
			<hbox margin="10,2,10,2" spacing=0>
				<combo id="pageVarCombo" width=145 height=24><toolbutton id="pageVarPlus" icon="vicon.add_default">
			</hbox>
			<hbox>
				<label id="pageEventLabel" text="페이지 이벤트 : ">
				<combo id="pageEventCombo" width=125>
			</hbox>
			<hbox>
				<label id="pageClassLabel" text="페이지 클래스 : ">
				<combo id="inheritCombo" width=160 height=24> 
				<combo id="classVarCombo" width=150 height=24> 
				<combo id="classFuncCombo" width=185 height=24> 
			</hbox>
			<space>
		</hbox>	
 		<tab id="tab">
	</page>
}
FuncManager:funcTree{
<page margin=2>
	<tree id="tree">
	<hbox>
		<combo id="func_type"><combo id="sort_field"><combo id="sort_order"><space>
		<toolbutton id="expandClose" icon="ficon.folder-horizontal" tooltip="전체 접기">
	</hbox>
</page>
}
FuncManager:main{
<page margin=0>
		<splitter stretchFactor="content">
			<div id="left">
			<div id="content">
		</splitter>
		<hbox>
			<canvas id="status" height=18>
		</hbox>
	</page>
}
FuncManager:funcGrid{
<page>
	<hbox>
		<label text="함수그룹 : "><input id="func_grp" width=85>
		<label text="함수명 : "><input id="func_nm" width=105>
		<label text="유형 : "><combo id="func_type"><space>
		<toolbutton id="initBtn" icon="vicon.asterisk_orange" tip="조회폼 초기화">
		<button id="search" text="조회" icon="igims.search">
	</hbox>
	<grid id="grid">
	<hbox>
		<label id="subStatus" stretch=1><button id="delete" text="삭제" icon="vicon.cancel_default">
	</hbox>
</page>
}
DevFuncEdit:confManager{
<page title="공통설정 관리" icon="ficon.application-list">
		<hbox>
			<combo id="conf_type" height=24>
			<input id="conf_cd" width=125 height=24>
			<input id="conf_id" width=140 height=24>
			<combo id="conf_kind" width=140 height=24><space>
			<button id="search" text="조회">
		</hbox>
		<editor id="src">
		<hbox>
			<button id="save" text="저장	">
			<input id="conf_nm" height=24><button id="copy" text="복사">
			<space>
			<button id="cancel" text="취소">
		</hbox>
	</page>
}
DevFuncEdit:coreFuncManager{
<page>
	<hbox>
		<combo id="funcCombo"><input id="funcName">
	</hbox>
	<tab id="tab">
</page>
}
DevFuncEdit:pageFuncTab{
<page>
	<tab id="tab">
</page>
}
DevFuncEdit:pageSourceLoadPopup{
<page title="페이지 선택" icon="ficon.application--plus">
	<group title="페이지 구분">
		<vbox>
			<hbox>
				<label text="페이지 파일선택:"><combo id="pageFileSelectCombo" width=200>
				<button id="pageOpen" text="페이지 열기"><space>		
			</hbox>
			<hbox>
				<label text="페이지 클래스 선택:"><combo id="pageClassTemplate" width=220><space>
			</hbox>
			<grid id="grid">
		</vbox>
	</group>
	<hbox>
		<label text="페이지 그룹 :"><combo id="pageGroup" height=24>
		<label text="페이지 코드 :"><combo id="pageCode" width=160 height=24>
		<button id="save" text="저장">
		<button id="cancel" text="취소">
	</hbox>
</page>
}
DevFuncEdit:pageSourceTab{
<page>
	<hbox>
		<label text="페이지 소스경로:"><input id="pageSourcePath"><space>
	</hbox>
	<hbox>
		<label text="클래스 소스경로:"><input id="classPath"><space>
	</hbox>
	<tab id="tab">
</page>
}
Common:PreviewFunction{
<page>
		<hbox margin=5 spacing=5>
			<hbox margin="0,0,10,0" spacing=2>
				<combo id="funcGroup" width=100 height=24>
				<combo id="funcName" width=180 height=24>
			</hbox>
			<combo id="funcHist" width=220 height=24>
			<button id="closeCurrent" text="소스 닫기">
			<button id="closeAll" text="전체소스 닫기">
			<label id="funcStatus" stretch=1>
			<toolbutton id="btnLock" icon="vicon.application_link" tip="화면고정 해제">
			<toolbutton id="btnClose" icon="ficon.cross-button" tip="창닫기">			
		</hbox>
		<div id="content">
	</page>
}
Common:ConfManager{
<page title="공통설정 관리" icon="ficon.application-list">
			<hbox>
				<combo id="conf_type" height=24>
				<input id="conf_cd" width=125 height=24>
				<input id="conf_id" width=140 height=24>
				<input id="conf_kind" width=140 height=24><space>
				<button id="search" text="조회">
			</hbox>
			<editor id="src">
			<hbox>
				<button id="save" text="저장	">
				<input id="conf_nm" height=24><button id="copy" text="복사">
				<space>
				<button id="cancel" text="창닫기">
			</hbox>
		</page>
}
Common:FuncEditPage{
<page>
			<editor id="src">
			<hbox>
				<button id="save" text="저장" icon="vicon.database_save">
				<button id="run" text="함수실행" icon="vicon.package_go">
				<label text="함수설명 : "><input id="func_desc" stretch=2>
				<combo id="func_type">
				<label stretch=1>
				<hbox margin="4,0,0,0" spacing=4>
					<label text="찾기 : ">
					<input id="inputSearch" width=115 height=24>
					<toolbutton id="btnSearchReplace" icon="ficon.table-draw" tip="찾기 & 찾아바꾸기">
				</hbox>	
			</hbox>
		</page>
}
Common:FileUpload{
<page>
		<hbox>
			<label text="업로드 파일선택:">
			<input id="uploadFile"><button id="fileSelect" text="파일선택">
			<space>
			<button id="upload" text="업로드">
		</hbox>
		<progress id="progress">
		<editor id="src">
	</page>
}
Common:confManager{
<page title="공통설정 관리" icon="ficon.application-list">
			<hbox>
				<combo id="conf_type" height=24>
				<input id="conf_cd" width=125 height=24>
				<input id="conf_id" width=140 height=24>
				<input id="conf_kind" width=140 height=24><space>
				<button id="search" text="조회">
			</hbox>
			<editor id="src">
			<hbox>
				<button id="save" text="저장	">
				<input id="conf_nm" height=24><button id="copy" text="복사">
				<space>
				<button id="cancel" text="취소">
			</hbox>
		</page>
}
Common:KioskUpdate{
<page>
			<label text="프로그램 업그레이드 (새로운 설치 버전이 존재합니다)">
			<progress id="prog">
			<hbox>
				<button id="upgrade" text="업그레이드" icon="vicon.cog_add"><space>
				<button id="cancel" text="취소" icon="vicon.cancel_defalut">
			</hbox>
		</page>
}
Common:Image{
<page margin=4> 
	<hbox margin=0>
		<label text="ID : " width=45  height=24 align=right>
		<input id=idSearch width=150 height=24>
		<combo id=imageGroup height=24>
		<button id=search text=조회 height=26>
		<space>
		<button id=clipboardCapture text="클립보드 캡쳐" height=26>
		<button id=newImage text="이미지생성" height=26>
	</hbox>
	<tree id=grid>
	<hbox>
		<button id=ok text="확인" height=26><space>
		<button id=cancel text="취소" height=26>
	</hbox>
	</page>
}
Common:searchFunc{
<page>
	<splitter stretchFactor="GridPage" type="vbox">
		<vbox id="GridPage" margin=0> 
			<group> 
				<hbox>
					<vbox>
						<hbox>
							<label text="유형: "><combo id="funcKind">
							<label text="그룹:"><combo id="group" width=200>
							<label text="코드:"><combo id="code" width=180>
							<space>
						</hbox>
						<hbox>
							<label text="함수명: "><input id="funcName">
							<label text="내용:"><input id="funcData">
							<space>
							<check id="appendSrc" text="Append 소스 ">
						</hbox>
					</vbox>
					<button id="search" text="조회" height=60>
				</hbox>
			</group>
			<grid id="grid" stretch=1>
			<hbox>
				<label id="gridStatus">
			</hbox>
		</vbox>
		<editor id="src">
	</splitter>
	<hbox>
			<button id="save" text="저장"><space>
			<button id="close" text="창닫기">
	</hbox>
	</page>
}
PageEdit:KioskHiTecEditMain{
<page margin=0>
		<hbox margin=4>
			<button id="goProjectGrid" text="프로젝트 선택">
			<label text="편집모드 : "><combo id="editTypeCombo">
			<label text="페이지 스타일 : "><combo id="pageStyle">
			<check id="touchUseCheck" text="터치사용">
			<check id="selectItemCheck" text="아이템선택 보기">
			<space>
			<hbox margin="0,0,15,0" spacing=1>
				<input id="inputSearch" width=120>
				<toolbutton id="scriptSearch" icon="vicon.zoom_defalut" tip="스크립트 조회">
			</hbox>
			<toolbutton id="scriptRun" icon="vicon.script_code_red" tip="스크립트 실행창">
			<toolbutton id="confManager" icon="vicon.cog_edit" tip="공통 설정보기"> 
			<toolbutton id="debugPage" icon="vicon.bug_go" tip="디버그 창열기"> 
		</hbox>
		<splitter stretchFactor="content">
			<tab id="leftTab">
			<div id="content">
		</splitter>
	</page>
}
PageEdit:KioskHiTecPageInfo{
<page>
		<hbox>
			<button id="addPage" text="페이지 추가" icon="ficon.inbox--plus">
			<space>
			<button id="deletePage" text="페이지 삭제" icon="ficon.inbox--minus">
			<button id="openPage" text="페이지 선택" icon="ficon.inbox--arrow">
		</hbox>
		<grid id="grid">
	</page>
}
PageEdit:KioskHiTecPageForm{
<page>
		<layout>
			<row>
				<label text="페이지그룹 :" width=90 align=right><input id="page_group">
				<label text="페이지코드 :" width=90 align=right><input id="page_code"><space>
			</row>
			<row>
				<label text="페이지 타이틀 :" width=90 align=right><hbox colspan=2>
					<input id="page_title"><toolbutton id="page_icon" icon="ficon.balloon-buzz-left" tip="페이지 아이콘 설정">
				</hbox>
			</row>
			<row>
				<label text="페이지유형 :" width=90 align=right><combo id="page_kind" width=120>
				<label text="페이지템플릿 :" width=90 align=right>
				<hbox>
					<combo id="page_template" width=120>
					<toolbutton id="applyTemplate" icon="ficon.table--pencil" tip="템플릿 관리">
					<space>
				</hbox>
				<space>
			</row>
		</layout>
		<group id="pageInfoGroup">
			<hbox>
				<label text="페이지 설명:"><input id="note" stretch=1>
				<toolbutton id="pageSetup" icon="vicon.folder_table" tip="페이지 설정 열기">
			</hbox>
		</group>
		<space>
		<tab id="pageSourceTab">
		<hbox>
			<button id="pageCreate" text="페이지 생성" icon="vicon.bricks_defalut"><space>
			<button id="pageInit" text="초기화">
		</hbox>
	</page>
}
PageEdit:KioskHiTecPageSrc{
<page>
		<editor id="src">
		<hbox>
			<button id="save" text="미리보기" icon="vicon.database_save">
			<label id="editorStatus" stretch=1>
			<hbox margin="4,0,0,0" spacing=4>
				<label text="찾기 : ">
				<input id="inputSearch" width=115 height=24>
				<toolbutton id="btnSearchReplace" icon="ficon.table-draw" tip="찾기 & 찾아바꾸기">
			</hbox>	
		</hbox>
	</page>
}
PageEdit:KioskHiTecDrawClassTree{
<page margin=4>
		<splitter stretchFactor="treePage" type="vbox">
			<vbox id="treePage" margin=0>
				<hbox>
					<combo id="classInheritCombo" width=120>
					<hbox spacing=0>
						<combo id="classFuncCombo" width=220>
						<toolbutton id="classFuncGrid" icon="vicon.add_default">
					</hbox>
					<button id="createClass" text="클래스 추가">
					<space>
				</hbox>
				<tree id="tree">
				<hbox>
					<label text="공용변수: "> <combo id="classVarCombo" width=120>
					<space>
					<button id="editNode" text="노드수정">
					<button id="reloadNode" text="새로고침">
				</hbox>
			</vbox>
			<div id="sourcePages">
		</splitter>
	</page>
}
PageEdit:KioskHiTecAttr{
<page margin=0>
		<grid id="grid">
		<hbox>
			<button id="initGridData" text="초기화"><space>
		</hbox>
	</page>
}
PageEdit:KioskHiTecCreateClass{
<page>
		<hbox>
			<label text="클래스명 : "><input id="className">
			<label text="컨트롤 클래스 참조 : "><combo id="controlClass" width=220>
			<check id="newPageCheck" text="새창에서 열기">
			<space>
			<button id="commonClassCreate" text="공통 클래스 생성" icon="vicon.brick_go">
			<button id="templateEdit" text="템플릿 수정" icon="ficon.blog-blue">
		</hbox>
		<editor id="src">
		<hbox>
			<button id="save" text="저장" icon="vicon.database_save">
			<label id="editorStatus" stretch=1>
			<hbox margin="4,0,0,0" spacing=4>
				<label text="찾기 : ">
				<input id="inputSearch" width=115 height=24>
				<toolbutton id="btnSearchReplace" icon="ficon.table-draw" tip="찾기 & 찾아바꾸기">
			</hbox>	
			<button id="close" text="닫기" icon="vicon.cancel_defalut">
		</hbox>
	</page>
}
PageEdit:KioskHiTecSrcTab{
<page>
		<tab id="tab">
	</page>
}
PageEdit:KioskHiTecSrcEdit{
<page margin=0>
		<editor id="src">
		<hbox>
			<button id="save" text="저장" icon="vicon.database_save">
			<button id="run" text="실행" icon="vicon.monitor_go">
			<label id="editorStatus" stretch=1>
			<hbox margin="4,0,0,0" spacing=4>
				<label text="찾기 : ">
				<input id="inputSearch" width=115 height=24>
				<toolbutton id="btnSearchReplace" icon="ficon.table-draw" tip="찾기 & 찾아바꾸기">
			</hbox>	
		</hbox>
	</page>
}
PageEdit:KioskHiTecClassFuncsEdit{
<page>
		<hbox>
			<label text="클래스 정보 : ">
			<input id="classInfo"><space>
		</hbox>
		<editor id="src">
		<hbox>
			<button id="save" text="저장" icon="vicon.database_save">
			<button id="run" text="실행" icon="vicon.monitor_go">
			<label id="editorStatus" stretch=1>
			<check id="autoRunCheck" text="저장시 자동실행">
			<hbox margin="4,0,0,0" spacing=4>
				<label text="찾기 : ">
				<input id="inputSearch" width=115 height=24>
				<toolbutton id="btnSearchReplace" icon="ficon.table-draw" tip="찾기 & 찾아바꾸기">
			</hbox>	
		</hbox>
	</page>
}
PageEdit:KioskHiTecPageFuncsEdit{
<page>
		<hbox>
			<label text="페이지 정보 : ">
			<input id="pageInfo"><space>
		</hbox>
		<editor id="src">
		<hbox>
			<button id="save" text="저장" icon="vicon.database_save">
			<button id="run" text="실행" icon="vicon.monitor_go">
			<label id="editorStatus" stretch=1>
			<check id="autoRunCheck" text="저장시 자동실행">
			<hbox margin="4,0,0,0" spacing=4>
				<label text="찾기 : ">
				<input id="inputSearch" width=115 height=24>
				<toolbutton id="btnSearchReplace" icon="ficon.table-draw" tip="찾기 & 찾아바꾸기">
			</hbox>	
		</hbox>
	</page>
}
PageEdit:KioskHiTecPageTagTree{
<page margin=4>
		<splitter stretchFactor="treePage" type="vbox">
			<vbox id="treePage" margin=0>
				<hbox>
					<label text="페이지 클래스 : ">
					<combo id="pageClassCombo" stretch=1>
				</hbox>
				<hbox>
					<combo id="classInheritCombo" width=120>
					<combo id="classFuncCombo" width=220> 
					<space>
					<label text="공용변수: "> <combo id="classVarCombo" width=120>
				</hbox>
				<tree id="tree">
			</vbox>
			<tab id="gridTab">
		</splitter>
	</page>
}
PageEdit:KioskHiTecDebugPage{
<page>
		<editor id="src">
		<hbox>
			<button id="clearEditor" text="지우기">
			<label id="messageFilter" text="메시지 필터 : ">
			<input id="inputFilter" width=100>
			<label id="editorStatus" stretch=1>
			<hbox margin="4,0,0,0" spacing=4>
				<label text="찾기 : ">
				<input id="inputSearch" width=115 height=24>
				<toolbutton id="btnSearchReplace" icon="ficon.table-draw" tip="찾기 & 찾아바꾸기">
			</hbox>	
		</hbox>
	</page>
}
KioskHiTec:main{
<page margin=0>
	<canvas id="canvas" scroll="yes">
</page>
}
KioskHiTec:PopupTest{
<page margin=0>
	<canvas id="canvas" scroll="yes">
</page>
}
KioskHiTec:protocalTest{
<page>
		<splitter>
			<tree id="tree">
			<vbox>
				<hbox>
					<combo id="reqType" width=220>
					<button id="serverInfoConf" text="설정">
					<button id="serverInfoReload" text="새로고침">
					<label text="URL: "><input id="requestUrl" stretch=1>
				</hbox>
				<hbox>
					<label text="매장그룹"><input id="MS_NO">
					<label text="매장그룹"><input id="POS_NO">
					<label text="사원번호"><input id="EMP_ID">
					<label text="비밀번호"><input id="EMP_PW">
					<space>
					<check id="autoApply" text="자동적용">
					<button id="sendLast" text="변경된 내용받기">
					<button id="sendAll" text="전체받기">
				</hbox>
				<grid id="grid">
				<hbox>
					<label id="gridStatus">
				</hbox>
				<editor id="src">
			</vbox>
		</splitter>
	</page>
}
KioskHiTec:dbManager{
<page>
		<tree id="grid">
		<hbox>
			<button id="add" text="DB연결추가" icon="vicon.database_add">
			<button id="reload" text="새로고침" icon="vicon.arrow_refresh">
			<space>
			<button id="delete" text="선택삭제" icon="vicon.delete_defalut">
			<button id="applyPassword" text="비밀번호 변경" icon="vicon.asterisk_orange">
			<button id="apply" text="적용" icon="vicon.database_save">
		</hbox>
	</page>
}
KioskHiTec:DbQuery{
<page>
	<splitter stretchFactor="content">
		<tree id="tree">
		<vbox id="content" margin="4,0,4,0">
			<splitter type="vbox">
				<vbox margin="0,0,0,0">
					<grid id="grid">
					<hbox>
						<button id="applyData" text="적용"><label id="gridStatus" stretch=1><label text="DB연결 : "><combo id="dbCombo">
					</hbox>
				</vbox>
				<vbox margin="0,0,0,0">
					<editor id="sqlEditor">
					<hbox>
						<button id="runQuery" text="쿼리실행">
						<space>	
						<input id="inputTitle">
						<check id="checkTreeNode" text="트리노드 생성">
					</hbox>
				</vbox>
			</splitter>
		</vbox>
	</splitter>
</page>
}
KioskHiTec:KioskLogViewer{
<page>
		<hbox>
			<button id="startLog" text="로그시작"><space>
			<button id="clearLog" text="로그지우기">
			<button id="startUpdate" text="업데이트시작">
			<button id="cancel" text="창닫기">
		</hbox>
		<editor id="log">
	</page>
}
KioskHiTec:PingTest{
<page>
	<splitter type="vbox" stretchFactor="logForm">
		<vbox id="logForm" margin="0,0,0,10">
			<hbox>
				<combo id="serverInfo"><input id="serverIp" width=220>
				<button id="serverSetup" text="서버 설정"><button id="reload" text="새로고침"><check id="showLog" text="실행로그 보기">
				<space>
				<button id="run" text="실행">
			</hbox>
			<editor id="log">
		</vbox>
		<group title="네트워크 모니터링 정보">
			<vbox id="logForm" margin="8,10,8,10">
				<hbox>
					<button id="start" text="모니터링 시작">
					<space>
				</hbox>
				<grid id="grid">
			</vbox>
		</group>
	</splitter> 
</page>
}
KioskHiTec:processInfoView{
<page>
	<grid id="grid">
	<hbox>
		<label text="감시주기: ">
		<spin id="sycleSpin">
		<input id="processName" width=180>
		<button id="processMonitor" text="프로세스 감시" icon="ficon.inbox--plus">
		<space>
		<button id="killProcess" text="선택 프로세스 종료" icon="vicon.delete_default">
		<button id="reload" text="다시조회" icon="ficon.application-search-result">
	</hbox>
</page>
}
KioskHiTec:LoginPage{
<page margin=0>
		<canvas id="c" height=127>
		<layout spacing=12 margin=10>
			<row>
				<label text="매장그룹 번호 : " style="font-size: 18pt;"><input id="ms_no" height=38 style="font-size: 18pt;font-weight: bold;">
				<label text="  POS 번호 : " style="font-size: 18pt;"><input id="pos_no" height=38 style="font-size: 18pt;font-weight: bold;">
			</row>
			<row>
				<label text="사원 번호 : " style="font-size: 18pt;"><input id="emp_id" height=38 style="font-size: 18pt;font-weight: bold;">
				<label text="  비밀 번호 : " style="font-size: 18pt;"><input id="emp_pwd" height=38 style="font-size: 18pt;font-weight: bold;">
			</row>
			<row>
				<label text="인터페이스 구분: " style="font-size: 18pt;"><combo id="int_gb" style="font-size: 18pt;font-weight: bold;" height=38><label colspan=2>
			</row>
		</layout>
		<label stretch=1>
		<hbox>
			<button id="apply" text="적용" style="font-size: 18pt;" width=150 height=38><space>
		</hbox>
	</page>
}
KioskHiTec:AdminMenu{
<page margin=0>
	<canvas id="canvas" scroll="yes">
</page>
}
KioskHiTec:OrderTool{
<page>
		<vbox stretch=3>
			<hbox>
				<label text="코너: "><combo id="cornerCombo">
				<label text="품절여부: "><combo id="soldOutCombo"><space>
				<button id="search" text="조회">
			</hbox>
			<grid id="g1">
			<hbox>
				<button id="cartSelect" text="주문담기">
				<button id="cartDelete" text="주문취소">
				<label id="gridStatus" stretch=1>
			</hbox>
		</vbox>
		<group title="주문정보" stretch=2>
			<vbox>
				<grid id="g2">
				<hbox>
					<button id="printBill" text="영수증 출력">
					<button id="printKitchen" text="주방프린터 출력">
					<button id="cardConfirm" text="카드결제">
					<button id="cardCancle" text="카드취소">
					<button id="xmlSend" text="전문 보내기">
					<space>
				</hbox>
			</vbox>
		</group> 
	</page>
}
KioskHiTec:__download{
<page>
	<hbox>
		<button id="ok" text="다운로드 시작"><space><check id="checkAll" text="다시 내려받기">
	</hbox>
	<editor id="e">
</page>
}
KioskHiTec:KioskLog{
<page>
		<editor id="log">
		<hbox>
			<button id="clearLog" text="로그지우기"><space>
			<button id="startUpdate" text="업데이트시작">
			<button id="startLog" text="로그시작">
			<button id="close" text="닫기">
		</hbox>
	</page>
}
KioskHiTec:adminSetup{
<page>
	<group title="키오스크 정보">
		<layout>
			<row>
				<label text="매장그룹 코드:"><input id="ms_no">
				<label text="포스번호 :"><input id="pos_no">
			</row>
			<row>
				<label text="사원 아이디:"><input id="emp_id">
				<label text="비밀번호 :"><input id="emp_pw">
			</row> 
		</layout> 
	</group>
	<hbox>
		<button id="reload" text="새로고침"><space>
		<button id="updateAll" text="전체 다시내려받기">
		<button id="dataInit" text="데이터 초기화 하기">
	</hbox>	
	<grid id="grid">
	<group title="다운로드 처리">
		<vbox>
			<hbox>
				<button id="goodsDown" text="상품 이미지 다시받기">
				<button id="cornerDown" text="코너 이미지 다시받기">
				<button id="adDown" text="광고 이미지 다시받기">
			</hbox>
		</vbox>
	</group>
	<editor id="log">
	<hbox>
		<button id="adminOpen" text="관리자 화면 열기"><space> 
		<button id="cancel" text="창닫기">
	</hbox>
	</page>
}
