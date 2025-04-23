<script>
	cmd(name) {		
		not(name) name='cmd'
		obj = _node("process.$name")
		if(obj.var(useModule)) {
			return obj;
		}
		include('classes/common/process.js')
		return addModule(obj, '@cmd', name)
	}
	was(mode) {
		obj = _node("was.object")
		if(obj.var(useModule)) {
			if(typeof(mode,'bool')) {
				obj.member(devMode, mode)
			}
			return obj;
		}
		include('classes/common/was.js')
		return addModule(obj, '@was', mode)
	}
	wss(name,port) {
		obj = _node("wss.$name")
		if(obj.var(useModule)) {
			return obj;
		}
		include('classes/common/wss.js')
		return addModule(obj, '@wss', port)
	}
	web(name, callback) {
		if(typeof(name,'func') ) {
			callback=name
			name=null
		}
		not(name) name='common'
		obj = _node("web.$name")
		if(obj.var(useModule)) {
			if(typeof(callback,'func')) {
				obj.member(finishFunc, callback)
			}
			return obj;
		}
		include('classes/common/process.js')
		not(callback) callback=null
		return addModule(obj, '@web', name, callback)
	}
	job(name, jobType, callback) {
		if(typeof(name,'func') ) {
			callback=name
			name=null
		}
		not(name) name='common'
		obj = _node("job.$name")		
		if(obj.var(useModule)) {
			return obj;
		}
		include('classes/common/process.js')
		return addModule(obj, '@job', name, jobType, callback)
	}
	json(name) {
		not(name) name='common'
		obj = _node("json.$name")
		if(obj.var(useModule)) {
			return obj;
		}
		include('classes/common/json.js')
		return addModule(obj, '@json')
	}
	webDownload(name, savePath, count) {
		if(typeof(name,'func') ) {
			callback=name
			name=null
		}
		not(name) name='common'
		obj = _node("webDownload.$name")
		if(obj.var(useModule)) {
			if(savePath) obj.member(savePath, savePath)
			return obj;
		}
		include('classes/common/process.js')
		return addModule(obj, '@webDownload', name, savePath, count)
	}
</script>

<script>
	color(param) {
		if(param.ch('#')) {
			return Baro.color(param.trim()); 
		} 
		switch(args().size()) {
		case 1:
			args(&s);
			type=s.findPos("(").trim();
			if(s.valid()) {
				r=0,g=0,b=0,a=255;
				ss=s.findPos(")");
				while(ss.valid(), idx) {
					left=ss.findPos(",");
					if(left.find('%')) {
						v=left.findPos("%").trim();
					} else {
						v=left.trim();
					}
					not(typeof(v,'num')) v=0;
					switch(idx) {
					case 0: r=v.toInt();
					case 1: g=v.toInt();
					case 2: b=v.toInt();
					case 3: a=v*256.0;
					}
				}
				c=Baro.color(type,r,g,b,a);
			} else {
				c=Baro.color('hsv',type, 100, 50)
			}
			return c;
		case 3:
			args(r,g,b)
			return Baro.color(r,g,b);
		case 4:
			args(r,g,b,a)
			return Baro.color(r,g,b,a);
		default:
		}
	}
	size(param) {
		if(typeof(param,'array')) return param.size();
		result=null;
		if(typeof(param,'rect')) {
			result=param.size();
		} else if(typeof(param,'widget')) {
			result=param.rect().size();
		} else if(typeof(param,'node')) {
			if(param.var(rect)) {
				result=param.var(rect).size();
			} else {
				return param.childCount();
			}
		} else {
			if(typeof(param,'string')) {
				return param.size();
			} else if(typeof(param,'num')) {
				return param;
			}
		}
		if(args().size()==2) {
			result.inject(width);
			return width;
		}
		return result;
	}
	pt(param) {	
		if( typeof(param,'point') ) {
			args(pt, pos);
			pt.inject(x,y);
			if( typeof(pos,'point') ) {
				args(2, chk);
				param.inject(x,y);
				pos.inject(dx, dy);
			} else {
				args(1, dx, dy, chk);
			}		
			if(chk) {
				x-=dx, y-=dy;
			} else {
				x+=dx, y+=dx;
			}
			return Baro.point(x, y);
		}
		sz=args().size();
		not(sz) return Baro.pt(0,0);	
		ty=typeof(param);
		if( sz.eq(1) ) {
			if( ty.eq('rect','point','rectf','pointf') ) {			
				param.inject(x,y);
			} else if( ty.eq('node(draw)') ) {
				param.rect().inject(x,y);
			}
		} else if( ty.eq('rect','rectf','node(draw)') ) {
			x=0, y=0;
			if(ty.eq('node(draw)') ) {
				args(dc,ty);
				rc=dc.rect();
			} else {
				args(rc, ty);
			}
			switch(ty) {
			case lt: rc.inject(x,y)
			case lb: rc.lb().inject(x,y)
			case rt: rc.rt().inject(x,y)
			case rb: rc.rb().inject(x,y)
			case tc: rc.tc().inject(x,y)
			case bc: rc.bc().inject(x,y)
			case center: rc.center().inject(x,y);
			default: rc.inject(x,y);
			}
			if(sz>2) {
				args(2,dx, dy);
				if( typeof(dx,'num') ) x+=dx;
				if( typeof(dy,'num') ) y+=dy;
			}
		} else if(typeof(param,'num') ) {
			args(x,y);
			not(typeof(y,'num') ) y=0;	
		}
		return Baro.pt(x,y);
	}   
	rc(param) {
		x=0,y=0,w=1,h=1;
		switch(args().size()) {
		case 0:
			rc=this.rect()
			if(typeof(rc,'rect')) {
				rc.inject(x,y,w,h);
			}
		case 1:
			if( typeof(param,'node') ) {
				rc=param.rect()
				if(typeof(rc,'rect')) {
					rc.inject(x,y,w,h);
				}
			} else if( typeof(param,'image') ) {
				param.imageSize().inject(w,h);
			}
		case 2:
			args(w,h);
		case 3:
			if(typeof(param,'num') ) {
				args(x,y,size);
				size.inject(w,h);
			} else if(typeof(param,'point')) {
				args(point, w,h);
				point.inject(x,y);
			}
		case 4: 
			if(typeof(param,'point')) {
				args(pos, w, h, ty)
				pos.inject(x,y)
				if(ty.eq('center')) {
					hw=w/2, hh=h/2;
					x-=hw, y-=hh;
				} 
			} else {
				args(x,y,w,h)
			}
		default:
		}
		return Baro.rc(x,y,w,h);
	} 
</script>