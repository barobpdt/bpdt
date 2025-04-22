#include "tmsmqclient.h"

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>



TmsMqClient::TmsMqClient(TreeNode* cf, QObject *parent) : QObject(parent)
{
    m_hClient = new QMqttClient(this);
    xcf=cf;
    connect(m_hClient, SIGNAL(stateChanged(ClientState)), this, SLOT(slotsUpdateLogStateChange()));
    connect(m_hClient, SIGNAL(disconnected()), this, SLOT(slotsBrokerDisconnected()));
    connect(m_hClient, SIGNAL(messageReceived(QByteArray,QMqttTopicName)), this, SLOT(slotsMqRecv(QByteArray,QMqttTopicName)));
    connect(m_hClient, SIGNAL(pingResponseReceived()), this, SLOT(slotsPingRecv()));
}

TmsMqClient::~TmsMqClient()
{
    if ( m_hClient->state() != QMqttClient::Disconnected ) {
        m_hClient->disconnected();
    }
    SAFE_DELETE(m_hClient);
}


void TmsMqClient::connMqServer(LPCC hostName, int port, LPCC userName, LPCC userPassword)
{
    m_hClient->setHostname(KR(hostName));
    m_hClient->setPort(port);
    m_hClient->setUsername(KR(userName));
    m_hClient->setPassword(KR(userPassword));
    m_hClient->connectToHost();
    qDebug("mq connect : %d", getState() );
}

int TmsMqClient::getState() {
    return m_hClient ? (int)m_hClient->state(): 0;
}
int TmsMqClient::mqSend(QMqttTopicFilter& topic, qint8 qos) {
    /*[state]
    SubscriptionState {
        Unsubscribed = 0,
        SubscriptionPending,
        Subscribed,
        UnsubscriptionPending,
        Error
    }
    */
    // QMqttTopicFilter;
    if ( m_hClient->state() == QMqttClient::Connected ) {
        QMqttSubscription* subscription = m_hClient->subscribe(topic, qos);
        return subscription ? (int)subscription->state(): 0;
    } else {
        qDebug("#0#mq send error not connected %d",getState());
        return 0;
    }
}
int TmsMqClient::mqSend(QString msg) {
    if ( m_hClient->state() == QMqttClient::Connected ) {
        QMqttSubscription* subscription = m_hClient->subscribe(msg);
        return subscription ? (int)subscription->state(): 0;
    } else {
        qDebug("#0#mq send error not connected %d",getState());
        return 0;
    }
}

void TmsMqClient::disConnMqServer()
{
    m_hClient->disconnectFromHost();
}

void TmsMqClient::slotsBrokerDisconnected()
{
    StrVar*sv= xcf->gv("onChange");
    if( SVCHK('f',0) ) {
        XFuncNode* fn = (XFuncNode*)SVO;
        PARR arrs=getLocalParams(fn);
        arrs->add()->set("disconnect");
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
}

void TmsMqClient::slotsMqRecv(const QByteArray &msg, const QMqttTopicName &topic)
{
    StrVar*sv= xcf->gv("onRecv");
    if( SVCHK('f',0) ) {
        XFuncNode* fn = (XFuncNode*)SVO;
        PARR arrs=getLocalParams(fn);
        arrs->add()->set(msg.constData(), msg.size());
        arrs->add()->set(Q2A(topic.name()));
        arrs->add()->setVar('0',0).addInt(topic.levelCount());
        for(int n=0;n<topic.levelCount(); n++ ) {
            QString level=topic.levels()[n];
            arrs->add()->add(Q2A(level));
        }
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
}

void TmsMqClient::slotsPingRecv()
{
    StrVar*sv= xcf->gv("onChange");
    if( SVCHK('f',0) ) {
        XFuncNode* fn = (XFuncNode*)SVO;
        PARR arrs=getLocalParams(fn);
        arrs->add()->set("ping");
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
}
void TmsMqClient::slotsUpdateLogStateChange()
{
    StrVar*sv= xcf->gv("onChange");
    if( SVCHK('f',0) ) {
        XFuncNode* fn = (XFuncNode*)SVO;
        PARR arrs=getLocalParams(fn);
        int state=m_hClient->state();
        arrs->add()->set("state");
        arrs->add()->setVar('0',0).addInt(state);
        setFuncNodeParams(fn, arrs);
        fn->call();
    }
}
