#include "func_util.h"
#include "../util/widget_util.h"

inline bool rangeEqCheck(StrVar* var, XListArr* arrs) {
    StrVar* sv = arrs->get(0);
    U16 stat = var->getU16(2);

    if( stat==5 ) {
        U16 ss=sv->getU16(2);
        int sz=sizeof(double);
        if(ss==5) {
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            double x1=sv->getDouble(4), y1=sv->getDouble(4+sz), w1=sv->getDouble(4+(2*sz)), h1=sv->getDouble(4+(3*sz));
            if(x==x1 && y==y1 && w==w1 && h==h1 ) {
                return true;
            } else {
                return false;
            }
        } else if(ss==4) {
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            double x1=(double)sv->getInt(4),y1=(double)sv->getInt(8),w1=(double)sv->getInt(12),h1=(double)sv->getInt(16);
            if(x==x1 && y==y1 && w==w1 && h==h1 ) {
                return true;
            } else {
                return false;
            }
        }
    } else if( stat==4 ) {
        U16 ss=sv->getU16(2);
        if(ss==5) {
            int sz=sizeof(double);
            int x=var->getInt(4), y=var->getInt(8), w=var->getInt(12), h=var->getInt(16);
            int x1=(int)sv->getDouble(4), y1=(int)sv->getDouble(4+sz), w1=(int)sv->getDouble(4+(2*sz)), h1=(int)sv->getDouble(4+(3*sz));
            if(x==x1 && y==y1 && w==w1 && h==h1 ) {
                return true;
            } else {
                return false;
            }
        } else if(ss==4) {
            int x=var->getInt(4), y=var->getInt(8), w=var->getInt(12), h=var->getInt(16);
            int x1=sv->getInt(4), y1=sv->getInt(8), w1=sv->getInt(12), h1=sv->getInt(16);
            if(x==x1 && y==y1 && w==w1 && h==h1 ) {
                return true;
            } else {
                return false;
            }
        }
    } else if( stat==20 ) {
        int sz=sizeof(double);
        double x=var->getDouble(4), y=var->getDouble(4+sz);
        if( SVCHK('i',20) ) {
            double x1=sv->getDouble(4), y1=sv->getDouble(4+sz);
            return x==x1&&y==y1;
        } else if( SVCHK('i',2) ) {
            double x1=(double)sv->getInt(4), y1=(double)sv->getInt(8);
            return x==x1&&y==y1;
        }
    } else if( stat==2 ) {
        int x=var->getInt(4), y=var->getInt(8);
        if( SVCHK('i',20) ) {
            int sz=sizeof(double);
            int x1=(int)sv->getDouble(4), y1=(int)sv->getDouble(4+sz);
            return x==x1&&y==y1;
        } else if( SVCHK('i',2) ) {
            int x1=sv->getInt(4), y1=sv->getInt(8);
            return x==x1&&y==y1;
        }
    }
    return false;
}
void regRangeFuncs() {
    if( isHashKey("range") )
        return;
    PHashKey hash = getHashKey("range");
    U16 uid = 1;
    hash->add("pos", uid);			uid++;
    hash->add("move", uid);         uid++;  //
    uid++;
    hash->add("size", uid);			uid++;
    hash->add("rect", uid);			uid++;
    uid++;  // hash->add("point", uid);
    hash->add("eq", uid);			uid++;
    hash->add("x", uid);				uid++;
    hash->add("y", uid);				uid++;
    hash->add("width", uid);			uid++;
    hash->add("height", uid);		uid++;
    uid++;      // hash->add("sp", uid);
    uid++;      // hash->add("cmp", uid);
    hash->add("right", uid);		uid++;
    hash->add("bottom", uid);		uid++;
    uid++;		// hash->add("position", uid);
    hash->add("rate", uid);			uid++;
    uid++;		// hash->add("toString", uid);
    uid = 20;
    hash->add("lc", uid);				uid++;
    hash->add("lt", uid);			uid++;
    hash->add("tc", uid);				uid++;
    hash->add("rt", uid);			uid++;
    hash->add("rc", uid);				uid++;
    hash->add("rb", uid);			uid++;
    hash->add("bc", uid);				uid++;
    hash->add("lb", uid);			uid++;
    hash->add("center", uid);		uid++;
    hash->add("contains", uid);		uid++;
    hash->add("intersected", uid);
    uid=31;
    hash->add("leftTop", uid);		uid++;
    hash->add("leftCenter", uid);		uid++;
    hash->add("leftBottom", uid);		uid++;
    hash->add("rightTop", uid);		uid++;
    hash->add("rightCenter", uid);		uid++;
    hash->add("rightBottom", uid);		uid++;
    hash->add("topCenter", uid);		uid++;
    hash->add("bottomCenter", uid);		uid++;
    hash->add("moveLeft", uid);		uid++;
    hash->add("moveTop", uid);		uid++;
    hash->add("moveBottom", uid);		uid++;
    hash->add("moveRight", uid);		uid++;	// 42
    hash->add("hbox", uid);		uid++;
    hash->add("vbox", uid);		uid++;
    hash->add("merge", uid);		uid++;
    hash->add("valid", uid);		uid++;
    uid = 51;
    hash->add("imageSize", uid);		uid++;
    hash->add("saveImage", uid);		uid++;
    uid = 401;
    hash->add("midColor", uid);		uid++;
    hash->add("lightColor", uid);	uid++;
    hash->add("darkColor", uid);		uid++;
    hash->add("mixColor", uid);		uid++;
    hash->add("invertColor", uid);	uid++;
    hash->add("difColor", uid);		uid++;
    hash->add("orColor", uid);		uid++;
    hash->add("alphaColor", uid);	uid++;
    uid = 600;
    hash->add("incrX", uid);				uid++;
    hash->add("incrY", uid);				uid++;
    hash->add("incrW", uid);				uid++;
    hash->add("incrH", uid);				uid++;
    hash->add("incr", uid);				uid++;
    hash->add("incrXY", uid);			uid++;
    hash->add("state", uid);				uid++;
    hash->add("min", uid);				uid++;
    hash->add("max", uid);				uid++;
    hash->add("incrXW", uid);			uid++;
    hash->add("incrYH", uid);			uid++;
    hash->add("incrWH", uid);			uid++;
    hash->add("margin", uid);			uid++;
    hash->add("offset", uid);			uid++;

    uid = 992;
    hash->add("inject", uid);			uid++;
    hash->add("put", uid);
}
inline
bool rectInResult(StrVar* var, StrVar* sv, StrVar* rst) {
    QRect rc=setQRect(var), rcCheck=setQRect(sv);
    int x=rcCheck.x(), y=rcCheck.y(), r=rcCheck.right(), b=rcCheck.bottom();
    rst->reuse();
    if( rc.contains(x,y) ) {
        if( rc.contains(r,y) ) {
            if( rc.contains(x,b) ) {
                rst->add("1234");
            } else {
                rst->add("12");
            }
        } else if( rc.contains(x,b) ) {
            if( rc.contains(r,b) ) {
                rst->add("1234");
            } else {
                rst->add("14");
            }
        } else {
            rst->add("1");
        }
    } else if( rc.contains(r,y) ) {
        if( rc.contains(r,b) ) {
            rst->add("23");
        } else {
            rst->add("2");
        }
    } else if( rc.contains(r,b) ) {
        if( rc.contains(x,b) ) {
            rst->add("34");
        } else {
            rst->add("3");
        }
    } else if( rc.contains(x,b) ) {
        rst->add("4");
    }
    return rst->length()? true: false;
}

bool execRangeFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* var, StrVar* rst) {
    int cnt = arrs ? arrs->size(): 0;
    // StrVar* obj = var;
    U16 stat = checkObject(var,'i') ? var->getU16(2) : 0;
    U32 flag= fc->xfflag&0xFF;
    bool selfChk=flag==0x8 && fc->getFuncSize()==0 ? true: false;
    switch(fc->xfuncRef) {
    case 1:     // pos
    case 2: {   // move
        if( stat==5 || stat==20 ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz);
            double w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt==0 ) {
                rst->setVar('i',20).addDouble(x).addDouble(y);
                return true;
            }
            StrVar* sv=NULL;
            if( cnt==1 ) {
                sv = arrs->get(0);
                if( SVCHK('i',2) || SVCHK('i',4) ) {
                    x=(double)sv->getInt(4), y=(double)sv->getInt(8);
                } else if( SVCHK('i',20) || SVCHK('i',5) ) {
                    x=sv->getDouble(4), y=sv->getDouble(4+sz);
                } else if( isNumberVar(sv) ) {
                    x=toDouble(sv), y=x;
                }
            } else if( cnt==2 ) {
                x=toDouble(arrs->get(0)), y=toDouble(arrs->get(1));
            } else {
                sv=arrs->get(2);
                if( SVCHK('3',1) ) {
                    x+=toDouble(arrs->get(0)), y+=toDouble(arrs->get(1));
                } else if( SVCHK('3',0) ) {
                    x-=toDouble(arrs->get(0)), y-=toDouble(arrs->get(1));
                } else {
                    double rate=toDouble(sv);
                    x+=toDouble(arrs->get(0)), y+=toDouble(arrs->get(1));
                    w*=rate, h*=rate;
                }
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
            if( selfChk && var!=rst && checkFuncObject(rst,'i',stat) ) {
                var->reuse()->add(rst);
            }
        } else if( stat==4 || stat==2 ) {
            StrVar* sv = arrs ? arrs->get(0) : NULL;
            int x=var->getInt(4), y=var->getInt(8), w=var->getInt(12), h=var->getInt(16);
            bool c=false;
            if( cnt==1 ) {
                if( checkObject(sv,'i') ) {
                    rst->setVar('i',stat).addInt(sv->getInt(4)).addInt(sv->getInt(8));
                    c= true;
                } else {
                    return false;
                }
            } else if( cnt==2 ) {
                if( checkObject(sv,'i') ) {
                    rst->setVar('i',stat).addInt(sv->getInt(4)).addInt(sv->getInt(8));
                    c= true;
                } else {
                    rst->setVar('i',stat).addInt(toInteger(arrs->get(0))).addInt(toInteger(arrs->get(1)));
                }
            } else {
                rst->setVar('i',stat).addInt(x).addInt(y);
            }
            if( stat==4 ) {
                if( c )
                    rst->addInt(w+(x-sv->getInt(4))).addInt(h+(y-sv->getInt(8)));
                else
                    rst->addInt(w).addInt(h);
            }
            if( selfChk && var!=rst && checkFuncObject(rst,'i',stat) ) {
                var->reuse()->add(rst);
            }
        } else {
            return false;
        }
    } break;
    case 4: { // size
        if( stat==0 )
            return false;
        if( stat==4 ) {
            if( cnt==2 ) {
                rst->setVar('i',4).addInt(var->getInt(4)).addInt(var->getInt(8)).addInt(toInteger(arrs->get(0))).addInt(toInteger(arrs->get(1)));
            } else {
                int w=var->getInt(12), h=var->getInt(16);
                rst->setVar('i',2).addInt(w).addInt(h);
            }
        } else if( stat==2 ) {
            rst->setVar('i',2).addInt(var->getInt(4)).addInt(var->getInt(8));
        } else if( stat==5 ) {
            int sz=sizeof(double);
            if( cnt==0 ) {
                double w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
                rst->setVar('i',20).addDouble(w).addDouble(h);
            } else if( cnt==2 ) {
                double w=toDouble(arrs->get(0)), h=toDouble(arrs->get(1));
                if( selfChk && var!=rst ) {
                    var->putDouble(w,4+(2*sz)).putDouble(h,4+(3*sz));
                } else {
                    rst->setVar('i',5).addDouble(var->getDouble(4)).addDouble(var->getDouble(4+sz)).addDouble(w).addDouble(h);
                }
            }
        }
    } break;
    case 5: { // rect
        // version 1.0.2
        if( var==NULL || stat==0 ) return false;
        if( cnt==0 ) {
            if( stat==5 ) {
                int sz=sizeof(double);
                double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
                rst->setVar('i',4).addInt((int)x).addInt((int)y).addInt((int)w).addInt((int)h);
            } else if( stat==4 ) {
                if( var!=rst ) {
                    rst->reuse()->add(var);
                }
            } else if( stat==2 ) {
                rst->setVar('i',4).addInt(0).addInt(0).addInt(var->getInt(4)).addInt(var->getInt(8));
            } else {
                rst->setVar('i',4).addInt(0).addInt(0).addInt(1).addInt(1);
            }
        }
    } break;
    case 7: { // eq
        if( cnt==0 ) return false;
        rst->setVar('3',rangeEqCheck(var, arrs)?1:0);

    } break;
    case 600: { // incrX
        if( cnt==0 ) {
            if( stat==5 ) {
                int sz=sizeof(double);
                double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
                x-=w, w*=2.;
                rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
            }
            return true;
        }
        if( stat==5 ) {
            int sz=sizeof(double);
            double d = toDouble(arrs->get(0));
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            x+=d;
            if( !isVarTrue(arrs->get(1)) ) {
                w-=d;
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
            if( selfChk && var!=rst ) {
                var->reuse()->add(rst);
            }
        } else if( stat==20 ) {
            double d=toDouble(arrs->get(0)), x=var->getDouble(4);
            StrVar* target=rst;
            if( selfChk && var!=rst  ) {
                target=var;
            } else {
                target->add(var);
            }
            target->putDouble(x+d, 4);
        } else if( stat==4|| stat==2 ) {
            int d = toInteger(arrs->get(0));
            int x=var->getInt(4), y=var->getInt(8);
            if( stat==2 ) {
                rst->setVar('i',2).addInt(x+d).addInt(y);
            } else {
                int w=var->getInt(12), h=var->getInt(16);
                int e=isVarTrue(arrs->get(1))? 0 : d;
                rst->setVar('i',4).addInt(x+d).addInt(y).addInt(w-e).addInt(h);
            }
            if( selfChk && var!=rst && checkFuncObject(rst,'i',stat) ) {
                var->reuse()->add(rst);
            }
        }
    } break;
    case 601: { // incrY
        if( cnt==0 ) {
            if( stat==5 ) {
                int sz=sizeof(double);
                double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
                y-=h, h*=2.;
                rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
            }
            return true;
        }
        if( stat==5 ) {
            int sz=sizeof(double);
            double d = toDouble(arrs->get(0));
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            y+=d;
            if( !isVarTrue(arrs->get(1)) ) {
                h-=d;
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
            if( selfChk && var!=rst ) {
                var->reuse()->add(rst);
            }
        } else if( stat==20 ) {
            int sz=sizeof(double);
            double d = toDouble(arrs->get(0));
            double y=var->getDouble(4+sz);
            StrVar* target=rst;
            if( selfChk && var!=rst ) {
                target=var;
            } else {
                target->add(var);
            }
            target->putDouble(y+d, 4+sz);
        } else if( stat==4|| stat==2 ) {
            int d = toInteger(arrs->get(0));
            int x=var->getInt(4), y=var->getInt(8), w=var->getInt(12), h=var->getInt(16);
            if( stat==2 ) {
                rst->setVar('i',2).addInt(x).addInt(y+d);
            } else {
                int e=isVarTrue(arrs->get(1))? 0 : d;
                rst->setVar('i',4).addInt(x).addInt(y+d).addInt(w).addInt(h-e);
            }
            if( selfChk && var!=rst && checkFuncObject(rst,'i',stat) ) {
                var->reuse()->add(rst);
            }
        }
    } break;
    case 602: { // incrW
        if( cnt==0 ) {
            if( stat==5 ) {
                int sz=sizeof(double);
                double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
                w*=2.;
                rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
            }
            return true;
        }
        if( stat==5 ) {
            int sz=sizeof(double);
            double d = toDouble(arrs->get(0));
            double w=var->getDouble(4+(2*sz));
            StrVar* target=rst;
            if( selfChk && var!=rst  ) {
                target=var;
            } else {
                target->add(var);
            }
            target->putDouble(w+d, 4+(2*sz));
        } else if( stat==4|| stat==2 ) {
            int d = toInteger(arrs->get(0));
            int x=var->getInt(4), y=var->getInt(8);
            if( stat==2 ) {
                rst->setVar('i',2).addInt(x+d).addInt(y);
            } else {
                int w=var->getInt(12), h=var->getInt(16);
                rst->setVar('i',4).addInt(x).addInt(y).addInt(w+d).addInt(h);
            }
            if( selfChk && var!=rst && checkFuncObject(rst,'i',stat) ) {
                var->reuse()->add(rst);
            }
        }
    } break;
    case 603: { // incrH
        if( cnt==0 ) {
            if( stat==5 ) {
                int sz=sizeof(double);
                double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
                h*=2.;
                rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
            }
            return true;
        }
        if( stat==5 ) {
            int sz=sizeof(double);
            double d = toDouble(arrs->get(0));
            double h=var->getDouble(4+(3*sz));
            StrVar* target=rst;
            if( selfChk && var!=rst ) {
                target=var;
            } else {
                target->add(var);
            }
            target->putDouble(h+d, 4+(3*sz));
        } else if( stat==4|| stat==2 ) {
            int d = toInteger(arrs->get(0));
            int x=var->getInt(4), y=var->getInt(8);
            if( stat==2 ) {
                rst->setVar('i',2).addInt(x+d).addInt(y);
            } else {
                int w=var->getInt(12), h=var->getInt(16);
                rst->setVar('i',4).addInt(x).addInt(y).addInt(w).addInt(h+d);
            }
            if( selfChk && var!=rst && checkFuncObject(rst,'i',stat) ) {
                var->reuse()->add(rst);
            }
        }
    } break;
    case 604: { // incr
        if( cnt==0 ) return false;
        if( stat==5|| stat==20 ) {
            int sz=sizeof(double);
            double dx=toDouble(arrs->get(0)), dy=isNumberVar(arrs->get(1))? toDouble(arrs->get(1)):dx;
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            x+=dx, y+=dy;
            if( !checkFuncObject(arrs->get(2),'3',0) ) w-=2*dx, h-=2*dy;
            if( selfChk && var!=rst  ) {
                var->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
            } else {
                rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
            }
        } else {
            StrVar* sv = arrs->get(0);
            int x=var->getInt(4), y=var->getInt(8), w=var->getInt(12), h=var->getInt(16);
            if( cnt==1 ) {
                int d1=0, d2=0;
                if( checkObject(sv,'i') ) {
                    d1 = sv->getInt(4), d2=sv->getInt(8);
                } else {
                    d1 = toInteger(sv), d2 = isNumberVar(arrs->get(1)) ? toInteger(arrs->get(1)): d1;
                }
                rst->setVar('i',stat).addInt(x+d1).addInt(y+d2);
                if( stat==4 ) rst->addInt(w+(-2*d1)).addInt(h+(-2*d2));
            } else {
                int top=toInteger(arrs->get(0)), bottom=toInteger(arrs->get(1)), left=0, right=0;
                if( cnt==3 ) {
                    left=right=toInteger(arrs->get(2));
                } else if( cnt==4 ) {
                    left	= toInteger(arrs->get(2));
                    right	= toInteger(arrs->get(3));
                }
                rst->setVar('i',stat).addInt(x+left).addInt(y+top);
                if( stat==4 ) rst->addInt(w+(-1*(left+right))).addInt(h+(-1*(top+bottom)));
            }
            if( selfChk && var!=rst && checkFuncObject(rst,'i',stat) ) {
                var->reuse()->add(rst);
            }
        }
    } break;
    case 605: { // incrXY
        if( cnt==0 ) return false;
        StrVar* sv=arrs->get(0);
        if( SVCHK('i',2) ) {
            if( stat==2 || stat==4 ) {
                int dx=sv->getInt(4), dy=sv->getInt(8);
                bool positive=true;
                sv=arrs->get(1);
                if( sv && SVCHK('3',0) ) {
                    positive=false;
                }
                int x=var->getInt(4), y=var->getInt(8);
                if( positive ) {
                    x+=dx, y+=dy;
                } else {
                    x-=dx, y-=dy;
                }
                rst->setVar('i',stat).addInt(x).addInt(y);
            }
        } else {
            if( stat==5|| stat==20 ) {
                int sz=sizeof(double);
                double dx = toDouble(arrs->get(0)), dy = cnt==1? dx:toDouble(arrs->get(1));
                double x=var->getDouble(4), y=var->getDouble(4+sz);
                if( stat==20 ) {
                    rst->setVar('i',20).addDouble(x+dx).addDouble(y+dy);
                } else {
                    double w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
                    if( isVarTrue(arrs->get(2)) ) {
                        rst->setVar('i',5).addDouble(x+dx).addDouble(y+dy).addDouble(w).addDouble(h);
                    } else {
                        w-=2*dx, h-=2*dy;
                        if( w>0 && h>0 ) {
                            rst->setVar('i',5).addDouble(x+dx).addDouble(y+dy).addDouble(w).addDouble(h);
                        }
                    }
                }
            } else if( stat==4|| stat==2 ) {
                int dx = toInteger(arrs->get(0)), dy = cnt==1? dx: toInteger(arrs->get(1));
                int x=var->getInt(4), y=var->getInt(8), w=var->getInt(12), h=var->getInt(16);
                if( stat==2 ) {
                    rst->setVar('i',2).addInt(x+dx).addInt(y+dy);
                } else {
                    if( isVarTrue(arrs->get(2)) ) {
                        rst->setVar('i',4).addInt(x+dx).addInt(y+dy).addInt(w).addInt(h);
                    } else {
                        w-=2*dx, h-=2*dy;
                        if( w>0 && h>0 )
                            rst->setVar('i',4).addInt(x+dx).addInt(y+dy).addInt(w-dx).addInt(h-dy);
                    }
                }
            }
        }
        if( selfChk && var!=rst && checkFuncObject(rst,'i',stat) ) {
            var->reuse()->add(rst);
        }
    } break;
    case 606: { // state
        if( stat==4 ) {
            int state = var->length()>20 ? var->getInt(20): 0;
            if( cnt==0 ) {
                rst->setVar('0',0).addInt(state);
            } else if( cnt==1 ) {
                StrVar* sv=arrs->get(0);
                if( isNumberVar(sv) ) {
                    int flag=toInteger(sv);
                    rst->setVar('3', state&flag ? 1: 0);
                }
            } else {
                StrVar* sv=arrs->get(0);
                if( isNumberVar(sv) ) {
                    int flag=toInteger(sv);
                    if( isVarTrue(arrs->get(1)) ) {
                        state|=flag;
                    } else {
                        state&=~flag;
                    }
                    if( var->length()==20 )
                        var->addInt(state,20);
                    else
                        var->putInt(state,20);
                }
                if( var!=rst ) {
                    rst->reuse()->add(var);
                }
            }
        }
    } break;
    case 607: { // min
        if( stat==4 ) {
            int w=var->getInt(12), h=var->getInt(16);
            int nw=min(w, toInteger(arrs->get(0)) );
            int nh=min(h, toInteger(arrs->get(1)) );
            if( nw>0 ) {
                if( w!=nw ) var->putInt(nw, 12);
            }
            if( nh>0 ) {
                if( w!=nh ) var->putInt(nh, 16);
            }
            if( var!=rst ) {
                rst->reuse()->add(var);
            }
        }
    } break;
    case 608: { // max
        if( stat==4 ) {
            int w=var->getInt(12), h=var->getInt(16);
            int nw=max(w, toInteger(arrs->get(0)) );
            int nh=max(h, toInteger(arrs->get(1)) );
            if( nw>0 ) {
                if( w!=nw ) var->putInt(nw, 12);
            }
            if( nh>0 ) {
                if( w!=nh ) var->putInt(nh, 16);
            }
            if( var!=rst ) {
                rst->reuse()->add(var);
            }
        }
    } break;
    case 609: { // incrXW
        if( cnt==0 ) return false;
        if( stat==5|| stat==20 ) {
            int sz=sizeof(double);
            double dx = toDouble(arrs->get(0)), dw = toDouble(arrs->get(1));
            double x=var->getDouble(4), w=var->getDouble(4+(2*sz));
            StrVar* target=rst;
            if( selfChk && var!=rst  ) {
                target=var;
            } else {
                target->add(var);
            }
            w-=dx;
            target->putDouble(x+dx, 4);
            target->putDouble(w-dw, 4+(2*sz));
        } else if( stat==4 ) {
            int x=var->getInt(4), y=var->getInt(8), w=var->getInt(12), h=var->getInt(16);
            int a=toInteger(arrs->get(0)), b=cnt==2? toInteger(arrs->get(1))-a: -2*a;
            rst->setVar('i',4).addInt(x+a).addInt(y).addInt(w+b).addInt(h);
            if( selfChk && var!=rst && checkFuncObject(rst,'i',stat) ) {
                var->reuse()->add(rst);
            }
        }

    } break;
    case 610: { // incrYH
        if( cnt==0 ) return false;
        if( stat==5|| stat==20 ) {
            int sz=sizeof(double);
            double dx = toDouble(arrs->get(0)), dh = toDouble(arrs->get(1));
            double y=var->getDouble(4+sz), h=var->getDouble(4+(3*sz));
            StrVar* target=rst;
            if( selfChk && var!=rst  ) {
                target=var;
            } else {
                target->add(var);
            }
            h-=dx;
            target->putDouble(y+dx, 4+sz);
            target->putDouble(h-dh, 4+(3*sz));
        } else if( stat==4 ) {
            int x=var->getInt(4), y=var->getInt(8), w=var->getInt(12), h=var->getInt(16);
            int a=toInteger(arrs->get(0)), b=cnt==2? toInteger(arrs->get(1))-a: -2*a;
            rst->setVar('i',4).addInt(x).addInt(y+a).addInt(w).addInt(h+b);
            if( selfChk && var!=rst && checkFuncObject(rst,'i',stat) ) {
                var->reuse()->add(rst);
            }
        }
    } break;
    case 611: { // incrWH
        if( cnt==0 ) return false;
        if( stat==5|| stat==20 ) {
            int sz=sizeof(double);
            double dx = toDouble(arrs->get(0)), dw = toDouble(arrs->get(1));
            double w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            StrVar* target=rst;
            if( selfChk && var!=rst ) {
                target=var;
            } else {
                target->add(var);
            }
            target->putDouble(w+dx, 4+(2*sz));
            target->putDouble(h+dw, 4+(3*sz));
        } else if( stat==4 ) {
            int x=var->getInt(4), y=var->getInt(8), w=var->getInt(12), h=var->getInt(16);
            int a=toInteger(arrs->get(0)), b=cnt==2? toInteger(arrs->get(1)): a;
            rst->setVar('i',4).addInt(x).addInt(y).addInt(w-a).addInt(h-b);
            if( selfChk && var!=rst && checkFuncObject(rst,'i',stat) ) {
                var->reuse()->add(rst);
            }
        }
    } break;
    case 612: { // margin
        if( cnt==0 ) return false;
        if( stat==4 ) {
            int x=var->getInt(4), y=var->getInt(8), w=var->getInt(12), h=var->getInt(16);
            StrVar* sv=arrs->get(0);
            if( SVCHK('a',0)) {
                arrs=(XListArr*)SVO;
                sv=arrs->get(0);
                cnt=arrs->size();
            }
            if( isNumberVar(sv) ) {
                int a=toInteger(sv);
                if( cnt==1 ) {
                    x+=a,y+=a,w-=2*a,h-=2*a;
                } else if( cnt==2 ) {
                    int b=toInteger(arrs->get(1));
                    x+=a,y+=b,w-=2*a,h-=2*b;
                } else {
                    int b=toInteger(arrs->get(1)), c=toInteger(arrs->get(2)), d=toInteger(arrs->get(3));
                    x+=a,w-=a,y+=b,h-=b,w-=c,h-=d;
                }
                rst->setVar('i',4).addInt(x).addInt(y).addInt(w).addInt(h);
            }
            if( selfChk && var!=rst ) {
                var->reuse()->add(rst);
            }
        } else if( stat==5 ) {
            int sz=sizeof(double);
            StrVar* sv=arrs->get(0);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( SVCHK('a',0)) {
                arrs=(XListArr*)SVO;
                sv=arrs->get(0);
                cnt=arrs->size();
            }
            if( isNumberVar(sv) ) {
                double a=toDouble(sv);
                if( cnt==1 ) {
                    x+=a,y+=a,w-=2*a,h-=2*a;
                } else if( cnt==2 ) {
                    double b=toDouble(arrs->get(1));
                    x+=a,y+=b,w-=2*a,h-=2*b;
                } else {
                    double b=toDouble(arrs->get(1)), c=toDouble(arrs->get(2)), d=toDouble(arrs->get(3));
                    x+=a,w-=a,y+=b,h-=b,w-=c,h-=d;
                }
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
            if( selfChk && var!=rst  ) {
                var->reuse()->add(rst);
            }
        }
        if( selfChk && var!=rst && checkFuncObject(rst,'i',stat) ) {
            var->reuse()->add(rst);
        }
    } break;
    case 613: { // offset
        if(cnt==0 ) {
            if( stat==5) {
                int sz=sizeof(double);
                double x=var->getDouble(4), y=var->getDouble(4+sz);
                rst->setVar('i',20).addDouble(x).addDouble(y);
            } else if( stat==4) {
                rst->setVar('i',20).addDouble(var->getInt(4)).addDouble(var->getInt(8));
            }
            return true;
        }
        StrVar* sv=arrs->get(0);
        double x=0, y=0, dx=0, dy=0;
        int sz=sizeof(double);
        if( SVCHK('i',20) || SVCHK('i',2) ) {
            if(SVCHK('i',2)) {
                dx=(double)sv->getInt(4), dy=(double)sv->getInt(8);
            } else {
                dx=sv->getDouble(4), dy=sv->getDouble(4+sz);
            }
            if( stat==2 || stat==4 ) {
                x=var->getInt(4), y=var->getInt(8);
            } else if( stat==20 || stat==5 ) {
                x=var->getDouble(4), y=var->getDouble(4+sz);
            }
            sv=arrs->get(1);
        } else if( isNumberVar(sv) ) {
            x=toDouble(sv), y=toDouble(arrs->get(1));
            sv=arrs->get(2);
        }
        if(SVCHK('3',0)) {
            x-=dx;
            y-=dy;
        } else {
            x+=dx;
            y+=dy;
        }
        if( stat==20 || stat==2 ) {
            rst->setVar('i',20).addDouble(x).addDouble(y);
        } else if( stat==5 || stat==4) {
            double w=0, h=0;
            if(stat==4) {
                w=(double)var->getInt(12), h=(double)var->getInt(16);
            } else {
                w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            }

            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
        }
    } break;
    case 8: { // x
        if( var==NULL || stat==0 ) return false;
        if( cnt==0 ) {
            if( stat==4 || stat==2)
                rst->setVar('0',0).addInt( var->getInt(4) );
            else if( stat==5 || stat==20 )
                rst->setVar('4',0).addDouble( var->getDouble(4) );
        } else if( isNumberVar(arrs->get(0)) ) {
            if( stat==4|| stat==2 ) {
                int d = toInteger(arrs->get(0));
                if( selfChk && var!=rst ) {
                    var->putInt(d,4);
                } else {
                    if( stat==4) {
                        rst->setVar('i',4).addInt(d).addInt(var->getInt(8)).addInt(var->getInt(12)).addInt(var->getInt(16));
                    } else {
                        rst->setVar('i',2).addInt(d).addInt(var->getInt(4));
                    }
                }
            } else if( stat==5|| stat==20) {
                int sz=sizeof(double);
                double d = toDouble(arrs->get(0));
                if( selfChk && var!=rst ) {
                    var->putDouble(d,4);
                } else {
                    if( stat==5) {
                        double x=var->getDouble(4), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
                        if( isVarTrue(arrs->get(1)) ) {                            
                            w-=(d-x);
                            rst->setVar('i',5).addDouble(d).addDouble(var->getDouble(4+sz)).addDouble(w).addDouble(h);
                        } else {
                            rst->setVar('i',5).addDouble(d).addDouble(var->getDouble(4+sz)).addDouble(w).addDouble(h);
                        }
                    } else {
                        rst->setVar('i',20).addDouble(d).addDouble(var->getDouble(4+sz));
                    }
                }
            }
        }
    } break;
    case 9: { // y
        if( var==NULL || stat==0 ) return false;
        if( cnt==0 ) {
            if( stat==4 || stat==2) {
                rst->setVar('0',0).addInt( var->getInt(8));
            } else if( stat==5 || stat==20 ) {
                int sz=sizeof(double);
                rst->setVar('4',0).addDouble(var->getDouble(4+sz));
            }
        } else if( isNumberVar(arrs->get(0)) ) {
            if( stat==4|| stat==2 ) {
                int d = toInteger(arrs->get(0));
                if( selfChk && var!=rst ) {
                    var->putInt(d,8);
                } else {
                    if( stat==4) {
                        rst->setVar('i',4).addInt(var->getInt(4)).addInt(d).addInt(var->getInt(12)).addInt(var->getInt(16));
                    } else {
                        rst->setVar('i',2).addInt(var->getInt(4)).addInt(d);
                    }
                }
            } else if( stat==5|| stat==20) {
                int sz=sizeof(double);
                double d = toDouble(arrs->get(0));
                if( selfChk && var!=rst ) {
                    var->putDouble(d,4+sz);
                } else {
                    if( stat==5) {
                        double y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
                        if( isVarTrue(arrs->get(1)) ) {
                            h-=(d-y);
                            rst->setVar('i',5).addDouble(var->getDouble(4)).addDouble(d).addDouble(w).addDouble(h);
                        } else {
                            rst->setVar('i',5).addDouble(var->getDouble(4)).addDouble(d).addDouble(w).addDouble(h);
                        }
                    } else {
                        rst->setVar('i',20).addDouble(var->getDouble(4)).addDouble(d);
                    }
                }
            }
        }
    } break;
    case 10: { // width
        if( var==NULL || stat==0 ) return false;
        if( cnt==0 ) {
            if( stat==4) {
                rst->setVar('0',0).addInt( var->getInt(12) );
            } else if( stat==5 ) {
                int sz=sizeof(double);
                rst->setVar('4',0).addDouble( var->getDouble(4+(2*sz)));
            } else if( stat==6 ) {
                QPixmap* img = (QPixmap*)var->getObject(FUNC_HEADER_POS);
                if( img ) rst->setVar('0',0).addInt(img->width());
            } else if( stat==9 ) {
                QImage* img = (QImage*)var->getObject(FUNC_HEADER_POS);
                if( img ) rst->setVar('0',0).addInt(img->width());
            } else
                rst->setVar('0',0).addInt( var->getInt(4) );
        } else if( isNumberVar(arrs->get(0)) ) {
            if( stat==4|| stat==2 ) {
                int d = toInteger(arrs->get(0));
                if( selfChk && var!=rst ) {
                    var->putInt(d, stat==4? 12: 4);
                } else {
                    if( stat==4) {
                        rst->setVar('i',4).addInt(var->getInt(4)).addInt(var->getInt(8)).addInt(d).addInt(var->getInt(16));
                    } else {
                        rst->setVar('i',2).addInt(d).addInt(var->getInt(8));
                    }
                }
            } else if( stat==5 ) {
                int sz=sizeof(double);
                double d = toDouble(arrs->get(0));
                if( selfChk && var!=rst ) {
                    var->putDouble(d,4+(2*sz));
                } else {
                    rst->setVar('i',5).addDouble(var->getDouble(4)).addDouble(var->getDouble(4+sz)).addDouble(d).addDouble(var->getDouble(4+(3*sz)));
                }
            }
        }
    } break;
    case 11: { // height
        if( var==NULL || stat==0 ) return false;
        if( cnt==0 ) {
            if( stat==4) {
                rst->setVar('0',0).addInt( var->getInt(16));
            } else if( stat==5 ) {
                int sz=sizeof(double);
                rst->setVar('4',0).addDouble( var->getDouble(4+(3*sz)));
            } else if( stat==6 ) {
                QPixmap* img = (QPixmap*)var->getObject(FUNC_HEADER_POS);
                if( img ) rst->setVar('0',0).addInt(img->height());
            } else if( stat==9 ) {
                QImage* img = (QImage*)var->getObject(FUNC_HEADER_POS);
                if( img ) rst->setVar('0',0).addInt(img->height());
            } else
                rst->setVar('0',0).addInt( var->getInt(8));
        } else if( isNumberVar(arrs->get(0)) ) {
            if( stat==4|| stat==2 ) {
                int d = toInteger(arrs->get(0));
                if( selfChk && var!=rst ) {
                    var->putInt(d, stat==4? 16: 8);
                } else {
                    if( stat==4) {
                        rst->setVar('i',4).addInt(var->getInt(4)).addInt(var->getInt(8)).addInt(d).addInt(var->getInt(16));
                    } else {
                        rst->setVar('i',2).addInt(var->getInt(4)).addInt(d);
                    }
                }
            } else if( stat==5 ) {
                int sz=sizeof(double);
                double d = toDouble(arrs->get(0));
                if( selfChk && var!=rst ) {
                    var->putDouble(d,4+(3*sz));
                } else {
                    rst->setVar('i',5).addDouble(var->getDouble(4)).addDouble(var->getDouble(4+sz)).addDouble(var->getDouble(4+(2*sz))).addDouble(d);
                }
            }
        }
    } break;
    case 12: { // sp
    } break;
    case 13: { // cmp
    } break;
    case 14: { // right
        if( stat==4 ) {
            if( cnt==0 ) {
                rst->setVar('0',0).addInt( var->getInt(4)+var->getInt(12) );
            } else if( isNumberVar(arrs->get(0)) ) {
                int x=var->getInt(4);
                int d = toInteger(arrs->get(0));
                int b = d-x;
                if( b<0 ) {
                    x+=b;
                    b*=-1;
                }
                if( selfChk && var!=rst ) {
                    var->putInt(b,12);
                } else {
                    rst->setVar('i',4).addInt(x).addInt(var->getInt(8)).addInt(b).addInt(var->getInt(16));
                }
            }
        } else if( stat==5 ) {
            int sz=sizeof(double);
            if( cnt==0 ) {
                rst->setVar('0',0).addInt( var->getDouble(4)+var->getDouble(4+(2*sz)) );
            } else if( isNumberVar(arrs->get(0)) ) {
                double x=var->getDouble(4);
                double d= toDouble(arrs->get(0));
                double b= d-x;
                if( b<0 ) {
                    x+=b;
                    b*=-1.0;
                }
                if( selfChk && var!=rst ) {
                    var->putInt(b,4+(2*sz));
                } else {
                    rst->setVar('i',5).addDouble(x).addDouble(var->getDouble(4+sz)).addDouble(b).addDouble(d);
                }
            }
        }
    } break;
    case 15: { // bottom
        if( stat==4 ) {
            if( cnt==0 ) {
                rst->setVar('0',0).addInt( var->getInt(8)+var->getInt(16) );
            } else if( isNumberVar(arrs->get(0)) ) {
                int y=var->getInt(8);
                int d= toInteger(arrs->get(0));
                int b= d-y;
                if( b<0 ) {
                    y+=b;
                    b*=-1;
                }
                if( selfChk && var!=rst ) {
                    var->putInt(b,16);
                }
                rst->setVar('i',4).addInt(var->getInt(4)).addInt(y).addInt(var->getInt(12)).addInt(b);
            }
        }  else if( stat==5 ) {
            int sz=sizeof(double);
            if( cnt==0 ) {
                rst->setVar('0',0).addInt( var->getDouble(4+sz)+var->getDouble(4+(3*sz)) );
            } else if( isNumberVar(arrs->get(0)) ) {
                double y=var->getDouble(4+sz);
                double d= toDouble(arrs->get(0));
                double b= d-y;
                if( b<0 ) {
                    y+=b;
                    b*=-1.0;
                }
                if( selfChk && var!=rst ) {
                    var->putInt(b,4+(3*sz));
                } else {
                    rst->setVar('i',5).addDouble(var->getDouble(4)).addDouble(y).addDouble(var->getDouble(4+(2*sz))).addDouble(d);
                }
            }
        }
    } break;
    case 16: { // position

    } break;
    case 17: { // rate
        if( cnt==0 ) return false;
        double rate=toDouble(arrs->get(0));
        if( stat==5 || stat==20 ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz);
            rst->setVar('i',stat).addDouble(x*rate).addDouble(y*rate);
            if( stat==5 ) {
                double w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
                rst->addDouble(w*rate).addDouble(h*rate);
            }
        } else if( stat==4) {
            int x=var->getInt(4), y=var->getInt(8), w=var->getInt(12), h=var->getInt(16);
            rst->setVar('i',4).addInt((int)rounding( (double)x * rate, 1)).addInt((int)rounding( (double)y * rate, 1)).
                addInt((int)rounding( (double)w * rate, 1)).addInt((int)rounding( (double)h * rate, 1));
        }
    } break;
    case 18: { // toString        
    } break;
    case 20: { // lc
        if( var==NULL || stat==0 ) return false;
        if( stat==5) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                x+=toDouble(arrs->get(0));
                if( cnt>1 ) y+=toDouble(arrs->get(1));
            }
            rst->setVar('i',20).addDouble(x).addDouble(y+(h/2));
        } else if( stat==4) {
            int y = var->getInt(8), h = var->getInt(16);
            rst->setVar('i',2).addInt(var->getInt(4)).addInt(h==0?y:y+(int)(h/2));
        }
    } break;
    case 21: { // lt
        if( var==NULL || stat==0 ) return false;
        if( stat==5) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz);
            if( cnt>0 ) {
                x+=toDouble(arrs->get(0));
                if( cnt>1 ) y+=toDouble(arrs->get(1));
            }
            rst->setVar('i',20).addDouble(x).addDouble(y);
        } else if( stat==4) {
            rst->setVar('i',2).addInt(var->getInt(4)).addInt(var->getInt(8));
        }
    } break;
    case 22: { // tc
        if( var==NULL || stat==0 ) return false;
        if( stat==5) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                x+=toDouble(arrs->get(0));
                if( cnt>1 ) y+=toDouble(arrs->get(1));
            }
            rst->setVar('i',20).addDouble(x+(w/2)).addDouble(y);
        } else if( stat==4) {
            int x = var->getInt(4);
            int w = var->getInt(12);
            rst->setVar('i',2).addInt(w==0?x:x+(int)w/2).addInt( var->getInt(8));

        }
    } break;
    case 23: { // rt
        if( var==NULL || stat==0 ) return false;
        if( stat==5) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                x+=toDouble(arrs->get(0));
                if( cnt>1 ) y+=toDouble(arrs->get(1));
            }
            rst->setVar('i',20).addDouble(x+w).addDouble(y);
        } else if( stat==4) {
            int x = var->getInt(4);
            int y = var->getInt(8);
            int w = var->getInt(12);
            rst->setVar('i',2).addInt(x+w+(cnt==1?toInteger(arrs->get(0)):0)).addInt(y);
        }
    } break;
    case 24: { // rc
        if( var==NULL || stat==0 ) return false;
        if( stat==5) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                x+=toDouble(arrs->get(0));
                if( cnt>1 ) y+=toDouble(arrs->get(1));
            }
            rst->setVar('i',20).addDouble(x+w).addDouble(y+(h/2.0));
        } else if( stat==4) {
            int y = var->getInt(8);
            int h = var->getInt(16);
            rst->setVar('i',2).addInt(var->getInt(4)+var->getInt(12)).addInt(h==0?y:y+(int)(h/2));
        }
    } break;
    case 25: { // rb
        if( var==NULL || stat==0 ) return false;
        if( stat==5) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                x+=toDouble(arrs->get(0));
                if( cnt>1 ) y+=toDouble(arrs->get(1));
            }
            rst->setVar('i',20).addDouble(x+w).addDouble(y+h);
        } else if( stat==4) {
            rst->setVar('i',2).addInt(var->getInt(4)+var->getInt(12)).addInt(var->getInt(8)+var->getInt(16));
        }
    } break;
    case 26: { // bc
        if( var==NULL || stat==0 ) return false;
        if( stat==5) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                x+=toDouble(arrs->get(0));
                if( cnt>1 ) y+=toDouble(arrs->get(1));
            }
            rst->setVar('i',20).addDouble(x+(w/2.0)).addDouble(y+h);
        } else if( stat==4) {
            int x = var->getInt(4);
            int w = var->getInt(12);
            rst->setVar('i',2).addInt(w==0?x:x+(int)w/2).addInt(var->getInt(8)+var->getInt(16));
        }
    } break;
    case 27: { // lb
        if( var==NULL || stat==0 ) return false;
        if( stat==5) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                x+=toDouble(arrs->get(0));
                if( cnt>1 ) y+=toDouble(arrs->get(1));
            }
            rst->setVar('i',20).addDouble(x).addDouble(y+h);
        } else if( stat==4) {
            rst->setVar('i',2).addInt(var->getInt(4)).addInt(var->getInt(8)+var->getInt(16)+(cnt==1?toInteger(arrs->get(0)):0));
        }
    } break;
    case 28: { // center
        if( var==NULL || stat==0 ) return false;
        if( stat==5) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt==0 ) {
                rst->setVar('i',20).addDouble(x+(w/2.0)).addDouble(y+(h/2.0));
            } else {
                double pw=toDouble(arrs->get(0)), ph=toDouble(arrs->get(1));
                if( isVarTrue(arrs->get(2)) ) {
                    if( getFullRectPos(w, h, pw, ph, rst) ) {
                        double sx=rst->getDouble(4), sy=rst->getDouble(4+sz), sw=rst->getDouble(4+(2*sz)), sh=rst->getDouble(4+(3*sz));
                        rst->setVar('i',5).addDouble(x+sx).addDouble(y+sy).addDouble(sw).addDouble(sh);
                    }
                } else {
                    QPointF pt=getQRectF(var).center();
                    x=pt.x()-(pw/2.0), y=pt.y()-(ph/2.0);
                    rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(pw).addDouble(ph);
                }
                if( selfChk && var!=rst && checkFuncObject(rst,'i',stat) ) {
                    var->reuse()->add(rst);
                }
            }
        } else if( stat==4) {
            QPointF pt=getQRectF(var).center();
            if( cnt==0 ) {                
                rst->setVar('i',20).addDouble(pt.x()).addDouble(pt.y());
            } else {
                StrVar* sv=arrs->get(0);
                if( isNumberVar(sv) ) {
                    double pw=toDouble(sv), ph=toDouble(arrs->get(1));
                    double x=pt.x()-(pw/2.0), y=pt.y()-(ph/2.0);
                    rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(pw).addDouble(ph);
                }
            }
        }
    } break;
    case 29: { // contains
        if( var==NULL || stat==0 ) return false;
        StrVar* sv=arrs->get(0);
        if( stat==5 ) {
            double px=-1.0, py=-1.0;
            int sz=sizeof(double);
            if( SVCHK('i',5) || SVCHK('i',4) ) {
                QRectF rc=getQRectF(sv);
                rst->setVar('3',getQRectF(var).contains(rc) ? 1:0  );
                return true;
            }
            if( SVCHK('i',20) ) {
                px=sv->getDouble(4), py=sv->getDouble(4+sz);
            } else if( SVCHK('i',2) ) {
                px=(double)sv->getInt(4), py=(double)sv->getInt(8);
            } else if( isNumberVar(sv) ) {
                px=toDouble(sv), py=toDouble(arrs->get(1));
            }
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            rst->setVar('3', (x<=px && px<=x+w && y<=py && py<=y+h) ? 1:0  );
        } else if( stat==4 ) {
            if( SVCHK('i', 4) ) {
                if( isVarTrue(arrs->get(1)) ) {
                    rectInResult(var, sv, rst);
                } else {
                     QRect rcCheck=setQRect(sv);
                     rst->setVar('3', setQRect(var).contains(rcCheck)?1:0 );
                }
                return true;
            }
            int px=-1, py=-1;
            if( SVCHK('i',2) ) {
                px=sv->getInt(4), py=sv->getInt(8);
            } else if( SVCHK('i',20) ) {
                px=sv->getDouble(4), py=sv->getDouble(4+sizeof(double));
            } else if( isNumberVar(sv) ) {
                px=toInteger(sv), py=toInteger(arrs->get(1));
            }
            int x=var->getInt(4), y=var->getInt(8), w=var->getInt(12), h=var->getInt(16);
            rst->setVar('3', (x<=px && px<=x+w && y<=py && py<=y+h) ? 1:0  );
        }
    } break;
    case 30: { // intersected
        if( var==NULL || stat==0 ) return false;
        if( stat==5 || stat==4 ) {
            StrVar* sv=arrs->get(0);
            rst->setVar('3', rangeEqCheck(var, arrs) || getQRectF(var).intersects(getQRectF(sv)) ? 1:0  );
        }
    } break;
    case 31: { // leftTop
        if( var==NULL || stat==0 ) return false;
        if( stat==5 ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                double pw=toDouble(arrs->get(0));
                if( pw<0 ) pw=w+pw;
                if( pw<=0 ) pw=pw==0?w:1;
                if( pw>0) {
                    w=pw;
                }
                if( cnt>1 ) {
                    double ph=toDouble(arrs->get(1));
                    if(ph<0) ph=h+ph;
                    if(ph<=0) ph=ph==0?h:1;
                    if(ph>0) h=ph;
                }
                if( cnt>2 ) x+=toDouble(arrs->get(2));
                if( cnt>3 ) y+=toDouble(arrs->get(3));
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
        }
    } break;
    case 32: { // leftCenter
        if( var==NULL || stat==0 ) return false;
        if( stat==5 ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                double pw=toDouble(arrs->get(0));
                if( pw<0 ) pw=w+pw;
                if( pw<=0 ) pw=pw==0?w:1;
                if( pw>0) {
                    w=pw;
                }
                if( cnt>1 ) {
                    double ph=toDouble(arrs->get(1));
                    if(ph<0) ph=h+ph;
                    if(ph<=0) ph=ph==0?h:1;
                    if( ph>0 ) {
                        y+=(h-ph)/2;
                        h=ph;
                    }
                }
                if( cnt>2 ) x+=toDouble(arrs->get(2));
                if( cnt>3 ) y+=toDouble(arrs->get(3));
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
        }
    } break;
    case 33: { // leftBottom
        if( var==NULL || stat==0 ) return false;
        if( stat==5 ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                double pw=toDouble(arrs->get(0));
                if( pw<0 ) pw=w+pw;
                if( pw<=0 ) pw=pw==0?w:1;
                if( pw>0) {
                    w=pw;
                }
                if( cnt>1 ) {
                    double ph=toDouble(arrs->get(1));
                    if(ph<0) ph=h+ph;
                    if(ph<=0) ph=ph==0?h:1;
                    if( ph>0 ) {
                        y+=h-ph;
                        h=ph;
                    }
                }
                if( cnt>2 ) x+=toDouble(arrs->get(2));
                if( cnt>3 ) y+=toDouble(arrs->get(3));
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
        }
    } break;
    case 34: { // rightTop
        if( var==NULL || stat==0 ) return false;
        if( stat==5 ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                double pw=toDouble(arrs->get(0));
                x+=w;
                if( pw<0 ) pw=w+pw;
                if( pw<=0 ) pw=pw==0?w:1;
                if( pw>0 ) {
                    w=pw;
                }
                x-=w;
                if( cnt>1 ) {
                    double ph=toDouble(arrs->get(1));
                    if(ph<0) ph=h+ph;
                    if(ph<=0) ph=ph==0?h:1;
                    if(ph>0) h=ph;
                }
                if( cnt>2 ) x+=toDouble(arrs->get(2));
                if( cnt>3 ) y+=toDouble(arrs->get(3));
            } else {
                x+=w;
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
        }
    } break;
    case 35: { // rightCenter
        if( var==NULL || stat==0 ) return false;
        if( stat==5 ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                double pw=toDouble(arrs->get(0));
                x+=w;
                if( pw<0 ) pw=w+pw;
                if( pw<=0 ) pw=pw==0?w:1;
                if( pw>0 ) {
                    w=pw;
                }
                x-=w;
                if( cnt>1 ) {
                    double ph=toDouble(arrs->get(1));
                    if(ph<0) ph=h+ph;
                    if(ph<=0) ph=ph==0?h:1;
                    if( ph>0 ) {
                        y+=(h-ph)/2.0;
                        h=ph;
                    }
                }
                if( cnt>2 ) x+=toDouble(arrs->get(2));
                if( cnt>3 ) y+=toDouble(arrs->get(3));
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
        }
    } break;
    case 36: { // rightBottom
        if( var==NULL || stat==0 ) return false;
        if( stat==5 ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                double pw=toDouble(arrs->get(0));
                x+=w;
                if( pw<0 ) pw=w+pw;
                if( pw<=0 ) pw=pw==0?w:1;
                if( pw>0 ) {
                    w=pw;
                }
                x-=w;
                if( cnt>1 ) {
                    double ph=toDouble(arrs->get(1));
                    if(ph<0) ph=h+ph;
                    if(ph<=0) ph=ph==0?h:1;
                    if( ph>0 ) {
                        y+=h-ph;
                        h=ph;
                    }
                }
                if( cnt>2 ) x+=toDouble(arrs->get(2));
                if( cnt>3 ) y+=toDouble(arrs->get(3));
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
        }
    } break;
    case 37: { // topCenter
        if( var==NULL || stat==0 ) return false;
        if( stat==5 ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                double pw=toDouble(arrs->get(0));
                if( pw<0 ) pw=w+pw;
                if( pw<=0 ) pw=pw==0?w:1;
                x+=(w-pw)/2.0;
                w=pw;
                if( cnt>1 ) {
                    double ph=toDouble(arrs->get(1));
                    if(ph<0) ph=h+ph;
                    if(ph<=0) ph=ph==0?h:1;
                    if( ph>0 ) {
                        h=ph;
                    }
                }
                if( h==0 ) h=var->getDouble(4+(3*sz));
                if( cnt>2 ) x+=toDouble(arrs->get(2));
                if( cnt>3 ) y+=toDouble(arrs->get(3));
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
        }
    } break;
    case 38: { // bottomCenter
        if( var==NULL || stat==0 ) return false;
        if( stat==5 ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                double pw=toDouble(arrs->get(0));
                if( pw<0 ) pw=w+pw;
                if( pw<=0 ) pw=pw==0?w:1;
                x+=(w-pw)/2.0;
                w=pw;
                if( cnt>1 ) {
                    double ph=toDouble(arrs->get(1));
                    if(ph<0) ph=h+ph;
                    if(ph<=0) ph=ph==0?h:1;
                    if( ph>0 ) {
                        y+=h-ph;
                        h=ph;
                    }
                }
                if( cnt>2 ) x+=toDouble(arrs->get(2));
                if( cnt>3 ) y+=toDouble(arrs->get(3));
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
        }
    } break;
    case 39: { // moveLeft
        if( var==NULL || stat==0 ) return false;
        if( stat==5 ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                double pw=toDouble(arrs->get(0));
                if( pw<0 ) pw=w+pw;
                if( pw<=0 ) pw=pw==0?w:1;
                if( pw>0) {
                    w=pw;
                }
                x-=w;
                if( cnt>1 ) {
                    double ph=toDouble(arrs->get(1));
                    if(ph<0) ph=h+ph;
                    if(ph<=0) ph=ph==0?h:1;
                    if( ph>0 ) {
                        if(isVarTrue(arrs->get(4))) y+=(h-ph)/2;
                        h=ph;
                    }
                }
                if( cnt>2 ) x+=toDouble(arrs->get(2));
                if( cnt>3 ) y+=toDouble(arrs->get(3));
            } else {
                x-=w;
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
        }
    } break;
    case 40: { // moveTop
        if( var==NULL || stat==0 ) return false;
        if( stat==5 ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                double pw=toDouble(arrs->get(0));
                if( pw<0 ) pw=w+pw;
                if( pw<=0 ) pw=pw==0?w:1;
                if(isVarTrue(arrs->get(4))) x+=(w-pw)/2.0;
                w=pw;
                if( cnt>1 ) {
                    double ph=toDouble(arrs->get(1));
                    if(ph<0) ph=h+ph;
                    if(ph<=0) ph=ph==0?h:1;
                    if( ph>0 ) {
                        h=ph;
                    }
                }
                if( h==0 ) h=var->getDouble(4+(3*sz));
                y=-h;
                if( cnt>2 ) x+=toDouble(arrs->get(2));
                if( cnt>3 ) y+=toDouble(arrs->get(3));
            } else {
                y-=h;
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
        }
    } break;
    case 41: { // moveBottom
        if( var==NULL || stat==0 ) return false;
        if( stat==5 ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                double pw=toDouble(arrs->get(0));
                if( pw<0 ) pw=w+pw;
                if( pw<=0 ) pw=pw==0?w:1;
                if(isVarTrue(arrs->get(4))) x+=(w-pw)/2.0;
                w=pw;
                if( cnt>1 ) {
                    double ph=toDouble(arrs->get(1));
                    if(ph<0) ph=h+ph;
                    if(ph<=0) ph=ph==0?h:1;
                    if( ph>0 ) {
                        h=ph;
                    }
                }
                if( h==0 ) h=var->getDouble(4+(3*sz));
                y+=h;
                if( cnt>2 ) x+=toDouble(arrs->get(2));
                if( cnt>3 ) y+=toDouble(arrs->get(3));
            } else {
                y=h;
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
        }
    } break;

    case 42: { // moveRight
        if( var==NULL || stat==0 ) return false;
        if( stat==5 ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            if( cnt>0 ) {
                double pw=toDouble(arrs->get(0));
                x+=w;
                if( pw<0 ) pw=w+pw;
                if( pw<=0 ) pw=pw==0?w:1;
                if( pw>0) {
                    w=pw;
                }
                if( cnt>1 ) {
                    double ph=toDouble(arrs->get(1));
                    if(ph<0) ph=h+ph;
                    if(ph<=0) ph=ph==0?h:1;
                    if( ph>0 ) {
                        if(isVarTrue(arrs->get(4))) y+=(h-ph)/2;
                        h=ph;
                    }
                }
                if( cnt>2 ) x+=toDouble(arrs->get(2));
                if( cnt>3 ) y+=toDouble(arrs->get(3));
            } else {
                x+=w;
            }
            rst->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
        }
    } break;

    case 43: { // hbox
    } break;
    case 44: { // vbox
    } break;
    case 45: { // merge
        if( cnt==0 )
            return false;
        if( stat==5 || stat==4 ) {
            QRectF rc=getQRectF(var);
            for(int n=0;n<cnt;n++ ) {
                StrVar* sv=arrs->get(n);
                if( SVCHK('5',1) || SVCHK('4',1) ) {
                    rc|=getQRectF(sv);
                }
            }
            setVarRectF(rst, rc);
        }
    } break;
    case 46: { // valid
        if( stat==5 ) {
            rst->setVar('3', getQRectF(var).isValid()?1:0);
        } else if( stat==4 ) {
            // int x=var->getInt(4), y=var->getInt(8),
            int w=var->getInt(12), h=var->getInt(16);
            if( w>0 && h>0 ) {
                rst->setVar('3',1);
            } else {
                rst->setVar('3',0);
            }
        } else
            rst->setVar('3',0);
    } break;
    case 51: { // imageSize
        StrVar* sv = var;
        if( SVCHK('i',6) ) {
            QPixmap* pix = (QPixmap*)SVO;
            if( pix ) {
                rst->setVar('i',2).addInt(pix->width()).addInt(pix->height());
            }
        } else if( SVCHK('i',9) ) {
            QImage* pix = (QImage*)SVO;
            if( pix ) {
                rst->setVar('i',2).addInt(pix->width()).addInt(pix->height());
            }
        }
    } break;
    case 52: { // saveImage
        if( cnt==0 ) return false;
        StrVar* sv = var;
        LPCC filenm = AS(0);
        LPCC ext=NULL;
        int pos=LastIndexOf( filenm,'.');
        if( pos>0 ) {
            ext=filenm+pos+1;
            int len=slen(ext);
            rst->reuse();
            for( int n=0;n<len;n++) {
                rst->add( (char)toupper(ext[n]) );
            }
            ext=rst->get();
        }
        QByteArray ba;
        QBuffer buffer( &ba );
        buffer.open( QIODevice::WriteOnly );

        if( SVCHK('i',6) ) {
            QPixmap* pix = (QPixmap*)SVO;
            if( pix ) {
                pix->save( &buffer, ext);
            }
        } else if( SVCHK('i',9) ) {
            QImage* pix = (QImage*)SVO;
            if( pix ) {
                pix->save( &buffer, ext);
            }
        }
        rst->setVar('3',0);
        if( ba.size() ) {
            QFile file(KR(filenm));
            if( file.open(QIODevice::WriteOnly) ) {
                file.write(ba);
                file.close();
                rst->setVar('3',1);
            }
        }
    } break;
    case 401: { // midColor
        if( stat!=3 ) {
            return false;
        }
        int ar=var->getInt(4), ag=var->getInt(8),ab=var->getInt(12);
        if( cnt==0 )
            rst->setVar('i',3).addInt(ar).addInt(ag).addInt(ab);
        else {
            StrVar* cv = NULL;
            if( checkObject(arrs->get(0),'i') || GetColorValue(arrs->get(0)->get(), arrs->get(0)) ) {
                cv = arrs->get(0);
                int br=cv->getInt(4), bg=cv->getInt(8),bb=cv->getInt(12);
                rst->setVar('i',3).addInt(MulDiv(7,ar,10) + MulDiv(3,br,10)).
                    addInt(MulDiv(7,ag,10) + MulDiv(3,bg,10)).
                    addInt(MulDiv(7,ab,10) + MulDiv(3,bb,10));
                if(var->length()==20) rst->addInt(var->getInt(16));
            }
        }
    } break;
    case 402: { // lightColor
        if( stat!=3 ) {
            return false;
        }
        int ar=var->getInt(4), ag=var->getInt(8),ab=var->getInt(12);
        int scale = cnt==0? 25: toInteger(arrs->get(0));
        rst->setVar('i',3).addInt(MulDiv(255-ar,scale,255)+ar).
            addInt(MulDiv(255-ag,scale,255)+ag).
            addInt(MulDiv(255-ab,scale,255)+ab);
        if(var->length()==20) rst->addInt(var->getInt(16));
    } break;
    case 403: { // darkColor
        if( stat!=3 ) {
            return false;
        }
        int ar=var->getInt(4), ag=var->getInt(8),ab=var->getInt(12);
        int scale = cnt==0? 25: toInteger(arrs->get(0));
        rst->setVar('i',3).addInt(MulDiv(ar,(255-scale),255)).
            addInt(MulDiv(ag,(255-scale),255)).
            addInt(MulDiv(ab,(255-scale),255));
        if(var->length()==20) rst->addInt(var->getInt(16));
    } break;
    case 404: { // mixColor
        if( stat!=3 ) {
            return false;
        }
        int ar=var->getInt(4), ag=var->getInt(8),ab=var->getInt(12);
        if( cnt==0 )
            rst->setVar('i',3).addInt(ar).addInt(ag).addInt(ab);
        else {
            StrVar* cv = NULL;
            if( checkObject(arrs->get(0),'i') || GetColorValue(arrs->get(0)->get(), arrs->get(0)) ) {
                cv = arrs->get(0);
                int br=cv->getInt(4),bg=cv->getInt(8),bb=cv->getInt(12);
                rst->setVar('i',3).addInt(MulDiv(86,ar,100) + MulDiv(14,ar,100)).
                    addInt(MulDiv(86,ag,100) + MulDiv(14,bg,100)).
                    addInt(MulDiv(86,ab,100) + MulDiv(14,bb,100));
                if(var->length()==20) rst->addInt(var->getInt(16));
            }
        }
    } break;
    case 405: { // invertColor
        if( stat!=3 ) {
            return false;
        }
        int ar=var->getInt(4), ag=var->getInt(8),ab=var->getInt(12);
        // int scale = cnt==0? 25: toInteger(arrs->get(0));
        rst->setVar('i',3).addInt(255 - ar).
            addInt(255 - ag).
            addInt(255 - ab);
        if(var->length()==20) rst->addInt(var->getInt(16));
    } break;
    case 406: { // difColor
        if( stat!=3 ) {
            return false;
        }
        int ar=var->getInt(4), ag=var->getInt(8),ab=var->getInt(12);
        int scale = cnt==0? 25: toInteger(arrs->get(0));
        rst->setVar('i',3).addInt(qAbs(ar-scale)).
            addInt(qAbs(ag-scale)).
            addInt(qAbs(ab-scale));
        if(var->length()==20) rst->addInt(var->getInt(16));
    } break;
    case 407: { // orColor
        if( stat!=3 ) {
            return false;
        }
        int ar=var->getInt(4), ag=var->getInt(8),ab=var->getInt(12);
        // int per = cnt==1? 25: toInteger(arrs->get(0));
        if( cnt==0 )
            rst->setVar('i',3).addInt(ar).addInt(ag).addInt(ab);
        else {
            StrVar* cv = NULL;
            if( checkObject(arrs->get(0),'i') || GetColorValue(arrs->get(0)->get(), arrs->get(0)) ) {
                cv = arrs->get(0);
                int br=cv->getInt(4), bg=cv->getInt(8),bb=cv->getInt(12);
                int per = cnt==1? toInteger(arrs->get(1)): 50;
                if( per<=0 ) per = 50;
                rst->setVar('i',3).addInt((ar*100 + ( per*(br-ar) ) ) /100).
                    addInt((ag*100 + ( per*(bg-ag) ) ) /100).
                    addInt((ab*100 + ( per*(bb-ab) ) ) /100);
                if(var->length()==20) rst->addInt(var->getInt(16));
            }
        }
    } break;
    case 408: { // alphaColor
        if( stat!=3 ) {
            return false;
        }
        if( cnt==0 ) {
            int aa=0;
            if( var->length()==20 ) {

            }
            rst->setVar('0',0).addInt(aa);
            return true;
        }
        int ar=var->getInt(4), ag=var->getInt(8),ab=var->getInt(12);
        int aa = toInteger(arrs->get(0));
        rst->setVar('i',3).addInt(ar).addInt(ag).addInt(ab).addInt(aa);
    } break;
    case 992: { // inject
        if( stat==5 ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
            for( int m=0; m<fc->getParamSize(); m++) {
                XFunc* fcParam=fc->getParam(m);
                LPCC code = fcParam->getVarName();
                double num = m==0 ? x : m==1? y: m==2? w: m==3? h : 0;
                fn->GetVar(code)->setVar('4',0).addDouble(num);
            }
        } else if( stat==2  ) {
            int x=var->getInt(4), y=var->getInt(8);
            for( int n=0;n<fc->getParamSize();n++) {
                LPCC code = fc->getParam(n)->getVarName();
                fn->GetVar(code)->setVar('0',0).addInt(n==0 ? x : n==1? y: 0);
            }
        } else if( stat==20  ) {
            int sz=sizeof(double);
            double x=var->getDouble(4), y=var->getDouble(4+sz);
            for( int n=0;n<fc->getParamSize();n++) {
                LPCC code = fc->getParam(n)->getVarName();
                fn->GetVar(code)->setVar('4',0).addDouble(n==0 ? x : n==1? y: 0);
            }
        } else if( stat==4 ) {
            int x=var->getInt(4), y=var->getInt(8), w=var->getInt(12), h=var->getInt(16);
            for( int m=0, n=0; m<fc->getParamSize(); m++) {
                XFunc* fcParam=fc->getParam(m);
                LPCC code = fcParam->getVarName();
                int num = n==0 ? x : n==1? y: n==2? w: n==3? h : 0;
                fn->GetVar(code)->setVar('0',0).addInt(num);
                n++;
            }
        }
    } break;
    case 993: { // put
        StrVar* sv=NULL;
        if( stat==2 ) {
            for( int n=0;n<fc->getParamSize();n++) {
                LPCC code = fc->getParam(n)->getVarName();
                sv=getFuncVar(fn, code);
                if( isNumberVar(sv) ) {
                    int num=toInteger(sv);
                    if( n==0 ) {
                        var->putInt(num, FUNC_HEADER_POS);
                    } else {
                        var->putInt(num, FUNC_HEADER_POS+4);
                    }
                }
            }
        } else if( stat==4 ) {
            for( int n=0;n<fc->getParamSize();n++) {
                LPCC code = fc->getParam(n)->getVarName();
                sv=getFuncVar(fn, code);
                if( isNumberVar(sv) ) {
                    int num=toInteger(sv);
                    if( n==0 ) {
                        var->putInt(num, FUNC_HEADER_POS);
                    } else if( n==1 ) {
                        var->putInt(num, FUNC_HEADER_POS+4);
                    } else if( n==2 ) {
                        var->putInt(num, FUNC_HEADER_POS+8);
                    } else if( n==3 ) {
                        var->putInt(num, FUNC_HEADER_POS+12);
                    }
                }
            }
        }
    } break;
    default: return false;
    }
    return true;
}

