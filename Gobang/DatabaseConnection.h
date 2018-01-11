#ifndef DATABASECONNECTION_H
#define DATABASECONNECTION_H
#include <QSqlDatabase>

class DatabaseConnection
{
public:
    DatabaseConnection();
    QSqlDatabase db();
    bool connect();
    void close();
private:
    QSqlDatabase m_db;
};

#endif // DATABASECONNECTION_H
