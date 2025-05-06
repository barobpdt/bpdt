<script>
	@webpage.init() {
		was=was()
		was.set('@api/api', true)
		was.set('@api/icons', true)
		
		@web.addUrl('/common/icon_list','iconList')
		@web.makeMetaMap('emoji-png')
		@web.makeMetaMap('emoji-svg')
		return;
	}
	@webpage.pythonCall(req, param) {
		fn = getAsyncFunc(@webpage.sendData, param)
		@python.command('zipinfo', fn)
	}
	@webpage.iconList(req, param) {
		path = conf('web.rootPath')
		src = fileRead("$path/common/icon_list.html")
		req.send( @web.parseTemplate(src) )
		
	}
	@webpage.sendData(data) {
		req.send(data)
	}
</script>