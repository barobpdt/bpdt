##> func { name=common_util }
	runSrc(code) {
		return runSource(cv(code))
	}
	vbox(rc, sizes, local) {
		if(local) {
			a=[]
		} else {
			a=_arr()
		}
		rc.inject(sx,sy,w,h)
		while(hh, recalc(h,sizes)) {
			a.add(rc(sx,sy,w,hh))
			sy+=hh;
		}
		return a;
	}
	hbox(rc, sizes, local) {
		if(local) {
			a=[]
		} else {
			a=_arr()
		}
		rc.inject(sx,sy,w,h)
		while(ww, recalc(w,sizes)) {
			a.add(rc(sx,sy,ww,h))
			sx+=ww;
		}
		return a;
	}
	availRect( rect, inputHeight ) {
		pt=rect.lt().incr(2,2);
		rect.inject(x,y,w,h);
		cnt=System.info('screenCount');
		while( n, cnt ) {
			rcAvail=System.info('availRect', n);
			not( rcAvail.contains(pt) ) continue;
			rb=rect.bottom(), ab=rcAvail.bottom();
			rr=rect.right(), ar=rcAvail.right();
			print(rect, pt, rcAvail, rb, rr, ab,ar );
			/* bottom이 벗어난 경우 */
			if( rb>ab ) {
				if( inputHeight ) {
					y-=inputHeight;
					y-=h;
					if( y.lt(0) ) y=0;
					rect.y(y);
				} else {
					rect.bottom(ab);
					if( rect.height() < h ) {
						dist= rect.height() - h;
						rect.incrY(dist);
					}
				}
			}
			/* right가 벗어난 경우 */
			if( rr>ar ) {
				if( inputHeight ) {
					rect.right(x);
				} else {
					rect.right(ar);
					if( rect.width() < w ) {
						dist= rect.width() - w;
						rect.incrX(dist);
					}
				}
			}
			rcAvail.inject(ax, ay);
			if( rect.x()<ax ) {
				rect.x(ax);
			}
			if( rect.y()<ay ) {
				rect.y(ay);
			}
			break;
		}
		return rect;
	}
	
	roundBox(dc, rcBox, rad, penColor) {
		dc.save()
		dc.mode()
		not(penColor) penColor=null
		rad2=rad*2;
		lt=rcBox.leftTop(rad2,rad2), rt=rcBox.rightTop(rad2,rad2);
		lb=rcBox.leftBottom(rad2,rad2), rb=rcBox.rightBottom(rad2,rad2);
		path=[]
		path.add('arc',rt, 0, 90, lt.tc(), 'arc', lt, 90,90, lb.lc(), 'arc',lb,180,90, rb.bc(), 'arc',rb,270,90)
		if(type.eq('gradient','radial','conical')) {
			arr=args(5)
			if(type=='gradient') {
				c1=arr.get(0), c2=arr.get(1)
				rc.lt().inject(sx,sy)
				rc.lb().inject(ex,ey)
				dc.drawPath(path, penColor, true,'gradient',sx,sy,ex,ey,0,c1,1,c2)
			} else if(type=='radial') {
				arr.inject(cx, cy, px, py, c1, c2)
				dc.drawPath(path, penColor, true,'radial',cx, cy, px, py,0,c1,1,c2)
			} else {
				arr.inject(cx, cy, c1, c2)
				dc.drawPath(path, penColor, true,'conical',cx, cy, 0,c1,1,c2)
			}
		} else {
			bgColor=type
			if(bgColor) {
				dc.drawPath(path, penColor, true,bgColor)	
			} else {
				dc.drawPath(path, penColor, true)
			}			
		}
		dc.restore()
	}
	roundRect(dc, rc, rad, penColor, type) {
		dc.save()
		dc.mode()
		if(penColor) {
			dc.pen(penColor)
		} else {
			dc.pen(null)
		}		
		path=[]
		path.add('round',rc, rad, rad)
		// ex) roundRect(dc, rcBox, 20, c.darkColor(100),'gradient', c, c.darkColor(100) )
		if(type.eq('gradient','radial','conical')) {
			arr=args(5)
			if(type=='gradient') {
				c1=arr.get(0), c2=arr.get(1)
				rc.lt().inject(sx,sy)
				rc.lb().inject(ex,ey)
				dc.drawPath(path, penColor, true,'gradient',sx,sy,ex,ey,0,c1,1,c2)
			} else if(type=='radial') {
				arr.inject(cx, cy, px, py, c1, c2)
				dc.drawPath(path, penColor, true,'radial',cx, cy, px, py,0,c1,1,c2)
			} else {
				arr.inject(cx, cy, c1, c2)
				dc.drawPath(path, penColor, true,'conical',cx, cy, 0,c1,1,c2)
			}
		} else {
			bgColor=type
			if(bgColor) {
				dc.drawPath(path, penColor, true,bgColor)	
			} else {
				dc.drawPath(path, penColor, true)
			}
			
		}
		dc.restore()
	}
	
##> func {name=service, note=기본제공 서비스}
	startPythonCommand() {
		py=pathJoin(conf('python.path'), 'python.exe')
		// addCmdWorker('runCommand', "$py -m pip list", @python.runProc)
		line=@python.execLine('runCommand.py')
		cmd=Baro.process('runCommand')
		if(cmd.cmp('@workerStatus', 'run') ) {
			print("runCommand가 실행중입니다");
		} else {
			addCmdWorker('runCommand', line, @python.runProc)
		}		
		
		return true;
	}
	startPythonWebview() {
		line=@python.execLine('webCommand.py')
		cmd=Baro.process('webCommand')
		if(cmd.cmp('@workerStatus', 'run') ) {
			print("webCommand가 실행중입니다");
		} else {
			addCmdWorker('webCommand', a, @python.webProc)
		}
	}