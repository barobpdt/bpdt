#include "func_util.h"
#include "modbus_device.h"


bool callModbusFunc(LPCC fnm, PARR arrs, TreeNode* tn, XFuncNode* fn, StrVar* rst, XFunc* fc) {
    ModbusDevice* md=NULL;
    StrVar* sv=tn->gv("@c");
    if( SVCHK('m',11) ) {
        md=(ModbusDevice*)SVO;
    } else {
        md=new ModbusDevice(tn,1);
        qDebug("callModbusFunc new modbus device: func:%s ", fnm);
        tn->GetVar("@c")->setVar('m',11,(LPVOID)md);
    }
    U32 ref = fc->xfuncRef;
    if( ref==0 ) {
        ref =
            ccmp(fnm,"connect") ? 1: ccmp(fnm,"isConnect") ? 2:
            ccmp(fnm,"read") ? 3: ccmp(fnm,"write") ? 4:
            ccmp(fnm,"close") ? 5: ccmp(fnm,"serverId") ? 6:
            ccmp(fnm,"info")? 7: ccmp(fnm,"is")? 8:
            0;
        fc->xfuncRef = ref;
    }
    switch( ref ) {
    case 1: { // connect
        if( arrs==NULL ) {
            rst->setVar('3',md->isConnect()?1:0);
            return true;
        }
        LPCC uri=AS(0);
        rst->setVar('3',0);
        if( slen(uri) ) {
            sv=arrs->get(1);
            if( isNumberVar(sv) ) {
                tn->setInt("@serverId", toInteger(sv));
            }
            md->connectServer(uri);
            rst->setVar('3',md->isConnect()?1:0);
        }
    } break;
    case 2: { // isConnect
        rst->setVar('3',md->isConnect()?1:0);
    } break;
    case 3: { // read
        if( arrs==NULL ) return true;
        LPCC regType=AS(3);
        // regType: coil, input, discrete, holding
        if( slen(regType)==0 ) regType="holding";
        int address=toInteger(arrs->get(0)), count=toInteger(arrs->get(1));
        if( address>=0 && count>0 ) {
            md->sendRead(regType, address, count );
        }
    } break;
    case 4: { // write
        if( arrs==NULL ) return true;
        LPCC regType=AS(3);
        // regType: coil, input, discrete, holding
        if( slen(regType)==0 ) regType="holding";
        int address=toInteger(arrs->get(0));
        sv=arrs->get(1);
        if( address>0 && SVCHK('a',0) ) {
            XListArr* dataArr=(XListArr*)SVO;
            md->sendWrite(regType, address, dataArr );
        }
    } break;
    case 5: { // close
        if( md->isConnect(false) ) {
            md->disconnectServer();
            rst->setVar('3',1);
        } else {
            rst->setVar('3',0);
        }
    } break;
    case 6: { // serverId
        if( arrs==NULL ) {

            rst->setVar('0',0).addInt(tn->getInt("@serverId"));
            return true;
        }
        sv=arrs->get(0);
        if( isNumberVar(sv) ) {
            int sid=toInteger(sv);
            if( sid ) {
                tn->setInt("@serverId",sid);
            }
        }

    } break;
    case 7: { // info
        rst->add(FMT("status:%d, state:%s, address:%d, server:%d ",
                     tn->getInt("@status"),
                     md->stateText(),
                     tn->getInt("@currentAddress"),
                     tn->getInt("@serverId")));
    } break;
    case 8: { // status
        if( arrs==NULL ) {
            // notset, connect, readStart, readSend, readFinish, writeStart, writeSend, writeFinish, error
            switch(tn->getInt("@status")) {
            case 0: rst->add("wait"); break;
            case 1: rst->add("connect"); break;
            case 2: rst->add("readStart"); break;
            case 3: rst->add("readSend"); break;
            case 4: rst->add("readFinish"); break;
            case 12: rst->add("writeStart"); break;
            case 13: rst->add("writeSend"); break;
            case 14: rst->add("writeFinish"); break;
            case 99: rst->add("error"); break;
            }
            return true;
        }

    } break;
    default:
        return false;
    }
    return true;
}
