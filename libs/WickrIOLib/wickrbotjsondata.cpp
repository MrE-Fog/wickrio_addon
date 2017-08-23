#include <QString>
#include <QList>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QJsonDocument>
#include "wickrbotjsondata.h"
#include "createjson.h"

WickrBotJsonData::WickrBotJsonData(OperationData *operation) :
    m_operation(operation),
    m_clientActions(nullptr)
{
    m_ttl = 0;
    m_action = "";
    m_userIDs.empty();
    m_userNames.empty();
    m_attachments.clear();
    m_message = "";
    m_vgroupid = QString("");
}

WickrBotJsonData::~WickrBotJsonData()
{
    if (m_clientActions != nullptr) {
        delete m_clientActions;
        m_clientActions = nullptr;
    }
}

int WickrBotJsonData::getJsonIntValue(QJsonObject feed, QString key)
{
    QJsonValue value;
    QJsonObject jsonObject;

    if (feed.contains(key)) {
        value = feed[key];
        jsonObject = value.toObject();

        value = jsonObject["$t"];
        if (value.isString()) {
            QString str = value.toString("0");
            return str.toInt();
        } else {
            return value.toInt();
        }
    }

    return 0;
}

QString WickrBotJsonData::getJsonStringValue(QJsonObject object, QString key)
{
    QJsonValue value;
    QJsonObject jsonObject;

    if (object.contains(key)) {
        value = object[key];
        jsonObject = value.toObject();

        value = jsonObject["$t"];
        if (value.isString()) {
            QString str = value.toString("");
            return str;
        }
    }

    return "";
}

QStringList WickrBotJsonData::getJsonStringList(QJsonObject object, QString attribute, QString key)
{
    QStringList retList;

    if (object.contains(attribute)) {
        QJsonValue value = object[attribute];
        if (value.isArray()) {
            QJsonArray array = value.toArray();

            for (int i=0; i< array.size(); i++) {
                QJsonValue arrayValue = array[i];
                if (arrayValue.isObject()) {
                    QJsonObject arrayEntry = arrayValue.toObject();
                    if (arrayEntry.contains(key)) {
                        QJsonValue keyValue = arrayEntry[key];
                        if (keyValue.isString())
                            retList += keyValue.toString();
                    }
                }
            }
        }
    }

    return retList;
}

bool WickrBotJsonData::saveAttachment(QString filename, QByteArray data)
{
    QFile saveFile(filename);

    if (!saveFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not save the file" << filename;
        return false;
    }

    saveFile.write(data, data.size());

    saveFile.close();
    return true;
}

void WickrBotJsonData::processAttachment(QJsonObject *object)
{
    QString filename = "";
    QJsonValue value;
    if (object->contains("attachment")) {
        value = object->value("attachment");
        QString valString = object->take("attachment").toString();
        QByteArray bytes = valString.toLatin1();
        // This will preserve the data, you will need to decode when you read it in:
        QByteArray res = QByteArray::fromBase64(bytes);

        if (filename != "")
            saveAttachment(filename, res);
    } else if (object->contains("filename")) {
        value = object->value("filename");
        filename = value.toString();
    }

}

bool WickrBotJsonData::processAttachmentURL(QString filename, QString url)
{
    if (m_operation->attachmentsDir.size() == 0) {
        return false;
    }

    if (filename == "") {
        return false;
    }

    QString cacheFilename = m_operation->m_botDB->getAttachment(url);

    m_rmAttachmentWhenDone = false;
    if (cacheFilename != NULL) {
        m_attachments.append(cacheFilename);
    } else {
        QString fullPath;

        qint64 milli = QDateTime::currentMSecsSinceEpoch();
        QString date = QString::number(milli);
        QFileInfo fi(filename);
        QString name = fi.fileName();

        fullPath = m_operation->attachmentsDir + "/attach_" + date + name;

        // Download the image
        //TODO: Need to determine if we have downloaded this file already
        if (m_operation->downloadImage(url, fullPath)) {
            m_attachments.append(fullPath);

            m_operation->m_botDB->insertAttachment(url, fullPath, (int)fi.size());
        } else {
            //TODO: Attachment not saved successfully
            return false;
        }
    }
    return true;
}

bool WickrBotJsonData::processAttachmentFile(QString filename)
{
    if (m_operation->attachmentsDir.size() == 0) {
        return false;
    }

    if (filename == "") {
        return false;
    }

    m_rmAttachmentWhenDone = false;
    QString fullPath;

    if (filename.at(0) == '/')
        fullPath = filename;
    else
        fullPath = m_operation->attachmentsDir + "/" + filename;

    QFileInfo fi(fullPath);
    bool exists = fi.exists();
    if (exists) {
        m_attachments.append(fullPath);
    } else {
        m_operation->error("Attachment does not exist: " + fullPath);
    }

    return exists;
}

bool WickrBotJsonData::processJsonDoc(QJsonDocument &jsonResponse)
{
//    QString json = bytes;
//    QJsonDocument jsonResponse = QJsonDocument::fromJson(json.toUtf8());

    QJsonObject jsonObject = jsonResponse.object();
    QJsonValue value;

    QJsonObject operationObject;

    // Start the operation
    if (jsonObject.contains("operation")) {
        value = jsonObject["operation"];
        operationObject = value.toObject();
    } else if (jsonObject.contains("command")) {
        value = jsonObject["command"];
        QString operation = value.toString("invalid");
        if (operation == "push_bot_message") {
            m_action = "sendmessage";
            return processSendMessageJsonDocV3(jsonObject);
        }
        return false;
    } else {
        m_operation->error("processContactReply: does not contain operation!");
        return false;
    }

    // Get the Action
    if (operationObject.contains("action")) {
        value = operationObject["action"];
        m_action = value.toString("invalid");
        if (m_action == "invalid") {
            m_operation->error(QString("action is invalid, value is %1").arg(m_action));
            return false;
        }
    }
    // Assume it is a send message
    else {
        m_action = "sendmessage";

    }

    if (m_action == "sendmessage") {
        return processSendMessageJsonDoc(operationObject);
    } else if (m_action == "login") {
        return processLoginJsonDoc(operationObject);
    }
    return false;
}

bool WickrBotJsonData::processLoginJsonDoc(const QJsonObject &operationObject)
{
    Q_UNUSED(operationObject);

    QJsonValue value;
    return true;
}

bool WickrBotJsonData::processSendMessageJsonDocV3(const QJsonObject &operationObject)
{
    QJsonValue value;

    // Parse the contacts
    if (operationObject.contains("uname")) {
        value = operationObject["uname"];
        m_userIDs.clear();
        QString uname=value.toString();
        m_userIDs.append(uname);
        if (m_operation->debug)
            qDebug() << "Uname:" << uname;
    }

    // Parse out any message
    if (operationObject.contains("message")) {
        value = operationObject["message"];
        m_message = value.toString();
        if (m_operation->debug)
            qDebug() << "Message:" << m_message;
    }

    if (operationObject.contains("runtime")) {
        value = operationObject["runtime"];

        // Use the current date and time if the one is invalid
        m_runTime = value.toVariant().toDateTime();
    } else {
        QDateTime dt = QDateTime::currentDateTime();
        m_runTime = dt;
    }

    // Parse out any attachments that may be part of this message
    processAttachments(operationObject);

    return true;
}

bool WickrBotJsonData::processSendMessageJsonDoc(const QJsonObject &operationObject)
{
    QJsonArray entryArray;
    QJsonValue value;

    // Parse the contacts
    if (operationObject.contains("users")) {
        m_vgroupid = QString("");
        value = operationObject["users"];
        entryArray = value.toArray();
        for (int i=0; i< entryArray.size(); i++) {
            QJsonValue arrayValue;

            arrayValue = entryArray[i];

            if (arrayValue.isObject()) {
                // Get the title for this contact entry
                QJsonObject arrayObject = arrayValue.toObject();

                if (arrayObject.contains("id")) {
                    QJsonValue idobj = arrayObject["id"];
                    QString id = idobj.toString();
                    m_userIDs.append(id);
                    if (m_operation->debug)
                        qDebug() << "User" << i+1 << id;
                } else if (arrayObject.contains("name")) {
                    QJsonValue nameobj = arrayObject["name"];
                    QString name = nameobj.toString();
                    m_userNames.append(name);
                    if (m_operation->debug)
                        qDebug() << "User" << i+1 << name;
                }
            }
        }
    } else if (operationObject.contains("vgroupid")) {
        value = operationObject["vgroupid"];
        m_vgroupid = value.toString();
        m_userNames.clear();
        m_userIDs.clear();
    } else {
        m_operation->error("Does not contain users!");
        return false;
    }

    if (operationObject.contains("runtime")) {
        value = operationObject["runtime"];

        // Use the current date and time if the one is invalid
        m_runTime = value.toVariant().toDateTime();
    } else {
        QDateTime dt = QDateTime::currentDateTime();
        m_runTime = dt;
    }

    processAttachments(operationObject);

    // Parse out any message
    if (operationObject.contains("message")) {
        value = operationObject["message"];
        m_message = value.toString();
        if (m_operation->debug)
            qDebug() << "Message:" << m_message;
    }

    // Parse out any TTL
    if (operationObject.contains("ttl")) {
        value = operationObject["ttl"];
        m_ttl = value.toInt(0);
        if (m_operation->debug)
            qDebug() << "TTL:" << m_ttl;
    }

    return true;
}

int WickrBotJsonData::processOperationData() {
    if (m_action == "login") {
        return processLoginRequest();
    } else if (m_action == "sendmessage") {
        return processSendMessage();
    }
    return 0;
}

int WickrBotJsonData::processLoginRequest()
{
    QString response;

    response = QString("{\"operation\" : {\"action\" : \"login\", \"status\" : 0 }}");
    m_operation->addResponseMessage(response);
    return 0;
}

void WickrBotJsonData::setClientType(const QString& clientType)
{
    if (m_clientActions == nullptr)
        delete m_clientActions;
    m_clientActions = new ClientActions(clientType, m_operation);
}

/**
 * @brief WickrBotJsonData::getClientID
 * This function will return the clientID that should be used for the next
 * message to be placed in the action_cache
 * @return
 */
int WickrBotJsonData::getClientID() {
    if (m_clientActions != nullptr) {
        int clientID = m_clientActions->nextClient();
        if (clientID != 0)
            return clientID;
    }
    if (m_operation->m_client != NULL) {
        return m_operation->m_client->id;
    }
    return 0;
}

bool WickrBotJsonData::processAttachments(const QJsonObject &operationObject)
{
    QJsonValue value;
    // Parse out any attachment
    if (operationObject.contains("attachments")) {
        value = operationObject["attachments"];
        QJsonArray attachments = value.toArray();

        for (int i=0; i<attachments.size(); i++) {
            QJsonObject attachment;
            attachment = attachments.at(i).toObject();

            QString filename;

            if (attachment.contains("filename")) {
                value = attachment["filename"];
                filename = value.toString();
            } else {
                filename = "";
            }

            if (attachment.contains("url")) {
                value = attachment["url"];
                QString valString = value.toString();

                processAttachmentURL(filename, valString);
            } else if (filename.length() > 0) {
                processAttachmentFile(filename);
            } else {
                m_operation->error("THERE IS NO FILENAME or URL FOR THIS ATTACHMENT!");
            }
        }
    } else if (operationObject.contains("attachment")) {
        value = operationObject["attachment"];
        QJsonObject attachment = value.toObject();

        QString filename;
        if (attachment.contains("filename")) {
            value = attachment["filename"];
            filename = value.toString();
        } else {
            filename = "";

            // TODO: need to generate a filename!!!
            qDebug() << "THERE IS NO FILENAME FOR THIS ATTACHMENT!";
        }

        if (attachment.contains("contents")) {
            value = attachment["contents"];
            QString valString = value.toString();
            QByteArray bytes = valString.toLatin1();
            // This will preserve the data, you will need to decode when you read it in:
            QByteArray res = QByteArray::fromBase64(bytes);

            if (m_operation->attachmentsDir.size() > 0) {
                if (filename != "") {
                    QString uniqueFileName;

                    qint64 milli = QDateTime::currentMSecsSinceEpoch();
                    QString date = QString::number(milli);
                    QFileInfo fi(filename);
                    QString name = fi.fileName();

    //                        QString fullPath = m_operation->attachmentsDir + "/" + filename;
                    QString fullPath = m_operation->attachmentsDir + "/attach_" + date + name;
                    if (saveAttachment(fullPath, res)) {
                        m_attachments.append(fullPath);
                        m_rmAttachmentWhenDone = true;
                    } else {
                        //TODO: Attachment not saved successfully
                    }
                }
            } else {
                // NO ATTACHMENT DIRECTORY, WHERE TO PUT THE ATTACHMENTS
            }
        } else {
            if (attachment.contains("url")) {
                value = attachment["url"];
                QString valString = value.toString();

                processAttachmentURL(filename, valString);
            }
        }
    }
    return true;
}

int WickrBotJsonData::processSendMessage() {
    int messageCount = 0;

    if (m_userIDs.size() == 0 && m_userNames.size() == 0 && m_vgroupid.isEmpty())
        return 0;

    for (int i=0; i<m_userIDs.size(); i++) {
        QString user = m_userIDs.at(i);
        if (m_message.size() > 0) {

            // output the script command to perform
            if (m_attachments.size() > 0) {
                // TODO: need to handle multiple attachments
//                qDebug() << "cronsendmsgwithattachment " << user << " " << m_ttl << " \"" << m_message << "\"" << " \"" << m_attachments << "\"\n";
            } else {
                qDebug() << "cronsendmsg " << user << " " << m_ttl << " \"" << m_message << "\"\n";
            }

            // put the command into the database
            CreateJsonAction *action = new CreateJsonAction("sendmessage", user, m_ttl, m_message, m_attachments);
            QByteArray json = action->toByteArray();
            delete action;
            messageCount++;
            int clientID = getClientID();

            m_operation->m_botDB->insertAction(json, m_runTime, clientID);
        } else {

        }
    }

    for (int i=0; i<m_userNames.size(); i++) {
        QString user = m_userNames.at(i);
        if (m_message.size() > 0) {

            // output the script command to perform
            if (m_attachments.size() > 0) {
            } else {
                qDebug() << "cronsendmsg " << user << " " << m_ttl << " \"" << m_message << "\"\n";
            }

            // TODO: FOr now create list of users, size of one
            QStringList users;
            users.append(user);

            // put the command into the database
            CreateJsonAction *action = new CreateJsonAction("sendmessage", users, m_ttl, m_message, m_attachments);
            QByteArray json = action->toByteArray();
            delete action;
            messageCount++;

            int clientID;
            if (m_operation->m_client == NULL) {
                clientID = 0;
            } else {
                clientID = m_operation->m_client->id;
            }

            m_operation->m_botDB->insertAction(json, m_runTime, clientID);

        } else {

        }
    }

    if (! m_vgroupid.isEmpty()) {
        if (m_message.size() > 0) {
            // put the command into the database
            CreateJsonAction *action = new CreateJsonAction("sendmessage", m_vgroupid, m_ttl, m_message, m_attachments, true);
            QByteArray json = action->toByteArray();
            delete action;
            messageCount++;

            int clientID;
            if (m_operation->m_client == NULL) {
                clientID = 0;
            } else {
                clientID = m_operation->m_client->id;
            }
            m_operation->m_botDB->insertAction(json, m_runTime, clientID);
        }
    }

    // Cleanup the messages
    if (m_attachments.size() > 0 && m_rmAttachmentWhenDone) {
        // TODO: Handle multiple attachments
//        qDebug() << "shell rm \"" << m_attachments << "\"\n";
    }

    QString response;

    response = QString("{\"operation\" : {\"action\" : \"sendmessage\", \"status\" : 0 }}");
    m_operation->addResponseMessage(response);

    return messageCount;
}

/**
 * @brief WickrBotJsonData::parse
 * @param operation
 * @param jsonString
 * @return
 */
bool WickrBotJsonData::parse(QByteArray jsonString) {
    QJsonParseError jsonError;
    QJsonDocument jsonResponse = QJsonDocument().fromJson(jsonString, &jsonError);

    if (jsonError.error != jsonError.NoError) {
        m_operation->log("Error with JSON document" + jsonError.errorString());
        return false;
    }

    if (! processJsonDoc(jsonResponse)) {
        m_operation->log("Error reading json data!");
        return false;
    }

    int processedCnt = processOperationData();
    m_operation->messageCount += processedCnt;

    QString message = QString("Wrote %1 messages").arg(processedCnt);
    m_operation->log(message);

    return true;
}

/**
 * @brief WickrBotJsonData::parseSendMessage
 * Call this function if the JSON string is specifically for a sendmessage action.
 * The send message values are not contained in an "operation" JSON object
 * @param operation
 * @param jsonString
 * @return
 */
bool
WickrBotJsonData::parseSendMessage(QByteArray jsonString) {
    QJsonParseError jsonError;
    QJsonDocument jsonResponse = QJsonDocument().fromJson(jsonString, &jsonError);

    if (jsonError.error != jsonError.NoError) {
        m_operation->log("Error with JSON document" + jsonError.errorString());
        return false;
    }

    if (! processSendMessageJsonDoc(jsonResponse.object())) {
        m_operation->log("Error reading json data!");
        return false;
    }

    m_action = "sendmessage";

    int processedCnt = processOperationData();
    m_operation->messageCount += processedCnt;

    QString message = QString("Wrote %1 messages").arg(processedCnt);
    m_operation->log(message);

    return true;
}
