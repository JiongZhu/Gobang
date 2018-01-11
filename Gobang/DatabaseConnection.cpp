#include "DatabaseConnection.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QCoreApplication>

DatabaseConnection::DatabaseConnection()
{

}

QSqlDatabase DatabaseConnection::db()
{
    return m_db;
}

bool DatabaseConnection::connect()
{
    //链接mysql数据库
    QCoreApplication::addLibraryPath("/opt/Qt5.9.3/5.9.3/gcc_64/plugins/sqldrivers/libqsqlmysql.so");
    if (QSqlDatabase::contains("qt_sql_default_connection"))
    {
        m_db = QSqlDatabase::database("qt_sql_default_connection");
    }
    else
    {
        m_db = QSqlDatabase::addDatabase("QMYSQL");
        m_db.setHostName("127.0.0.1");
        m_db.setDatabaseName("gobang");
        m_db.setUserName("root");
        m_db.setPassword("zhxy");
    }
    return m_db.open();
}

void DatabaseConnection::close()
{
    QString connection;
    connection = m_db.connectionName();
    m_db.close();
    m_db = QSqlDatabase();
    m_db.removeDatabase(connection);
}

