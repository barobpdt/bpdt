<script module="@page">
margin() {
	box=this.child(0)
	tag=box.tag
	not(tag.eq('vbox','hbox','form')) return print("page margin 오류 레이아웃 미정의");
	switch(args().size()) {
	case 2:
		args(1,a)
		box.margin(a)
	case 3:
		args(1,a,b)
		box.margin(a,b,a,b)
	case 5:
		args(1,a,b,c,d)
		box.margin(a,b,c,d)
	}
}
spacing(page, num) {
	not(num) num=0
	box=this.child(0)
	tag=box.tag
	not(tag.eq('vbox','hbox','form')) return print("page margin 오류 레이아웃 미정의");
	box.spacing(num)
}	
positionSave() {
	page=this
	code=page.var(baseCode) not(code) return;
	page.geo().inject(x,y,w,h);
	y-=31;
	conf("pagePosition.${code}", "$x,$y,$w,$h", true)
}
positionLoad(w,h) {
	page=this
	code=page.var(baseCode) not(code) return;
	if(page.parentWidget()) {
		return;
	}
	rcOpen=null
	s=conf("pagePosition.${code}")
	if(s) {
		split(s).inject(x,y,w,h)
		rc=rc(x,y,w,h)
		// 이전 저장 위치가 시스템 스크린 영역에 포함되는지 체크
		while(n=0,System.info("screenCount")) {
			rcScreen=System.info("screenRect",n);
			if( rcScreen.contains(rc) ) {
				rcOpen = rc
				break;
			}
		}
	} 
	not(rcOpen ) {
		not(w) w=800
		not(h) h=600
		rcOpen = System.info("screenRect").center(w,h)
	}
	page.move(rcOpen)
	page.open();
	page.active();
}
</script>
