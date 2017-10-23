#ifndef WICKRIORECEIVETHREAD_H
#define WICKRIORECEIVETHREAD_H

#include <QObject>
#include <QThread>
#include <QTimer>

#include "wickrIOThread.h"
#include "operationdata.h"

#include "services/wickrMessageService.h"
#include "services/wickrSwitchboardService.h"

#include "messaging/wickrMessage.h"
#include "filetransfer//wickrFileInfo.h"

class WickrIOReceiveThread : public WickrIOThread
{
    Q_OBJECT
public:
    WickrIOReceiveThread(OperationData *operation);
    ~WickrIOReceiveThread();

    Q_INVOKABLE void startReceiving();
    Q_INVOKABLE void stopReceiving();

    Q_INVOKABLE void logCounts()
    {
        if (m_messagesRecv > 0) {
            m_operation->log("Messages received", m_messagesRecv);
            m_messagesRecv = 0;
        }
        if (m_messagesRecvFailed > 0) {
            m_operation->log("Messages received failed", m_messagesRecvFailed);
            m_messagesRecvFailed = 0;
        }
    }

private:
    OperationData *m_operation;
    bool m_enableSwitchboard;
    bool m_receiving;
    int m_timerStatsTicker;

    // Used to process incoming messages
    WickrConvoList *m_convoList;

    // List of possible messages to respond with, when receiving messages
    QStringList         m_responseMessagesList;
    QMap<QString, int>  m_userResponseIndexMap;

    const QString& responseMessageText(const QString& userid);

    int m_messagesRecv;
    int m_messagesRecvFailed;
    void initCounts()
    {
        m_messagesRecv = 0;
        m_messagesRecvFailed = 0;
    }

    void startSwitchboard();
    void stopSwitchboard();
    void initMessageServicesConnections();
    void attachConvos();
    void detachConvos();

    void attachConvosMessages(WickrNotifyList *msgList);
    void detachConvosMessages(WickrNotifyList *msgList);
    bool processMessage(WickrDBObject *item);

protected:
    void processStarted();
    void onTimerAction();

private slots:
    void slotProcessMessage(WickrDBObject *item);

    void slotConvoAdded(WickrDBObject *item, bool existing = false);
    void slotConvoChanged(WickrDBObject *item);
    void slotConvoDeleted(WickrDBObject *inItem);

signals:
    void signalProcessStarted();
    void signalReceivingStarted();
    void signalReceivingEnded();

    void signalMessageCheck(WickrApplicationState appContext);

};

#endif // WICKRIORECEIVETHREAD_H
