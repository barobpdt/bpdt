mdc(code, param) {
	map=object("user.mdcMap")
	asize=args().size() not(asize) return map;
	d=map.get(code)
	if( asize.eq(1) ) {
		return d;
	}
	img=null
	if(typeof(param,'string')) {
		img=Cf.imageLoad(param)
	} else if(typeof(param,'image')) {
		img=param
	} else if(typeof(param,'node')) {
		img=param.var(image)
	} 
	if(img) {
		if(d) return d;
		d=Baro.drawObject(img)
		d.name=code
		map.set(code, d)
		return d;
	}

	if(typeof(param,'point')) {
		param.inject(width, height);
	} else if(typeof(param,'rect')) { 
		param.size().inject(width, height);
	} else {
		args(1,width, height);
	}
	not(typeof(width,'num') || typeof(height,'num')) return print("메모리 DC생성 영역오류(폭:$width 높이:$height)", param);
	
	if(d) {
		rc=d.rect();
		rc.size().inject(w, h);
		if(width.eq(w) && height.eq(h) ) {
			d.flag(FLAG.new);
			return d;
		}
		d.destroy();
		d.painter(width, height);
		print("mdc set ($width, $height)");
	} else {
		d=Baro.drawObject(width, height);
		d.name=code;
		map.set(code, d);
	}  
	d.var(first,true);
	return d;
}
textSize(text, fontSize) {
	dc=mdc('textSize',2048,500)
	dc.font(fontSize)
	return dc.textSize(str)
}
hbox(width, height, info, sx, sy) {
	not(sx) sx=0 not(sy) sy=0
	a=recalc(width, info)
	arr=_arr()
	while(w,a) {
		arr.add(rc(sx,sy,w,height))
		sx+=w;
	}
	return arr;
}
vbox(width, height, info, sx, sy) {
	not(sx) sx=0 not(sy) sy=0
	a=recalc(height, info)
	arr=_arr()
	while(h,a) {
		arr.add(rc(sx,sy,width,h))
		sy+=h;
	}
	return arr;
}