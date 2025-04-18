initPage() {
	@editor=widget('editor')
	@editorTitle=widget('editorTitle')
	@editorInfo=widget('editorInfo')
	@searchInput=widget('searchInput')
	
	addModule(editor,'editor:editorSource')
	setEvent(searchInput,'onEnter',this.searchText, this)
	this.setEditorSyntax() 
	this.setKeyMap()
	editorTitle.value("소스테스트 실행창")
}