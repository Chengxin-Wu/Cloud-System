#include "opedb.h"
#include <QMessageBox>

OpeDB::OpeDB(QObject *parent)
    : QObject{parent}
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");

}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;
    return instance;
}

void OpeDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("/Users/chengxinwu/Desktop/Project/TcpServer/cloud.db");
    if (m_db.open())
    {
        QSqlQuery query;
        query.exec("select * from usrInfo");
        while (query.next())
        {
            QString data = QString("%1,%2,%3").arg(query.value(0).toString()).arg(query.value(1).toString()).arg(query.value(2).toString());
//            ui->textEdit->append(data);
            qDebug() << data;
        }
    }
    else
    {
        QMessageBox::critical(NULL, "Open Database", "Fail");
    }
}

OpeDB::~OpeDB()
{
    m_db.close();
}

bool OpeDB::handleRegist(const char *name, const char *pwd)
{
    if (name == NULL || pwd == NULL)
    {
        return false;
    }
    QString data = QString("insert into usrInfo(name, pwd) values(\'%1\', \'%2\')").arg(name).arg(pwd);
    QSqlQuery query;
    return query.exec(data);
}

bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    if (name == NULL || pwd == NULL)
    {
        return false;
    }
    QString data = QString("select * from usrInfo where name=\'%1\' and pwd=\'%2\' and online=0").arg(name).arg(pwd);
    QSqlQuery query;
    query.exec(data);
    if (query.next())
    {
        QString data = QString("update usrInfo set online=1 where name=\'%1\' and pwd=\'%2\'").arg(name).arg(pwd);
        QSqlQuery query;
        query.exec(data);

        return true;
    }
    else
    {
        return false;
    }
}

void OpeDB::handleOffline(const char *name)
{
    if (NULL == name)
    {
        qDebug() << "Name is NuLL";
        return;
    }
    QString data = QString("update usrInfo set online=0 where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
}

QStringList OpeDB::handleAllOnline()
{
    QString data = QString("select name from usrInfo where online=1");

    QSqlQuery query;
    query.exec(data);
    QStringList result;
    result.clear();

    while (query.next())
    {
        result.append(query.value(0).toString());
    }
    return result;
}

int OpeDB::handleSearchUsr(const char *name)
{
    if (name == NULL)
    {
        return -1;
    }
    QString data = QString("select online from usrInfo where name=\'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
    if (query.next())
    {
        int ret = query.value(0).toInt();
        if (ret == 1)
        {
            return 1;
        }
        else if (ret == 0)
        {
            return 0;
        }
    }
    else
    {
        return -1;
    }
}

int OpeDB::handleAddFriend(const char *pername, const char *name)
{
    if (pername == NULL || name == NULL)
    {
        return -1;
    }
    QString data = QString("select * from friend where (id=(select id from usrInfo where name=\'%1\') and friendId=(select id from usrInfo where name=\'%2\'))"
                           " or (id=(select id from usrInfo where name=\'%3\') and friendId=(select id from usrInfo where name=\'%4\'))").arg(pername).arg(name).arg(name).arg(pername);
    QSqlQuery query;
    query.exec(data);
    if (query.next())
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


