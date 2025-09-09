<widgets>
	<page id="p1">
		<canvas id="c">
		<hbox>
			<input id="filenm">
			<button id="fileOpen" text="파일불러오기">
			<label text="row"><input id="row">
			<label text="column"><input id="column">
			<button id="crop" text="잘르기">
			<button id="line" text="라인넣기">
			<space>
		</hbox>
	</page>
</widgets>
~~
c=@python.cmdExec(#[##> exec:
driver.get('https://www.freepik.com/search?format=search&img=1&last_filter=img&last_value=1&query=2d+sprites')
log(f'openEditor: {driver.page_source}')
])
~~
<func>
	@parse.openEditor(&s) {
		node=this
		node.fullPath='c:/TEMP/sprites.html'
		fileWrite(node.fullPath, s)
		@job.addPost('openEditor',node)
}
	@job.openEditor#post(node) {
		if(node.fullPath ) {
			cmd= _s('notepad "${node.fullPath}"')
			print("openEditor POST: $node CMD:$cmd")
			@job.cmdRun(cmd)
		}
	}

</func>
~~
p=page('test:p1')
p.open()
line = p.get('line')
_event(p.get('line'),'onClick', @spriteTool.clickLine, p)
c.filenm='c:/bpdt/data/sprites/ani01.jpg'

c.mx=200
c.my=300
c=p.get('c')
~~
<func>
	@spriteTool.clickLine() {
		print("xxx", this)
		btn=Cf.funcNode().get('@sender')
		canvas.row=this.get('row').value()
		canvas.column=this.get('column').value()
		if( canvas.cmp('@mode','line') ) {
			canvas.set('@mode','')
			btn.value('라인넣기')
		} else {
			canvas.set('@mode','line')
			btn.value('라인빼기')
		}
	}
</func>
~~
c[
	onDraw(dc, rc) {
		fullpath = this.filenm
		this.inject(mx, my)
		dc.fill('#fff')
		not(fullpath) {
			return;
		}
		rc.inject(x,y,w,h)
		// dc.image(rc, fullpath, mx, my, w, h)
		dc.image(rc, fullpath)
		if(this.cmp('@mode','line')) {
			this.inject(row,column)
		}
	}
]