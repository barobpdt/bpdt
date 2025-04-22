#ifndef TMSMQCLIENT_H
#define TMSMQCLIENT_H

#include <QObject>
#include <QJsonObject>
#include "../baroscript/func_util.h"
#include "../mqtt/qmqttclient.h"

class TmsMqClient : public QObject
{
    Q_OBJECT
public:
    explicit TmsMqClient(TreeNode* cf, QObject *parent = 0);
    ~TmsMqClient();
    void connMqServer(LPCC hostName, int port, LPCC userName, LPCC userPassword);
    void disConnMqServer();
    int mqSend(QMqttTopicFilter& topic, qint8 qos);
    int mqSend(QString msg);
    int getState();

private:
    void parseMqttCommand(QJsonObject joData);

public:
    QMqttClient *m_hClient;
    TreeNode* xcf;

signals:
    void signalMqRecvData(QString strMsg, QString strSR);
    void signalLinkCommand(QStringList strListData);
    void signalMqttDisConnectSet();

public slots:
    void slotsUpdateLogStateChange();
    void slotsBrokerDisconnected();
    void slotsMqRecv(const QByteArray &msg, const QMqttTopicName &topic);
    void slotsPingRecv();
};

#endif // TMSMQCLIENT_H
