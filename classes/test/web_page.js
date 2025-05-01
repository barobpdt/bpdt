<script>
	@web.addUrl(url, fc, skip) {
		if(typeof(fc,'string')) {
			if(fc.find('.')) {
				fc=call(fc)
			} else {
				fnm = Cf.val('webpage.',fc)
				fc=call(fnm)
			}
		}
		not(typeof(fc,'function')) {
			if(skip) return;
			return print("URL 등록오류 : $url 실행 함수 미정의");
		}
		map = Baro.was().uriMap()
		prev = map.get(url)
		if(typeof(prev,'function')) {
			if(skip) return;
			return print("이미등록된 URL 입니다 (경로:$url)");
		}
		map.set(url, fc)
	} 
	@webpage.iconList() {
		@web.addUrl('/common/icon_list','iconList', true)
		body = 
		
	}
</script>