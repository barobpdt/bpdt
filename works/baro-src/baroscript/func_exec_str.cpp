#include "func_util.h"
#include "../webserver/HttpNode.h"
#include "LZ77LIB.h"
#include <QTextCodec>
StrVar gLocalVar;

inline char mapStartChar(char c) {
    return
        c=='<' ? '>':
        c=='('? ')' :
        c=='['? ']' :
        c=='{'? '}' :
        c=='>' ? '<':
        c==')'? '(' :
        c==']'? '[' :
        c=='}'? '{' :  0;
}



void regStrFuncs() {
    if( isHashKey("str") )
        return;
    PHashKey hash = getHashKey("str");
    U16 uid = 1;
    hash->add("find", uid);			uid++;
    hash->add("findPos", uid);		uid++;
    hash->add("findLast", uid);		uid++; //
    hash->add("indexOf", uid);		uid++; //
    hash->add("lastIndexOf", uid);	uid++;
    hash->add("right", uid);			uid++;
    hash->add("valid", uid);			uid++;
    hash->add("pos", uid);			uid++; //
    hash->add("ch", uid);			uid++;
    hash->add("cur", uid);			uid++;
    hash->add("match", uid);			uid++; //
    hash->add("left", uid);			uid++; //
    hash->add("trim", uid);			uid++; //
    hash->add("size", uid);			uid++; //
    hash->add("length", uid);		uid++; //
    hash->add("lower", uid);			uid++; //
    hash->add("upper", uid);			uid++; //
    hash->add("escape", uid);			uid++; //
    uid = 30;
    hash->add("findNum", uid);		uid++; //
    hash->add("start", uid);			uid++; //
    hash->add("count", uid);			uid++; //
    hash->add("finds", uid);			uid++; //
    uid = 40;
    hash->add("add", uid);			uid++; //
    hash->add("append", uid);		uid++; //
    hash->add("split", uid);			uid++; //
    hash->add("word", uid);			uid++; //
    hash->add("in", uid);			uid++; //
    hash->add("move", uid);			uid++; //
    hash->add("next", uid);			uid++; //
    hash->add("end", uid);			uid++; //
    uid = 50;
    hash->add("kr", uid);			uid++; //
    hash->add("utf8", uid);			uid++; //
    hash->add("str", uid);			uid++; //
    hash->add("again", uid);			uid++; //
    hash->add("check", uid);			uid++; //
    hash->add("encode", uid);		uid++; //
    hash->add("decode", uid);		uid++; //
    hash->add("lpad", uid);			uid++; //
    hash->add("replace", uid);		uid++; //
    uid = 60;
    uid++; //
    hash->add("substr", uid);		uid++; //
    hash->add("ref", uid);			uid++; //
    hash->add("bufferAdd", uid);     uid++; //
    hash->add("bufferRead", uid);    uid++; //
    hash->add("is", uid);			uid++; //
    hash->add("charAt", uid);		uid++; //
    hash->add("prevChar", uid);		uid++; //
    hash->add("lessThan", uid);		uid++; //
    uid++; //
    hash->add("isBlank", uid);		uid++; //
    hash->add("regRemove", uid);		uid++; //
    hash->add("regReplace", uid);	uid++; //
    hash->add("equals", uid);        uid++;  //
    hash->add("toByte", uid);		uid++; //
    hash->add("findReg", uid);		uid++; //
    uid++; //   hash->add("indexOf", uid);
    hash->add("typeValue", uid);		uid++; //
    hash->add("exp", uid);			uid++; //
    hash->add("prevWord", uid);		uid++; //
    hash->add("insert", uid);		uid++; //
    hash->add("remove", uid);		uid++; //
    uid=801;
    hash->add("toNumber", uid);		uid++;	// 801
    hash->add("toInt", uid);			uid++;	// 2
    hash->add("toDouble", uid);		uid++;	// 3
    hash->add("toLong", uid);		uid++;	// 4
    hash->add("toString", uid);		uid++;	// 5
    hash->add("eq", uid);			uid++;	// 6
    hash->add("ne", uid);			uid++;	// 7
    hash->add("incr", uid);			uid++;	// 808
    uid=811;
    hash->add("value", uid);
	uid=820;
	hash->add("lt", uid);			uid++;
	hash->add("le", uid);			uid++;
	hash->add("gt", uid);			uid++;
	hash->add("ge", uid);			uid++;
	hash->add("between", uid);		uid++;
}
inline bool cncmp(LPCC data, int datasize,  LPCC find, int findsize ) {
    if( findsize==-1 )
        findsize = slen(find);
    if( datasize==findsize ) {
        for(int n=0; n<datasize; n++ )
            if( *find++!=*data++ ) return false;
        return true;
    }
    return false;
}

inline bool checkNum(StrVar* var) {
    if( var==NULL || var->length()==0 )
        return false;
    if( var->charAt(0)=='\b' ) {
        switch(var->charAt(1) ) {
        case '0':
        case '1':
        case '2':
        case '4':
            return true;
        }
    }
    return false;
}

inline void codePointToUTF8(long cp, StrVar* rst) {
   if (cp <= 0x7f) {
      rst->add( static_cast<char>(cp) );
   }
   else if (cp <= 0x7FF)
   {
      rst->add( static_cast<char>(0xC0 | (0x1f & (cp >> 6))) );
      rst->add( static_cast<char>(0x80 | (0x3f & cp)) );
   }
   else if (cp <= 0xFFFF)
   {
      rst->add( (char)(0xE0 | static_cast<char>((0xf & (cp >> 12)))) );
      rst->add( (char)(0x80 | static_cast<char>((0x3f & (cp >> 6)))) );
      rst->add( static_cast<char>(0x80 | (0x3f & cp)) );
   }
   else if (cp <= 0x10FFFF)
   {
      rst->add( static_cast<char>(0xF0 | (0x7 & (cp >> 18))) );
      rst->add( static_cast<char>(0x80 | (0x3f & (cp >> 12))) );
      rst->add( static_cast<char>(0x80 | (0x3f & (cp >> 6))) );
      rst->add( static_cast<char>(0x80 | (0x3f & cp)) );
   }
}
inline int getBinaryShift(U32 num, U32 flag=0) {
    int shift=4;
    if( flag==0 ) {
        if( num < 0x80) {
            shift=1;
            return shift;
        }
        if( num < 0xC0) {
            qDebug("invalid utf8 lead byte");
            return 0;
        }
        if( num < 0xE0) {
            shift=1;
        } else if( num < 0xF0) {
            shift=2;
        } else if( num < 0xF8) {
            shift=3;
        }
    } else {
        if( num<=0xFF ) {
            shift=1;
        } else if( num<=0xFFFF ) {
            shift=2;
        } else if( num<=0xFFFFFFFF ) {
            shift=4;
        }
    }
    return shift;
}

void decodeUnicode(StrVar* sv, int sp, int ep, StrVar* out) {
    if( sv==NULL || out==NULL ) return;
    if( sp>=ep ) {
        return;
    }
    LPCC unicodeStr=sv->get(sp);
    char iChar;
    out->reuse();
    for (int i = sp; i < ep; i++){
        iChar = unicodeStr[i];
        if(iChar == '\\'){ // got escape char
            switch ( unicodeStr[++i] )
            {
            case '"': out->add('"'); break;
            case '/': out->add('/'); break;
            case '\\': out->add('\\'); break;
            case 'b': out->add('\b'); break;
            case 'f': out->add('\f'); break;
            case 'n': out->add('\n'); break;
            case 'r': out->add('\r'); break;
            case 't': out->add('\t'); break;
            case 'u':
            {
                long unicode= 0;
                for (int j = 0; j < 4; j++){
                    iChar = unicodeStr[++i];
                    unicode *= 16;
                    if ( iChar >= '0'  &&  iChar<= '9' )
                        unicode += iChar - '0';
                    else if ( iChar >= 'a'  &&  iChar <= 'f' )
                        unicode += iChar - 'a' + 10;
                    else if ( iChar >= 'A'  &&  iChar <= 'F' )
                        unicode += iChar - 'A' + 10;
                    else
                        return;
                }
                if( unicode>= 0xD800 && unicode <= 0xDBFF ) {
                    if( unicodeStr[++i]=='\\' && unicodeStr[++i]=='u' ) {
                        long pair= 0;
                        for (int j = 0; j < 4; j++){
                            iChar = unicodeStr[++i];
                            pair *= 16;
                            if ( iChar >= '0'  &&  iChar<= '9' )
                                pair += iChar - '0';
                            else if ( iChar >= 'a'  &&  iChar <= 'f' )
                                pair += iChar - 'a' + 10;
                            else if ( iChar >= 'A'  &&  iChar <= 'F' )
                                pair += iChar - 'A' + 10;
                            else
                                return;
                        }
                        unicode = 0x10000 + ((unicode & 0x3FF) << 10) + (pair & 0x3FF);
                        codePointToUTF8( unicode, out);
                    } else {
                        return;
                    }
                } else {
                    codePointToUTF8( unicode, out);
                }
            } break;
            default: return;
            }
        } else {
            out->add(iChar);
        }
    }
}

bool execStrFunc(XFunc* fc, PARR arrs, XFuncNode* fn, StrVar* sv, StrVar* rst, StrVar* orgin) {
    int sp=0,ep=0,cpos=0,ppos=0;
    StrVar* var=NULL;
    bool bref=true;
    int cnt = arrs? arrs->size():0;
    if( SVCHK('s',1) ) {
        sv=(StrVar*)SVO;
    }
    if( SVCHK('s',0) ) {
        sp=sv->getInt(OBJ_PROP_START_POS);
        ep=sv->getInt(OBJ_PROP_START_POS+4);
        cpos=sv->getInt(OBJ_PROP_START_POS+8);
        ppos=sv->getInt(OBJ_PROP_START_POS+12);
        var = sv;
        sv=(StrVar*)SVO;
        rst->reuse();
        if(fc->xfuncRef==40 || fc->xfuncRef==41 ) {
            StrVar* data=NULL;
            if( sv!=rst ) {
                for( int n=0;n<cnt;n++ ) {
                    data=arrs->get(n);
                    if( checkFuncObject(data,'s',0) ) {
                        int s=0,e=0;
                        data=getStrVarPointer(data,&s,&e);
                        if( s<e ) {
                            rst->add(data->get(sp), ep-sp);
                        }
                    } else {
                        toString(data, rst);
                    }
                }
                if( rst->length() ) {
                    sv->add(rst);
                    ep+=rst->length();
                    var->setVar('s',0,LPVOID(sv)).addInt(sp).addInt(ep).addInt(sp).addInt(ep);
                }
            }

            return true;
        }
    } else {
        bref=false;
    }
    int send=sv->length();
    if( ep==0 && cpos==0 )
        ep = send;
    else if( ep>send )
        ep = send;


    // int prev=sp;
    bool bchk= fc->xfflag==0 || fc->xfflag&FFLAG_PARAM;

    switch(fc->xfuncRef) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5: {	// find
        if( cnt==0 ) {
            return false;
        }
        LPCC str=NULL;
        U32 flag=0;
        str=AS(0);
        if( cpos==-1 || slen(str)==0 ) {
            if( fc->xfuncRef>3 ) {
                rst->setVar('0',0).addInt(-1);
            } else {
                rst->setVar('3',0);
            }
            return true;
        }
        if( cnt>1 && isNumberVar(arrs->get(1)) ) {
            int chk = toInteger(arrs->get(1));
            if( chk&1 ) flag|=FIND_SKIP;
            if( chk&2 ) flag|=FIND_NOCASE;
            if( chk&4 ) flag|=FIND_CHARS;
            if( chk&8 ) flag|=FIND_FUNCSTR;
            if( chk&16 ) flag|=FIND_WORD;
            if( chk&0x100 ) flag|=FIND_BACK;
        }
        XParseVar pv(sv,sp,ep );
        if( flag==0 ) {
            flag=FIND_FORWORD;
        } else if( flag&FIND_BACK ) {
            pv.start = pv.wep-1;
        }

        int pos=0;
        ppos = pv.wep;
        if( fc->xfuncRef==3 || fc->xfuncRef==5 ) {
            // version 1.0.2 => findLast
            pos=pv.findPrev(str,pv.wep,pv.wsp,flag);
            if( fc->xfuncRef==5 ) {
                // lastIndexOf
                rst->setVar('0',0).addInt(pos);
                return true;
            }
            if( pos==-1 ) {
                sp=ep=pos=pv.wep;
            } else {
                pos+= flag&FIND_CHARS ? 1 : slen(str);
                sp=min(pv.prev,pv.start);
                if( cnt>2 ) {
                    ep=pos;
                } else {
                    ep=pv.cur;
                }
                if( sp==ep ) ep=sp+1;
            }
        } else {
            if( flag&FIND_BACK ) {
                // pos = pv.findPos(str,pv.wsp, flag);
                StrVar* pvar=pv.GetVar();
                if( pvar ) {
                    pos = pvar->findPos(str,pv.start,pv.wsp,flag,NULL );
                    if( pos!=-1) {
                        pv.prev = pv.wsp;
                        pv.start = pos;
                        pv.cur = pos;
                    }
                } else {
                    pos=-1;
                }
                if( fc->xfuncRef==2 ) {
                    if( rst==sv ) {
                        sv=getStrVarTemp();
                        sv->add(rst);
                    }
                    if( pos==-1 ) {
                        rst->setVar('s',0,(LPVOID)sv).addInt(pv.wsp).addInt(pv.wep).addInt(pos).addInt(pv.wep);
                        if( var && sv!=var ) {
                            var->setVar('s',0,(LPVOID)sv).addInt(0).addInt(0).addInt(pos).addInt(pv.wep);
                        }
                    } else {
                        if( flag&FIND_CHARS )
                            pos+=1;
                        else
                            pos+=slen(str);
                        rst->setVar('s',0,(LPVOID)sv).addInt(pos).addInt(pv.wep).addInt(pv.wsp).addInt(pv.cur);
                        if( var && sv!=var ) {
                            if( pv.wsp==0 && pv.cur==0 ) {
                                var->setVar('s',0,(LPVOID)sv).addInt(0).addInt(0).addInt(-1).addInt(pv.wep);
                            } else {
                                var->setVar('s',0,(LPVOID)sv).addInt(pv.wsp).addInt(pv.cur).addInt(pos).addInt(pv.wep);
                            }
                        }
                    }
                    return true;
                } else if( fc->xfuncRef==4 ) {
                    rst->setVar('0',0).addInt(pos);
                    return true;
                }
            } else {
                pos = pv.findPos(str,pv.wep, flag);
            }
            if( fc->xfuncRef==4 ) {
                rst->setVar('0',0).addInt(pos);
                return true;
            }

            if( pos==-1 ) {                
                if( fc->xfuncRef==2 ) {
                    ppos=pos=pv.wep;
                } else {
                    if( isTrueCheck(fc) ) {
                        rst->setVar('3', 0 );
                        return true;
                    }
                    sp=ep=pos=pv.wep;
                }
            } else {
                if( fc->xfuncRef==1 && isTrueCheck(fc) ) {
                    rst->setVar('3', 1 );
                    return true;
                }
                sp=pv.prev;
                ep=pv.cur;
                if( sp==0 && ep==0 ) {
                    ppos=pv.wep;
                }
                if( !isVarTrue(arrs->get(2)) ) {
                    if( flag&FIND_CHARS )
                        pos+=1;
                    else
                        pos+=slen(str);
                }
            }
        }
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        if( sp==0 && ep==0 ) {
            if( pv.wsp==0 ) {
                // qDebug(">> findPos (%d, %d)\n\n", pv.start, pv.wep );
                rst->reuse();
            } else {
                rst->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(-1).addInt(pos);
            }
        } else {
            rst->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(pos).addInt(ppos);
        }
        if( fc->xfuncRef==2 || fc->xfuncRef==3 ) {
            if(ppos<pos) {
                qDebug("#0#string ref error %d, %d\n", pos, ppos);
                ppos=pos;
            }
            if( orgin ) {
                if(orgin==sv) {
                    qDebug("#0#string ref var eq %d, %d\n", pos, ppos);
                } else {
                    orgin->setVar('s',0,(LPVOID)sv).addInt(pos).addInt(ppos).addInt(sp).addInt(ep);
                }
            } else if( var && sv!=var ) {
                var->setVar('s',0,(LPVOID)sv).addInt(pos).addInt(ppos).addInt(sp).addInt(ep);
            }
        }
    } break;

    case 808: {	// incr
        if( cnt==0 ) {
            sp++;
        } else {
            if( isNumberVar(arrs->get(0)) ) {
                int x=toInteger(arrs->get(0));
                if( x==0 ) {
                    XParseVar pv(sv,sp+1,ep);
                    pv.ch();
                    sp=pv.start;
                } else if(x<0) {
                    if( isVarTrue(arrs->get(1)) ) {
                        ep+=x;
                        if( ep<0 ) ep=0;
                    } else {
                        sp+=x;
                    }
                } else {
                    sp+=x;
                }
            } else {
                sp+=1;
            }
        }
        if( rst!=sv ) {
            if( ep==0 && cpos!=-1 ) {
                ep = send;
            }
            if( sp < ep ) {
                rst->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(cpos).addInt(ppos);
            } else {
                rst->setVar('s',0,(LPVOID)sv).addInt(0).addInt(0).addInt(-1).addInt(0);
            }
        }
        if( orgin ) {
            orgin->putInt(sp,OBJ_PROP_START_POS);
        } else if( var && sv!=var ) {
            if( sp==0 && ep==0 ) {
                var->setVar('s',0,(LPVOID)sv).addInt(0).addInt(0).addInt(-1).addInt(0);
            } else {
                var->putInt(sp,OBJ_PROP_START_POS).putInt(ep,OBJ_PROP_START_POS+4);
            }
            // rst->reuse()->add(var);
        }
    } break;
    case 6: {	// right
        if( arrs ) {
            LPCC str=AS(0);
            XParseVar pv(sv,sp,ep );
            int pos = isVarTrue(arrs->get(1))? pv.findPrev(str,ep, sp, FIND_BACK) : pv.findPos(str,ep, FIND_FORWORD);
            ppos = ep;
            if( pos==-1 ) {
                if( rst!=sv ) {
                    rst->reuse();
                } else if(var) {
                    var->setVar('s',0,(LPVOID)sv).addInt(ep).addInt(ep).addInt(pos).addInt(ep);
                }
            } else {
                pos+=slen(str);
                if( pos<sp ) pos=sp;
                else if( pos>ep ) pos=ep;
                if( rst!=sv ) {
                    rst->set(sv->get(pos), ep-pos );
                } else {
                    if(var) {
                        var->setVar('s',0,(LPVOID)sv).addInt(pos).addInt(ep).addInt(pos).addInt(ep);
                    } else {
                        LPCC val=gBuf.add( sv->get(pos), ep-pos );
                        rst->set(val);
                    }
                }
            }
        } else {
            if( cpos==-1 ) {
                ep = send;
                rst->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(ep);
            }
            if( cpos>sp ) sp=cpos;
            if( ppos>ep ) ep=ppos;
            if( ep==0 )
                ep = send;
            rst->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(ep);
        }
    } break;
    case 7: {	// valid
        if( !bref ) {
            rst->setVar('3', 0);
            return true;
        }

        if( cpos==-1 || (ep>0 && sp>=ep) ) {
            rst->setVar('3', 0);
            return true;
        }
        if( ep==0 ) {
            ep= send;
        }
        if( orgin ) {
            qDebug("#### str valid origin (%d, %d) ####",sp, ep);
            if( sp>=ep ) {
                rst->setVar('3',0);
            } else {
                if( cpos<=ppos ) {
                    rst->setVar('s',0,(LPVOID)sv).addInt(cpos).addInt(ppos).addInt(cpos).addInt(ppos);
                } else {
                    rst->setVar('3',1);
                }
            }
        } else {
            // if(sp>ep) qDebug("#### str valid error (%d, %d, %d, %d len:%d %s) ####",sp, ep, cpos, ppos, sv->length(), var?"ok":"null");
            rst->setVar('3', sp>=ep ? 0 : 1);
        }
    } break;
    case 8: {	// pos
        if( cnt==0 ) {
            if( bchk && cpos!=-1 ) {
                rst->setVar('0',0).addInt(min(cpos,ep));
            } else if(var) {
                var->putInt(min(cpos,ep),OBJ_PROP_START_POS);
                rst->reuse()->add(var);
            }
            // rst->setVar('0',0).addInt(sp);
            return true;
        }
        if( isNumberVar(arrs->get(0)) ) {
            sp=  toInteger(arrs->get(0));
            if( cnt==2 ) {
                ep=isNumberVar(arrs->get(1))? toInteger(arrs->get(1)): 0;
            }
            if( ep==0 || ep>send )
                ep=send;
        } else {
            LPCC ty=AS(0);
            if( ccmp(ty,"end") ) {
                sp=ep;
            }
        }
        if( sp>ep ) {
            ep =sp;
        }
        if( bchk && rst!=sv )
            rst->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(sp);
        else if( var )
            var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(sp);
    } break;
    case 9: {	// ch
        if( cpos==-1 ) {
            rst->setVar('3',0);
            return true;
        }
        XParseVar pv(sv,sp,ep);
        bool move=true, paramCheck=false;
        if( cnt ) {
            StrVar* chk=arrs->get(0);
            if( checkNum(chk) ) {
                // a='a.b.c.d      '; a.ch(-1, true);  ===> : "a,b,c,d"
                int pos = toInteger(chk);
                chk=arrs->get(1);
                rst->reuse();
                if( pos<0 ) {
                    if( checkFuncObject(chk,'3',1) ) {
                        ep=pv.wep+pos;
                        rst->reuse()->add( pv.prevChar(ep, &pos) );
                        if( var && sv!=var ) {
                            if( pos<=0 ) {
                                var->setVar('s',0,(LPVOID)sv).addInt(0).addInt(0).addInt(-1).addInt(ep);
                            } else {
                                var->setVar('s',0,(LPVOID)sv).addInt(pv.wsp).addInt(pos+1).addInt(sp).addInt(ep);
                            }
                        }
                    } else {
                        ep=pv.wep+pos;
                        if( ep>0 ) rst->add(pv.CharAt(ep) );
                    }
                } else {
                    char ch='\0';
                    pos+=sp;
                    if(chk==NULL ) {
                        if( sp<=pos && pos<ep) {
                            ch=sv->charAt(pos);
                        }
                        rst->add(ch);
                    } else {
                        bool ok=true;
                        if( sp<=pos && pos<ep) {
                            LPCC str=sv->get(pos);
                            for(int n=1;n<cnt;n++) {
                                LPCC val=AS(n);
                                if(!StartWith(str,val)) {
                                    ok=false;
                                    break;
                                }

                            }
                        }
                        rst->setVar('3', ok ? 1: 0 );
                    }
                }
                return true;
            }
            if( checkFuncObject(chk,'3',0) ) {
                move=false;
            } else {
                paramCheck=true;
            }
        }
        char ch=pv.ch();
        int start=pv.start;
        rst->reuse();
        if( ch ) {
            if( paramCheck ) {
                move=false;
                for( int n=0; n<cnt; n++ ) {
                    LPCC val=AS(n);
                    if( val && ch==val[0] ) {
                        move=true;
                        rst->add(ch);
                        break;
                    }
                }
            } else {
                rst->add(ch);
            }
        }
        if( move ) {
            if( orgin ) var=orgin;
            if( sp<start ) {
                if( var && sv!=var ) {
                    var->putInt(start,OBJ_PROP_START_POS);
                }
            }
            if( start > cpos ) {
                // if( orgin ) qDebug("#### str ch  (%d, %d) ####",start, cpos);
                if( var && sv!=var ) {
                    var->putInt(start,OBJ_PROP_START_POS+8);
                }
            }
        }

    } break;
    case 10: {	// cur
        if( cnt==0 ) {
            rst->setVar('0',0).addInt(sp);
            return true;
        }
        if( isNumberVar(arrs->get(0)) ) {
            int pos = toInteger(arrs->get(0));
            if( ep==0 && cpos!=-1 ) {
                ep = send;
            }
            if( pos<0 ) {
                pos+=ep;
                if( pos<0 ) pos=0;
            } else {
                pos+=sp;
                if( sp>ep ) sp = ep;
            }
            rst->setVar('0',0).addInt(pos);
        }

    } break;
    case 11: {	// match
        if( cpos==-1 ) {
            rst->setVar('3',0);
            return true;
        }
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        XParseVar pv(sv,sp,ep);
        bool mchk=false;

        U32 flag=0;
        int chk=0;
        if( cnt==3 ) {
            chk = toInteger(arrs->get(2));
        } else if( cnt==1 ) {
            chk = toInteger(arrs->get(0));
        }
        if( chk ) {
            if( chk&1 ) flag|=FIND_SKIP;
            if( chk&2 ) flag|=FIND_NOCASE;
            if( chk&4 ) flag|=FIND_CHARS;
            if( chk&8 ) flag|=FIND_FUNCSTR;
            if( chk&16 ) flag|=FIND_WORD;
            if( chk&0x100 ) flag|=FIND_BACK;
        }
        if( chk&0x100 ) {
            pv.start = pv.wep-1;
        }
        LPCC st=NULL, et=NULL;
        if( cnt>1 ) {
            st=AS(0), et=AS(1);
            mchk=pv.match(st,et,flag);
        } else {
            char ch=pv.ch();
            if( ch=='\''||ch=='"' ) {
                mchk = pv.MatchStringPos(ch);
            } else if( ch=='/' ) {
                ch=pv.ch(1);
                if( ch=='*' ) {
                    mchk = pv.match("/*","*/");
                } else if( ch=='#' ) {
                    mchk = pv.match("/#","#/", flag);
                } else if( ch=='@' ) {
                    mchk = pv.match("/@","@/", flag);
                } else if( ch=='?' ) {
                    mchk = pv.match("/?","?/", flag);
                }
            } else if( ch=='<' ) {
                ch=pv.ch(1);
                if( ch=='[' ) {
                    mchk = pv.match("<[","]>", flag);
                } else if( ch=='!' && pv.ch(2)=='-' ) {
                    mchk = pv.match("<!--","-->", flag);
                } else if( ch=='#' ) {
                    mchk = pv.match("<#","#>", flag);
                } else if( ch=='@' ) {
                    mchk = pv.match("<@","@>", flag);
                } else if( ch=='?' ) {
                    mchk = pv.match("<?","?>", flag);
                } else {
                    mchk = pv.match("<",">", flag);
                }
            } else {
                char ec = mapStartChar(ch);
                if( ec ) {
                    mchk = pv.match(gBuf.fmt("%c",ch),gBuf.fmt("%c",ec), flag);
                }
            }
        }
        // version 1.0.2 if( match() )
        // XFunc* pfc=fc->parent();
        if( chk&0x100 ) {
            pv.start = pv.wep-1;
            if( mchk ) {
                bool ok=true;
                if( bchk ) {
                    XFunc* pfc=fc->parent();
                    if( pfc && pfc->getType()==FTYPE_ARRAY ) {
                        pfc = pfc->parent();
                    }
                    if( pfc && pfc->getType()==FTYPE_CONTROL ) {
                        rst->setVar('3', 1 );
                        ok=false;
                    }
                }
                if( ok ) {
                    rst->setVar('s',0,(LPVOID)sv).addInt(pv.prev).addInt(pv.cur).addInt(pv.start).addInt(pv.wep);
                }
                if( orgin ) {
                    orgin->putInt(pv.start,OBJ_PROP_START_POS);
                } else if( var && sv!=var ) {
                    var->setVar('s',0,(LPVOID)sv).addInt(pv.wsp).addInt(pv.prev-slen(st)).addInt(sp).addInt(ep);
                }
            } else {
                rst->setVar('3',0);
                if( var && sv!=var ) {
                    var->putInt(pv.start-1,OBJ_PROP_START_POS);
                }
            }
        } else {
            if( mchk ) {
                bool ok=true;
                if( bchk ) {
                    XFunc* pfc=fc->parent();
                    if( pfc && pfc->getType()==FTYPE_ARRAY ) {
                        pfc = pfc->parent();
                    }
                    if( pfc && pfc->getType()==FTYPE_CONTROL ) {
                        rst->setVar('3', 1 );
                        ok=false;
                    }
                }
                if( ok ) {
                    rst->setVar('s',0,(LPVOID)sv).addInt(pv.prev).addInt(pv.cur).addInt(pv.start).addInt(pv.wep);
                }
                if( orgin ) {
                    orgin->putInt(pv.start,OBJ_PROP_START_POS);
                } else if( var && sv!=var ) {
                    var->putInt(pv.start,OBJ_PROP_START_POS);
                }
            } else {
                // rst->setVar('s',0,(LPVOID)sv).addInt(pv.wep).addInt(pv.wep).addInt(-1).addInt(pv.wep);
                rst->setVar('3',0);
                if( orgin ) {
                    orgin->putInt(pv.start+1,OBJ_PROP_START_POS);
                } else if( var && sv!=var ) {
                    var->putInt(pv.start+1,OBJ_PROP_START_POS);
                }
            }
        }
    } break;
    case 12: {	// left
        if( arrs ) {
            LPCC str=AS(0);
            XParseVar pv(sv,sp,ep );
            int pos = isVarTrue(arrs->get(1))? pv.findPrev(str,ep, sp, FIND_BACK) : pv.findPos(str,ep, FIND_FORWORD);
            // int pos = pv.findPos(str,ep, FIND_FORWORD);
            ppos = ep;
            if( pos==-1 ) {
                if( rst!=sv ) {
                    rst->reuse();
                    if( sp<ep ) {
                        rst->add(sv->get(sp), ep-sp);
                    }
                } else if(var) {
                    var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(pos).addInt(ep);
                }
            } else {
                if( pos<sp ) pos=sp;
                else if( pos>ep ) pos=ep;
                if( rst!=sv ) {
                    rst->set(sv->get(sp), pos-sp);
                } else if(var) {
                    if( var ) {
                        var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(pos).addInt(pos).addInt(ep);
                    } else {
                        LPCC val=gBuf.add( sv->get(sp), pos-sp );
                        rst->set(val);
                    }
                }
            }
        } else {
            if( cpos==-1 ) {
                ep = send;
                rst->setVar('s',0,(LPVOID)sv).addInt(0).addInt(0).addInt(sp).addInt(ep);
            } else {
                rst->setVar('s',0,(LPVOID)sv).addInt(cpos).addInt(ppos).addInt(sp).addInt(ep);
            }
        }
    } break;
    case 805:	// toString
    case 811: {	// value
        if( cpos==-1 ) {    // || (ep>0 && sp>=ep)
            rst->reuse();
            return true;
        }
        if( ep==0 && cpos!=-1 ) {
            ep = send;
        }
        if( cnt==1 ) {
            sp=toInteger(arrs->get(0));
        } else if( cnt==2) {
            int e = toInteger(arrs->get(1));
            sp=toInteger(arrs->get(0));
            if( e<0 ) {
                ep+=e;
            } else if( e<ep ) {
                ep=e;
            }
        } else if( cnt==3) {
            sp = toInteger(arrs->get(0)), ep = toInteger(arrs->get(1));
        }
        if( ep>sv->length() ) ep=sv->length();
        if( sp>=0 && sp<ep ) {
            if( rst==sv ) {
                LPCC val=gBuf.add(sv->get(sp), ep-sp);
                rst->set(val);
            } else {
                rst->set(sv->get(sp), ep-sp);
            }
        } else {
            rst->reuse();
        }
    } break;
    case 13: {	// trim
        if( cpos==-1 ) {
            rst->reuse();
            return true;
        }
        if( ep==0 ) {
            ep = send;
        }
        if( cnt==1 ) {
            if( isNumberVar(arrs->get(0)) ) {
                sp= toInteger(arrs->get(0));
            } else {
                LPCC ty=AS(0);
                XParseVar pv(sv,sp,ep);
                rst->reuse();
                if( ccmp(ty,"right") ) {
                    pv.prevChar(pv.wep-1, &ep);
                    if( ep<pv.wep ) ep++;
                    if( sp<ep ) {
                        rst->set(sv->get(sp), ep-sp);
                    }
                } else if( ccmp(ty,"left") ) {
                    pv.ch();
                    sp=pv.start;
                    if( sp<ep ) {
                        rst->set(sv->get(sp), ep-sp);
                    }
                } else {
                    sp=pv.wsp, ep=pv.wep;
                    sv->trim(sp, ep);
                    if( sp<ep ) {
                        rst->set(sv->get(sp), ep-sp);
                    }
                }
                return true;
            }
        } else if( cnt==2) {
            int e = toInteger(arrs->get(1));
            sp=toInteger(arrs->get(0));
            if( e<0 ) {
                ep+=e;
                if( ep<0 ) ep=0;
            } else if( e<ep ) {
                ep=e;
            }
        } else if( cnt==3) {
            sp = toInteger(arrs->get(0)), ep = toInteger(arrs->get(1));
        }
        if( ep>sv->length() ) ep=sv->length();
        if( sp>=0 && sp<ep ) {            
            LPCC val=sv->trimv(sp,ep);
            if( sv==rst && val==rst->get() ) {
                qDebug("trim sv==rst");
            } else {
                rst->set(val);
            }
        }
    } break;
    case 14: {	// size
        if( cpos==-1 || (ep>0 && sp>=ep) ) {
            rst->setVar('0',0).addInt(0);
            return true;
        }
        if( sp==0 && ep==0 ) {
            ep=send;
        }
        rst->setVar('0',0).addInt(sp<ep ? ep-sp:0);
    } break;
    case 15: {	// length
        if( cpos==-1 ) {
            rst->setVar('3',0);
            return true;
        }
        LPCC txt=NULL;
        if( sp==0 && ep==0 ) {
            ep=send;
            txt = sv->get();
        }
        if( txt==NULL ) txt = sv->get(sp,ep);
        if( slen(txt) ) {
            QString str(KR(txt));
            rst->setVar('0',0).addInt(str.length());
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 16: {	// lower
        if( sp==0 && ep==0 && cpos!=-1 ) {
            ep=send;
        }
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        int n=0;
        int num = arrs? toInteger(arrs->get(0)): 0;
        rst->reuse();
        for( ;sp<ep;sp++, n++) {
            if( num==0 || (n<num)) {
                rst->add( (char)tolower(sv->charAt(sp)) );
            } else {
                rst->add(sv->charAt(sp) );
            }
        }
    } break;
    case 17: {	// upper
        if( sp==0 && ep==0 && cpos!=-1 ) {
            ep=send;
        }
        int n=0;
        int num = arrs? toInteger(arrs->get(0)): 0;
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        rst->reuse();
        for( ;sp<ep;sp++,n++) {
            if( num==0 || (n<num)) {
                rst->add( (char)toupper(sv->charAt(sp)) );
            } else {
                rst->add(sv->charAt(sp) );
            }
        }
    } break;
    case 18: {	// escape
        if( sp==0 && ep==0 && cpos!=-1 ) {
            ep=send;
        }
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        char ch='\"';
        char c=0;

        rst->reuse();
        if(cnt==0) {
            for( ;sp<ep;sp++) {
                c=sv->charAt(sp);
                if(c=='\\') {
                    c=sv->charAt(++sp);
                    if(c=='n') {
                        rst->add('\n');
                    } else if(c=='r') {
                        rst->add('\r');
                    } else if(c=='t') {
                        rst->add('\t');
                    } else if(c=='\\') {
                        rst->add('\\');
                    } else if(c==ch) {
                        rst->add(ch);
                    } else {
                        rst->add(c);
                    }
                } else {
                    rst->add(c);
                }
            }
        }
    } break;
    case 73:    // equals
    case 806: 	// eq
    case 807: {	// ne
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        if( sp==0 && ep==0 && cpos!=-1 ) {
            ep=send;
        }        
        int sz=ep-sp;
        if( cnt ) {
            bool bchk=false;
            StrVar* check = arrs->get(0);
            if( cnt==1 && checkFuncObject(check,'a',0) ) {
                arrs=(XListArr*)check->getObject(FUNC_HEADER_POS);
                cnt = arrs->size();
            }
            if( fc->xfuncRef==73 ) {
                StrVar* tmp=NULL;
                for( int n=0;n<cnt;n++) {
                    tmp=arrs->get(n);
                    if( sz==0 ) {
                        if( checkFuncObject(tmp,'9',0) || slen(toString(tmp)) )  {
                            bchk = true; break;
                        }
                    } else {
                        LPCC str=toString(tmp);
                        if( ccmpl(str,slen(str),sv->get(sp),sz) ) {
                            bchk=true; break;
                        }
                    }
                }
            } else {
                for( int n=0;n<cnt;n++) {
                    LPCC str=AS(n);
                    if( sz==0 ) {
                        if( slen(str)==0 ) {
                            bchk = true; break;
                        }
                    } else if( cncmp(str,slen(str),sv->get(sp),sz) ) {
                        bchk=true; break;
                    }
                }
            }
            if( fc->xfuncRef==807 )
                rst->setVar('3', bchk ? 0: 1);
            else
                rst->setVar('3', bchk ? 1: 0);

        } else {
            if( sz==1 ) {
                char ch = sv->charAt(sp);
                if( fc->xfuncRef==807 )
                    rst->setVar('3', ch=='\'' || ch=='"'? 0:1 );
                else
                    rst->setVar('3', ch=='\'' || ch=='"'? 1:0 );
            } else {
                rst->setVar('3',0);
            }
        }
    } break;
	case 820: 	// lt
	case 821: {	// le
		if( rst==sv ) {
			sv=getStrVarTemp();
			sv->add(rst);
		}
		if( sp==0 && ep==0 && cpos!=-1 ) {
			ep=send;
		}
		int sz=ep-sp, len=0;
		bool chk=false;
		LPCC str=gBuf.add(sv->get(sp),sz);
		if(cnt>0 && isnum(str,&len,&chk)) {
			if(chk) {
				double a = (double)atof(str);
				double b = toDouble(arrs->get(0));
				if(fc->xfuncRef==820) {
					rst->setVar('3', a<b? 1:0);
				} else {
					rst->setVar('3', a<=b? 1:0);
				}
			} else {
				UL64 a=(UL64)atol(str);
				UL64 b=toUL64(arrs->get(0));
				if(fc->xfuncRef==820) {
					rst->setVar('3', a<b? 1:0);
				} else {
					rst->setVar('3', a<=b? 1:0);
				}
			}
		} else {
			rst->setVar('3',0);
		}
	} break;
	case 822:	// gt
	case 823: {	// ge
		if( rst==sv ) {
			sv=getStrVarTemp();
			sv->add(rst);
		}
		if( sp==0 && ep==0 && cpos!=-1 ) {
			ep=send;
		}
		int sz=ep-sp, len=0;
		bool chk=false;
		LPCC str=gBuf.add(sv->get(sp),sz);
		if(cnt>0 && isnum(str,&len,&chk)) {
			if(chk) {
				double a = (double)atof(str);
				double b = toDouble(arrs->get(0));
				if(fc->xfuncRef==822) {
					rst->setVar('3', a>b? 1:0);
				} else {
					rst->setVar('3', a>=b? 1:0);
				}
			} else {
				UL64 a=(UL64)atol(str);
				UL64 b=toUL64(arrs->get(0));
				if(fc->xfuncRef==822) {
					rst->setVar('3', a>b? 1:0);
				} else {
					rst->setVar('3', a>=b? 1:0);
				}
			}
		} else {
			rst->setVar('3',0);
		}
	} break;
	case 824: {	// between
		if( rst==sv ) {
			sv=getStrVarTemp();
			sv->add(rst);
		}
		if( sp==0 && ep==0 && cpos!=-1 ) {
			ep=send;
		}
		int sz=ep-sp, len=0;
		bool chk=false;
		LPCC str=gBuf.add(sv->get(sp),sz);
		if(cnt>0 && isnum(str,&len,&chk)) {
			if(chk) {
				double a = (double)atof(str);
				double sp = toDouble(arrs->get(0));
				double ep = toDouble(arrs->get(1));
				rst->setVar('3', sp<=a && a<ep? 1:0);
			} else {
				UL64 a=(UL64)atol(str);
				UL64 sp=toUL64(arrs->get(0));
				UL64 ep=toUL64(arrs->get(1));
				rst->setVar('3', sp<=a && a<ep? 1:0);
			}
		} else {
			rst->setVar('3',0);
		}
	} break;
    case 30: {	// findNum
        rst->reuse();
        if( cpos==-1 ) return true;
        if( ep==0 ) {
            ep=send;
        }
        XParseVar pv(sv, sp,ep );
        bool sign=false;
        char ch=pv.ch();
        int start=pv.start, end=0;
        if( ch=='+' || ch=='-' ) {
            sign=true;
            pv.start++;
        }
        end=pv.start;
        while( pv.valid() ) {
            ch=pv.CharAt(end);
            if( IS_DIGIT(ch) || ch=='.' ) {
                end++;
                continue;
            }
            break;
        }
        int dist=end-start;
        if( dist > 0 ) {
            if( sign && dist==1 ) {
                return true;
            }
            rst->setVar('s',0,(LPVOID)sv).addInt(start).addInt(end).addInt(end).addInt(ep);
            if( var && sv!=var ) {
                var->setVar('s',0,(LPVOID)sv).addInt(end).addInt(ep).addInt(start).addInt(ep);
            }
        }
    } break;
    case 31: {	// start
        if( sp==0 && ep==0 && cpos!=-1 ) {
            ep=send;
        }
        if( cnt && sp<ep ) {
            bool bmove=false, bfind=false;
            LPCC str=AS(0), ty=NULL;
            StrVar* check=arrs->get(1);
            if( checkFuncObject(check,'3',1) ) {
                bmove=true;
            } else if(check) {
                ty=toString(check);
            }
            int len=slen(str), sz=ep-sp;

            if( ccmp(ty,"lower") ) {
                if( len<=sz && ccmpl(str,len,sv->get(sp),len) ) {
                    bfind=true;
                }
            } else {
                if( len<=sz && StartWith(sv->get(sp),str,slen(str),sz) ) {
                    bfind=true;
                }
            }
            if( bfind ) {
                if( bmove ) {
                    if( var && sv!=var ) {
                        sp+=len;
                        var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(ep);
                    }
                }
                rst->setVar('3',1);
            } else {
                rst->setVar('3',0);
            }
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 32: {	// count
        char c=' ';
        int num=0;
        if( cnt ) {
            LPCC ty=AS(0);
            int len=slen(ty);
            if( len ) {
                if( len>1 ) {
                    if( ccmp(ty,"blank") ) c=1;
                    else if( ccmp(ty,"number") ) c=2;
                    else if( ccmp(ty,"word") ) c=3;
                    else c=1;
                } else {
                    c=ty[0];
                }
            }
        }
        for( int n=sp;n<ep;n++ ) {
            char ch= sv->charAt(n,ep);
            if( ISBLANK(ch) ) {
                if( c==1 || c==ch )
                    num++;
                else if( num )
                    break;
            } else {
                if( c==1 || IS_OPERATION(ch) )
                    break;
                if( c==2 ) {
                    if( IS_DIGIT(ch) ) num++;
                    else break;
                } else if( c==ch || c==3 ) {
                    num++;
                } else
                    break;
            }
        }
        rst->setVar('0',0).addInt(num);
    } break;
    case 33: {	// finds
        if( cpos==-1 ) {
            rst->setVar('3',0);
            return true;
        }
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        if( sp==0 && ep==0 ) {
            ep=send;
        }
        XParseVar pv(sv,sp,ep );
        if( cnt ) {
            bool ok = false;
            // int pos=0;
            for( int n=0;n<cnt;n++ ) {
                LPCC str=AS(n);
                int pos = pv.find(str); // , len=slen(str);
                if( pos!=-1 ) {
                    ok=true;
                    break;
                    /*
                    if( (sp+pos+len) == pv.wep ) {
                        ok = true;
                        break;
                    }
                    */
                }
            }
            rst->setVar('3', ok ? 1 : 0);
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 40:
    case 41: {	// append
        if( cnt ) {
            if( rst!=sv ) {	// && ep<sv->length()
                StrVar* data=NULL;
                for( int n=0;n<cnt;n++ ) {
                    data=arrs->get(n);
                    if( checkFuncObject(data,'s',0) ) {
                        int sp=0,ep=0;
                        data=getStrVarPointer(data,&sp,&ep);
                        if( sp<ep ) {
                            sv->add(data->get(sp), ep-sp);
                        }
                    } else {
                        toString(data, sv);
                    }
                }
                if( fc->xfuncRef==41 ) {
                    XFunc* pfc=fc->parent();
                    if( pfc && pfc->xftype==FTYPE_RETURN ) {
                        rst->reuse()->add(sv);
                    }
                }
            }
        }

    } break;
    case 42: {	// split
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        XListArr* a= NULL;
        if( cnt==2 ) {
            StrVar* aval=arrs->get(1);
            if( checkFuncObject(aval, 'a', 0) ) {
                a=(XListArr*)aval->getObject(FUNC_HEADER_POS);
            }
        }
        if( a==NULL ) {
            a=getListArrTemp();
        }

        XParseVar pv(sv, sp, ep);
        LPCC sep= arrs? AS(0): ",";
        while( pv.valid() ) {
            pv.findEnd(sep);
            a->add()->set(pv.v());
        }
        rst->setVar('a',0,(LPVOID)a);
    } break;
    case 43: {	// word
        if( cpos==-1 ) return true;
        XParseVar pv(sv,sp,ep);
        pv.ch();
        rst->add(pv.NextWord());
    } break;
    case 44: {	// in

    } break;
    case 45: 	// move
    case 46: {	// next
        if( cpos==-1 ) return true;
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        if( cnt==0 ) {
            XParseVar pv(sv,sp,ep);
            pv.ch();
            sp=pv.start;
            pv.moveNext();
            ep=pv.start;
            // version 1.0.2 => move()
            if( bchk && rst!=sv ) {
                if( fc->xfuncRef==45 ) {
                    rst->set(sv, sp, ep); // setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(ep).addInt(pv.wep);
                } else {
                    rst->setVar('s',0,(LPVOID)sv).addInt(ep).addInt(pv.wep).addInt(ep).addInt(ppos);
                }
            }
            if( orgin ) {
                orgin->putInt(ep,OBJ_PROP_START_POS);
            } else if( var && sv!=var ) {
                var->setVar('s',0,(LPVOID)sv).addInt(ep).addInt(pv.wep).addInt(ep).addInt(ppos);
            }
        } else {
            int pos=toInteger(arrs->get(0));
            if( isNumberVar(arrs->get(0)) ) {
                if( orgin ) {
                    orgin->putInt(pos,OBJ_PROP_START_POS);
                } else if( var && var!=sv  ) {
                    var->putInt(pos,OBJ_PROP_START_POS);
                }
            }
            if( bchk && rst!=sv )
                rst->setVar('s',0,(LPVOID)sv).addInt(pos).addInt(ep).addInt(sp).addInt(ep);
        }
    } break;
    case 47: {	// end
        if( ep==0 && cpos!=-1 )
            ep=send;
        rst->setVar('0',0).addInt(ep);
    } break;
    case 50: {	// kr
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        if( ep==0 && cpos!=-1 )
            ep=send;
        if( sp<ep ) UTF2C(sv->get(sp),ep-sp,rst,1);
    } break;
    case 51: {	// utf8
        if( ep==0 && cpos!=-1 )
            ep=send;
        QByteArray ba;
        QTextCodec *codec = QTextCodec::codecForName("EUC-KR");
        ba.setRawData(sv->get(sp), ep-sp);
        // qDebug("xxx dcode utf 8 xxx %d %d\n", sp, ep);
        QString str = codec->toUnicode(ba);
        rst->add(Q2A(str));
    } break;
    case 52: {	// str
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        if( ep==0 && cpos!=-1 )
            ep=send;
        if( rst ) {
            XFunc* pfc=fc->parent();
            if( pfc && pfc->xftype==FTYPE_ARRAY ) {
                rst->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(sp);
            } else {
                bool chk=rst==sv ? false: true;
                if( chk) {
                    StrVar* cv=getStrVarTemp();
                    cv->add(sv);
                    if( var==NULL && bchk==false ) {
                        sv->setVar('s',0,(LPVOID)cv).addInt(sp).addInt(ep).addInt(sp).addInt(sp);
                    }
                }
                if( bchk && rst!=sv )
                    rst->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(sp);
            }
        }
    } break;
    case 53: {	// again
        if( cnt==0) return false;
        LPCC a=AS(0);
        int num=1;
        if( isNumberVar(arrs->get(1)) )
            num=toInteger(arrs->get(1));
        for( int n=0;n<num;n++ ) sv->add(a);
        if( var && var!=sv ) var->setVar('s',0,(LPVOID)sv).addInt(0).addInt(send).addInt(0).addInt(0);
    } break;
    case 54: {	// check
        if( cnt==0) return false;
        LPCC ty=AS(0);
        rst->reuse();
        if( ep==0 && cpos!=-1 )
            ep=send;
        if( ccmp(ty,"upperCase") ) {
            for( ;sp<ep;sp++ ) {
                if( !isUpperCase(sv->charAt(sp)) ) return true;
            }
            rst->setVar('3',1);
        } else if( ccmp(ty,"lowerCase") ) {
            for( ;sp<ep;sp++ ) {
                if( isUpperCase(sv->charAt(sp)) ) return true;
            }
            rst->setVar('3',1);
        } else {
            for( ;sp<ep;sp++ ) {
                bool chk=false;
                for( int n=0, len=slen(ty); n<len; n++ ) {
                    if( sv->charAt(sp)==ty[n] ) chk=true;
                }
                if( chk==false ) return true;
            }
            rst->setVar('3',1);
        }
    } break;
    case 55: {	// encode
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        rst->reuse();
        if( ep==0 ) ep=send;
        if( sp>=ep ) return true;
        if( cnt==0 ) {
            rst->add(sv);
            ZipVar z(rst);
            z.Encode();
        } else {
            LPCC ty = AS(0);
            if( ccmp(ty,"base64") ) {
                unsigned char *buf=NULL;
                int buflen=0;
                if( CLZ77Lib::BASE64_Encode((PU8)sv->get(sp), ep-sp, &buf, &buflen) ) {
                    rst->set((LPCC)buf, buflen);
                    free(buf);
                }
            } else if( ccmp(ty,"url") ) {
                LPCC txt=sv->get(sp, ep);
                if( slen(txt) ) {
                    getUrlEncode(txt, rst);
                }
            } else if( ccmp(ty,"hex") ) {
                int mode=1;
                if( isNumberVar(arrs->get(1)) ) {
                    mode=toInteger(arrs->get(1));
                }
                if( mode==0 ) {
                    LPCC text=sv->get(sp, ep);
                    if( isnum(text) ) {
                        long nLong = strtol( text, NULL, 10 );
                        rst->format(16,"%08x", nLong );
                    }
                } else if( mode==1 ) {
                    for( int n=sp; n<ep; n++ ) {
                        rst->format(8,"%02x", (int)sv->charAt(n) );
                    }
                }
            } else if( ccmp(ty,"entry") ) {
                LPCC txt=sv->get(sp, ep);
                LPCC st="\\", et="";
                int mod=10, base=0, shift=1;
                if( isNumberVar(arrs->get(1)) ) {
                    mod=toInteger(arrs->get(1));
                }
                if( isNumberVar(arrs->get(2)) ) {
                    base=toInteger(arrs->get(2));
                }
                if( arrs->get(3) ) st=AS(3); else if( arrs->get(4) ) et=AS(4);
                encodeEntities(txt, st, et, mod, shift, base, rst);
            } else if( ccmp(ty,"keyCode") ) {

            } else if( ccmp(ty,"unicode") ) {
                LPCC txt=sv->get(sp, ep);
                LPCC st="\\", et="";
                int mod=3, shift=1, base=0;
                encodeEntities(txt, st, et, mod, shift, base, rst);
            } else if( ccmp(ty,"hash") ) {
                QByteArray ba = GetHash( QByteArray(sv->get(sp),ep-sp) );
                rst->set(ba.toHex().toUpper().constData());
            }
        }
    } break;
    case 56: {	// decode
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        rst->reuse();
        if( ep==0 ) ep=send;
        if( sp>=ep ) return true;
        if( cnt==0 ) {
            rst->add(sv->get(sp), ep-sp);
            ZipVar z(rst);
            z.Decode();
        } else {
            LPCC ty = AS(0);
            if( ccmp(ty,"base64") ) {
                unsigned char *buf=NULL;
                int buflen=0;
                if( CLZ77Lib::BASE64_Decode((PU8)sv->get(sp), ep-sp, &buf, &buflen) ) {
                    rst->set((LPCC)buf, buflen);
                    free(buf);
                }
            } else if( ccmp(ty,"utf8") ) {
                LPCC txt=sv->get(sp, ep);
                if( slen(txt) ) {
                    QString str=QString::fromUtf8(txt);
                    rst->set(Q2A(str));
                }
            } else if( ccmp(ty,"hex") ) {
                int mode=1;
                if( isNumberVar(arrs->get(1)) ) {
                    mode=toInteger(arrs->get(1));
                }
                if( mode==0 ) {
                    UL64 num = (UL64)strtol( sv->get(sp,ep), NULL, 16 );
                    rst->setVar('1',0).addUL64(num);
                } else if( mode==1 ) {
                    char tmp[4];
                    LPCC buf=sv->get(sp);
                    int num=0, sz=ep-sp;
                    for( int n=0; n<sz; n+=2 ) {
                       memcpy(tmp, buf+n, 2);
                       tmp[2]=0;
                       sscanf(tmp,"%02x",&num);
                       rst->add((char)num );
                    }
                }
            } else if( ccmp(ty,"hashIndex") ) {
                LPCC buf=sv->get(sp);
                int sz=ep-sp;
                int tot=0, mod=toInteger(arrs->get(1));
                for( int n=0; n<sz; n++ ) {
                    tot+=(int)buf[n];
                }
                if(mod<=0) mod=32;
                rst->setVar('0',0).addInt(tot%mod);
            } else if( ccmp(ty,"url") ) {
                LPCC txt=sv->get(sp, ep);
                if( slen(txt) ) getUrlDecode(txt, rst );
            } else if( ccmp(ty,"entry") ) {
                LPCC txt=sv->get(sp, ep);
                if( slen(txt) ) {
                    LPCC re="\\\\(\\d+)";
                    int mod=3, base=0;
                    if( isNumberVar(arrs->get(1)) ) {
                        mod=toInteger(arrs->get(1));
                    }
                    if( isNumberVar(arrs->get(2)) ) {
                        base=toInteger(arrs->get(2));
                    }
                    if( arrs->get(3) ) re=AS(3);
                    decodeEntities( KR(txt), KR(re), mod, base, rst);
                }
            } else if( ccmp(ty,"charCode") ) {
                if( isNumberVar(arrs->get(1)) ) {
                    codePointToUTF8((long)toUL64(arrs->get(1)),rst);
                }
            } else if( ccmp(ty,"charCodeAdd") ) {
                if( isNumberVar(arrs->get(1)) ) {
                    LPCC val=NULL, sep=AS(2);
                    codePointToUTF8((long)toUL64(arrs->get(1)),rst);
                    val=gBuf.add(rst->get());
                    rst->reuse();
                    if(slen(val)) {
                        if(sep) {
                            XParseVar pv(sv,sp,ep);
                            while(pv.valid()) {
                                pv.findEnd(sep);
                                rst->add(pv.GetValue(pv.prev), pv.cur-pv.prev);
                                if( pv.valid() )
                                    rst->add(val);
                                else break;
                            }
                        } else {
                            QString s=KR(toString(sv));
                            for(int n=0; n<s.length(); n++) {
                                if(n>0) rst->add(val);
                                rst->add(Q2A(QString(s.at(n))));
                            }
                        }
                    }
                }
            } else if( ccmp(ty,"unicode") ) {
                decodeUnicode(sv, sp, ep, rst );
            } else if( ccmp(ty,"h2i") ) {
                LPCC txt=sv->get(sp, ep);
                if( slen(txt) ) {
                    bool ok=false;
                    QString str=KR(txt);
                    rst->setVar('0',0).addInt( str.toInt(&ok, 16) );
                }
            }
        }
    } break;
    case 57: {	// lpad
        if( cnt==0 )
            return false;
        int num=0,mx=4,ch='0';
        StrVar* var = sv;
        if( isNumberVar(sv) ) {
            num=toInteger(sv);
            sv=arrs->get(0);
            if( isNumberVar(sv) ) {
                mx=toInteger(sv);
            }
            LPCC c=AS(1);
            if( slen(c) ) ch=c[0];
            lpad(num, mx, rst, ch);
        } else {
            LPCC txt=toString(sv);
            sv=arrs->get(0);
            if( isNumberVar(sv) ) {
                mx=toInteger(sv);
            }
            LPCC c=AS(1);
            if( slen(c) ) ch=c[0];
            lpadString(txt, mx, rst, ch);
        }
        if( FUNCVAR_CHECK && var!=rst ) var->reuse()->add(rst);
    } break;
    case 58: {	// replace
        if( cnt==0 ) return true;
        rst->reuse();
        if( cpos==-1 ) return true;
        bool change=false;
        LPCC find=AS(0), rep=AS(1);
        U32 flag=0;
        int chk=toInteger(arrs->get(2));
        if( chk ) {
            if( chk&1 ) flag|=FIND_SKIP;
            if( chk&2 ) flag|=FIND_NOCASE;
            if( chk&4 ) flag|=FIND_CHARS;
            if( chk&8 ) flag|=FIND_FUNCSTR;
            if( chk&16 ) flag|=FIND_WORD;
            if( chk&0x100 ) flag|=FIND_BACK;
        }
        if( isVarTrue(arrs->get(3)) ) {
            change=true;
        }
        int findLen=slen(find), repLen=slen(rep);
        int dist=findLen-repLen, total=0;
        if( findLen ) {
            if( change ) {
                int p=sp;
                for( int n=0; n<2048; n++ ) {
                   p=sv->find(find,p,ep,flag);
                   if( p==-1 ) {
                        break;
                   }
                   if( dist>=0 ) {
                       for( int x=0;x<repLen; x++ ) {
                           sv->setAt(p+x,rep[x],ep);
                       }
                       if( dist>0 ) {
                           sv->remove(p+repLen, dist);
                           total-=dist;
                       }
                   } else {
                       for( int x=0;x<findLen; x++ ) {
                           sv->setAt(p+x,rep[x],ep);
                       }
                       sv->insert(p+findLen, rep+findLen, -1*dist);
                       total-=dist;
                   }
                   p+=repLen;
                }
                if( total!=0 ) {
                    if( var && sv!=var ) {
                        ep+=total;
                        var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(-1).addInt(ep);
                    }
                }
            } else {
                XParseVar pv(sv,sp,ep);
                while(true ) {
                    pv.findEnd(find,flag);
                    rst->add(pv.GetValue(pv.prev), pv.cur-pv.prev);
                    if( pv.valid() )
                        rst->add(rep);
                    else break;
                }
            }
        }
    } break;
    case 61: {	// substr
        if( cpos==-1 ) return true;
        LPCC txt = NULL;
        if( ep==0 ) {
            txt = sv->get();
        }
        if( cnt>0 ) {
            if( txt==NULL ) {
                txt = sv->get(sp,ep);
            }
            QString str(KR(txt));
            sp=isNumberVar(arrs->get(0)) ? toInteger(arrs->get(0)): 0;
            if( cnt==1 ) {
                rst->set(Q2A(str.mid(sp)) );
            } else if( cnt==2 ) {
                int e=isNumberVar(arrs->get(1))? toInteger(arrs->get(1)): 0;
                if( e<0 ) {
                    ep = str.length() + e;
                } else {
                    ep = e>str.length() ? str.length(): e;
                }
                rst->set(Q2A(str.mid(sp,ep-sp)) );
            }
        } else
        if( sp>=0 && sp<ep ) {
            rst->set(sv->get(sp), ep-sp);
        } else {
            rst->reuse();
        }
    } break;
    case 62: {	// ref
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        if( ep==0 && cpos!=-1 )
            ep=send;
        if( cnt==0 ) {
            if( ( (fc->xfflag&0xF) && ((fc->xfflag&(FFLAG_PARAM|FFLAG_SUBS))==0)) && var!=rst ) {
                StrVar* cv=getStrVarTemp();
                cv->add(sv);
                sv->setVar('s',0,(LPVOID)cv).addInt(sp).addInt(ep).addInt(sp).addInt(sp);
            } else {
                rst->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(sp);
            }
        } else if( cnt==1 ) {
            int pos=toInteger(arrs->get(0));
            if( pos>sp ) sp=pos;
        } else if( cnt==2 ) {
            int e=toInteger(arrs->get(1));
            sp=toInteger(arrs->get(0));
            if( e<0 ) {
                ep+=e;
            } else {
                ep=e;
            }
        } else if( cnt==3 ) {
            sp = toInteger(arrs->get(0)), ep = toInteger(arrs->get(1));
        }
        if( ep>sv->length() ) ep=sv->length();
        if( ep<sp ) {
            rst->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(sp).addInt(-1).addInt(sp);
        } else {
            rst->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(ep);
        }
    } break;
    case 63: {	// bufferAdd
        rst->reuse();
        if( cnt==0 || cpos==-1 ) return true;
        if( ep==0 ) {
            ep=send;
        }
        StrVar* tmp=arrs->get(0);
        int sz=0;
        if( checkFuncObject(tmp,'0',0) ) {
            if( isVarTrue(arrs->get(1)) ) {
                short num=(short)tmp->getInt(4);
                sz=sizeof(num);
                memcpy(sv->memalloc(sz), &num, sz );
            } else {
                int num=tmp->getInt(4);
                sz=sizeof(num);
                memcpy(sv->memalloc(sz), &num, sz );
            }
        } else if( checkFuncObject(tmp,'4',0) ) {
            double num=tmp->getDouble(4);
            sz=sizeof(num);
            memcpy(sv->memalloc(sz), &num, sz);
        } else if( checkFuncObject(tmp,'1',0) ) {
            UL64 num=tmp->getUL64(4);
            sz=sizeof(num);
            memcpy(sv->memalloc(sz), &num, sz);
        } else {
            int a=0, b=0;
            tmp=getStrVarPointer(tmp, &a, &b);
            sz=b-a;
            memcpy(sv->memalloc(sz), tmp->get(a), sz);
        }
        if( sz>0 ) {
            sv->movepos(sz);
            if( var && sv!=var ) {
                ep+=sz;
                var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(-1).addInt(ep);
            }
        }
    } break;
    case 64: {	// bufferRead
        rst->reuse();
        if( cnt==0 || cpos==-1 ) return true;
        if( ep==0 ) {
            ep=send;
        }
        int a=toInteger(arrs->get(0)), sz=toInteger(arrs->get(1));
        if( sp<a ) {
            sp=a;
        }
        if( sz==0 ) {
            sz=4;
        }
        int end=sp+sz;
        if( end<=ep ) {
            ep=end;
        }
        if( sp<ep ) {
            if( sz<=4 ) {
                int num=0;
                if( sz==1 ) {
                    num=(int)sv->charAt(sp);
                } else if( sz==2 ) {
                    short* a=(short*)sv->get(sp,sp+2);
                    num=(int)*a;
                } else {
                    memcpy(&num, sv->get(sp), sz );
                }
                rst->setVar('0',0).addInt(num);
            } else if( isVarTrue(arrs->get(2)) ) {
                double num=0;
                memcpy(&num, sv->get(sp), sz );
                rst->setVar('4',0).addDouble(num);
            } else {
                UL64 num=0;
                memcpy(&num, sv->get(sp), sz );
                rst->setVar('1',0).addUL64(num);
            }
        } else {
            rst->setVar('0',0).addInt(0);
        }
    } break;
    case 65: {	// is
        rst->reuse();
        if( cpos==-1 ) return true;
        if( ep==0 ) ep=send;

        LPCC type = arrs ? AS(0): NULL;
        if( sp>=ep ) {
            if( ccmp(type,"empty")||ccmp(type,"blank") ) {
                rst->setVar('3',1);
            } else {
                rst->setVar('3',0);
            }
            return true;
        }
        if( ccmp(type,"blank") ) {
            char ch = sv->charAt(sp);
            rst->setVar('3', ISBLANK(ch)?1:0 );
            return true;
        }
        if( ccmp(type,"digit") ) {
            char ch=sv->charAt(sp);
            if( IS_DIGIT(ch) || ch=='.' ) {
                rst->setVar('3',1);
            } else {
                rst->setVar('3',0);
            }
            return true;
        }
        if( ccmp(type,"oper") ) {
            char ch=sv->charAt(sp);
            if( IS_OPERATION(ch) || ch=='.' ) {
                rst->setVar('3',1);
            } else {
                rst->setVar('3',0);
            }
            return true;
        }
        if( ccmp(type,"num") || ccmp(type,"number") ) {
            int n=sp;
            char ch = sv->charAt(n);
            if( ch=='+' || ch=='-' ) n++;
            for( ; n<ep; n++) {
                ch = sv->charAt(n);
                if( IS_DIGIT(ch) ) continue;
                if( ch!='.') {
                    rst->setVar('3',0);
                    return true;
                }
            }
            rst->setVar('3',1);
            return true;
        }
        if( ((ep-sp)==1) && (ccmp(type,"upper") || ccmp(type,"utf8")) ) {
            int n=0;
            int c = (unsigned char) sv->charAt(sp);
            if ((c & 0xF0) == 0xE0) n=2;
            else if ((c & 0xF8) == 0xF0) n=3;
            else if( ccmp(type,"upper") ) {
                if( c>='A' && c<='Z' ) n=4;
            }
            rst->setVar('3', n>1? 1: 0);
            return true;
        }
        if( ccmp(type,"utf8") ) {
            if( sp<ep ) {
                int x=sp, c=0, ix=ep-sp;
                int n=0,j=0;
                rst->reuse();
                for( ; x<ep; x++) {
                    c = (unsigned char)sv->charAt(x);
                    if (0x00 <= c && c <= 0x7f) n=0; // 0bbbbbbb
                    else if ((c & 0xE0) == 0xC0) n=1; // 110bbbbb
                    else if ( c==0xed && x<(ep-1) && ((unsigned char)sv->charAt(x+1) & 0xa0)==0xa0) return true; //U+d800 to U+dfff
                    else if ((c & 0xF0) == 0xE0) n=2; // 1110bbbb
                    else if ((c & 0xF8) == 0xF0) n=3; // 11110bbb
                    //else if (($c & 0xFC) == 0xF8) n=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
                    //else if (($c & 0xFE) == 0xFC) n=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
                    else return true;
                    for (j=0; j<n && x<ix; j++) { // n bytes matching 10bbbbbb follow ?
                        if ((++x == ix) || (( (unsigned char)sv->charAt(x) & 0xC0) != 0x80)) {
                            return true;
                        }
                    }
                }
                rst->setVar('3', 1);
            } else {
                rst->setVar('3', 0);
            }
            return true;
        }


        int flag = type==NULL ? 4:
            ccmp(type,"lower") ? 1:
            ccmp(type,"upper") ? 2:
            ccmp(type,"word") ? 3:
            ccmp(type,"var") ? 4:
            ccmp(type,"alpha") ? 7:
            ccmp(type,"alphanum") ? 8:
            ccmp(type,"number") ? 64:
            ccmp(type,"findOper") ? 128:
            ccmp(type,"findNumber") ? 129:
            8;

        int n=sp;
        rst->setVar('3',1);
        for( ; n<ep; n++) {
            char ch = sv->charAt(n);
            if( ch=='_' ) {
                if( flag==32 ) {
                    rst->setVar('3',0);
                    return true;
                }
                continue;
            }
            switch( flag ) {
            case 1: {
                if( ch > 'z' || ch < 'a' ) { rst->setVar('3',0); return true; }
            } break;
            case 2: {
                if( ch > 'Z' || ch < 'A' ) { rst->setVar('3',0); return true; }
            } break;
            case 3: {
                if( (ch >= 'a' && ch <= 'z' ) || (ch >= 'A' && ch <= 'Z' ) ) continue;
                rst->setVar('3',0);
                return true;
            } break;
            case 4: {
                if( n==sp && ((ch > 'z' || ch < 'a') || (ch > 'Z' || ch < 'A' )) )  {
                    rst->setVar('3',0); return true;
                }
                if( !isalnum((int)ch) ) { rst->setVar('3',0); return true; }
            } break;
            case 7: {
                if( !isalpha((int)ch) ) {
                    rst->setVar('3',0); return true;
                }
            } break;
            case 8: {
                if( !isalnum((int)ch) ) { rst->setVar('3',0); return true; }
            } break;
            case 16: {
                if( n==sp && (ch > 'Z' || ch < 'A') )  { rst->setVar('3',0); return true; }
                if( !isalnum((int)ch) ) { rst->setVar('3',0); return true; }
            } break;
            case 32: {
                if(IS_OPERATION(ch)) {
                    continue;
                } else {
                    rst->setVar('3',0); return true;
                }
            } break;
            case 128: {
                if(IS_OPERATION(ch)) {
                    rst->setVar('3',1); return true;
                }
            } break;
            case 129: {
                if( ch >= '0' && ch <= '9' ) {
                    rst->setVar('3',1); return true;
                }
            } break;
            default: {
                rst->setVar('3',0); return true;
            } break;
            }
        }
        if( flag>127 ) {
            rst->setVar('3',0);
        } else {
            rst->setVar('3',1);
        }
    } break;
    case 66: {	// charAt
        if( cpos==-1 ) return true;
        LPCC txt=NULL;
        if( ep==0 ) {
            ep = send;
            txt = sv->get();
        } else if( sp<ep ) {
            txt=sv->get(sp,ep);
        }
        if( slen(txt)==0 ) {
            rst->reuse();
            return true;
        }
        QString str(KR(txt));
        int len=str.length();
        int pos=0;
        if( arrs ) {
            sv=arrs->get(0);
            if( isNumberVar(sv) ) {
                pos=toInteger(sv);
            }
        }
        if( pos<0 ) {
            pos=len+pos;
        }

        if( pos>=0 && pos<len ) {
            QString s(str.at(pos));
            rst->set(Q2A(s));
        } else {
            rst->reuse();
        }
    } break;
    case 67: {	// prevChar
        rst->reuse();
        if( cpos==-1 ) return true;
        if( ep==0 ) ep = send;
        if( sp>=ep && var ) {
            var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(sp).addInt(-1).addInt(ep);
            return true;
        }
        bool bset=true, skip=false;
        XParseVar pv(sv,sp,ep);
        char ch = pv.prevChar(pv.wep-1);
        if( cnt==0 ) {
            rst->add(ch);
        } else {
            StrVar* checkVar=NULL;
            for( int n=0; n<cnt; n++ ) {
                checkVar=arrs->get(n);
                if( checkFuncObject(checkVar, '3',0) || checkFuncObject(checkVar, '3',1) )  {
                    bset=false;
                    break;
                } else if( skip ) {
                    continue;
                }
                LPCC val=toString(checkVar);
                if( slen(val)==1 && ch==val[0] ) {
                    rst->add(ch);
                    skip=true;
                }
            }
        }
        if( rst->length()==1 && bset ) {
            if( orgin ) {
                orgin->putInt(pv.start,OBJ_PROP_START_POS);
            } else if( var && sv!=var ) {
                int pos=pv.prev;
                if( pos<=sp ) {
                    var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(sp).addInt(-1).addInt(ep);
                } else {
                    var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(pos).addInt(sp).addInt(ep);
                }
            }
        }
    } break;
    case 68: {	// lessThan
        rst->reuse();
        if( cnt==0 || cpos==-1 ) return true;
        if( ep==0 ) ep = send;
        if( isVarTrue(arrs->get(1)) )
            rst->setVar('3', stringNameCmp(AS(0),toString(sv))>0? 1:0);
        else
            rst->setVar('3', stringNameSort(AS(0),toString(sv))>0? 1:0);
    } break;
    case 70: {	// isBlank
        if( cpos==-1 ) {
            rst->setVar('3',1);
            return true;
        }
        if( ep==0 ) ep=send;
        int n=sp;
        if( arrs && isNumberVar(arrs->get(0)) ) {
            ep = sp+toInteger(arrs->get(0));
        }
        for( ; n<ep; n++) {
            char ch = sv->charAt(n);
            if( !ISBLANK(ch) ) {
                rst->setVar('3',0);
                return true;
            }
        }
        rst->setVar('3',1);
    } break;
    case 71: {	// regRemove
        if( cnt==0 ) return true;
        LPCC text=AS(0); //ex)<[^>]*>
        LPCC val= bref ? sv->get(sp,ep): sv->get();
        QString str=QString(KR(val)).remove(QRegExp(KR(text)) );
        rst->set(Q2A(str));
    } break;
    case 72: {	// regReplace
        if( cnt==0 ) return true;
        LPCC src=AS(0), rep=AS(1); //ex)<[^>]*>
        LPCC val= bref ? sv->get(sp,ep): sv->get();
        QString str=QString(KR(val)).replace(QRegExp(KR(src)), KR(rep));
        rst->set(Q2A(str));
    } break;
    case 74: {	// toByte
        if( cnt==0 ) return true;
        int idx=arrs? toInteger(arrs->get(0)): 0;
        if( cpos==-1 ) {
            rst->setVar('0',0).addInt(0);
            return true;
        }
        if( ep==0 ) ep=send;
        if( idx<ep ) {
            char c=sv->charAt(idx);
            rst->setVar('0',0).addInt((int)c);
        } else {
            rst->setVar('0',0).addInt(0);
        }
    } break;
    case 801: {	// toNumber
        rst->setVar('0',0).addInt(toInteger(sv));
    } break;
    case 802: {	// toInt
        rst->setVar('0',0).addInt(toInteger(sv));
    } break;
    case 803: {	// toDouble
        if( ep==0 ) ep=send;
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        LPCC val= bref ? sv->get(sp,ep): sv->get();
        rst->setVar('4',0).addDouble( isnum(val)? (double)atof(val): 0 );
    } break;
    case 804: {	// toLong
        if( ep==0 ) ep=send;
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        LPCC val= bref ? sv->get(sp,ep): sv->get();
        rst->setVar('1',0).addUL64( isnum(val)? (UL64)atoll(val): 0 );
    } break;
    case 75: {	// findReg
        if( cnt==0 ) return true;
        if( ep==0 ) ep=send;
        LPCC val= bref ? sv->get(sp,ep): sv->get();
        LPCC find=AS(0);
        QString text=KR(val);
        // U32 flag=0;
        QRegExp::PatternSyntax syntax = QRegExp::RegExp;
        Qt::CaseSensitivity cs = Qt::CaseSensitive;
        int matchIndex=0;
        bool chkFindFoword=true, moveCheck=false, matchArray=false, startCheck=false;
        if( isNumberVar(arrs->get(1)) ) {
            int chk = toInteger(arrs->get(1));
            matchIndex=(int)chk&0xFF;
            if( chk&0x100 ) chkFindFoword=false;
            if( chk&0x200 ) moveCheck=true;
            if( chk&0x400 ) matchArray=true;
            if( chk&0x800 ) cs=Qt::CaseInsensitive;
            if( chk&0x1000 ) syntax = QRegExp::Wildcard;
            if( chk&0x2000 ) syntax = QRegExp::WildcardUnix;
            if( chk&0x4000 ) syntax = QRegExp::FixedString;
            if( chk&0x8000 ) syntax = QRegExp::RegExp2;
            if( chk&0x10000 ) startCheck = true;
        }
        QRegExp regexp(KR(find), cs, syntax );
        bool bctrl=isTrueCheck(fc);
        int pos= chkFindFoword ? text.indexOf(regexp): text.lastIndexOf(regexp);
        if( rst==sv ) {
            qDebug("xxxxxxxxxxxxxxxxxxxxxxx findReg rst==sv ) xxxxxxxxxxxxxxxxxxxxxxxxx");
        }
        StrVar* matchVar=NULL;
        if( moveCheck ) {
            if(pos==-1) {
                if(chkFindFoword) {
                    if( rst!=sv ) rst->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(pos).addInt(ep);
                    if( var && sv!=var ) {
                        var->setVar('s',0,(LPVOID)sv).addInt(0).addInt(0).addInt(sp).addInt(ep);
                    }
                } else {
                    if( rst!=sv ) rst->setVar('s',0,(LPVOID)sv).addInt(0).addInt(0).addInt(sp).addInt(ep);
                    if( var && sv!=var ) {
                        var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(ep);
                    }
                }
            } else {
                QString s = regexp.cap(0);
                matchVar=getStrVarTemp();
                matchVar->add(Q2A(s));
                int matchSize = matchVar->length();
                int movePos=pos+matchSize;
                if(chkFindFoword) {
                    if( rst!=sv ) rst->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(sp+pos).addInt(sp+pos).addInt(ep);
                    if( var && sv!=var ) {
                        var->setVar('s',0,(LPVOID)sv).addInt(sp+movePos).addInt(ep).addInt(sp).addInt(sp+pos);
                    }
                } else {
                    if( rst!=sv ) rst->setVar('s',0,(LPVOID)sv).addInt(sp+movePos).addInt(ep).addInt(sp).addInt(sp+pos);
                    if( var && sv!=var ) {
                        var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(sp+pos).addInt(sp+movePos).addInt(ep);
                    }
                }
            }
        }
        if( bctrl ) {
            if(startCheck) {
                rst->setVar('3', pos==0 ? 1: 0);
            } else {
                rst->setVar('3', pos==-1 ? 0: 1);
            }
        } else {
            if(moveCheck) {

            } else if(matchArray) {
                XListArr* a=getListArrTemp();
                if(pos!=-1) {
                    int cnt=regexp.captureCount();
                    for(int n=0; n<cnt; n++) {
                        a->add()->add(Q2A(regexp.cap(n)));
                    }
                }
                rst->setVar('a',0,(LPVOID)a);
            } else {
                rst->setVar('0',0).addInt(sp+pos);
            }
        }
        if(matchVar !=NULL ) {
            fn->GetVar("@matchString")->reuse()->add(matchVar);
        }
    } break;
    case 76: {
        if( cnt==0 ) return true;
        /*
        LPCC find=AS(0);
        int flag= isNumberVar(arrs->get(1)) ? toInteger(arrs->get(1)): 0;
        int pos=sv->findPos(find, sp, (U32)flag);
        rst->setVar('0',0).addInt(pos);
        */
    } break;
    case 77: {	// typeValue
        // if( cnt==0 ) return true;
        if( cpos==-1 ) return true;
        if( ep==0 ) {
            ep=send;
        }
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
            rst->reuse();
        }
        getStringTypeValue(sv, sp, ep, rst);

    } break;
    case 78: {	// exp
        if( cnt==0 ) return true;
        if( cpos==-1 ) return true;
        if( ep==0 ) {
            ep=send;
        }
        StrVar* chk=arrs->get(1);
        LPCC e=AS(0);
        if( slen(e) && checkFuncObject(chk,'f',1) ) {
            XFuncSrc* fsrc=(XFuncSrc*)chk->getObject(FUNC_HEADER_POS);
            rst->set(sv,sp,ep);
            LPCC val=rst->get();
            QString str=KR(val);
            QRegExp exp(KR(e));
            XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc, fn);
            fnCur->GetVar("@this")->reuse()->add(fn->gv("@this"));
            fnCur->GetVar("@str")->setVar('s',11,(LPVOID)&str);
            fnCur->GetVar("exp")->setVar('s',12, (LPVOID)&exp);
            fnCur->call(NULL, rst->reuse());
            funcNodeDelete(fnCur);
        }
    } break;
    case 79: {	// prevWord
        rst->reuse();
        if( cpos==-1 ) return true;
        if( rst==sv ) {
            sv=getStrVarTemp();
            sv->add(rst);
        }
        if( ep==0 ) ep = send;
        if( sp>=ep && var ) {
            var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(sp).addInt(sp).addInt(ep);
            return true;
        }
        char c=0;
        int pos = ep-1, end=ep;
        while( pos>=sp ) {
            c=sv->charAt(pos, ep);
            if( ISBLANK(c)   ) {
                pos--;
                end--;
            } else {
                break;
            }
        }
        int prev=pos;
        while( pos>=sp ) {
            c=sv->charAt(pos, ep);
            if( ISBLANK(c) || IS_OPERATION(c) ) {
                if( pos < prev ) {
                    pos++;
                }
                break;
            }
            pos--;
        }
        if( pos<0 ) pos=0;
        rst->add(sv->get(pos), end-pos);
        if( orgin ) {
            orgin->putInt(pos,OBJ_PROP_START_POS);
        } else if( var && sv!=var ) {
            if( pos<=sp ) {
                var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(sp).addInt(-1).addInt(ep);
            } else {
                var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(pos).addInt(sp).addInt(ep);
            }
        }
    } break;
    case 80: { // insert
        if( cnt==0 ) return true;
        LPCC str=AS(0);
        int len=slen(str);
        if( len>0 ) {
            if( cpos==-1 ) return true;
            if( ep==0 ) ep = send;
            if( sp>=ep ) {
                return true;
            }
            int pos=-1;
            if( isNumberVar(arrs->get(1)) ) {
                pos=toInteger(arrs->get(1));
            }
            if( pos==-1 || pos>=ep ) {
                sv->add(str,len);
                if( var && sv!=var ) {
                    ep+=len;
                    var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(-1).addInt(ep);
                }
            } else {
                if( pos<sp ) pos=sp;
                sv->insert(pos, str, len );
                if( var && sv!=var ) {
                    ep+=len;
                    var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(ep);
                }
            }
        }
    } break;
    case 81: { // remove
        if( cnt==0 ) return true;
        int start=toInteger(arrs->get(0)), removeSize=toInteger(arrs->get(1));
        if( removeSize>0 ) {
            if( cpos==-1 ) return true;
            if( ep==0 ) ep = send;
            if( start<sp ) start=sp;
            if( (start+removeSize)> ep ) {
                removeSize=ep-start;
            }
            sv->remove(start, removeSize );
            if( var && sv!=var ) {
                ep-=removeSize;
                var->setVar('s',0,(LPVOID)sv).addInt(sp).addInt(ep).addInt(sp).addInt(ep);
            }
        }
    } break;
    default: return false;
    }
    return true;
}


int execFuncStr(XFuncNode* fnParent, StrVar* sv, int sp, int ep, StrVar* rst, LPCC param) {
    if( fnParent==NULL || sp>=ep ) return 0;
    XFuncSrc* fsrc=NULL;
    // XFuncNode* fnParent=(XFuncNode*)SVO;
    LPCC funcNm=NULL;
    bool sfunc=false;
    XParseVar pv(sv,sp,ep);
    if( pv.ch()=='@' ) {
        pv.start++;
        sp=pv.start;
        pv.moveNext();
        if( pv.ch()=='#' ) pv.incr().moveNext();
        LPCC grp=pv.Trim(sp, pv.start);
        if( pv.ch()=='.' ) {
            pv.start++;
            sp=pv.start;
            pv.moveNext();
            if( pv.ch()=='#' ) pv.incr().moveNext();
            if( pv.ch()=='(' ) {
                sfunc=true;
                funcNm=gBuf.fmt("%s::%s", grp, pv.Trim(sp, pv.start) );
            }
        }
    } else {
        sp=pv.start;
        pv.moveNext();
        if( pv.ch()=='#' ) pv.incr().moveNext();
        ep=pv.start;
        if( pv.ch()=='(' ) {
            funcNm=pv.Trim(sp,ep);
        }
    }

    if( slen(funcNm) ) {
        XListArr* list=NULL;
        if( pv.match("(",")",FIND_SKIP) ) {
            XParseVar pvParam( pv.GetVar(), pv.prev, pv.cur );
            if( ccmp(funcNm,"conf") ) {
                LPCC ccode=NULL;
                char ch=pvParam.ch();
                if( ch=='\'' || ch=='\"' || ch=='`' ) {
                    pvParam.MatchStringPos(ch);
                    ccode=pvParam.v();
                } else {
                    sp=pvParam.start;
                    pvParam.moveNext();
                    if( pvParam.ch()=='.' ) {
                        pvParam.incr().moveNext();
                    }
                    StrVar* var=getStrVarTemp();
                    XParseVar sub(pvParam.GetVar(), sp, pvParam.start);
                    setResultVal( &sub, fnParent, var, NULL);
                    ccode=toString(var);
                }
                if( IndexOf(ccode,'.')>0 ) {
                    rst->add(toString(confVar(ccode)));
                    return 1;
                }
                return 0;
            }
            while( pvParam.valid() ) {
                char ch=pvParam.ch();
                if( ch=='\0' ) break;
                if( list==NULL ) list=getLocalArray(true);
                StrVar* var=list->add(); // ->add( getFuncVar(fnParent,vnm) );
                if( ch=='\'' || ch=='\"' || ch=='`' ) {
                    pvParam.MatchStringPos(ch);
                    if( ch=='`') {
                        XParseVar sub(pvParam.GetVar(), pvParam.prev, pvParam.cur);
                        makeTextData(sub, fnParent, var);
                    } else {
                        var->set(pvParam.GetVar(), pvParam.prev, pvParam.cur );
                    }
                    if( pvParam.ch()==',' ) {
                        pvParam.incr();
                    } else {
                        break;
                    }
                } else {
                    int pos=baroFuncEndPos(pvParam.GetVar(), pvParam.start, pvParam.wep );
                    if( pos!=-1 ) {
                        execFuncStr( fnParent, pvParam.GetVar(), pvParam.start, pos, var, param );
                        pvParam.start=pos;
                    } else {
                       pvParam.findEnd(",");
                       XParseVar sub(pvParam.GetVar(), pvParam.prev, pvParam.cur);
                       setResultVal( &sub, fnParent, var, NULL);
                    }
                }
            }
        }
        if( sfunc ) {
            sv=getSubFuncSrc(funcNm,NULL); // gum.mapping("ssrc",funcNm);
        } else {
            sv=getFuncVar(fnParent, funcNm); //fnParent->gv( funcNm ); //
            if( !SVCHK('f',1) ) {
                sv=getStrVar("fsrc",funcNm);
            }
        }
        if( SVCHK('f',1) ) {
            fsrc=(XFuncSrc*)SVO;
            XFuncNode* fnCur=gfns.getFuncNode(fsrc->xfunc, fnParent );
            // LPCC param=toString(arrs->get(2) );
            if( slen(param) ) {
                sv=getFuncVar(fnParent, param); // fnParent->gv(param);
                fnCur->GetVar(param)->reuse()->add(sv);
            }
            setFuncSrcParam(fnCur,list,fsrc,0,fnParent);
            fnCur->call();
            rst->add( toString(&(fnCur->xrst)) );
            funcNodeDelete(fnCur);
        }
        if( list ) {
            releaseLocalArray(list);
        }
    }
    if( fsrc==NULL ) {
        qDebug("#9#%s call function fail\n", funcNm );
    }

    return 0;
}
void lpadString(LPCC a, int mx, StrVar* rst, char ch) {
    int b=slen(a);
    if( mx>b ) {
        for(int n=0,sz=mx-b;n<sz;n++) rst->add(ch);
        for(int n=0;n<b;n++) rst->add(a[n]);
    } else {
        for(int n=0;n<mx;n++) rst->add(a[n]);
    }
}
void lpad(int num, int mx, StrVar* rst, char ch) {
#ifdef WINDOW_USE
    char a[32];
    itoa(num,a,10);
    lpadString(a, mx, rst, ch);
#endif
}

void getStringTypeValue(StrVar* sv, int sp, int ep, StrVar* rst, bool bset) {
    if( sv==NULL ) return;
    int type= sp<ep ? 1: 0;
    int n=sp;
    char ch = sv->charAt(n), firstCh=ch;
    if( ch=='+' || ch=='-' ) n++;
    for( ; n<ep; n++) {
        ch = sv->charAt(n);
        if( IS_DIGIT(ch) ) continue;
        if( ch=='.') {
            type=4;
            if( type==4 ) type=0;
        } else {
            type=0;
            break;
        }
    }
    if( type==1 && n>8 ) {
        type=2;
    }
    rst->reuse();
    if( sp>=0 && sp<ep ) {
        rst->set(sv->trimv(sp,ep));
    }
    if( type==1 ) {
        int num=toInteger(rst);
        rst->setVar('0',0).addInt(num);
        if( bset ) sv->reuse()->add(rst);
    } else if( type==2 ) {
        UL64 num=toUL64(rst);
        rst->setVar('1',0).addUL64(num);
        if( bset ) sv->reuse()->add(rst);
    } else if( type==4 ) {
        double num=toDouble(rst);
        rst->setVar('4',0).addDouble(num);
        if( bset ) sv->reuse()->add(rst);
    } else {
        LPCC val=rst->get();
        if( firstCh=='t' && ccmp(val,"true") ) {
            rst->setVar('3',1);
            if( bset ) sv->reuse()->add(rst);
        } else if( firstCh=='f' && ccmp(val,"false") ) {
            rst->setVar('3',0);
            if( bset ) sv->reuse()->add(rst);
        } else if( firstCh=='n' && ccmp(val,"null") ) {
            rst->setVar('9',0);
        }
#ifdef TEST_USE
        else if( rst->length() ) {
            XParseVar pv(rst);
            LPCC tag=NULL;
            int sp=0, ep=0;
            if( pv.ch()=='<' && pv.match("<",">") ) {
                XParseVar sub(pv.GetVar(), pv.prev, pv.cur);
                tag=sub.NextWord();
                sp=sub.start, ep=pv.cur;
            }
            if( tag==NULL ) {
                tag=pv.NextWord();
                if( pv.ch()=='<' && pv.match("<",">") ) {
                    sp=pv.prev, ep=pv.cur;
                }
            }
            if( sp<ep && slen(tag) ) {
                XParseVar sub(pv.GetVar(), sp, ep);
                if( sub.ch()==':' )
                    sub.incr();
                if( ccmp(tag,"node") ) {
                    TreeNode* h = new TreeNode(2, true);
                    h->setJson(sub);
                    rst->setVar('n',0,(LPVOID)h);
                } else if( ccmp(tag,"array") ) {
                    XListArr* arr = new XListArr(true);
                    if( sub.ch()=='[' ) sub.incr();
                    getCf()->setArray(arr,sub);
                    rst->setVar('a',0,(LPVOID)arr);
                } else {
                    XListArr* arrs=getListArrTemp();
                    while( sub.valid() ) {
                        arrs->add()->add( sub.findEnd(",").v() );
                    }
                    rst->reuse();
                    if( ccmp(tag,"draw") || ccmp(tag,"drawObject") || ccmp(tag,"canvas") || ccmp(tag,"image") ) {
                        setClassCanvas(arrs, rst );
                    } else if( ccmp(tag,"rest") ) {
                        setClassRect(arrs, rst );
                    } else if( ccmp(tag,"size") || ccmp(tag,"point") ) {
                        setClassPoint(arrs, rst );
                    } else if( ccmp(tag,"color") ) {
                        setClassColor(arrs, rst );
                    } else {
                        if( ccmp(tag,"db") || ccmp(tag,"dsn") ) tag="model";
                        setClassTagObject(tag, arrs, rst );
                    }
                }
            }
        }
#endif
    }
    if( bset ) sv->reuse()->add(rst);
}


void encodeEntities(LPCC src, LPCC st, LPCC et, int mod, int shift, int base, StrVar* rst ) {	// , const QString& force=QString()
    if( rst==NULL ) return;
    if( base==0 ) {
        if( mod==3 ) {
            base=8;
        } else if( mod==10 || mod==8 || mod==16 ) {
            base=mod;
        } else {
            base=10;
        }
    }
    LPCC fmt=NULL;
    gLocalVar.set("%s");
    switch( base ) {
    case 8:		gLocalVar.add("%o"); break;
    case 10:	gLocalVar.add("%d"); break;
    case 16:	gLocalVar.add("%X"); break;
    }
    gLocalVar.add("%s");
    fmt=gLocalVar.get();
    rst->reuse();
    if( mod==0 ) {
        QString str=KR(src);
        for(int n=0,len=str.length(); n<len; n++ ) {
            U16 code=str[n].unicode();
            if( code > 128 ) {		//  || force.contains(tmp[i])
                rst->format(32,fmt,st,code,et);
            } else {
                rst->add((char)code);
            }
        }
    } else {
        StrVar var;
        int idx=0;
        U8 c=0;
        for( int n=0,len=slen(src);n<len;n++ ) {
            c=(U8)src[n];
            switch(mod ) {
            case 1: {
                if( idx==0 ) {
                    shift=getBinaryShift((int)c,0);
                    if( shift==1 ) {
                        rst->add((char)c);
                    } else {
                        var.add((char)c);
                        idx++;
                    }
                } else {
                    var.add((char)c);
                    if(idx==shift ) {
                        LPCC txt=var.get();
                        QString str=KR(txt);
                        if( str.length()==1 ) {
                            rst->format(32,fmt,st,str.at(0).unicode(),et);
                        }
                        var.reuse();
                        idx=0;
                    } else {
                        idx++;
                    }
                }
            } break;
            case 3: {
                if( idx==0 ) {
                    shift=getBinaryShift((int)c,0);
                    if( shift==1 ) {
                        rst->add((char)c);
                    } else {
                        rst->format(32,fmt,st,c,et);
                        idx++;
                    }
                } else  {
                    rst->format(32,fmt,st,c,et);
                    if( idx==shift ) {
                        idx=0;
                    } else {
                        idx++;
                    }
                }
            } break;
            default: {
                rst->format(32,fmt,st,c,et);
            } break;
            }
        }
    }
}

void decodeEntities( const QString& src, const QString& exp, int mod, int base, StrVar* rst ) {
    QRegExp re(exp);
    // re.setMinimal(true);
    LPCC val=NULL;
    int pos=0, n=0, chk=0, prev=0;
    U32 num=0;
    if( base==0 ) {
        if( mod==3 ) {
            base=8;
        } else if( mod==10 || mod==8 || mod==16 ) {
            base=mod;
        } else {
            base=10;
        }
    }
    bool convert=false;
    if( mod==3 ) {
        convert=true;
        chk=2;
    }
    int srcLen=src.length();
    rst->reuse();
    while( (pos = re.indexIn(src, pos)) != -1 ) {
        num=re.cap(1).toInt(0,base);
        switch(mod) {
        case 1: {
            if( prev<pos ) {
                rst->add( Q2A(src.mid(prev, pos-prev)) );
            }
            switch( getBinaryShift(num,0) ) {
            case 1:
                rst->add((char)num); break;
            case 2:
                rst->addU16((U16)num); break;
            default: {
                QString str=QChar(num);
                rst->add(Q2A(str));
            } break;
            }
        } break;
        case 3: {
            if( n==chk ) {
                rst->add( (char)num );
                n=0;
            } else {
                if( n==0 && prev<pos ) {
                    rst->add( Q2A(src.mid(prev, pos-prev)) );
                }
                rst->add( (char)num );
                n++;
            }
        } break;
        case 8:
        case 10:
        case 16: {
            if( prev<pos ) {
                rst->add( Q2A(src.mid(prev, pos-prev)) );
            }
            switch( getBinaryShift(num,1) ) {
            case 1: rst->add((char)num); break;
            case 2:	rst->addU16((U16)num); break;
            case 4: rst->addU32(num); break;
            }
        } break;
        default: {
            if( prev<pos ) {
                rst->add( Q2A(src.mid(prev, pos-prev)) );
            }
            QString str=QChar(num);
            rst->add(Q2A(str));
        } break;
        }
        pos += re.matchedLength();
        prev=pos;
    }
    if( prev < srcLen ) {
        rst->add( Q2A(src.mid(prev, srcLen-prev)) );
    }
}
