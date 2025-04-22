#include "modbus_device.h"
#ifdef QT5_USE
static float twoRegsToFloat(const QModbusDataUnit& dataUnit)
{
    union UValue {
        float f;
        unsigned short w[2];
    } v;

    v.w[0] = dataUnit.value(0);
    v.w[1] = dataUnit.value(1);

    return v.f;
}
inline LPCC registerTypeText(const QModbusDataUnit& dataUnit, StrVar* rst) {
    if( rst==NULL ) return NULL;
    switch( dataUnit.registerType() ) {
    case QModbusDataUnit::Invalid:  rst->set("invalid"); break;
    case QModbusDataUnit::DiscreteInputs:   rst->set("discrete"); break;
    case QModbusDataUnit::Coils:    rst->set("coil"); break;
    case QModbusDataUnit::InputRegisters:   rst->set("input"); break;
    case QModbusDataUnit::HoldingRegisters: rst->set("holding"); break;
    default: rst->reuse();
    }
    return rst->get();
}
inline void dataUnitValue(const QModbusDataUnit& dataUnit, StrVar* rst, XListArr* arr) {
    if( rst==NULL || arr==NULL ) return;
    for( uint n=0; n<dataUnit.valueCount(); n++) {
        arr->add()->addU16(dataUnit.value(n));
    }
    rst->setVar('a',0,(LPVOID)arr);
}


ModbusDevice::ModbusDevice(TreeNode* cf, int serverAddress) : xcf(cf), xmodbusDevice(NULL), xserverAddress(serverAddress) {
    config()->set("@mode", "tcp");
    config()->setInt("@status", 0);
    config()->setInt("@serverId", xserverAddress);
}
ModbusDevice::~ModbusDevice() {
    disconnectServer();
}
bool ModbusDevice::changeMode(LPCC mode) {
    if( ccmp(mode,"tcp") || ccmp(mode,"serial") ) {
        if( config()->cmp("@mode",mode) ) return false;
        disconnectServer();
        return true;
    }
    return false;
}
void ModbusDevice::connectServer(LPCC connectString) {
    qDebug("ModbusDevice connectServer connectString:%s",connectString);
    if( xmodbusDevice==NULL ) {
        if( config()->cmp("@mode","serial") ) {
            xmodbusDevice=new QModbusRtuSerialMaster(this);
        } else {
            xmodbusDevice=new QModbusTcpClient(this);
        }
        connect(xmodbusDevice, &QModbusClient::stateChanged, this, &ModbusDevice::onStateChanged);
        connect(xmodbusDevice, &QModbusClient::errorOccurred, this, &ModbusDevice::onDeviceError );
    }
    if( xmodbusDevice->state()!=QModbusDevice::ConnectedState ) {
        int responseTime = config()->getInt("@responseTime",1000);
        int numberOfRetries = config()->getInt("@numberOfRetries",3);
        if( config()->cmp("@mode","serial") ) {
            LPCC commPort=config()->get("@commPort");
            int parity = config()->getInt("@parity",2);         // QSerialPort::EvenParity;
            int baud = config()->getInt("@baud", 19200);        // QSerialPort::Baud19200;
            int dataBits = config()->getInt("@dataBits",8);     // QSerialPort::Data8;
            int stopBits = config()->getInt("@stopBits",1);     // QSerialPort::OneStop;
            xmodbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter, KR(commPort));
            xmodbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter, parity);
            xmodbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, baud);
            xmodbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, dataBits);
            xmodbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, stopBits);
        } else {
            // @mode: tcp
            const QUrl url = QUrl::fromUserInput(KR(connectString));
            xmodbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, url.port());
            xmodbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());
        }
        xmodbusDevice->setTimeout( responseTime);
        xmodbusDevice->setNumberOfRetries( numberOfRetries);
        if ( xmodbusDevice->connectDevice() ) {
            qDebug("#0#modbus connect device ok (mode:%s, state:%s)", config()->get("@mode"),stateText() );
            config()->setInt("@status", 1);
        } else {
            config()->setInt("@status", 0);
            XFuncNode* fnCur=getEventFuncNode(config(), "onError");
            xerrorMessage=QString("modbus device connect failed: %1").arg(xmodbusDevice->errorString());
            if( fnCur ) {
                XListArr* arrs=getLocalParams(fnCur);
                arrs->add()->set("connect");
                arrs->add()->set(Q2A(xerrorMessage));
                setFuncNodeParams(fnCur, arrs);
                fnCur->call();
            } else {
                qDebug("#9# %s", Q2A(xerrorMessage) );
            }
        }
    } else {
        // xmodbusDevice->disconnectDevice();
    }
}
void ModbusDevice::disconnectServer() {
    if( xmodbusDevice ) {
        qDebug("disconnect device state:%d", xmodbusDevice->state());
        config()->setInt("@status", 0);
        config()->GetVar("@c")->reuse();
        xmodbusDevice->disconnectDevice();
        SAFE_DELETE(xmodbusDevice);
    }
}
bool ModbusDevice::isConnect(bool statusCheck) {
    if( xmodbusDevice ) {
        /*
        qDebug("#0#modbus isConnect (mode:%s, state:%s, status:%d)",
               config()->get("@mode"), stateText(), config()->getInt("@status") );
        */
        if( xmodbusDevice->state()==QModbusDevice::ConnectedState ) {
            return true;
        }
    }
    return false;
}
bool ModbusDevice::sendRead(LPCC regType, int address, int count, U32 flag) {
    qDebug("#0#modbus sendRead (mode:%s, state:%s, status:%d)",
           config()->get("@mode"), stateText(), config()->getInt("@status") );

    if( xmodbusDevice==NULL ) {
        qDebug("#9#read error server:%d type:%s address:%d, count:%d", config()->getInt("@serverId"), regType, address, count);
        return false;
    }
    config()->setInt("@status", 2);
    QModbusDataUnit unit(ccmp(regType,"coil")?QModbusDataUnit::Coils:
                         ccmp(regType,"holding")?QModbusDataUnit::HoldingRegisters:
                         ccmp(regType,"input")?QModbusDataUnit::InputRegisters:
                         ccmp(regType,"discrete")?QModbusDataUnit::DiscreteInputs:
                         QModbusDataUnit::HoldingRegisters, address, count);
    config()->set("@readRegType", regType);
    if( auto *reply = xmodbusDevice->sendReadRequest(unit, config()->getInt("@serverId")) ) {
        if( !reply->isFinished() ) {
            config()->setInt("@status", 3);
            config()->setInt("@currentAddress", address);
            connect(reply, &QModbusReply::finished, this, &ModbusDevice::onReadFinish );
            return true;
        } else {
            config()->setInt("@status", 1);
            qDebug("#0#modbus read reply error(address:%d)", address);
            delete reply;
        }
    } else {
        XFuncNode* fnCur=getEventFuncNode(config(), "onError");
        xerrorMessage=QString("Read Reply fail: %1").arg(xmodbusDevice->errorString());
        if( fnCur ) {
            XListArr* arrs=getLocalParams(fnCur);
            arrs->add()->set("read");
            arrs->add()->set(Q2A(xerrorMessage));
            setFuncNodeParams(fnCur, arrs);
            fnCur->call();
        } else {
            qDebug("#9# %s", Q2A(xerrorMessage) );
        }
        config()->setInt("@status", 1);
    }
    return false;
}
bool ModbusDevice::sendWrite(LPCC regType, int address, XListArr* arr, U32 flag) {
    if( xmodbusDevice==NULL ) {
        qDebug("#9#write error server:%d type:%s address:%d, count:%d", config()->getInt("@serverId"), regType, address, arr?arr->size():0);
        return false;
    }
    config()->setInt("@status", 12);
    xerrorMessage="";
    if( arr && (ccmp(regType,"coil") || ccmp(regType,"holding"))  ) {
        int count=arr->size();
        QModbusDataUnit unit(ccmp(regType,"coil")?QModbusDataUnit::Coils:
                             ccmp(regType,"holding")?QModbusDataUnit::HoldingRegisters:
                             QModbusDataUnit::Coils, address, count);
        config()->set("@writeRegType", regType);
        for( int n=0;n<count; n++ ) {
            unit.setValue(n, arr->get(n)->getU16());
        }
        if( auto *reply = xmodbusDevice->sendWriteRequest(unit, config()->getInt("@serverId")) ) {
            if( !reply->isFinished() ) {
                config()->setInt("@status", 13);
                config()->setInt("@currentAddress", address);
                connect(reply, &QModbusReply::finished, this, &ModbusDevice::onWriteFinish );
                return true;
            } else {
                config()->setInt("@status", 1);
                qDebug("#0#modbus write reply error(address:%d)", address);
                delete reply;
            }
        } else {
            xerrorMessage=QString("Write Reply fail: %1").arg(xmodbusDevice->errorString());
        }
    } else {
        xerrorMessage=QString("Write regsite type error");
    }
    if( !xerrorMessage.isEmpty() ) {
        XFuncNode* fnCur=getEventFuncNode(config(), "onError");
        if( fnCur ) {
            XListArr* arrs=getLocalParams(fnCur);
            arrs->add()->set("write");
            arrs->add()->set(Q2A(xerrorMessage));
            setFuncNodeParams(fnCur, arrs);
            fnCur->call();
        } else {
            qDebug("#9# %s", Q2A(xerrorMessage) );
        }
        config()->setInt("@status", 1);
    }

    return false;
}

LPCC ModbusDevice::stateText() {
    StrVar* sv=config()->GetVar("@state")->reuse();
    if( xmodbusDevice ) {
        if( xmodbusDevice->state() == QModbusDevice::ConnectedState ) {
            sv->set("conneted");
        } else if( xmodbusDevice->state() == QModbusDevice::ConnectingState ) {
            sv->set("conneting");
        } else if( xmodbusDevice->state() == QModbusDevice::ClosingState ) {
            sv->set("closing");
        } else if( xmodbusDevice->state() == QModbusDevice::UnconnectedState ) {
            sv->set("closed");
        }
    }
    return sv->get();
}

void ModbusDevice::onReadFinish() {
    auto reply = qobject_cast<QModbusReply *>(sender());
    if( !reply) return;
    if( reply==NULL || reply->error()!=QModbusDevice::NoError ) {
        if( reply==NULL ) {
            xerrorMessage=QString("Read response is null");
        } else if( reply->error() == QModbusDevice::ProtocolError ) {
            xerrorMessage=QString("Read response error: %1 (Mobus exception: 0x%2)").
                    arg(reply->errorString()).
                    arg(reply->rawResult().exceptionCode(), -1, 16);
        } else {
            xerrorMessage=QString("Read response error: %1 (code: 0x%2)").
                    arg(reply->errorString()).
                    arg(reply->rawResult().exceptionCode(), -1, 16);
        }
        XFuncNode* fnCur=getEventFuncNode(config(), "onError");
        config()->setInt("@status", 99);
        if( fnCur ) {
            XListArr* arrs=getLocalParams(fnCur);
            arrs->add()->set("read");
            arrs->add()->set(Q2A(xerrorMessage));
            setFuncNodeParams(fnCur, arrs);
            fnCur->call();
        } else {
            qDebug("#9# %s", Q2A(xerrorMessage) );
        }
    } else {
        XFuncNode* fnCur=getEventFuncNode(config(), "onReadFinish");
        xerrorMessage="";
        config()->setInt("@status", 4);
        if( fnCur ) {
            XListArr* arrs=getLocalParams(fnCur);
            const QModbusDataUnit unit = reply->result();
            arrs->add()->setVar('0',0).addInt(unit.startAddress());
            dataUnitValue(unit,arrs->add(), xreadArray.reuse() );
            registerTypeText(unit,arrs->add());
            setFuncNodeParams(fnCur, arrs);
            fnCur->call();
            qDebug("#0#modbus read callback finished");
        } else {
            qDebug("#0#modbus read callback not defined");
        }
    }
    config()->setInt("@status", 1);
    reply->deleteLater();
}
void ModbusDevice::onWriteFinish() {
    auto reply = qobject_cast<QModbusReply *>(sender());
    if( !reply) return;
    if( reply==NULL || reply->error()!=QModbusDevice::NoError ) {
        if( reply==NULL ) {
            xerrorMessage=QString("Write response is null");
        } else if( reply->error() == QModbusDevice::ProtocolError ) {
            xerrorMessage=QString("Write response error: %1 (Mobus exception: 0x%2)").
                    arg(reply->errorString()).
                    arg(reply->rawResult().exceptionCode(), -1, 16);
        } else {
            xerrorMessage=QString("Write response error: %1 (code: 0x%2)").
                    arg(reply->errorString()).
                    arg(reply->rawResult().exceptionCode(), -1, 16);
        }
        XFuncNode* fnCur=getEventFuncNode(config(), "onError");
        config()->setInt("@status", 99);
        if( fnCur ) {
            XListArr* arrs=getLocalParams(fnCur);
            arrs->add()->set("write");
            arrs->add()->set(Q2A(xerrorMessage));
            setFuncNodeParams(fnCur, arrs);
            fnCur->call();
        } else {
            qDebug("#9# %s", Q2A(xerrorMessage) );
        }
    } else {
        XFuncNode* fnCur=getEventFuncNode(config(), "onWriteFinish");
        xerrorMessage="";
        config()->setInt("@status", 14);
        if( fnCur ) {
            XListArr* arrs=getLocalParams(fnCur);
            const QModbusDataUnit unit = reply->result();
            arrs->add()->setVar('0',0).addInt(unit.startAddress());
            registerTypeText(unit,arrs->add());
            setFuncNodeParams(fnCur, arrs);
            fnCur->call();
        }
    }
    config()->setInt("@status", 1);
    reply->deleteLater();
}
void ModbusDevice::onStateChanged(int state) {
    XFuncNode* fnCur=getEventFuncNode(config(), "onStateChange");
    if( xmodbusDevice->state() == QModbusDevice::UnconnectedState ) {
        config()->setInt("@status", 0);
    }
    if( fnCur ) {
        XListArr* arrs=getLocalParams(fnCur);
        arrs->add()->set(stateText());
        setFuncNodeParams(fnCur, arrs);
        fnCur->call();
    }
}
void ModbusDevice::onDeviceError() {

}


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
#endif
