#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QTcpSocket>
#include "opewidget.h"
#include "protocol.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:
    TcpClient(QWidget *parent = nullptr);
    ~TcpClient();
    void loadConfig();

    static TcpClient &getInstance();

    QTcpSocket &getTcpSocket();
    QString loginName();
    QString curPath();
    void updateCurPath(QString path);

public slots:
    void showConnect();
    void recvMsg();

private slots:
//    void on_send_bt_clicked();

    void on_login_bt_clicked();

    void on_regist_bt_clicked();

    void on_Logout_bt_clicked();

private:
    Ui::TcpClient *ui;
    QString IP_address;
    quint16 port;

    QTcpSocket tcpSocket;
    QString m_strLoginName;

    QString m_strCurPath;
    QFile m_file;
};
#endif // TCPCLIENT_H
