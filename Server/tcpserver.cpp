#include "tcpserver.h"
#include "./ui_tcpserver.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include <QFile>


TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);

    loadConfig();

    myTcpServer::getInstance().listen(QHostAddress(IP_address), port);
}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    QFile file(":/server.config");
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray data = file.readAll();
        QString strData = data.toStdString().c_str();
        file.close();

        strData.replace("\n", " ");
        QStringList data_list = strData.split(" ");

        IP_address = data_list.at(0);
        port = data_list.at(1).toUShort();
//        qDebug() << "IP: " << IP_address << " PORT: " << port;
    }
    else
    {
        QMessageBox::critical(this, "Open Config", "Fail");
    }
}


