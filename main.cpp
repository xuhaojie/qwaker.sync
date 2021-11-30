#include <QDebug>
#include <QCoreApplication>
#include <QThread>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QStandardPaths>
#include "config.h"
#include "waker.h"

void genDefaultConfig(Config& config) {
    config.set("url","https://user.ddns.net:443");
    config.set("user","admin");
    config.set("password","admin");
    QVariantMap map;
    map.insert("PC","11:22:33:44:55:66");
    map.insert("Printer","AA:BB:CC:DD:EE:FF");
    config.set("targets", map);
}

QString findTarget(const Config& config, const QString& name) {
    QVariant v = config.get("targets");
    QString mac;
    auto map = v.toMap();
    auto it = map.find(name);
    if (it != map.end()) {
        mac = it.value().toString();
    }
    return mac;
}

int main(int argc, char *argv[]) {

    QCoreApplication app(argc, argv);

//    app.setApplicationName("qwaker");
//    app.setOrganizationName("www.autopard.com");

    QCommandLineParser parser;
//    parser.setApplicationDescription("a wake on lan tools");
//    parser.addHelpOption();
//    parser.addVersionOption();


    QCommandLineOption genOption( QStringList() << "g" << "gen", QCoreApplication::translate("main", "generate sample config."));
    parser.addOption(genOption);

    QCommandLineOption listption( QStringList() << "l" << "list", QCoreApplication::translate("main", "list targets."));
    parser.addOption(listption);

    QCommandLineOption targetMacOption(QStringList() << "m" << "mac", QCoreApplication::translate("main", "wake up tager by mac"), QCoreApplication::translate("main", "mac"));
    parser.addOption(targetMacOption);

    QCommandLineOption targetNameOption(QStringList() << "n" << "name", QCoreApplication::translate("main", "wake up tager by name"), QCoreApplication::translate("main", "name"));
    parser.addOption(targetNameOption);

    parser.process(app);

    const QStringList args = parser.positionalArguments();

    const QString configFileName = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qwaker.cfg";

    bool genConfig = parser.isSet(genOption);
    if(genConfig) {
        Config config;
        genDefaultConfig(config);
        if(config.save(configFileName)){
            printf("sample config generated.\n");
        } else {
            printf("generate sample config failed.\n");
        }
        return 0;
    }

    Config config;

    if(config.load(configFileName)) {
        printf("config file %s loaded.\n", configFileName.toLocal8Bit().data());
    } else {
        printf("can't load config file %s!\n", configFileName.toLocal8Bit().data());
        return 0;
    }

    bool listTarget = parser.isSet(listption);
    if(listTarget) {
        QVariant v = config.get("targets");
        QVariantMap map = v.toMap();

        QVariantMap::iterator it;
        for(it=map.begin(); it!=map.end(); it++) {
            printf("%10s : %s\n",it.key().toStdString().c_str(),it.value().toString().toStdString().c_str());
        }
        return 0;
    }

    QString targetMac;
    if(parser.isSet(targetMacOption)) {
        targetMac = parser.value(targetMacOption);
    }

    QString targetName;
    if(parser.isSet(targetNameOption)) {
        targetName = parser.value(targetNameOption);
        targetMac = findTarget(config, targetName);
        if(targetMac.isEmpty()){
            printf("target not found!\n");
            return -1;
        }
    }

    if(targetMac.isEmpty()) {
        parser.showHelp();
        return 0;
    }

    // http://stackoverflow.com/questions/27982443/qnetworkaccessmanager-crash-related-to-ssl
    // qunsetenv("OPENSSL_CONF");

    const QString url = config.get("url").toString();
    const QString user = config.get("user").toString();
    const QString password = config.get("password").toString();

    Waker *w = new Waker(url, user, password);

    if(w->login()){
        printf("login succeed.\n");
    } else {
        printf("login failed.\n");
        return -1;
    }

    QString cmd = "ether-wake+-i+br0+-b+" + QUrl::toPercentEncoding(targetMac);
    if (w->execute_command(cmd)) {
        printf("execute command succeed.\n");
    } else {
        printf("execute command failed.\n");
    }

    if (w->logout()) {
        printf("logout succeed.\n");
    } else {
        printf("logout failed.\n");
        return -1;
    }
    return 0;
}

