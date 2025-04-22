#include "func_util.h"
#include "tmsmqclient.h"


bool callMqFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc) {
    StrVar* sv=tn->gv("@c");
    TmsMqClient* client=NULL;
    if( SVCHK('v',3) ) {
        client=(TmsMqClient*)SVO;
    } else {
        client = new TmsMqClient(tn);
        tn->GetVar("@c")->setVar('v',3,(LPVOID)client);
    }
    U32 ref = fc ? fc->xfuncRef: 0;
    if( ref==0 ) {
        ref=
            ccmp(fnm,"connect") ? 1 :
            ccmp(fnm,"send")    ? 2 :
            ccmp(fnm,"disconnect")    ? 3 :
            ccmp(fnm,"state")   ? 4 : 0;
    }
    if( ref==0 )
        return false;

    switch( ref ) {
    case 1: {		// connect
        if(arrs==NULL ) {
            rst->setVar('0',0).addInt(client->getState());
            return true;
        }
        LPCC host=AS(0), userName=AS(2), userPassword=AS(3);
        int port=toInteger(arrs->get(1));
        if( slen(host)>0 && port>0 ) {
            sv=arrs->get(4);
            if(SVCHK('f',1) ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                setCallbackFunction(fsrc, fn, tn, NULL, tn->GetVar("onRecv"));
            }
            sv=arrs->get(5);
            if(SVCHK('f',1) ) {
                XFuncSrc* fsrc=(XFuncSrc*)SVO;
                setCallbackFunction(fsrc, fn, tn, NULL, tn->GetVar("onChange"));
            }
            client->connMqServer(host, port, userName, userPassword);
            qDebug("mq connect %s %d %s %s",host, port, userName, userPassword);
        }
    } break;
    case 2: {		// send
        if(arrs==NULL ) {
            return false;
        }
        int size=arrs->size();
        LPCC data=AS(0);
        if(size==1) {
            client->mqSend(KR(data));
        } else if(size==2) {
            QMqttTopicFilter topic(KR(data));
            quint8 qos=(quint8)toInteger(arrs->get(1));
            client->mqSend( topic, qos);
        }
    } break;
    case 3: {		// disconnect
        client->disConnMqServer();
    } break;
    case 4: {		// state
        rst->setVar('0',0).addInt(client->getState());
    } break;
    default:
        return  false;
    }
    return true;
}

