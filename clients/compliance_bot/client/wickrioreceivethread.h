#ifndef WICKRIORECEIVETHREAD_H
#define WICKRIORECEIVETHREAD_H

#include <QObject>
#include <QThread>
#include <QTimer>

#include "wickriothread.h"
#include "operationdata.h"

#include "services/wickrMessageService.h"
#include "services/wickrSwitchboardService.h"

#include "messaging/wickrMessage.h"
#include "filetransfer//wickrFileInfo.h"

#include "common/wickrMessageMgr.h"

class WickrIOReceiverMgr : public WickrMessageMgr
{
public:
    WickrIOReceiverMgr() {}

    bool dispatch(WickrCore::WickrInbox *msg);
};

class WickrIORxDownloadFile
{
public:
    WickrIORxDownloadFile(WickrCore::WickrMessage *msg, WickrCore::FileInfo fileinfo, QString attachFilename, QString realFilename) :
        m_msg(msg),
        m_fileInfo(fileinfo),
        m_attachmentFileName(attachFilename),
        m_realFileName(realFilename),
        m_downloaded(false),
        m_downloading(false) {
    }

    WickrCore::WickrMessage *m_msg;
    WickrCore::FileInfo m_fileInfo;
    QString m_attachmentFileName;
    QString m_realFileName;
    bool m_downloaded;
    bool m_downloading;
};


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
    WickrIOReceiverMgr m_msgReceiver;

    OperationData *m_operation;
    bool m_enableSwitchboard;
    bool m_receiving;
    int m_timerStatsTicker;

    // Used to process incoming messages
    WickrConvoList *m_convoList;

    int m_messagesRecv;
    int m_messagesRecvFailed;
    void initCounts()
    {
        m_messagesRecv = 0;
        m_messagesRecvFailed = 0;
    }

    bool processKeyVerificationMsg(QJsonObject& jsonObject,  WickrCore::WickrMessage *msg);
    bool processControlMsg(QJsonObject& jsonObject,  WickrCore::WickrMessage *msg);

    QString getAttachmentFile(const QByteArray &data, QString extension);

    // File Download definitions
    QMap<QString, WickrIORxDownloadFile *> m_activeDownloads;

    void startSwitchboard();
    void stopSwitchboard();
    void initMessageServicesConnections();
#if 0
    void attachConvos();
    void detachConvos();

    void attachConvosMessages(WickrNotifyList *msgList);
    void detachConvosMessages(WickrNotifyList *msgList);
#endif

    bool processMessage(WickrDBObject *item);

protected:
    void processStarted();
    void onTimerAction();

private slots:
    void slotProcessMessage(WickrDBObject *item);
#if 0
    void slotConvoAdded(WickrDBObject *item, bool existing = false);
    void slotConvoChanged(WickrDBObject *item);
    void slotConvoDeleted(WickrDBObject *inItem);
#endif
signals:
    void signalProcessStarted();
    void signalReceivingStarted();
    void signalReceivingEnded();

    void signalMessageCheck(WickrApplicationState appContext);

};

#endif // WICKRIORECEIVETHREAD_H
