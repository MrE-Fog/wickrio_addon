#ifndef SERVER_COMMON_H
#define SERVER_COMMON_H

#include <QString>
#include <QStringList>
#include <QSettings>
#include <QList>

#include "wickrbotclients.h"
#include "wickrIOParsers.h"

/**
 * @brief The WBIOBotTypes class
 * This class is used to identify the hardcoded types of Bots supported (i.e. hubot, supportbot)
 * It should be used to support customer additional bot types.
 */
class WBIOBotTypes
{
public:
    WBIOBotTypes(const QString& name, const QString& type, const QString& swLoc, const QString& installer, const QString& configure) :
        m_name(name),
        m_type(type),
        m_swLocation(swLoc),
        m_installer(installer),
        m_configure(configure) {}

    QString m_name;
    QString m_type;
    QString m_swLocation;
    QString m_installer;
    QString m_configure;

    QString name()       { return m_name; }
    QString type()       { return m_type; }
    QString swLocation() { return m_swLocation; }
    QString installer()  { return m_installer; }
    QString configure()  { return m_configure; }
};


/**
 * @brief The WBIOClientApps class
 * This class is used to identify the applications associated with the known client apps
 */
class WBIOClientApps
{
public:
    WBIOClientApps(const QString& bot, const QString& provision, const QString& parser, bool pwRequired, bool isMotherBot) :
        m_botApp(bot),
        m_provisionApp(provision),
        m_parserApp(parser),
        m_pwRequired(pwRequired),
        m_isMotherBot(isMotherBot) {}

    QString m_botApp;
    QString m_provisionApp;
    QString m_parserApp;
    bool    m_pwRequired;
    bool    m_isMotherBot;

    QList<WBIOBotTypes *> m_supportedBots;

    QString bot()       { return m_botApp; }
    QString provision() { return m_provisionApp; }
    QString parser()    { return m_parserApp; }
    bool pwRequired()   { return m_pwRequired; }
    bool isMotherBot()  { return m_isMotherBot; }

    QList<WBIOBotTypes *> supportedBots() { return m_supportedBots; }
    void addBot(WBIOBotTypes *bot) { m_supportedBots.append(bot); }
};

/**
 * @brief The WBIOServerCommon class
 * This class identifies common WBIO server static functions
 */
class WBIOServerCommon
{
public:
    WBIOServerCommon() {}

    static QSettings *getSettings();
    static QString getDBLocation();

    static void initClientApps();
    static QString getClientProcessName(WickrBotClients *client);
    static QStringList getAvailableClientApps();
    static QString getProvisionApp(const QString& clientApp);
    static QString getParserApp(const QString& clientApp);
    static bool isValidClientApp(const QString& binaryName);
    static bool isPasswordRequired(const QString& binaryName);

    static QString getParserProcessName(WickrIOParsers * parser);
    static QStringList getAvailableParserApps();

    static QStringList getAvailableMotherClients();

    static QList<WBIOBotTypes *> getBotsSupported(const QString& clientApp);
    static QString getBotSoftwarePath(const QString& botType);
    static QString getBotInstaller(const QString& botType);
    static QString getBotConfigure(const QString& botType);

private:
    static bool                     m_initialized;
    static QList<WBIOClientApps *>  m_botApps;
    static QStringList              m_bots;
    static QStringList              m_parsers;

    static QList<WBIOBotTypes *>    m_supportedBots;
};


#endif // SERVER_COMMON_H
