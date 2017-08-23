#ifndef WICKRIOLOGINHDLR_H
#define WICKRIOLOGINHDLR_H

#include <QObject>
#include <QString>

#include "common/wickrRequests.h"
#include "session/wickrPreRegistrationIface.h"

#include "operationdata.h"

class WickrBotLogin {
public:
    QString m_name;
    QString m_pass;
    QString m_userName;
    QString m_transactionID;
    int m_sent;
    int m_failedLogin;
    bool m_creating;

    WickrBotLogin(const QString& name, const QString& pass, const QString& userName, const QString& transID, bool create) :
        m_name(name),
        m_pass(pass),
        m_userName(userName),
        m_transactionID(transID),
        m_creating(create) {}
};

typedef enum { LoggedOut, InProcess, LoggedIn, LoggingOut, LoginsFailed } WickrIOLoginState;


class WickrIOLoginHdlr : public QObject
{
    Q_OBJECT
public:
    explicit WickrIOLoginHdlr();
    ~WickrIOLoginHdlr();

    void initiateLogin();

    WickrIOLoginState getLoginState() { return m_loginState; }

    /**
     * @brief addLogin
     * Add a user to the list of users used to login with.
     * @param user The username of the new userSS
     * @param pass The password for the new user
     */
    void addLogin(const QString& user, const QString& pass, const QString& userName, const QString& transid=QString(), bool create=false) {
        m_logins.append(new WickrBotLogin(user, pass, userName, transid, create));
    }


    void preRegistrationInit();
    QStringList preRegistrationGetKeyStrings();
    QByteArray getSigningKey() { return m_userSigningKey; }

private:
    int m_curLoginIndex;
    WickrIOLoginState m_loginState;
    QList<WickrBotLogin *> m_logins;
    int m_consecutiveLoginFailures;
    bool m_firstLogin;
    long m_backupVersion;
    QByteArray m_userSigningKey;

    void loginNextUser();
    void refreshDirectory();



    WickrCore::WickrPreRegistrationIface *m_preRegDataIface;

signals:
    void signalExit();
    void signalLoginFailed();
    void signalLoginSuccess();
    void signalOnlineFlag(bool);

    void signalNetworkStatus(bool);

private slots:
    void slotRegistrationDone(WickrRegisterUserContext *c);
    void slotLoginDone(WickrLoginContext *ls);

    void slotRefreshDirectoryDone(WickrDirectoryGetContext* context);

    void slotLoginStart(const QString& username, const QString& password);

public slots:
    void slotRegisterOnPrem(const QString &username, const QString &password, const QString &newPassword, const QString &salt, const QString &transactionid, bool newUser, bool sync);
    void slotRegisterUser(const QString &wickrid, const QString &password, const QString &transactionid, bool newUser, bool sync, bool isRekey);

};

#endif // WICKRIOLOGINHDLR_H
