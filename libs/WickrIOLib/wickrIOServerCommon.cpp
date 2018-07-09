#include <QStandardPaths>
#include <QDir>
#include <QDebug>

#include "wickrIOCommon.h"
#include "wickrIOServerCommon.h"
#include "wickrbotsettings.h"
#include "wickrioapi.h"

bool WBIOServerCommon::m_initialized = false;
QList<WBIOClientApps *>  WBIOServerCommon::m_botApps;
QStringList              WBIOServerCommon::m_bots;
QStringList              WBIOServerCommon::m_parsers;
QList<WBIOBotTypes *>    WBIOServerCommon::m_supportedBots;

/**
 * @brief WBIOCommon::getSettings
 * This function will return the QSettings object which is associated with the
 * settings/configuration for the WickrIO server applications.
 * @return
 */
QSettings *
WBIOServerCommon::getSettings()
{
    QSettings *settings;

#ifdef Q_OS_WIN
    settings = new QSettings(QString(WBIO_SERVER_SETTINGS_FORMAT).arg(WBIO_ORGANIZATION).arg(WBIO_GENERAL_TARGET), QSettings::NativeFormat);
#else
    QString settingsFile = QString("/opt/%1/%2.ini").arg(WBIO_GENERAL_TARGET).arg(WBIO_GENERAL_TARGET);
    settings = new QSettings(settingsFile, QSettings::NativeFormat);
#endif

    return settings;
}

/**
 * @brief WBIOCommon::getDBLocation
 * This function will return the default database location for the WickrIO
 * server based applications.
 * @return
 */
QString
WBIOServerCommon::getDBLocation()
{
    QString dbLocation;
    QSettings *settings = WBIOServerCommon::getSettings();

    settings->beginGroup(WBSETTINGS_DATABASE_HEADER);
    dbLocation = settings->value(WBSETTINGS_DATABASE_DIRNAME, "").toString();
    if (dbLocation.isEmpty()) {
#ifdef Q_OS_WIN
        dbLocation = QString("%1/%2/%3")
                .arg(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation))
                .arg(WBIO_ORGANIZATION)
                .arg(WBIO_GENERAL_TARGET);
#else
        dbLocation = QString("/opt/%1").arg(WBIO_GENERAL_TARGET);
#endif
        settings->setValue(WBSETTINGS_DATABASE_DIRNAME, dbLocation);
        settings->sync();
    }
    settings->endGroup();

    WBIOCommon::makeDirectory(dbLocation);
    return dbLocation;
}


QString
WBIOServerCommon::getClientProcessName(WickrBotClients * client)
{
    QString processName = QString("%1.%2").arg(client->binary).arg(client->name);
    return processName;
}

QString
WBIOServerCommon::getParserProcessName(WickrIOParsers * parser)
{
    QString processName(parser->name);
    return processName;
}
/**
 * @brief WBIOServerCommon::getParserProcessName
 * Gets name of the parser. Currently allows only one parser per system
 * @return
 */
QString
WBIOServerCommon::getParserProcessName()
{
    QString processName = WBIO_PARSER_PROCESS;
    return processName;
}
/**
 * @brief WBIOServerCommon::getParserApp
 * Gets the parser executable name. Curretnly allows only ony parser per system
 * @return
 */
QString
WBIOServerCommon::getParserApp(){
QString appName = WBIO_PARSER_TARGET;
    return appName;
}

void
WBIOServerCommon::initClientApps()
{
    if (!m_initialized) {

        WBIOBotTypes *hubot      = new WBIOBotTypes("hubot",      "hubot",      APIURL_MSGRECVCBACK,
                                                    "/usr/lib/wickr/integrations/software/hubot/software.tar.gz",
                                                    "install.sh", "configure.sh", "start.sh", "stop.sh" );
        WBIOBotTypes *supportBot = new WBIOBotTypes("supportBot", "supportbot", APIURL_MSGRECVLIB,
                                                    "/usr/lib/wickr/integrations/software/supportbot/software.tar.gz",
                                                    "install.sh", "configure.sh", "start.sh", "stop.sh" );
        WBIOServerCommon::m_supportedBots.append(hubot);
        WBIOServerCommon::m_supportedBots.append(supportBot);

        WBIOClientApps *wickrIOAlpha = new WBIOClientApps("wickrio_botAlpha", "wickrio_provAlpha", nullptr, false, false);
        WBIOClientApps *wickrIOBeta  = new WBIOClientApps("wickrio_botBeta",  "wickrio_provBeta",  nullptr, false, false);
        WBIOClientApps *wickrIOProd  = new WBIOClientApps("wickrio_bot",      "wickrio_prov",      nullptr, false, false);
        wickrIOAlpha->addBot(hubot); wickrIOAlpha->addBot(supportBot);
        wickrIOBeta->addBot(hubot);  wickrIOBeta->addBot(supportBot);
        wickrIOProd->addBot(hubot);  wickrIOProd->addBot(supportBot);

        WBIOServerCommon::m_botApps.append(wickrIOAlpha);
        WBIOServerCommon::m_botApps.append(wickrIOBeta);
        WBIOServerCommon::m_botApps.append(wickrIOProd);
        WBIOServerCommon::m_botApps.append(new WBIOClientApps("compliance_botAlpha", "compliance_provAlpha", nullptr,               true,  false));
        WBIOServerCommon::m_botApps.append(new WBIOClientApps("compliance_botBeta",  "compliance_provBeta",  nullptr,               true,  false));
        WBIOServerCommon::m_botApps.append(new WBIOClientApps("compliance_bot",      "compliance_prov",      nullptr,               true,  false));
        WBIOServerCommon::m_botApps.append(new WBIOClientApps("welcome_botAlpha",    nullptr,                "welcome_parserAlpha", false, false));
        WBIOServerCommon::m_botApps.append(new WBIOClientApps("welcome_botBeta",     nullptr,                "welcome_parserBeta",  false, false));
        WBIOServerCommon::m_botApps.append(new WBIOClientApps("welcome_bot",         nullptr,                "welcome_parser",      false, false));
        WBIOServerCommon::m_botApps.append(new WBIOClientApps("core_botAlpha",       nullptr,                nullptr,               false, true));
        WBIOServerCommon::m_botApps.append(new WBIOClientApps("core_botBeta",        nullptr,                nullptr,               false, true));
        WBIOServerCommon::m_botApps.append(new WBIOClientApps("core_bot",            nullptr,                nullptr,               false, true));

        for (WBIOClientApps *botapp : WBIOServerCommon::m_botApps) {
            m_bots.append(botapp->bot());
            if (!botapp->m_parserApp.isEmpty()) {
                WBIOServerCommon::m_parsers.append(botapp->m_parserApp);
            }
        }
        m_initialized = true;
    }
}

QStringList
WBIOServerCommon::getAvailableClientApps()
{
    initClientApps();
    QStringList availableBinaries;

    for (QString binary : WBIOServerCommon::m_bots) {
        QString filePath = QString("/usr/bin/%1").arg(binary);
        QFileInfo fi(filePath);
        if (fi.exists()) {
            availableBinaries.append(binary);
        }
    }
    return availableBinaries;
}

/**
 * @brief WBIOServerCommon::getAvailableParserApps
 * Returns a list of the binary names for the parser applications
 * @return
 */
QStringList
WBIOServerCommon::getAvailableParserApps()
{
    initClientApps();
    QStringList availableBinaries;

    for (QString binary : WBIOServerCommon::m_parsers) {
        QString filePath = QString("/usr/bin/%1").arg(binary);
        QFileInfo fi(filePath);
        if (fi.exists()) {
            availableBinaries.append(binary);
        }
    }
    return availableBinaries;
}

/**
 * @brief WBIOServerCommon::getAvailableMotherClients
 * Returns a list of the binary names for clients that are mother bots
 * @return
 */
QStringList
WBIOServerCommon::getAvailableMotherClients()
{
    initClientApps();
    QStringList availableBinaries;

    for (WBIOClientApps *botapp : WBIOServerCommon::m_botApps) {
        if (botapp->m_isMotherBot) {
            QString filePath = QString("/usr/bin/%1").arg(botapp->m_botApp);
            QFileInfo fi(filePath);
            if (fi.exists()) {
                availableBinaries.append(botapp->m_botApp);
            }
        }
    }
    return availableBinaries;
}

QString
WBIOServerCommon::getProvisionApp(const QString& clientApp)
{
    initClientApps();

    for (WBIOClientApps *botapp : WBIOServerCommon::m_botApps) {
        if (botapp->bot() == clientApp) {
            return botapp->provision();
        }
    }
    return nullptr;
}

QString
WBIOServerCommon::getParserApp(const QString& clientApp)
{
    initClientApps();

    for (WBIOClientApps *botapp : WBIOServerCommon::m_botApps) {
        if (botapp->bot() == clientApp) {
            return botapp->parser();
        }
    }
    return nullptr;
}

bool
WBIOServerCommon::isValidClientApp(const QString& binaryName)
{
    QStringList binaries = getAvailableClientApps();

    for (QString binary : binaries) {
        if (binary == binaryName) {
            return true;
        }
    }
    return false;
}

bool
WBIOServerCommon::isPasswordRequired(const QString& binaryName)
{
    WBIOServerCommon::initClientApps();

    for (WBIOClientApps *botapp : WBIOServerCommon::m_botApps) {
        if (botapp->bot() == binaryName) {
            return botapp->pwRequired();
        }
    }
    return false;
}

QList<WBIOBotTypes *>
WBIOServerCommon::getBotsSupported(const QString& clientApp)
{
    WBIOServerCommon::initClientApps();

    for (WBIOClientApps *botapp : WBIOServerCommon::m_botApps) {
        if (botapp->bot() == clientApp) {
            return botapp->supportedBots();
        }
    }
    return QList<WBIOBotTypes*>();
}

QString
WBIOServerCommon::getBotSoftwarePath(const QString& botType)
{
    WBIOServerCommon::initClientApps();

    for (WBIOBotTypes *bottypes : WBIOServerCommon::m_supportedBots) {
        if (bottypes->m_name == botType) {
            if (bottypes->m_swLocation.isEmpty())
                return QString();

            QString path = bottypes->swLocation();
            return path;
        }
    }
    return QString();
}

QString
WBIOServerCommon::getBotInstaller(const QString& botType)
{
    WBIOServerCommon::initClientApps();

    for (WBIOBotTypes *bottypes : WBIOServerCommon::m_supportedBots) {
        if (bottypes->m_name == botType) {
            return bottypes->m_installer;
        }
    }
    return QString();
}

QString
WBIOServerCommon::getBotConfigure(const QString& botType)
{
    WBIOServerCommon::initClientApps();

    for (WBIOBotTypes *bottypes : WBIOServerCommon::m_supportedBots) {
        if (bottypes->m_name == botType) {
            return bottypes->m_configure;
        }
    }
    return QString();
}

QString
WBIOServerCommon::getBotStartCmd(const QString& botType)
{
    WBIOServerCommon::initClientApps();

    for (WBIOBotTypes *bottypes : WBIOServerCommon::m_supportedBots) {
        if (bottypes->m_name == botType) {
            return bottypes->m_startCmd;
        }
    }
    return QString();
}

QString
WBIOServerCommon::getBotStopCmd(const QString& botType)
{
    WBIOServerCommon::initClientApps();

    for (WBIOBotTypes *bottypes : WBIOServerCommon::m_supportedBots) {
        if (bottypes->m_name == botType) {
            return bottypes->m_stopCmd;
        }
    }
    return QString();
}

QString
WBIOServerCommon::getBotMsgIface(const QString& botType)
{
    WBIOServerCommon::initClientApps();

    for (WBIOBotTypes *bottypes : WBIOServerCommon::m_supportedBots) {
        if (bottypes->m_name == botType) {
            return bottypes->m_msgIface;
        }
    }
    return QString();
}
