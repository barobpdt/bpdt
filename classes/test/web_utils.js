<script>
	@web.mskeCssSvg(path) {
		not(path) path='C:\WORK\baro\data\solid'
		Baro.file().list(p, func(info) {
			while(info.next()) {
				info.inject(type, name, ext, fullPath)
				nm = left(name,'.')
				src = @web.parseSvg(fileRead(fullPath) )
				if(src) {
					print(">> name=>css.$nm")
					conf("css.$nm", src, true)
				}
			}
		})
	}
	@web.parseSvg(&s) {
		ss=s.match('<svg','</svg>')
		if(typeof(ss,'bool')) return;
		rst = ss.findPos('>').trim()
		while(ss.valid()) {
			ss.findPos('<path')
			not(ss.ch()) break;
			ss.findPos('d=')
			rst.add("\r\n",ss.match().trim() )
		}
		return rst;
	}
	
	@web.parseTemplate(&s) {
		rst=''
		while(s.valid() ) {
			left=s.findPos('{{',0, 1)
			not(s.ch()) {
				rst.add(left)
				break;
			}
			src=s.match('{{','}}')
			if(typeof(src,'bool')) break;
			if(left.find("\n")) {
				ss=left.findLast("\n")
				rst.add(ss)
				line = left.right()
			} else {
				line = left	
			}
			type=line.move()
			print('line==========>', type, line)
		}
	}
</script>