test1(req, param, uri) {
	a=uri.findPos('/')
	param.set('@workType','webscrap')
	param.set('@workUrl','https://wikidocs.net/book/14452')
	param.set('@timerFunc','@wc.wikidocs')
	
	param.set('@apiResult','')
	globalTimeout(param)
	tick = System.tick()
	System.sleep(200)
	while(n=0, 10) {
		if( param.get('@apiResult') ) break;
		System.sleep(250)
	}
	dist = System.tick() - tick;
	print("수행시간 ($dist)ms")
	result = param.get('@apiResult')
	not(result) result='호출결과가 없습니다'
	req.send(result)
}