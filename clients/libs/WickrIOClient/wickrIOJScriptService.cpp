#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

#include "wickrIOJScriptService.h"
#include "wickriodatabase.h"
#include "wickrIOClientRuntime.h"
#include "wickrIOAPIInterface.h"
#include "wickrIOCommon.h"

QString WickrIOJScriptService::jsServiceBaseName = "WickrIOJScriptThread";

WickrIOJScriptService::WickrIOJScriptService() : WickrIOServiceBase(jsServiceBaseName),
    m_lock(QReadWriteLock::Recursive),
    m_state(WickrServiceState::SERVICE_UNINITIALIZED),
    m_cbThread(nullptr)
{
  qRegisterMetaType<WickrServiceState>("WickrServiceState");
  qRegisterMetaType<WickrApplicationState>("WickrApplicationState");

  // Start threads
  startThreads();

  setObjectName("WickrIOJScriptThread");
  qDebug() << "WICKRIOJAVASCRIPT SERVICE: Started.";
  m_state = WickrServiceState::SERVICE_STARTED;
}

/**
* @brief WickrIOJScriptService::~WickrIOJScriptService
* Destructor
*/
WickrIOJScriptService::~WickrIOJScriptService() {
    // Stop threads
    stopThreads();

    qDebug() << "WICKRIOJAVASCRIPT SERVICE: Shutdown.";
    m_state = WickrServiceState::SERVICE_SHUTDOWN;
}

/**
 * @brief WickrIOJScriptService::startThreads
 * Starts all threads on message service.
 */
void WickrIOJScriptService::startThreads()
{
    QWriteLocker lockGuard(&m_lock);

    // Allocate threads
    m_cbThread = new WickrIOJScriptThread(&m_thread,this);

    // Connect internal threads signals and slots
    connect(this, &WickrIOJScriptService::signalMessagesPending,
            m_cbThread, &WickrIOJScriptThread::slotProcessMessages, Qt::QueuedConnection);
    connect(this, &WickrIOJScriptService::signalStartScript,
            m_cbThread, &WickrIOJScriptThread::slotStartScript, Qt::QueuedConnection);

    // Perform startup here, creating and configuring ressources.
    m_thread.start();
}

/**
 * @brief WickrIOJScriptService::stopThreads
 * Stops all threads on JavaScript service.
 */
void WickrIOJScriptService::stopThreads()
{
    QWriteLocker lockGuard(&m_lock);

    // Perform shutdown here, wait for resources to quit, and cleanup

    // Task Service
    m_thread.quit();
    m_thread.wait();
    qDebug("JSCRIPT THREAD: Shutdown Thread (%p)", &m_thread);
}

void WickrIOJScriptService::messagesPending()
{
    emit signalMessagesPending();
}

void WickrIOJScriptService::startScript()
{
    emit signalStartScript();
}

/**
 * @brief WickrIOEventService::isHealthy
 * This function will return false if the health of this services is in a bad state. For the
 * Event Handler this is typically related to a stuck event.
 * @return
 */
bool WickrIOJScriptService::isHealthy()
{
    return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief WickrIOJScriptThread::asStringState
 * @param state
 * @return
 */
QString WickrIOJScriptThread::jsStringState(JSThreadState state)
{
    switch(state) {
    case JSThreadState::JS_UNINITIALIZED:   return "Uninitialized";
    case JSThreadState::JS_STARTED:         return "Started";
    case JSThreadState::JS_PROCESSING:      return "Processing";
    case JSThreadState::JS_FINISHED:        return "Finished";
    default:                                return "Unknown";
    }
}

/**
 * @brief WickrIOJScriptThread::WickrIOJScriptThread
 * Constructor
 */
WickrIOJScriptThread::WickrIOJScriptThread(QThread *thread, WickrIOJScriptService *cbSvc, QObject *parent)
    : QObject(parent),
      m_parent(cbSvc)
    , m_state(JSThreadState::JS_UNINITIALIZED)
{
    thread->setObjectName("WickrIOJScriptThread");
    this->moveToThread(thread);

    // Signal to cleanup worker
    connect(thread, &QThread::finished, this, &QObject::deleteLater);

    m_state = JSThreadState::JS_STARTED;
}

/**
 * @brief WickrIOJScriptThread::~WickrIOJScriptThread
 * Destructor
 */
WickrIOJScriptThread::~WickrIOJScriptThread() {
    qDebug() << "WICKRIOJAVASCRIPT THREAD: Worker Destroyed.";
}

void
WickrIOJScriptThread::slotProcessMessages()
{
    // Don't want to start multiple processing to be initiated
    if (m_state == JSThreadState::JS_PROCESSING)
        return;

    m_state = JSThreadState::JS_PROCESSING;

#if 0
    jScriptSendMessage();
#endif
    m_state = JSThreadState::JS_STARTED;
}

void
WickrIOJScriptThread::slotStartScript()
{
    // Don't want to start multiple processing to be initiated
    if (m_state == JSThreadState::JS_PROCESSING)
        return;
    m_state = JSThreadState::JS_PROCESSING;

    m_zctx = nzmqt::createDefaultContext();
    m_zsocket = m_zctx->createSocket(nzmqt::ZMQSocket::TYP_REP, this);
    m_zsocket->setObjectName("Replier.Socket.socket(REP)");
    connect(m_zsocket, &nzmqt::ZMQSocket::messageReceived,
            this, &WickrIOJScriptThread::slotMessageReceived, Qt::QueuedConnection);

    m_zctx->start();

    OperationData* operation = WickrIOClientRuntime::operationData();
    QString queueDirName = QString(WBIO_CLIENT_SOCKETDIR_FORMAT).arg(WBIO_DEFAULT_DBLOCATION).arg(operation->m_client->name);
    QDir    queueDir(queueDirName);
    if (!queueDir.exists()) {
        if (!queueDir.mkpath(queueDirName)) {
            qDebug() << "Cannot create message queue directory!";
        }
    }
    QString queueName = QString(WBIO_CLIENT_RXSOCKET_FORMAT).arg(WBIO_DEFAULT_DBLOCATION).arg(operation->m_client->name);
    m_zsocket->bindTo(queueName);

    // Set the permission of the queue file so that normal user programs can access
    QString queueFileName = QString(WBIO_CLIENT_SOCKETFILE_FORMAT).arg(WBIO_DEFAULT_DBLOCATION).arg(operation->m_client->name);
    QFile zmqFile(queueFileName);
    if(!zmqFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                               QFile::ReadUser | QFile::WriteUser | QFile::ExeUser  |
                               QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup |
                               QFile::ReadOther | QFile::WriteOther | QFile::ExeOther)) {
        qDebug("Something wrong setting permissions of the queue file!");
    }

    m_state = JSThreadState::JS_STARTED;
}

/**********************************************************************************************
 * QUEUING FUNCTIONS
 **********************************************************************************************/

void
WickrIOJScriptThread::slotMessageReceived(const QList<QByteArray>& messages)
{
//    qDebug() << "entered slotMessageReceived!";
    for (QByteArray mesg : messages) {
        QString response = processRequest(mesg);

        QList<QByteArray> reply;
        reply += response.toLocal8Bit();
        if (response.length() > 0)
            qDebug() << "Replier::sendReply> " << response;
        m_zsocket->sendMessage(reply);
    }
}


QString
WickrIOJScriptThread::processRequest(const QByteArray& request)
{
    QJsonParseError jsonError;
    QJsonDocument jsonRequest = QJsonDocument().fromJson(request, &jsonError);
    if (jsonError.error != jsonError.NoError) {
        return "Parsing error!";
    }

    QJsonObject object = jsonRequest.object();

    if (!object.contains("action")) {
        return "Malformed request: no action!";
    }

    QJsonValue value;
    QString action;

    value = object["action"];
    action = value.toString().toLower();

    OperationData* operation = WickrIOClientRuntime::operationData();
    if (!operation) {
        return "Internal failure!";
    }
    WickrIOAPIInterface apiInterface(operation);

    QString responseString;

    // MESSAGE ACTIONS
    if (action == "send_message") {
        if (apiInterface.sendMessage(request, responseString)) {
            responseString = "Sending message";
        }
    }
    else if (action == "get_received_messages") {
        apiInterface.getReceivedMessages(responseString);
    }
    // ROOM ACTIONS
    else if (action == "add_room") {
        apiInterface.addRoom(request, responseString);
    }
    else if (action == "modify_room") {
        if (!object.contains("vgroupid")) {
            return "Malformed request: no vGroupID!";
        }
        value = object["vgroupid"];
        QString vGroupID = value.toString();

        apiInterface.updateRoom(vGroupID, request, responseString);
    }
    else if (action == "delete_room") {
        if (!object.contains("vgroupid")) {
            return "Malformed request: no vGroupID!";
        }
        value = object["vgroupid"];
        QString vGroupID = value.toString();

        apiInterface.deleteRoom(vGroupID, responseString);
    }
    else if (action == "leave_room") {
        if (!object.contains("vgroupid")) {
            return "Malformed request: no vGroupID!";
        }
        value = object["vgroupid"];
        QString vGroupID = value.toString();

        apiInterface.leaveRoom(vGroupID, responseString);
    }
    else if (action == "get_room") {
        if (!object.contains("vgroupid")) {
            return "Malformed request: no vGroupID!";
        }
        value = object["vgroupid"];
        QString vGroupID = value.toString();

        apiInterface.getRoom(vGroupID, responseString);
    }
    else if (action == "get_rooms") {
        apiInterface.getRooms(responseString);
    }
    // GROUP CONVERSATION ACTIONS
    else if (action == "add_groupconvo") {
        apiInterface.addGroupConvo(request, responseString);
    }
    else if (action == "delete_groupconvo") {
        if (!object.contains("vgroupid")) {
            return "Malformed request: no vGroupID!";
        }
        value = object["vgroupid"];
        QString vGroupID = value.toString();

        apiInterface.deleteGroupConvo(vGroupID, responseString);
    }
    else if (action == "get_groupconvo") {
        if (!object.contains("vgroupid")) {
            return "Malformed request: no vGroupID!";
        }
        value = object["vgroupid"];
        QString vGroupID = value.toString();

        apiInterface.getGroupConvo(vGroupID, responseString);
    }
    else if (action == "get_groupconvos") {
        apiInterface.getGroupConvos(responseString);
    }
    // STATISTICS ACTIONS
    else if (action == "get_statistics") {
        apiInterface.getStatistics("", responseString);
    }
    else if (action == "clear_statistics") {
        apiInterface.clearStatistics("", responseString);
    }
    // Asynchronous message and event handling
    else if (action == "start_async_messages") {
        apiInterface.startAsyncMessages(responseString);
    } else if (action == "stop_async_messages") {
        apiInterface.stopAsyncMessages(responseString);
    } else if (action == "start_async_events") {
        apiInterface.startAsyncEvents(responseString);
    } else if (action == "stop_async_events") {
        apiInterface.stopAsyncEvents(responseString);
    }
    else {
        responseString = QString("action '%1' unknown!").arg(action);
    }

    return responseString;
}

