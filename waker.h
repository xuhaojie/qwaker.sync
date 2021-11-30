#ifndef __WAKER_H__
#define __WAKER_H__

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCookieJar>
#include <QNetworkCookie>

const QString LOGIN_URL ="https://192.168.5.1:8443/login.cgi";

class Waker : public QObject {
    Q_OBJECT
public:

    explicit Waker(const QString&url, const QString& userName, const QString& password);
    virtual ~Waker();
    bool login();
    bool execute_command(const QString& cmd);
    bool logout();

protected:
    QSslConfiguration ssl_config;
    QNetworkAccessManager* manager;
    QString base_url;
    QString auth;
    QString token;
};

#endif // __WAKER_H__
