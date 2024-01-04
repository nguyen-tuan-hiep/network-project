#include "SqlConnector.h"

SqlConnector::SqlConnector() {
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("sql12.freemysqlhosting.net");
    db.setPort(3306);
    db.setDatabaseName("sql12674676");
    db.setUserName("sql12674676");
    db.setPassword("3MbHcwp7GH");
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
