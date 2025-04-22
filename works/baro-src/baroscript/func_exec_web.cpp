#include "func_util.h"
#include "HttpConnection.h"
#ifdef WEBVIEW_USE
#include "CutyCapt.h"
#endif

bool callWebFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc) {
    StrVar* sv=tn->GetVar("@c");
    HttpConnection* w = NULL;
    if( SVCHK('h',0) ) {
        w=(HttpConnection*)SVO;
    } else {
        w=new HttpConnection(tn);
        tn->GetVar("@c")->setVar('h',0,(LPVOID)w);
    }
    U32 ref=fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        ref=
            ccmp(fnm,"call") ? 1 :
            ccmp(fnm,"timeout") ? 2 :
            ccmp(fnm,"header") ? 3 :
            ccmp(fnm,"download") ? 4 :
            ccmp(fnm,"upload") ? 5 :
            ccmp(fnm,"capture") ? 6 :
            ccmp(fnm,"isRun") ? 7 :
            ccmp(fnm,"close") ? 8 :
            ccmp(fnm,"readData") ? 9 :
            ccmp(fnm,"request") ? 10 :
            ccmp(fnm,"stop") ? 11 :
            ccmp(fnm,"param") ? 12 :
            ccmp(fnm,"is") ? 13 :
            ccmp(fnm,"captureCreate") ? 14 :
			ccmp(fnm,"callback") ? 15 :
            ccmp(fnm,"uploadFile") ? 16 :
			0;
        if( fc ) fc->xfuncRef=ref;
    }
    switch( ref ) {
    case 1:  // call
    case 10: { // request
        if( arrs==NULL ) return false;
        int sz=arrs->size();
        LPCC url=AS(0);
        TreeNode* thisNode=NULL;
        XFuncNode* fnCallback=NULL;
        tn->set("method","GET");
        tn->set("url", url);
        for(int n=1;n<sz;n++) {
            sv=arrs->get(n);
            if( SVCHK('f',1) ) {
                fnCallback=setWebCallback(w, url, tn, fn, sv, rst);
                if( thisNode && fnCallback ) {
                    fnCallback->GetVar("sendor")->setVar('n',0,(LPVOID)tn);
                    fnCallback->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                }
            } else if( SVCHK('n',0) ) {
                thisNode=(TreeNode*)SVO;                
                if( fnCallback ) {
                    fnCallback->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                }
            } else if( sv ) {
                tn->set("method", toString(sv));
            }
        }

        if( w->call() ) {
            rst->setVar('3',1);
        }
    } break;
    case 2: { // timeout
        if( arrs==NULL ) {
            int timeout=toInteger(tn->gv("@timeout") );
            rst->setVar('0',0).addInt(timeout);
        } else if( isNumberVar(arrs->get(0)) ) {
            int timeout=toInteger(arrs->get(0));
            tn->GetVar("@timeout")->setVar('0',0).addInt(timeout);
        }
    } break;
    case 3: { // header
        TreeNode* header=tn->addNode("@header");
        if( arrs && arrs->size()==2 ) {
            LPCC code=AS(0);
            int sp=0, ep=0;
            sv=getStrVarPointer(arrs->get(1), &sp, &ep);
            header->GetVar(code)->set(sv,sp,ep);
        }
        rst->setVar('n',0,(LPVOID)header);
    } break;
    case 4: { // download
        if( arrs==NULL ) return false;
        LPCC url=AS(0);
        tn->set("fileName", AS(1));
        TreeNode* thisNode=NULL;
        XFuncNode* fnCallback=NULL;
        for(int n=2;n<arrs->size();n++) {
            sv=arrs->get(n);
            if( SVCHK('f',1) ) {
                fnCallback=setWebCallback(w, url, tn, fn, sv, rst);
                if( thisNode && fnCallback ) {
                    fnCallback->GetVar("sendor")->setVar('n',0,(LPVOID)tn);
                    fnCallback->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                }
            } else if( SVCHK('n',0) ) {
                thisNode=(TreeNode*)SVO;
                if( fnCallback ) {
                    fnCallback->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                }
            } else {
                tn->set("method", toString(sv));
            }
        }
        tn->set("type", "download");
        if( w->call() ) {
            rst->setVar('3',1);
        }
    } break;
    case 5: { // upload
        if( arrs==NULL ) return false;
        // web.upload(url, fileName, callback)
        // web.upload(url, fileName, method, callback)
        LPCC url=AS(0);
        tn->set("fileName", AS(1));
        TreeNode* thisNode=NULL;
        XFuncNode* fnCallback=NULL;
        for(int n=2;n<arrs->size();n++) {
            sv=arrs->get(n);
            if( SVCHK('f',1) ) {
                fnCallback=setWebCallback(w, url, tn, fn, sv, rst);
                if( thisNode && fnCallback ) {
                    fnCallback->GetVar("sendor")->setVar('n',0,(LPVOID)tn);
                    fnCallback->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                }
            } else if( SVCHK('n',0) ) {
                thisNode=(TreeNode*)SVO;
                if( fnCallback ) {
                    fnCallback->GetVar("@this")->setVar('n',0,(LPVOID)thisNode);
                }
            } else {
                tn->set("method", toString(sv));
            }
        }
        tn->set("type", "upload");
        tn->set("fileName", AS(1));
        if( w->call() ) {
            rst->setVar('3',1);
        }
    } break;
    case 7: { // isRun
        if( w->xrun ) {
            rst->setVar('3',1);
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 8: { // close
        w->setStop();
    } break;
    case 9: { // method
    } break;
    case 11: { // stop
        w->setStop();
    } break;
    case 12: { // param
        TreeNode* params=tn->addNode("@params");
        if( arrs && arrs->size()==2 ) {
            LPCC code=AS(0);
            int sp=0, ep=0;
            sv=getStrVarPointer(arrs->get(1), &sp, &ep);
            params->GetVar(code)->set(sv,sp,ep);
        }
        rst->setVar('n',0,(LPVOID)params);
    } break;
    case 13: { // is
        if( arrs==NULL ) {
            rst->set("run, start, download, upload");
            return true;
        }
        LPCC ty= AS(0);
        if( ccmp(ty,"run") ) {
            rst->setVar('3', tn->isNodeFlag(FLAG_RUN) && tn==w->config() ? 1 : 0 );
        } else if( ccmp(ty, "start") ) {
            rst->setVar('3', tn->isNodeFlag(FLAG_START) ? 1 : 0 );
        } else if( ccmp(ty, "download") ) {
            rst->setVar('3', tn->isNodeFlag(HTTP_DOWNLOAD) ? 1 : 0 );
        } else if( ccmp(ty, "upload") ) {
            rst->setVar('3', tn->isNodeFlag(HTTP_UPLOAD) ? 1 : 0 );
        }
    } break;
    case 6: { // capture
#ifdef WEBVIEW_USE
        /*
        [example]
        node={
            url: http://www.templatemonster.com/demo/58282.html,
            minWidth: 1200,
            minHeight: 2600,
            delay: 1000,
            output: c:/temp/web_capture.jpg
        };
        Class.web().capture(node, callback() {
            System.open(node.output);
        });
        */
        if( arrs==NULL ) {
            return true;
        }
        XFuncSrc* fsrc=NULL;
        rst->reuse();
        sv=arrs->get(0);
        tn->GetVar("@captureEnd")->setVar('3',0);
        if( SVCHK('n',0) ) {
            TreeNode* cf=(TreeNode*)SVO;
            LPCC url = cf->get("url");
            sv=arrs->get(1);
            if( SVCHK('f',1) ) {
                fsrc=(XFuncSrc*)SVO;
            }
            qDebug("#0#web page capture 01 %s\n", url);
            if( slen(url)==0 ) {
                qDebug("#9#web page capture error url not define (node: %s)\n", cf->log() );
                return true;
            }
            int delay = 0;
            int minWidth = 800;
            int minHeight = 600;
            LPCC script=NULL, propName=NULL;
            LPCC format=NULL;
            LPCC output	= "memory";
            sv=cf->gv("minWidth");		if( isNumberVar(sv) ) minWidth=toInteger(sv);
            sv=cf->gv("minHeight");		if( isNumberVar(sv) ) minHeight=toInteger(sv);
            sv=cf->gv("delay");			if( isNumberVar(sv) ) delay=toInteger(sv);
            sv=cf->gv("output");		if( sv ) output=toString(sv);
            sv=cf->gv("script");
            // x,y, width, height
            if( sv ) {
                script=toString(sv);
            }
            sv=cf->gv("propName");
            if( sv ) {
                propName=toString(sv);
            }
            sv=cf->gv("format");
            if( sv ) {
                format=toString(sv);
            } else {
                int pos=IndexOf(output,'.');
                if( pos>0 ) {
                    format=output+pos+1;
                }
            }
            /* capture start */
            if( !cf->isNodeFlag(FLAG_RUN) ) {
                cf->setNodeFlag(FLAG_RUN);
                CutyPage page;
                QNetworkAccessManager::Operation method = QNetworkAccessManager::GetOperation;
                QNetworkRequest req;
                /*
                NetworkAccessManager manager;
                page.setNetworkAccessManager(&manager);
                page.setAttribute(QWebSettings::JavascriptEnabled, "on");
                */
                LPCC outputFileName=slen(output)?getFullFileName(output):"";
                CutyCapt main(&page, KR(outputFileName), delay, CutyCapt::PngFormat,
                    propName ? KR(propName) : QString(),
                    script ? KR(script): QString(),
                    false,		// ssl error
                    true		// smoth
                );
                req.setUrl( QUrl::fromEncoded(url) );
                QObject::connect(&page, SIGNAL(loadFinished(bool)), &main, SLOT(DocumentComplete(bool)));
                page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
                page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
                page.setViewportSize( QSize(minWidth, minHeight) );
                QObject::connect(page.mainFrame(), SIGNAL(initialLayoutCompleted()), &main, SLOT(InitialLayoutCompleted()));
                if( slen(script) || slen(propName) ) {
                    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
                    QObject::connect(page.mainFrame(),SIGNAL(javaScriptWindowObjectCleared()), &main, SLOT(JavaScriptWindowObjectCleared()));
                }
                if(fsrc) {
                    main.setCallback(setCallbackFunction(fsrc, fn, tn, main.getCallback()) );
                }
                tn->GetVar("@captureInfo")->setVar('n',0,(LPVOID)cf);
                main.setFormat(format);
                main.setConfig(cf);
                QEventLoop loop;
                QTimer timer;
                cf->GetVar("@loop")->setVar('e',11,(LPVOID)&loop);
                int interval=35000;
                sv=cf->gv("timeout"); if( isNumberVar(sv) ) interval=toInteger(sv);
                timer.setInterval(interval);
                timer.setSingleShot(true);
                QObject::connect(&timer, SIGNAL(timeout()), &main, SLOT(Timeout()));
                timer.start();                
                page.mainFrame()->load(req, method);
                loop.exec();
                rst->setVar('3,',1);                
                qDebug("#0#web page capture ok (url:%s)\n", url);
            } else {
                qDebug("#0#web page capture cancel (url:%s)\n", url);
            }
        }
        tn->GetVar("@captureEnd")->setVar('3',1);
#endif
    } break;
    case 14: { // captureCreate

    } break;
	case 15: { // callback
		if( arrs==NULL ) {
			if(w->xfnCur) {
				rst->setVar('f',0,(LPVOID)w->xfnCur);
			} else {

			}
			return true;
		}
		if( SVCHK('f',1) ) {
			setWebCallback(w, NULL, tn, fn, sv, rst);
		}
	} break;
    case 16: { // uploadFile
        if( arrs==NULL ) return false;
        LPCC url=NULL, fileName=NULL, type=NULL, field=NULL;
        for(int n=0;n<arrs->size();n++) {
            sv=arrs->get(n);
            if( SVCHK('f',1) ) {
                setWebCallback(w, url, tn, fn, sv, rst);
            } else {
                if(url==NULL) url=toString(sv);
                else if(fileName==NULL) fileName=toString(sv);
                else if(type==NULL) type=toString(sv);
                else if(field==NULL) field=toString(sv);
            }
        }
        StrVar* err=tn->GetVar("@error");
        if(slen(url)==0 ) {
            err->add("upload URL not define");
        } else if(slen(fileName)==0 ) {
            err->add(fileName).add("upload file not define");
        } else {
            LPCC name=NULL;
            int pos=LastIndexOf(fileName,'/');
            if(pos!=-1 ) {
                name=fileName+pos+1;
            } else {
                pos=LastIndexOf(fileName,'\\');
                if(pos!=-1) name=fileName+pos+1;
                else name=fileName;
            }
            QFileInfo fi(KR(fileName));
            if(fi.isFile() ) {
                // type ==>
                // multiPartParam ==>
                if(slen(field)==0) field="uploadFile";
                if(slen(type)==0) type="image/jpeg";
                tn->set("method","POST");
                tn->set("type", "upload");
                tn->set("fileName", fileName);
                tn->setBool("multiPart", true);
                tn->set("multiPartType", type);
                tn->set("multiPartParam", gBuf.fmt("form-data; name=\"%s\"; filename=\"%s\"",field,name));
                if( w->call() ) {
                    rst->setVar('3',1);
                }
            } else {
                err->add(fileName).add(" file not found");
            }
        }
        rst->setVar('3',err->length()? 0: 1);
    } break;
    default: return false;
    }
    return true;
}

XFuncNode* setWebCallback(HttpConnection* w, LPCC url, TreeNode* tn, XFuncNode* fn, StrVar* callback, StrVar* rst ) {
    if( tn->isNodeFlag(FLAG_RUN) && tn==w->config() ) {
        qDebug("#0##web callback error (url:%s)\n", url);
        return NULL;
    }    
    if( slen(url) ) {
        tn->set("url", url);
	}
	/*
	else {
        url=tn->get("url");
        if( slen(url)==0 ) {
            qDebug("#9##web callback error: url is null\n");
            return NULL;
        }
    }
	*/
    rst->setVar('n',0,(LPVOID)tn);
    XFuncNode* fnCur=NULL;
    StrVar* sv=callback;
    if( SVCHK('f',1) ) {
        XFuncSrc* fsrc=(XFuncSrc*)SVO;
        fnCur=setCallbackFunction(fsrc, fn, tn, NULL, tn->GetVar("@callback"));
        w->xfnCur = fnCur;
        qDebug("web callback function ok (url:%s)\n", url);
    }
    return fnCur;
}

