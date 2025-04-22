#include "listobject.h"
#ifdef WIDGET_USE
#include "widget_util.h"
#endif

#ifdef FUNCNODE_USE
#include "../baroscript/func_util.h"
#endif

#ifdef WIDGET_USE
StrVar gResultVar;
StrVar gTempVar;

int gVarTempIdx=0;
XListArr gVarTempArr;
StrVar* getVarTemp() {
    if( gVarTempIdx>=1024 ) {
        // qDebug("-- getVarTemp :%d",gVarTempIdx);
        gVarTempIdx=0;
    }
    if( gVarTempArr.size()==0 ) {
        for(int n=0; n<1024;n++ ) gVarTempArr.add();
    }
    return gVarTempArr.get(gVarTempIdx++)->reuse();
}

#ifndef _WIN32
QString KR( QString str ) {
    QTextCodec* codec=QTextCodec::codecForName("UTF-8");
    return codec->toUnicode( str.toAscii() );
    // return str; // QString::fromUtf8(str);
}
unsigned long GetTickCount(){
    struct timeval tick;
    gettimeofday(&tick, NULL);
    return tick.tv_sec*1000 + tick.tv_usec/1000;
}
#endif
#endif

bool checkObject(StrVar* var, char ch) {
    return var && var->charAt(0)=='\b' && var->charAt(1)==ch;
}

bool checkFuncObject(StrVar* var, char ch, U16 state) {
    return checkObject(var,ch) && var->getU16(2)==state;
}
bool ccmpl(LPCC code, int csize, LPCC data, int dsize ) {
    if( dsize==0 ) {
        if( csize==0 ) {
            csize=dsize=slen(data);
        } else {
            dsize=slen(data);
        }
    }
    if( code==NULL || dsize==0 || csize!=dsize )
        return false;
    int n=0;
    for( ; n<csize && *data && *code ; n++ ) {
        if( tolower(*code++)!=tolower(*data++) ) return false;
    }
    return n==csize ? true: false;
}
bool ccmpl(LPCC code, LPCC data, int size ) {
    if( code==NULL || data==NULL ) {
        return false;
    }
    if( size==0 )
        size = slen(data);
    int n=0;
    for( ; n<size && *code; n++) {
        if(*code++!=*data++) return false;
    }
    return (n==size) ? true: false;
}
bool svcmp(StrVar* sv, LPCC val) {
    return ccmp(toString(sv),val);
}


bool isoper(char c) {
    return ( c=='=' || c=='!' || c=='+' || c=='-' || c=='*' || c=='/' || c=='%' || c=='<' || c=='>' || c=='~' || c=='&' || c=='|') ? true : false;
}

bool isVarTrue(StrVar* var ) {
    if( var==NULL )
        return false;
    if( var->charAt(0)=='\b' ) {
        switch( var->charAt(1) ) {
        case '0':
            return var->getInt(FUNC_HEADER_POS)==0 ? false : true;
        case '1':
            return var->getUL64(FUNC_HEADER_POS)==0 ? false : true;
        case '3':
            return var->getU16(2)==0 ? false : true;
        case '4': {
            float a=(float)var->getDouble(FUNC_HEADER_POS);
            return a==0.0 ? false : true;
        }
        case '9': return false;
        case 'i': return true;
        case 's': {
            if( var->getU16(2)==0 ) {
                int s=var->getInt(OBJ_PROP_START_POS), e=var->getInt(OBJ_PROP_START_POS+4);
                return s<e ? true: false;
            }
        }
        default:
            return var->getObject(FUNC_HEADER_POS) ? true : false;
        }
    }
    return var->length()==0 ? false : true;
}

bool isNumberVar(StrVar* var) {
    if( var==NULL || var->length()==0 )
        return false;
    if( var->charAt(0)=='\b' ) {
        switch(var->charAt(1) ) {
        case '0':
        case '1':
        case '2':
        case '4':
            return true;
        case 'n':
        case 'a':
            return false;
        default: {
            LPCC s = toString(var);
            if( (slen(s)>1 && s[0]=='0' && (s[1]=='x'|| s[1]=='X')) || isnum(s) )
                return true;
        } break;
        }
    } else {
        LPCC s = var->get();
        if( (slen(s)>1 && s[0]=='0' && (s[1]=='x'|| s[1]=='X')) || isnum(s) )
            return true;
    }
    return false;
}

LPCC toLower(LPCC str) {
    StrVar* rst=gTempVar.reuse();
    for( int n=0,len=slen(str); n<len; n++ ) {
        rst->add( (char)tolower(str[n]));
    }
    return rst->get();
}
LPCC toUpper(LPCC str) {
    StrVar* rst=gTempVar.reuse();
    for( int n=0,len=slen(str); n<len; n++ ) {
        rst->add( (char)toupper(str[n]));
    }
    return rst->get();
}


int toInteger(StrVar* var) {
    if( var==NULL || var->length()==0 )
        return 0;
    LPCC val=NULL;
    int rtn = 0;
    if( var->charAt(0)=='\b' ) {
        switch(var->charAt(1) ) {
        case '0': {
            rtn = var->getInt(FUNC_HEADER_POS);
        } break;
        case '1': {
            rtn = (int)var->getUL64(FUNC_HEADER_POS);
        } break;
        case '2': {
            rtn = (int)var->getU32(FUNC_HEADER_POS);
        } break;
        case '3': {
            rtn = (int)var->getU16(2);
        } break;
        case '4': {
            rtn = (int)var->getDouble(FUNC_HEADER_POS);
        } break;
        default: {
            val = toString(var);
        } break;
        }
    } else {
        val = var->get();
    }
    int len=slen(val);
    if( len ) {
        if( len>1 && val[0]=='0' && (val[1]=='x'|| val[1]=='X') ) {
            LPCC hex = val+2;
            rtn = (int)strtol(hex, NULL, 16);
        } else if( isnum(val) ) {
            rtn = atoi(val);
        }
    }
    return rtn;
}

UL64 toUL64(StrVar* var) {
    if( var==NULL || var->length()==0 )
        return 0;
    LPCC s=NULL;
    UL64 rtn = 0;
    if( var->charAt(0)=='\b' ) {
        switch(var->charAt(1) ) {
        case '0': {
            if(var->getU16(2)==1) {
                rtn = (UL64)var->getU32(FUNC_HEADER_POS);
            } else {
                rtn = (UL64)var->getInt(FUNC_HEADER_POS);
            }
        } break;
        case '1': {
            rtn = var->getUL64(FUNC_HEADER_POS);
        } break;
        case '2': {
            rtn = (UL64)var->getU32(FUNC_HEADER_POS);
        } break;
        case '3': {
            rtn = (UL64)var->getU16(2);
        } break;
        case '4': {
            rtn = (UL64)var->getDouble(FUNC_HEADER_POS);
        } break;
        default: {
            s = toString(var);
        } break;
        }
    } else {
        s = var->get();
    }
    if( slen(s) ) {
        if( s[0]=='0' && (s[1]=='x'|| s[1]=='X') ) {
            LPCC hex = s+2;
            rtn = (UL64)strtol(hex, NULL, 16);
        } else if( isnum(s) ) {
            rtn = (UL64)atol(s); //(UL64)atoll(s);
        } else if( var->length()==8 ) {
            rtn = *((UL64*)s);
        }
    }
    return rtn;
}


double toDouble(StrVar* var) {
    if( var==NULL || var->length()==0 )
        return 0;
    LPCC s=NULL;
    double rtn = 0;
    if( var->charAt(0)=='\b' ) {
        switch(var->charAt(1) ) {
        case '0': {
            if(var->getU16(2)==1) {
                rtn = (double)var->getU32(FUNC_HEADER_POS);
            } else {
                rtn = (double)var->getInt(FUNC_HEADER_POS);
            }
        } break;
        case '1': {
            rtn = (double)var->getUL64(FUNC_HEADER_POS);
        } break;
        case '2': {
            rtn = (double)var->getU32(FUNC_HEADER_POS);
        } break;
        case '3': {
            rtn = (double)var->getU16(2);
        } break;
        case '4': {
            rtn = (double)var->getDouble(FUNC_HEADER_POS);
        } break;
        default: {
            s = toString(var);
        } break;
        }
    } else {
        s = var->get();
    }
    if( slen(s) ) {
        if( s[0]=='0' && (s[1]=='x'|| s[1]=='X') ) {
            LPCC hex = s+2;
            rtn = (double)strtol(hex, NULL, 16);
        } else if( isnum(s) ) {
            rtn = (double)atof(s);
        } else if( var->length()==8 ) {
            rtn = *((double*)s);
        }
    }
    return rtn;
}

LPCC toString(StrVar* var, StrVar* rst, bool objectCheck, int depth) {
    if( var==NULL || var->length()==0 )
        return NULL;
    if( objectCheck==false ) {
        if( depth>0 ) objectCheck=true;
    }
    if( var->charAt(0)=='\b' ) {
        if( rst==NULL ) {
            rst=getVarTemp();
        }
        switch(var->charAt(1) ) {
        case '0': {
            if(var->getU16(2)==1) {
                U32 n = var->getU32(FUNC_HEADER_POS);
                rst->format(16,"%u",n);
            } else {
                int n = var->getInt(FUNC_HEADER_POS);
                rst->format(16,"%d",n);
            }
        } break;
        case '1': {
            UL64 n = var->getUL64(FUNC_HEADER_POS);
            rst->format(16,"%lld",n);
        } break;
        case '2': {
            U32 n = var->getU32(FUNC_HEADER_POS);
            rst->format(16,"%u",n);
        } break;
        case '3': {
            U16 n = var->getU16(2);
            rst->add(n==0? "false": "true");
        } break;
        case '4': {
            double n = var->getDouble(FUNC_HEADER_POS);
            rst->format(32,"%f",n);
        } break;
        case '9': {
            if( objectCheck==false ) {
                rst->add("");
            } else {
                rst->add("NULL");
            }
        } break;
        case 'w': {
            rst->add("widget");
        } break;
        case 's': {
            U16 stat=var->getU16(2);
            if( stat==1 ) {
                var=(StrVar*)var->getObject(FUNC_HEADER_POS);
                stat=var->getU16(2);
            }
            if( stat==0 ) {
                StrVar* str = (StrVar*)var->getObject(FUNC_HEADER_POS);
                if( str ) {
                    int sp=var->getInt(OBJ_PROP_START_POS), ep=var->getInt(OBJ_PROP_START_POS+4);
                    if( sp >= ep) {
                        return NULL;
                    }
                    int end=str->length();
                    if( sp==0 && ep==0 ) {
                        int cpos=var->getInt(16);
                        if( cpos!=-1 ) {
                            ep=end;
                        }
                    } else if( ep>end ) {
                        ep=end;
                    }
                    if( sp<ep ) {
                        rst->add(str->get(sp), ep-sp);
                    } else if( sp>ep ) {
                        _LOG("toString position not valid : %d, %d\n",sp,ep);
                    }
                }
            }
        } break;
        case 'i': {
            U16 stat = var->getU16(2);
            int len=var->length();
            if( stat==2 ) {
                rst->format(64,"point<%d,%d>",var->getInt(4), var->getInt(8) );
            } else if( stat==3 ) {
                if( len==16 ) {
                    rst->format(64,"color<#%02X%02X%02X>", var->getInt(4), var->getInt(8), var->getInt(12) );
                } else if( len==20 ) {
                    rst->format(64,"color<#%02X%02X%02X%02X>", var->getInt(4), var->getInt(8), var->getInt(12), var->getInt(16));
                }
            } else if( stat==4 ) {
                rst->format(64,"rect<%d,%d,%d,%d>",var->getInt(4), var->getInt(8), var->getInt(12), var->getInt(16) );
            } else if( stat==5 ) {
                int sz=sizeof(double);
                double x=var->getDouble(4), y=var->getDouble(4+sz), w=var->getDouble(4+(2*sz)), h=var->getDouble(4+(3*sz));
                rst->format(64,"rectf<%d,%d,%d,%d>",(int)x, (int)y, (int)w, (int)h );
            } else if( stat==20 ) {
                int sz=sizeof(double);
                double x=var->getDouble(4), y=var->getDouble(4+sz);
                rst->format(64,"pointf<%d,%d>",(int)x, (int)y );
            } else if( stat==6 ) {
                rst->add("png");
            } else if( stat==7 ) {
                rst->add("buffer");
            } else if( stat==9 ) {
                rst->add("image");
            }
        } break;
        case 'a': {
            U16 stat=var->getU16(2);
            if( stat==0 ) {
                XListArr* arr = (XListArr*)var->getObject(FUNC_HEADER_POS);
                if( arr ) {
                    rst->add("[");
                    if( objectCheck ) {
                        rst->format(64, "array size:%d", arr->size() );
                    } else {
                        for( int n=0,size=arr->size();n<size;n++) {
                            if( n>0 ) rst->add(", ");
                            StrVar* sv=sv=arr->get(n);
                            toString(sv, rst, true);
                        }
                    }
                    rst->add("]");
                }
            }
        } break;
        case 'n': {
            U16 state = var->getU16(2);
            if( state>10 ) return NULL;
            TreeNode* node = (TreeNode*)var->getObject(FUNC_HEADER_POS);
            WBoxNode* bn = node->First();
            StrVar* sv = NULL;
            rst->add("{");
            int size = node->childCount();
            switch( node->xstat ) {
            case FNSTATE_DRAW:			rst->add("node(draw)"); break;
            case FNSTATE_DB:			rst->add("node(db)"); break;
            case FNSTATE_MODEL:			rst->add("node(dataModel)"); break;
            case FNSTATE_FILE:			rst->add("node(file)"); break;
            case FNSTATE_FILEFIND:		rst->add("node(filefind)"); break;
            case FNSTATE_PROCESS:		rst->add("node(process)"); break;
            case FNSTATE_JOB:			rst->add("node(job)"); break;
            case FNSTATE_CRON:			rst->add("node(cron)"); break;
            case FNSTATE_WORKER:		rst->add("node(worker)"); break;
            case FNSTATE_EXCEL:         rst->add("node(excel)"); break;
            case FNSTATE_WEBCLIENT:		rst->add("node(web)"); break;
            case FNSTATE_WEBSERVER:		rst->add("node(webServer)"); break;
            case FNSTATE_WATCHER:		rst->add("node(watcher)"); break;
            case FNSTATE_PDF:			rst->add("node(pdf)"); break;
            case FNSTATE_ZIP:			rst->add("node(zip)"); break;
            case FNSTATE_SOCKET:		rst->add("node(socket)"); break;
            case FNSTATE_ACTION:		rst->add("node(action)"); break;
            case FNSTATE_MENU:			rst->add("node(menu)"); break;
            }
            if( objectCheck ) {
                // node->log(rst,true,depth);
                rst->format(64, "node: childCount: %d}", size );
            } else {
                // rst->format(64," (type:%d,state:%d) ", node->xtype, node->xstat );
                if( size ) rst->format(64,"childCount: %d, ", size);
                int num=0;
                for(int n=0; n<1024 && bn; bn = node->Next(), n++ ) {
                    LPCC code=node->getCurCode();
                    if( ccmp(code,"@result") ) continue;
                    if( num++ ) rst->add(",");
                    rst->add("\n\t");
                    if( depth ) {
                        for( int x=0; x<depth; x++ ) rst->add("\t");
                    }
                    rst->add(code).add(": ");
                    sv=&(bn->value);
                    if( SVCHK('n',0) ) {
                        TreeNode* cur=(TreeNode*)SVO;
                        if( node==cur ) {
                            rst->add("node");
                            continue;
                        }
                    }
                    toString(sv, rst, true, depth+1);
                }
                if( rst->findPos("\n")!=-1 ) {
                    rst->add("\n");
                    if( depth ) {
                        for( int x=0; x<depth; x++ ) rst->add("\t");
                    }
                }
                rst->add("}");
            }
        } break;
        case 'f': {
            rst->add("func");
        } break;
        case 'd': {
            rst->add("db");
        } break;
        case 'l': {
            rst->add("layout");
        } break;
        case 'm': {
            rst->add("model");
        } break;
        case 'g': {
            rst->add("painter");
        } break;
        case 'c': {
            rst->add("drawItem");
        } break;
        case 't': {
            U16 stat=var->getU16(2);
            if( stat==21) {
                // 't',21
                rst->add("watcher");
            } else {
                rst->add("thread");
            }
        } break;
        case 'v': {
            U16 stat=var->getU16(2);
            if( stat==0 ) {
                rst->add("was");
            } else if( stat==1 ) {
                rst->add("connection");
            }
        } break;
        case 'p': {
            U16 stat=var->getU16(2);
            if( stat==11 ) {
                rst->add("printer");
            }
        } break;
        case 'e': {
            rst->add("error");
        } break;
        case 'x': {
            U16 state = var->getU16(2);
            if( state==21 ) {
                int pos=var->find("\f",FUNC_HEADER_POS,64);
                if( pos>FUNC_HEADER_POS ) {
                    rst->add("pair<").add(var,FUNC_HEADER_POS, pos).add(':');
                    if( var->charAt(pos+1)=='\b' ) {
                        StrVar* sv=cfVar("@temp",true);
                        sv->add(var, pos+1);
                        rst->add(toString(sv));
                    } else {
                        rst->add(var->get(pos+1));
                    }
                }
            }
        } break;
        default: {
            qDebug("xxxx>toString default (kind:%c, stat:%d)", var->charAt(1), var->getU16(2));
        }break;
        }
    } else if( rst ) {
        if( objectCheck && var->length()>256 ) {
            XParseVar pv(var);
            pv.ch();
            rst->add(pv.findEnd("\n").v() );
        } else {
            rst->add(var);
        }
    }
    return rst? rst->get() : var->get();
}

bool makeTextValue(XParseVar& pv, StrVar* rst, TreeNode* tn ) {
    char ch = 0;
    int sp=0,ep=0;
    StrVar* sv = NULL;
    while( pv.valid() ) {
        pv.findEnd("$",FIND_FORWORD);
        if( pv.cur>pv.prev ) {
            rst->add(pv.GetVar(),pv.prev,pv.cur);
        } else {
            return false;
        }
        ch = pv.ch();
        if( ch==0 )
            break;
        if( ch!='{') {
            rst->add("$");
            continue;
        }
        if( ch=='$') {
            rst->add("$$");
            pv.incr();
            continue;
        }
        sp = pv.start, ep=pv.findPos("}");
        if( ep==-1 ) {
            return false;
        }
        LPCC var = pv.Trim(sp,ep);
        if( tn ) {
            sv = tn->gv(var);
            rst->add(toString(sv) );
        }
        pv.start=ep+1;
    }
    return true;
}
bool makeTextValueVar(StrVar* var, int sp, int ep, StrVar* rst, TreeNode* tn ) {
    XParseVar pv(var, sp, ep);
    return makeTextValue(pv, rst, tn);
}


#ifdef WIDGET_USE
inline void hsvColor(double hh, double hs, double hv, int alpha, StrVar* rst) {
    double      ha, p, q, t, ff;
    long        i;
    double rr=0, rg=0, rb=0;

    if(hs <= 0.0) {       // < is bogus, just shuts up warnings
        rr = hv;
        rg = hv;
        rb = hv;
    } else {
        ha = hh;
        if(ha >= 360.0) ha = 0.0;
        ha /= 60.0;
        i = (long)ha;
        ff = ha - i;
        p = hv * (1.0 - hs);
        q = hv * (1.0 - (hs * ff));
        t = hv * (1.0 - (hs * (1.0 - ff)));

        switch(i) {
        case 0:
            rr = hv;
            rg = t;
            rb = p;
            break;
        case 1:
            rr = q;
            rg = hv;
            rb = p;
            break;
        case 2:
            rr = p;
            rg = hv;
            rb = t;
            break;

        case 3:
            rr = p;
            rg = q;
            rb = hv;
            break;
        case 4:
            rr = t;
            rg = p;
            rb = hv;
            break;
        case 5:
        default:
            rr = hv;
            rg = p;
            rb = q;
            break;
        }
    }
    rst->setVar('i',3).addInt(rr*255).addInt(rg*255).addInt(rb*255);
    if( alpha>=0 ) rst->addInt(alpha);
}

inline void getColorVar(LPCC s, StrVar* rst) {
    if( s==NULL ) return;
    if(*s=='#')
        s++;
    int len = slen(s);
    if( len<2 )
        return;
    char buf[4];
    buf[2] = 0;
    BYTE r, g, b, a;
    r = g = b = a = 0;
    if( len==3 || len==4 ) {
        buf[0] = tolower(s[0]);
        buf[1] = 0;
        sscanf(buf,"%x",&r); r*=16;
        buf[0] = tolower(s[1]);
        sscanf(buf,"%x",&g); g*=16;
        buf[0] = tolower(s[2]);
        sscanf(buf,"%x",&b); b*=16;
        if( len==4 ) {
            buf[0] = tolower(s[3]);
            sscanf(buf,"%x",&a); a*=16;
        }
    } else if( len==6|| len==8 ) {
        buf[0] = tolower(s[0]); buf[1] = tolower(s[1]);
        sscanf(buf,"%x",&r);
        buf[0] = tolower(s[2]); buf[1] = tolower(s[3]);
        sscanf(buf,"%x",&g);
        buf[0] = tolower(s[4]); buf[1] = tolower(s[5]);
        sscanf(buf,"%x",&b);
        if( len==8 ) {
            buf[0] = tolower(s[6]); buf[1] = tolower(s[7]);
            sscanf(buf,"%x",&a);
        }
    }
}

inline void setNodeTagValue(LPCC tag, StrVar *sv, StrVar *sub, int sp, int ep)
{
    XParseVar pv(sub,sp,ep);
    if( !pv.valid() ) return;

    if( ccmp(tag,"rect") ) {
        LPCC val=NULL;
        int x=0,y=0,w=0,h=0;
        for(int n=0; n<4 && pv.valid(); n++ ) {
            if( !pv.ch() ) break;
            val=pv.findEnd(",").v();
            if( isnum(val) ) {
                switch( n ) {
                case 0: x=atoi(val); break;
                case 1: y=atoi(val); break;
                case 2: w=atoi(val); break;
                case 3: h=atoi(val); break;
                }
            } else break;
        }
        sv->setVar('i',4).addInt(x).addInt(y).addInt(w).addInt(h);
    } else if( ccmp(tag,"rectf") ) {
        LPCC val=NULL;
        float x=0,y=0,w=0,h=0;
        for(int n=0; n<4 && pv.valid(); n++ ) {
            if( !pv.ch() ) break;
            val=pv.findEnd(",").v();
            if( isnum(val) ) {
                switch( n ) {
                case 0: x=atof(val); break;
                case 1: y=atof(val); break;
                case 2: w=atof(val); break;
                case 3: h=atof(val); break;
                }
            } else break;
        }
        sv->setVar('i',5).addDouble(x).addDouble(y).addDouble(w).addDouble(h);
    } else if( ccmp(tag,"color") ) {
        sv->set(sub->get(sp), ep-sp);
        getGlobalColor(sv,"update");
    }
}
#endif

//
// #WBoxNode
//
WBoxNode::WBoxNode() :  type(0), value(0), uid(0)
{
}
WBoxNode::~WBoxNode()
{
    value.close();
}

//
// #XListArr
//
XListArr::XListArr(bool newInst) {
    buffer = bufferpos = NULL;
    buffersize = 0;
    total = last =0;
    objsize = xflag =0;
    if( newInst ) {
        xflag|=FLAG_NEW;
    }
}

XListArr::~XListArr() {
    if(size() ) Close();
}

BOOL XListArr::IsCreate()
{
    return (buffer) ? TRUE: FALSE;
}


void XListArr::Create(int size)
{
    if(buffer) Close();
    if( size<=0 ) size = 8;
    objsize = sizeof(StrVar);
    buffersize = size*objsize;
    buffer =(char *) malloc( buffersize );
    reuse();
}


void XListArr::Close()
{
    for(int n=0;n<size();n++) {
        get(n)->close();
    }
    SAFE_FREE(buffer);
}


XListArr* XListArr::reuse()
{
    if(buffer==NULL ) Create();
    if(total>last ) {
        last=total;
    }
    for( int n=0;n<total;n++ ) {
        get(n)->reuse();
    }
    bufferpos = buffer;
    total = 0;
    xflag = 0;
    return this;
}


int XListArr::size() { return (int)total; }


int XListArr::length() { return (bufferpos-buffer); }

void	XListArr::grow_buffer(){
    size_t length = bufferpos - buffer;
    buffer =(char *) realloc(buffer,buffersize);
    bufferpos = buffer + length;	// readjust current position
}


BOOL	XListArr::grow(int txtlen ) {
    if( txtlen==0 || bufferpos<buffer  ) return FALSE;
    U32 tlen = (bufferpos-buffer) + txtlen;
    if(tlen>buffersize) {
        while(tlen>buffersize ) {
            buffersize = buffersize *2;
        }
        grow_buffer();
    }
    return TRUE;
}


StrVar*	XListArr::add() {
    if( buffer==NULL) Create();
    StrVar* rtn = NULL;
    /*
    if(total<last ) {
        bufferpos=buffer+(total*objsize);
        rtn=(StrVar*)bufferpos;
    } else if( grow(objsize) ) {
        memset(bufferpos,0,objsize);
        rtn=(StrVar*)bufferpos;
    }
    */
    if( grow(objsize) ) {
        bufferpos=buffer+(total*objsize);
        memset(bufferpos,0,objsize);
        rtn=(StrVar*)bufferpos;
    }
    if(rtn) {
        bufferpos+=objsize;
        total++;
        rtn->reuse();
    } else {
        qDebug("XListArr::add object is NULL (%d,%d) buffer(%d,%d)\n", total, last, buffersize, objsize);
    }
    return rtn;
}


StrVar*	XListArr::get(int n) {
    if( buffer==NULL )
        return NULL;
    StrVar* rtn = (StrVar*)buffer;
    return ( n>=0 && n<total ) ? &rtn[n]: NULL;
}

int XListArr::remove(int n, int end) {
    if( buffer==NULL ) Create();
    int removeNum=1;
    if( end>0 ) {
        if( end>total ) end=total;
        if( n<end ) {
            removeNum=end-n;
        }
    }
    if( removeNum > total || removeNum<=0 ) {
        return 0;
    }
    int len = length();
    int startPos	= n*objsize;
    int removeSize	= removeNum * objsize;
    // int nCount = objsize;
    if( len<removeSize || startPos>=len )
        return 0;
    if( len==removeSize ) {
        reuse();
        return removeSize;
    }
    char* cur = buffer+startPos;
    char* curRemove=cur+removeSize;

    for( int n=0; n<removeNum; n++ ) {
        StrVar* tmp = (StrVar*)(buffer+startPos);
        tmp->close();
        startPos+=objsize;
    }

    int movelen = bufferpos - curRemove;
    memmove(cur, curRemove, movelen);
    total-=removeNum;
    bufferpos = buffer+(total*objsize);
    return movelen;
}

StrVar* XListArr::insert(int n ) {
    if( buffer==NULL ) Create();
    if( !grow(objsize) )
        return NULL;

    StrVar* rtn = NULL;
    int nPos = n*objsize;
    char* loc=buffer+nPos;
    int movelen= bufferpos - loc;
    memmove(loc+objsize,loc,movelen);
    memset(loc,0,objsize);
    rtn=(StrVar*)loc;
    rtn->reuse();
    total++;
    bufferpos+=objsize;
    return rtn;
}

StrVar* XListArr::operator[] (int n) {
    return get(n);
}



//
// WBox
//
WBox::WBox(int s) : hash(s) {
}

WBox::~WBox() {
    hash.Close();
}

void WBox::Create(int s )
{
    hash.Create(s);
}
void WBox::Close()
{
    hash.Close();
}

BOOL WBox::isCreate()
{
    return hash.IsCreate();
}

WBoxNode* WBox::GetBox(LPCC code) {
    if( slen(code)==0 ) return NULL;
    WBoxNode* box = GetHash(code);
    if( box==NULL ) {
        WBoxNode node;
        hash.Grow(code);
        box = hash.add(code, node);
        if( box) {
            box->value.create(16);
        }
    }
    return box;
}

WBoxNode* WBox::GetHash(LPCC code)
{
//	return (code && ccmp(code,hash.Code()) ) ? hash.Get() : hash.get(code);
    return (code) ? hash.get(code) : NULL;
}


StrVar* WBox::GetVar(LPCC code)
{
    if( slen(code)==0 ) return gTempVar.reuse();
    // hash.Grow(code);
    WBoxNode* box = GetBox(code);
    return (box) ? &box->value : gTempVar.reuse();
}

StrVar* WBox::gv(LPCC code)
{
    if( slen(code)==0 ) return NULL;
    // hash.Grow(code);
    WBoxNode* box = GetHash(code);
    return (box) ? &box->value : NULL;
}
void WBox::set(LPCC code,LPCC value, int size , U16 type ) {
    if( slen(code)==0 ) return;
    WBoxNode* box = GetBox(code);
    if( box )
    {
        if( type ) box->type = type;
        box->value.reuse();
        box->value.add(value,size);
    }
}
LPCC WBox::get(LPCC code)
{
    WBoxNode* bn = GetHash(code);
    if( !bn ) return NULL;
    return bn->value.get();
}

BOOL WBox::isset(LPCC code) {
    StrVar* var = gv(code);
    if( var==NULL || checkFuncObject(var,'9',0) ) return FALSE;
    return var->length() ? TRUE: FALSE;
}

bool WBox::remove( LPCC code ) {
    int rst= hash.remove(code);
    return rst==1? true : false;
}

int WBox::incrNum(LPCC code, int incr)
{
    int prev=0;
    StrVar* sv=GetVar(code);
    if( isNumberVar(sv) ) {
        prev=toInteger(sv);
    }
    setInt(code, prev+incr);
    return prev;
}

bool WBox::isNum(LPCC code) {
    StrVar* sv=gv(code);
    return isNumberVar(sv);
}

int WBox::getInt(LPCC code, int def) {
    StrVar* sv=gv(code);
    if( !isNumberVar(sv) ) return def;
    int v=toInteger(sv);
    if( sv->charAt(0)!='\b' ) sv->setVar('0',0).addInt(v);
    return v;
}

bool WBox::getBool(LPCC code, bool def) {
    StrVar* sv=gv(code);
    if( !checkObject(sv,'3') ) return def;
    return SVCHK('3',1);

}
U32 WBox::getU32(LPCC code, U32 def) {
    StrVar* sv=gv(code);
    if( !isNumberVar(sv) ) return def;
    U32 v=(U32)toInteger(sv);
    if( sv && sv->charAt(0)!='\b' ) sv->setVar('2',0).addU32(v);
    return v;
}
UL64 WBox::getLong(LPCC code, UL64 def) {
    StrVar* sv=gv(code);
    if( !isNumberVar(sv) ) return def;
    UL64 v=toUL64(sv);
    if( sv->charAt(0)!='\b' ) sv->setVar('1',0).addUL64(v);
    return v;
}
double WBox::getDouble(LPCC code, double def) {
    StrVar* sv=gv(code);
    if( !isNumberVar(sv) ) return def;
    double v= toDouble(sv);
    if( sv->charAt(0)!='\b' ) sv->setVar('4',0).addDouble(v);
    return v;
}

void WBox::setInt(LPCC code, int val) {
    GetVar(code)->setVar('0',0).addInt(val);
}
void WBox::setU32(LPCC code, U32 val) {
    GetVar(code)->setVar('2',0).addU32(val);
}
void WBox::setLong(LPCC code, UL64 val) {
    GetVar(code)->setVar('1',0).addUL64(val);
}
void WBox::setDouble(LPCC code, double val) {
    GetVar(code)->setVar('4',0).addDouble(val);
}
void WBox::setBool(LPCC code, bool val) {
    GetVar(code)->setVar('3', val? 1: 0 );
}
void WBox::setNull(LPCC code) {
    GetVar(code)->setVar('9',0);
}
bool WBox::cmp(LPCC field, LPCC val)
{
    LPCC str=NULL;
    StrVar* sv=gv(field);
    if( sv ) {
        if( sv->charAt(0)=='\b' ) {
            if( sv->charAt(1)=='w' || SVCHK('n',0) || SVCHK('a',0) ) {
                return false;
            }
            str=toString(sv);
        } else {
            str=sv->get();
        }
    }
    return ccmp(val,str);
}

//
// TreeNode
//
TreeNode::TreeNode(int sz, bool newInst) : WBox(sz), xtype(0), xstat(0), xparent(NULL) {
    xflag=0;
    if( newInst ) xflag|=FLAG_NEW;
}

TreeNode::~TreeNode() {    
    reset(true);
    xchilds.Close();
    xarrs.Close();
    Close();
}
bool TreeNode::reset(bool reuse) {
    // removeAll
    for(int n=0, cnt=xchilds.size(); n<cnt ; n++ ) {
        TreeNode* node=xchilds.getvalue(n);
        if( node->isNodeFlag(FLAG_NEW) ) {
            // qDebug("node child delete %d",n);
            SAFE_DELETE(node);
        }
    }

    if( reuse ) {
        /*
        StrVar* sv=gv("@setNodes");
        if(SVCHK('a',0)) {
            XListArr* arr=(XListArr*)SVO;
            for(int n=0;n<arr->size();n++) {
                sv=arr->get(n);
                if(SVCHK('n',0)) {
                    TreeNode* node=(TreeNode*)SVO;
                    if( node->isNodeFlag(FLAG_NEW) ) {
                        qDebug("node child delete %d",n);
                        SAFE_DELETE(node);
                    }
                }
            }
        }
        */
        for(WBoxNode* bn=this->First(); bn; bn=this->Next()) {
            bn->value.close();
        }
        for(int n=0,cnt=xarrs.size(); n<cnt ; n++ ) {
            XListArr* arr=xarrs.getvalue(n);
            if( arr->isNodeFlag(FLAG_NEW) ) {
                SAFE_DELETE(arr);
            }
        }
        xarrs.reuse();
        this->reuse();
    } else {
        StrVar* sv=NULL;
        for(WBoxNode* bn=this->First(); bn; bn=this->Next()) {
            sv=&(bn->value);
            if(SVCHK('n',0)) {
                sv->reuse();
            }
        }
    }
    xchilds.reuse();
    StrVar* var=gv("@lastIndex");
    if(var) var->reuse();
    return true;
}

bool TreeNode::remove(TreeNode* node) {
    for(int n=0,cnt=childCount();n<cnt;n++) {
        if( child(n)==node ) {
            if( xchilds.remove(n)==0 ) {
                qDebug("#9#node remove error xchilds.remove(n)==0\n");
                return false;
            }
            if(!node->isNodeFlag(FLAG_VARCHECK)) {
                incrNum("@lastIndex",-1);
            }
            return true;
        }
    }
    return false;
}

bool TreeNode::removeAll(bool del, bool check) {
    if( del ) {
        reset(check);
    } else {
        reset();
    }
    return true;
}

TreeNode* TreeNode::reuseAll(bool del) {
    for(WBoxNode* bn=this->First(); bn; bn=this->Next()) {
        if(!del) {
            LPCC code=getCurCode();
            if( slen(code)>1 && code[0]=='@' ) continue;
        }
        bn->value.close();
    }
    if( del) {
        TreeNode* node=NULL;
        int cnt=childCount();
        reuse();
        for(int n=0;n<cnt; n++) {
            node=child(n);
            if( node->isNodeFlag(FLAG_NEW) && node->gv("@this")==NULL ) {
                SAFE_DELETE(node);
            }
        }
        xchilds.reuse();
    } else {
        xchilds.reuse();
    }
    StrVar* var=gv("@lastIndex");
    if(var) var->reuse();
    return this;
}

int TreeNode::row() {
    TreeNode* p=parent();
    if( p==NULL || p==this )
        return 0;
    for(int n=0,cnt=p->childCount();n<cnt;n++) {
        if( p->child(n)==this ) return n;
    }
    return 0;
}

int TreeNode::find(TreeNode *node) {
    if( node==NULL ) return -1;
    for( int n=0; n<xchilds.size(); n++ ) {
        if( node==xchilds.getvalue(n) ) return n;
    }
    return -1;
}

int TreeNode::clone(TreeNode *srcNode, bool overwrite) {
    if( srcNode==NULL || srcNode==this ) return 0;
    int num=0;
    StrVar* sv=NULL;
    if(overwrite) {
        for( int n=0; n<srcNode->xchilds.size(); n++ ) {
            TreeNode* cur=srcNode->xchilds.getvalue(n);
            TreeNode* sub=addNode(NULL);
            sub->clone(cur, true);
        }
    } else {
        xchilds.reuse();
        for( int n=0; n<srcNode->xchilds.size(); n++ ) {
            TreeNode* cur=srcNode->xchilds.getvalue(n);
            xchilds.add(cur);
        }
    }
    GetVar("@lastIndex")->setVar('0',0).addInt(xchilds.size() );
    for( WBoxNode* bn=srcNode->First(); bn; bn=srcNode->Next() ) {
        LPCC code=srcNode->getCurCode();
        if(code[0]=='@') continue;
        if( !overwrite && isset(code) ) {
            continue;
        }
        sv=&(bn->value);
        if( overwrite ) {
            if(SVCHK('n',0)) {
                TreeNode* sub=(TreeNode*)SVO;
                if(sub->gv("@this")) {
                    GetVar(code)->reuse()->add(sv);
                } else {
                    int findIdx=srcNode->find(sub);
                    if(findIdx!=-1) {
                        sub=child(findIdx);
                        GetVar(code)->setVar('n',0,(LPVOID)sub);
                    } else {
                        TreeNode* cur=addNode(code);
                        cur->clone(sub, true);
                    }
                }
            } else if(SVCHK('a',0)) {
                XListArr* arr=(XListArr*)SVO;
                XListArr* cur=addArray(code);
                for(int n=0;n<arr->size(); n++) {
                    sv=arr->get(n);
                    cur->add()->add(sv);
                }
            } else {
                GetVar(code)->reuse()->add(sv);
            }
        } else {
            GetVar(code)->reuse()->add(sv);
        }
        num++;
    }
    return num;
}

TreeNode* TreeNode::addNode(LPCC code, bool reuse) {
    TreeNode* node=NULL;
    StrVar* sv=NULL;
    if( slen(code) ) {
        sv=gv(code);
        if( SVCHK('n',0) ) {
            node=(TreeNode*)SVO;
            /*
            if(node->isNodeFlag(FLAG_VARCHECK) ) {
                if( !find(node) && node->parent()==this ) {
                    xchilds.add(node);
                }
            }
            */
            if( reuse ) {
                node->removeAll(true, true);
            }
        }
    }
    if( node==NULL ) {
        node = new TreeNode(2, true);
        if( slen(code) ) {
            GetVar(code)->setVar('n',0,(LPVOID)node );
            node->setNodeFlag(FLAG_VARCHECK);
        } else {
            incrNum("@lastIndex");
        }
        node->xparent=this;
        xchilds.add(node);
    }
    return node;
}
TreeNode* TreeNode::getNode(LPCC code) {
    TreeNode* node=NULL;
    StrVar* sv=NULL;
    if( slen(code) ) {
        sv=gv(code);
        if( SVCHK('n',0) ) {
            node=(TreeNode*)SVO;
        }
    }
    return node;
}

TreeNode* TreeNode::setNode(LPCC code) {
    TreeNode* node=NULL;
    StrVar* sv=NULL;
    if( slen(code) ) {
        sv=gv(code);
        if( SVCHK('n',0) ) {
            node=(TreeNode*)SVO;
            node->reset(true);
        }
    }
    if( node==NULL ) {
        XListArr* arr=addArray("@setNodes", false);
        node = new TreeNode(2, true);
        if( slen(code)==0) {
            int idx=arr->size()+1;
            code=gBuf.fmt("setNode:%d",idx);
        }
        if( slen(code) ) {
            GetVar(code)->setVar('n',0,(LPVOID)node );
            node->setNodeFlag(FLAG_VARCHECK);
        }
        arr->add()->setVar('n',0,(LPVOID)node);
        node->xparent=this;
    }
    return node;
}

XListArr* TreeNode::getArray(LPCC code) {
    XListArr* arr=NULL;
    StrVar* sv=slen(code) ? gv(code):NULL;
    if( SVCHK('a',0) ) {
        arr=(XListArr*)SVO;
    }
    return arr;
}
XListArr* TreeNode::addArray(LPCC code, bool reuse) {
    XListArr* arr=NULL;
    StrVar* sv=slen(code) ? gv(code):NULL;
    if( SVCHK('a',0) ) {
        arr=(XListArr*)SVO;
        if( reuse ) arr->reuse();
    } else {
        arr=new XListArr(true);
        if( slen(code) ) {
            sv=GetVar(code);
            sv->setVar('a',0,(LPVOID)arr);
        }
        xarrs.add(arr);
    }
    return arr;
}
TreeNode* TreeNode::addChild(TreeNode* node) {
    if( node==NULL || node==this || find(node)!=-1 ) return node;
    xchilds.add(node);
    node->xparent=this;
    return node;
}


int TreeNode::setArrayValue(XListArr* arrs, XParseVar& pv, WBox* box ) {
    int rtn = 0;
    char ch = pv.ch();
    if( ch==']' ) {
        pv.incr();
        return rtn;
    }
    while( pv.valid() ) {
        StrVar* sv = arrs->add();
        if( setJsonValue(pv,sv,0,box)==0 )
            break;
        rtn++;
        ch = pv.ch();
        if( ch==']' ) {
            pv.incr();
            break;
        }
        if( ch==',' || ch==';' )
            pv.incr();
    }
    return rtn;
}

LPCC TreeNode::log(StrVar* rst, bool bChild, int depth) {
    if( rst==NULL ) rst=getVarTemp();
    TreeNode* node=this;
    StrVar* sv = NULL;
    int indent=depth+1;
    if( depth ) {
        rst->add("\n");
        for( int x=0;x<depth;x++) rst->add("\t");
    }
    rst->format(64,"{(type:%d,state:%d) ", node->xtype, node->xstat );

    int size = node->childCount();
    if( size && bChild==false ) rst->format(64,"childCount: %d, ", size);
    int num=0;
    for(WBoxNode* bn=node->First(); bn; bn=node->Next() ) {
        LPCC code=node->getCurCode();
        if( slen(code)==0 ) continue;
        if( num ) rst->add(", ");
        rst->add("\n");
        for( int x=0;x<indent;x++) rst->add("\t");
        num++;
        rst->add(code).add(": ");
        sv=&(bn->value);
        if( SVCHK('n',0) ) {
            TreeNode* cur=(TreeNode*)SVO;
            if( cur==node ) {
                rst->add("node");
                continue;
            }
        }
        toString(sv, rst, true, indent );        
    }
    if( size && bChild ) {
        rst->add("\n");
        for( int x=0;x<indent;x++) rst->add("\t");
        rst->format(64, "children(%d):[", size);
        for( int n=0;n<size;n++) {
            TreeNode* cur=node->child(n);
            cur->log(rst, true, indent);
        }
        rst->add("/]");
    }
    rst->add("\n");
    for( int x=0;x<depth;x++) rst->add("\t");
    rst->add("}");
    return rst->get();
}


int TreeNode::setJsonValue(XParseVar& pv, StrVar* sv, char ch, WBox* box, LPCC field) {
    bool achk = (ch==0 || ch=='[') ? true: false;
    if( ch==0 ) ch=pv.ch();
    if( ch=='{' ) {
        TreeNode* sub = NULL;
        if( SVCHK('n',0) ) {
            sub=(TreeNode*)SVO;
        } else {
            if(slen(field)) {
                sub=addNode(field);
            } else {
                sub=addNode();
                sv->setVar('n',0,(LPVOID)sub);
            }
        }
        sub->setJson(pv, box, true);
    } else if( ch=='<' ) {
        int sp= pv.start;
        StrVar* var=pv.GetVar();
        LPCC tag=pv.incr().NextWord();
        if( ccmp(tag,"data")|| ccmp(tag,"text")|| ccmp(tag,"json") ) {
            if( pv.ch()=='>' ) {
                pv.incr();
            } else {
                pv.findEnd(">", FIND_FORWORD);
            }
            if( ccmp(tag,"data") ) {
                pv.findEnd("</data>", FIND_FORWORD);
                sv->set(var,pv.prev,pv.cur);
            } else if( ccmp(tag,"text") ) {
                pv.findEnd("</value>", FIND_FORWORD);
                makeTextValueVar(var, pv.prev,pv.cur, sv, this );
            } else {
                XParseVar pvCur(var,pv.prev,pv.cur);
                pv.findEnd("</json>", FIND_FORWORD);
                nodeJsonData(pvCur, this, sv);
            }
        } else if( ccmp(tag,"page") ) {
            pv.start=sp;
            if( pv.match("<page","</page>") ) {
                TreeNode* root=new TreeNode(2, true);
                // xobjects.add()->setVar('n',0,(LPVOID)root );
                xchilds.add(root);
                root->setNodeFlag(FLAG_VARCHECK);
                if( sv ) {
                    sv->setVar('n',0,(LPVOID)root );
                }
                root->set("tag", "page");
                sp=parseProp(root, var, pv.prev,pv.cur);
                if( sp!=-1 ) parsePageNode(root, var, sp, pv.cur);
                _LOG("jsonValue page: %s\n", root->log() );
            }
        } else if( ccmp(tag,"xml") ) {
            pv.start=sp;
            if( pv.match("<xml","</xml>") ) {
                TreeNode* root=new TreeNode(2, true);
                root->xparent=NULL;
                // xobjects.add()->setVar('n',1,(LPVOID)root );
                xchilds.add(root);
                root->setNodeFlag(FLAG_VARCHECK);
                if( sv ) {
                    sv->setVar('n',0,(LPVOID)root );
                }
                sp=parseProp(root, var, pv.prev, pv.cur);
                if( sp!=-1 ) parseXml(root, var, sp, pv.cur);
            }
        } else if( ccmp(tag,"layout") ) {
            pv.start=sp;
            if( pv.match("<layout","</layout>") ) {
                WidgetConf* root=new WidgetConf();
                root->xparent=NULL;
                // xobjects.add()->setVar('n',1,(LPVOID)root );
                xchilds.add(root);
                root->setNodeFlag(FLAG_VARCHECK);
                if( sv ) {
                    sv->setVar('n',1,(LPVOID)root );
                }
                int idx=1;
                sp=parseProp(root, var, pv.prev,pv.cur);
                if( sp!=-1 ) parsePage(root, var, sp, pv.cur, root, &idx);
                _LOG("layout last index: %d (sp:%d)\n", idx, sp);
            }
        } else  {
            pv.findEnd(">", FIND_FORWORD|FIND_SKIP);
            sv->set(var,pv.prev,pv.cur);
            // setNodeTagValue(tag, sv, pv.GetVar(),pv.prev,pv.cur);
        }
    } else if( ch=='[' ) {
        pv.start++;
        XListArr* arr = NULL;
        if( SVCHK('a',0) ) {
            arr=(XListArr*)SVO;
            arr->reuse();
        } else {
            arr=new XListArr();
            sv->setVar('a',0,(LPVOID)arr);
            // xobjects.add()->setVar('a',0,(LPVOID)arr);
            xarrs.add(arr);
        }
        setArrayValue(arr,pv,box);
    } else if( (ch=='\'' || ch=='\"')  && pv.MatchStringPos(ch) ) {
        if( ch=='"' && isNodeFlag(FLAG_RESULT) ) {
            XParseVar sub(pv.GetVar(),pv.prev,pv.cur);
#ifdef FUNCNODE_USE
            XFuncNode* fn=static_cast<XFuncNode*>(box);
            makeTextData(sub,fn,sv);
            if( fn ) {
                getStringTypeValue(sv, 0, sv->length(), getStrVarTemp(), true);
            }
#else
            makeTextData(sub,NULL,sv);
            if( fn ) {
                getStringTypeValue(sv, 0, sv->length(), getStrVarTemp(), true);
            }
#endif
        } else {
            int size = 0;
            LPCC val = pv.left(&size);
            sv->set(val, size);
        }
    } else  {
        LPCC val=NULL;
        int sp = pv.start, ep = 0, end = 0;
        if( ch=='-' || ch=='+' ) {
            pv.incr();
        }
        pv.moveNext();
        end = ep = pv.start;
        ch = pv.ch();
        if( ch=='.' || ch=='=' ) {
            while( ch=='.' ) {
                ch=pv.incr().moveNext().ch();
            }
            end = pv.start;
        }
        ch = pv.ch();
        if( ch=='\0' || ch==',' ) {
            ep=pv.start;
            val=pv.Trim(sp,ep);
        } else {
            pv.start = sp;
            int ep = pv.find("\n");
            if( ep==-1 ) {
                ep = pv.wep;
            }
            int a = pv.find("{",NULL,0,sp,ep);
            if( a==-1 ) {
                a = pv.find("[",NULL,0,sp,ep);
                if(a==-1) a=pv.find(achk?"]":"}",NULL,0,sp,ep);
            }
            int pos = ep;
            if( a!=-1 ) {
                int b = pv.find(",",NULL,FIND_SKIP,sp,ep);
                pos = b==-1 ? a: min(a,b);
            } else {
                int b = pv.find(",",NULL,FIND_SKIP,sp,ep);
                pos = b==-1 ? ep: b;
            }
            if( pos!=-1 )
                ep = pos;
            pv.start = ep;
            val=pv.Trim(sp,ep);
        }
        if( ccmp(val,"0") ) {
            sv->setVar('0',0).addInt(0);
        } else if( ccmp(val,"true") || ccmp(val,"false") ) {
            sv->setVar('3', ccmp(val,"true")?1: 0);
        } else {
            sv->set( val );
        }
    }
    return pv.valid() ? 1: 0;
}

int TreeNode::setJson(StrVar* data, int sp, int ep ) {
    XParseVar pv(data,sp,ep);
    return setJson(pv,NULL);
}
inline bool isFuncCheck(StrVar* var, int sp, int ep) {
    XParseVar pv(var, sp, ep);
    char ch=pv.ch();
    if(ch=='(') {
        if( pv.MatchWordPos("(",")",FIND_SKIP) ) {
            ch=pv.ch();
        } else {
            return false;
        }

    } else {
        ch=pv.moveNext().ch();
        while(ch==',') {
            ch=pv.moveNext().ch();
        }
    }
    if(ch=='=' && pv.ch(1)=='>') {
        return true;
    }
    return false;
}
int TreeNode::setJson(XParseVar& pv, WBox* box, bool bchk ) {
    char ch = pv.ch();
    if( ch=='{' ) {
        pv.incr();
    } else if( ch=='[' ) {
        setJsonValue(pv, GetVar("@array"), ch, box);
    }
    int sp=0, num=0, begin=0;
    LPCC next = NULL;
    while( pv.valid() ) {
        ch = pv.ch();
        if( ch=='}' ) {
            pv.incr();
            break;
        }
        if(ch=='{') {
            bool ok=true;
            while(ch=='{') {
                TreeNode* sub = addNode();
                int start=pv.start;
                if( pv.MatchWordPos("{","}",FIND_SKIP) ) {
                    XParseVar var(pv.GetVar(), start, pv.start);
                    sub->setJson(var, box, true);
                    ch=pv.ch();
                    if(ch==','||ch==';' ) {
                       ch=pv.incr().ch();
                    }
                } else {
                    ok=false;
                    break;
                }
            }
            if(!ok) {
                qDebug("#0#****************** node setJson match error ******************");
                break;
            }
            if( ch=='}' ) {
                pv.incr();
                break;
            }
        }
        sp=begin=pv.start;        
        next = NULL;
        if( (ch=='\'' || ch=='\"')  && pv.MatchStringPos(ch) ) {
            next = gBuf.add(pv.get(pv.prev), pv.cur-pv.prev );
        } else {
            if( ch=='@' || ch=='/' ) pv.incr();
            pv.moveNext();
            ch=pv.ch();
            while( ch=='-' || ch=='.' || ch=='@' || ch=='#' || ch=='/' ) {
                ch=pv.incr().moveNext().ch();
            }
            next = pv.Trim(sp, pv.start);
        }
        if( slen(next)==0 )
            break;
        ch = pv.ch();
        num++;
        if( ch==0 || ch==',' || ch=='}' || ch==']' ) {
            if(box && isNodeFlag(FLAG_RESULT) ) {
                int sp=0, ep=0;
                StrVar* var=getStrVarPointer(box->gv(next), &sp, &ep);
                GetVar(next)->reuse()->add(var, sp, ep);
            }
            if( ch==',' || ch=='}' || ch==']' ) {
                pv.incr();
                if( ch==',') continue;
                break;
            }
            if( ch==0 )
                break;
        } else if( ch==':' || ch=='=' || ch=='{' || ch=='[' || ch=='<' ) {
            StrVar* sv=NULL;
            if( ch==':' || ch=='=' ) {
                ch = pv.incr().ch();
            }
            if( isNodeFlag(FLAG_SKIP) ) {
                sv=gv(next);
            }
            if( sv==NULL ) {
                sv=GetVar(next);
                if((ch=='#'||ch=='V') && pv.ch(1)=='[') {
                    pv.incr();
                    if( pv.MatchWordPos("[","]",FIND_SKIP) ) {
                        if(ch=='#') {
                            makeTextValueVar(pv.GetVar(),pv.prev,pv.cur,sv,this);
                        } else {
                            sv->add(pv.GetVar(),pv.prev,pv.cur);
                        }
                    } else {
                        qDebug("#9#parse json [%s] string not match", next );
                    }
                } else if(ch=='@') {
                    LPCC varName=pv.incr().NextWord();
                    XFuncNode* fn=(XFuncNode*)box;
                    StrVar* var= NULL;
                    StrVar* rst= sv->reuse();
                    if( slen(varName) && fn ) {
                        int sp=0, ep=0;
                        sv=fn->gv("@targetPage");
                        if(SVCHK('n',0)) {
                            TreeNode* target=(TreeNode*)SVO;
                            sv=target->gv("onInit");
                            if(SVCHK('f',0)) {
                                XFuncNode* fnInit=(XFuncNode*)SVO;
                                var=fnInit->gv(varName);
                            }
                        }
                        if(var==NULL ) {
                            var=getStrVarPointer(getFuncVar(fn,varName), &sp, &ep);
                        } else {
                            var=getStrVarPointer(var, &sp, &ep);
                        }
                        if(sp<ep) rst->add(var, sp, ep);
                    }
                } else if( setJsonValue(pv,sv,ch,box,next)==0 ) {
                    // qDebug("#0#setJson %s parse Error =============== ",next);
                    break;
                }
            }
            if( pv.ch()=='}' ) {
                pv.incr();
                if( bchk ) {
                    return 0;
                }
            }
        } else if( ch=='(' ) {
            sp=pv.start;
            if( pv.MatchWordPos("(",")",FIND_SKIP) ) {
                ch=pv.ch();
                if( ch=='{' && pv.MatchWordPos("{","}",FIND_SKIP) ) {
#ifdef NODEFUNC_USE
                    setMemberSource(this, NULL, pv.GetVar(), begin, pv.start, StartWith(next,"on")?"event":"inline");
#else
                    StrVar* sv=NULL;
                    if( isNodeFlag(FLAG_SKIP) ) {
                        sv=gv(next);
                    }
                    if( sv==NULL ) {
                        sv=GetVar(next);
                        sv->reuse();
                        sv->add(pv.GetVar(), sp, pv.start);
                    }
#endif
                } else {
                    qDebug("node object function not match [name:%s]", next);
                }
            } else {
                break;
            }
        } else {
            break;
        }
        ch = pv.ch();
        if( ch==',' || ch==';' || ch=='}' ) {
            pv.incr();
            if( ch=='}' )
                break;
        }
    }
    return num;
}

int _intVal(LPCC val, int def) { return isnum(val) ? atoi(val) : def; }

bool parsePageNode(TreeNode* root, StrVar* var, int sp, int ep, TreeNode* base, U32 flags, StrVar* rst) {
    char ch=0;
    int pos=0;
    LPCC tag=NULL;
    TreeNode* cur=NULL;
    XParseVar pv(var, sp, ep);
    /*
    if( flags&PROP_SOURCE && rst==NULL ) {
        return false;
    }
    int idx=0;
    */
    while( pv.valid() ) {        
        ch = pv.ch();
        if( ch=='/' && pv.ch(1)=='*' ) {
            if( !pv.match("/*","*/",FLAG_SKIP ) ) {
                break;
            }
            continue;
        }
        if( ch!='<') break;
        sp = pv.start;
        ch = pv.incr().ch();
        if( ch=='/' || ch=='!' ) {
            if( ch=='/') {
                pv.findEnd(">");
            } else {
                pv.findEnd("-->");
            }
            continue;
        }        
        tag = pv.NextWord();
        if( slen(tag)==0 ) {
            break;
        }
        /*
        if( flags&PROP_SOURCE ) {
            cur=root->child(idx++);
            if( cur==NULL || !cur->cmp("tag", tag) ) break;
            pos = parseProp(cur, pv.GetVar(), pv.start, pv.wep, flags, rst );
        } else */
        {
            cur = root->addNode();
            cur->set("tag",	tag);
            pos = parseProp(cur, pv.GetVar(), pv.start, pv.wep);
        }
        if( pos==-1 ) {
            qDebug("#9#parsePageNode prop parse error tag:%s\n",tag);
            break;
        }
        /* match tag use
            ccmp(tag,"canvas") || ccmp(tag,"splitter") || ccmp(tag,"div") || ccmp(tag,"tab")
            if( ccmp(tag,"canvas") ) {
                XParseVar sub(pv.GetVar(),pos, pv.cur);
                if( sub.ch()=='<' ) {
                    cur->GetVar("@layout")->set(pv.GetVar(), pos, pv.cur);
                }
            }
        */
        if( ccmp(tag,"vbox") || ccmp(tag,"hbox") || ccmp(tag,"form") || ccmp(tag,"row") || ccmp(tag,"group") )
        {
            pv.setPos(sp);
            if( pv.match(FMT("<%s",tag),FMT("</%s>",tag),FIND_SKIP) ) {
                parsePageNode(cur, pv.GetVar(), pos, pv.cur, base, flags, rst);
            } else {
                qDebug("#9#pagesePage tag match error (tag:%s)\n",tag);
                break;
            }
        } else {
            pv.setPos(pos);
            ch = pv.ch();
            if(ch==0) break;
            if(ch!='<' ) {
                pv.setPos(sp);
                if( pv.match(FMT("<%s",tag),FMT("</%s>",tag),FIND_SKIP) ) {
                    cur->GetVar("@source")->set(pv.GetVar(), pos, pv.cur);
                } else {
                    qDebug("#9#pagesePage tag source match error (tag:%s)\n",tag);
                    break;
                }
            } else {
                // qDebug("#0#parsePageNode single tag: %s (ch:%c)",tag, pv.ch());
            }
        }
    }
    return true;
}
