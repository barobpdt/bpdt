<script>
	divideArray(tot, &s) {
		not(typeof(tot,'num')) return print("divideArray 오류 전체값이 숫자가 아닙니다");
		if(typeof(s,'num')) {
			return Cf.array().recalc(tot,s);
		}
		not(s.find('px')) {
			return Cf.array().recalc(tot,s);
		}
		pa=_arr(), pb=_arr();
		ps=0, an=0, bn=0;
		sa='',sb=''
		ss=s;
		while(s.valid() ) {
			v=s.findPos(',')
			if(v.find('px')) {
				p=v.findPos('px').trim()
				if(sa) sa.add(',')
				sa.add(p)
				ps+=p;
			} else {
				if(sb) sb.add(',')
				sb.add(v.trim())
				bn++;
			}
		}
		d=tot-ps;
		if(d<0) {
			d=bn*2;
			if(sa.find(',')) {
				pa.recalc(tot-d,sa)	
			} else {
				pa.add(tot-d)
			}
		} else {
			if(sa.find(',')) {
				pa.recalc(ps,sa)
			} else {
				pa.add(ps)
			}
		}
		if(sb.find(',')) {
			pb.recalc(d,sb)
		} else {
			pb.add(d)
		}
		s=ss
		an=0, bn=0;
		a=_arr()
		while(s.valid() ) {
			v=s.findPos(',')
			if(v.find('px')) {
				a.add(pa.get(an++));
			} else {
				a.add(pb.get(bn++));
			}
		}
		return a;
	}
	hbox(rc, info, a) {
		not(a) a=_arr() 
		not(typeof(rc,'rect') ) {
			print("hbox 오류 영역변수 미정의 $rc")
			return a;
		}
		rc.inject(x,y,w,h) 
		while(ww, _arr().recalc(w,info)) {
			a.add(Baro.rc(x,y,ww,h))
			x+=ww;
		}
		return a;
	}
	vbox(rc, info, a) {
		print("vbox => ", rc, info, a)
		not(a) a=_arr()
		not(typeof(rc,'rect') ) {
			print("vbox 오류 영역변수 미정의 $rc")
			return a;
		}
		rc.inject(x,y,w,h)
		while(hh, _arr().recalc(h,info) ) { 
			a.add(Baro.rc(x,y,w,hh)) 
			y+=hh;
		}
		return a;
	}
	containsRect(r1, r2) {
		if(r1.eq(r2)) return true;
		r1.inject(x,y,w,h)
		r2.inject(x1,y1,w1,h1)
		r=x+w, b=y+h;
		r1=x1+w1, b1=y1+h1;
		c0=x.lt(x1) && y.lt(y1)
		c1=r.gt(r1) && b.gt(b1)
		return c0 && c1;
	} 

	toRect(rc) {
		rc.inject(x,y,w,h);
		return Baro.rect(x,y,w,h);
	}
	toRc(rc) {
		rc.inject(x,y,w,h);
		return Baro.rc(x,y,w,h);
	} 
	point(param) {
		if(typeof(param,'rect')) {
			return param.lt();
		} else if(typeof(param,'point')) {
			return param;
		} else if(typeof(param,'num')) {
			args(width, height);
			return Baro.point(width, height);
		}
		return Baro.point(0,0);
	} 
	mergeRect(param) {
		cx=0,cy=0,cr=0,cb=0;
		if(typeof(param,'array')) arr=param
		else arr=args()
		while(rc, arr, idx) {
			not(typeof(rc,'rect')) continue;
			rc.inject(x,y,w,h);
			r=x+w, b=y+h;
			if(idx==0) {
				cx=x,cy=y,cr=r,cb=b;
			} else {
				if(x<cx) cx=x;
				if(y<cy) cy=y;
				if(cr<r) cr=r;
				if(cb<b) cb=b;
			}
		}
		w=cr-cx;
		h=cb-cy;
		return rc(cx, cy, w, h)
	}
	@range.margin(rc) {
		switch(args().size()) {
		case 2:
			args(1,margin)
			return rc.incr(margin);
		case 3:
			args(1, xw, yh) 
			rc.inject(x,y,w,h)
			x+=xw, w-=xw;
			y+=yh, h-=yh;
			return rc(x,y,w,h);
		case 4:
			args(1, cx, cw, yh) 
			rc.inject(x,y,w,h)
			x+=cx;
			w-=cw;
			y+=yh;
			h-=yh;
			return rc(x,y,w,h);
		case 5:
			args(1, cx, cy, cw, ch) 
			rc.inject(x,y,w,h)
			x+=cx;
			y+=cy;
			w-=cw;
			h-=ch;
			return rc(x,y,w,h);
		default:
		}
		return rc;
	}
	@range.rectArray(rc,cx,cy,iw,ih,num) {
		arr=[]
		rc.inject(x,y,w,h)
		th=ih*num
		gap=h - th;
		gap/=num-1;
		if(gap>0 ) {
			while(n=0, num) {
				rcImg=rc(cx, cy, iw, ih)
				cy+=ih;
				cy+=gap;
				arr.add(rcImg)
			}
		}
		return arr;
	}
	@range.textSize(str, style) {
		dc=mdc('text',1000,100)
		dc.font(style)
		return dc.textSize(str)
	}
	@range.vbox(rc, info, padding, gap, mode) {
		arr=[]
		a=_arr()
		total=rc.height()
		a.div(info,total)
		gg=0;
		if(gap) {
			not(mode) mode='around'
			last=a.size()-1;
			if(mode.eq('around')) {
				gg=gap/2;
				tg=gap*last
			} else {
				xx=last-1
				tg=gap*xx
			}
			total-=tg;
			if(total.lt(last)) {
				total=last
			}
			a.reuse()
			a.div(info,total)
		}
		pa=null
		if(padding) {
			if(typeof(padding,'array')) {
				pa=padding
			} else if(typeof(padding,'string')) {
				if(padding.find(',')) pa=padding.split(',')
			}
		}
		rc.inject(x,y,w,h)
		cx=x, cw=w;
		cy=y;
		if(gg) cy+=gg;
		while(n=1,a.size()) { 			
			d=a.dist(n-1,1)
			rc=rc(cx,cy,cw,d)			
			if(pa) {
				y=cy+d;
				switch(pa.size()){
				case 2:
					pa.inject(a,b)
					cx+=a;
					cw-=2*a;
					cy+=b;
					d-=2*b;
				case 4:
					pa.inject(a,b,c,d)
					cx+=a;
					cy+=b;
					cw-=c;
					d-=d;
				default:
				}
				rc=rc(cx,cy,cw,d)
				cy=y;
			} else {
				if(padding) {
					rc.incr(padding)
				}
				cy+=d;
			}
			if(gap) cy+=gap;
			arr.add(rc)
		}
		return arr;
	}
	@range.hbox(rc, info, padding, gap, mode) {
		arr=[]
		a=_arr()
		total=rc.width()
		a.div(info,total)
		gg=0;
		if(gap) {
			not(mode) mode='around'
			last=a.size()-1;
			if(mode.eq('around')) {
				gg=gap/2;
				tg=gap*last
			} else {
				xx=last-1
				tg=gap*xx
			}
			total-=tg;
			if(total.lt(last)) {
				total=last
			}
			a.reuse()
			a.div(info,total)
		}
		pa=null
		if(padding) {
			if(typeof(padding,'array')) {
				pa=padding
			} else if(typeof(padding,'string')) {
				if(padding.find(',')) pa=padding.split(',')
			}
		}
		rc.inject(x,y,w,h)
		cx=x, cy=y, ch=h;
		if(gg) cx+=gg; 
		while(n=1,a.size()) {
			d=a.dist(n-1,1)
			rc=rc(cx,cy,d,ch)
			if(pa) {
				x=cx+d;
				switch(pa.size()){
				case 2:
					pa.inject(a,b)
					cx+=a;
					d-=2*a;
					cy+=b;
					ch-=2*b;
				case 4:
					pa.inject(a,b,c,d)
					cx+=a;
					cy+=b;
					d-=c;
					ch-=d;
				default:
				}
				rc=rc(cx,cy,d,ch)
				cx=x
			} else {
				if(padding) {
					rc.incr(padding)
				}
				cx+=d
			} 
			if(gap) cx+=gap
			arr.add(rc)
		}
		return arr;
	}
</script>

<script module="rect">
	getRect() {
		this.inject(rect, rcDraw)
		return nvl(rcDraw, rect)
	}
</script>
