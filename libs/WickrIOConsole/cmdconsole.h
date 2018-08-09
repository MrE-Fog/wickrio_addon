#ifndef CMDCONSOLE_H
#define CMDCONSOLE_H

#include <QObject>
#include <QCoreApplication>
#include <QString>
#include <QSettings>

#include "wickriodatabase.h"
#include "wickrIOAppSettings.h"
#include "cmdbase.h"
#include "cmdoperation.h"
#include "consoleserver.h"

class CmdConsole : public CmdBase
{
    Q_OBJECT

public:
    explicit CmdConsole(CmdOperation *operation);
    ~CmdConsole();

    bool runCommands(QString commands=QString());
    void status();

private:
    void addConsoleUser();
    void configConsole();
    void deleteConsoleUser(int index);
    void listConsoleUsers();
    void modifyConsoleUser(int index);
    bool validateIndex(int consoleUserIndex);

    bool getConsoleUserValues(WickrIOConsoleUser *cuser);

    bool chkInterfaceExists(const QString& iface, int port);

private:
    CmdOperation *m_operation;
    WickrIOSSLSettings m_sslSettings;
    QList<WickrIOConsoleUser *> m_consoleUsers;
    ConsoleServer *m_consoleServer;
};

#endif // CMDCONSOLE_H
