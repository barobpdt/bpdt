#ifndef MODBUS_DEVICE_H
#define MODBUS_DEVICE_H

#include "func_util.h"

#ifdef QT5_USE
#include <QModbusDevice>
#include <QModbusRtuSerialMaster>
#include <QModbusClient>
#include <QModbusTcpClient>
#include <QModbusDevice>
#include <QModbusDataUnit>
#include <QSerialPort>
#include <QUrl>
#include <QObject>

//CONSTANTS
#define MESSAGETIMEOUT 5000

class ModbusDevice: public QObject
{
    Q_OBJECT
public:
    ModbusDevice(TreeNode* cf, int serverAddress=1);
    ~ModbusDevice();
    void connectServer(LPCC connectString);
    void disconnectServer();
    bool sendRead(LPCC regType, int address, int count, U32 flag=0);
    bool sendWrite(LPCC regType, int address, XListArr* arr, U32 flag=0);
    bool isConnect(bool statusCheck=true);
    LPCC stateText();
    bool changeMode(LPCC mode);
    TreeNode* config() {
        if( xcf==NULL ) xcf=new TreeNode(2,true);
        return xcf;
    }
public:
    TreeNode*           xcf;
    QModbusClient*      xmodbusDevice;
    QModbusDataUnit     xdataUnit;
    QString             xerrorMessage;
    XListArr            xreadArray;
    int xserverAddress;

private slots:
    void onReadFinish();
    void onWriteFinish();
    void onStateChanged(int state);
    void onDeviceError();
};
#endif

#endif // MODBUS_DEVICE_H
