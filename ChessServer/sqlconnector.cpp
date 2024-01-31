#include "sqlconnector.h"

SqlConnector::SqlConnector() {
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("mysql-d373576-qt-chess.a.aivencloud.com");
    db.setPort(24452);
    db.setDatabaseName("defaultdb");
    db.setUserName("avnadmin");
    db.setPassword("AVNS_W3hw6cGxX-WFBdPk-Jn");
}

SqlConnector::~SqlConnector() {
    if (db.isOpen()) {
        db.close();
    }
}

bool SqlConnector::openConnection() {
    return db.open();
}

void SqlConnector::closeConnection() {
    db.close();
}

QSqlQuery SqlConnector::executeQuery(const QString &queryString) {
    QSqlQuery query;
    query.exec(queryString);
    return query;
}
