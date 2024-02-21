#include "mytcpserver.h"
#include <QDebug>

myTcpServer::myTcpServer()
{

}

myTcpServer &myTcpServer::getInstance()
{
    static myTcpServer instance;
    return instance;
}

void myTcpServer::incomingConnection(qintptr socketDescriptor)
{
    qDebug() << "New Client Connect";
    myTcpSocket *pTcpSocket = new myTcpSocket;
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    tcpSocketList.append(pTcpSocket);

    connect(pTcpSocket, SIGNAL(offline(myTcpSocket*)), this, SLOT(deleteSocket(myTcpSocket*)));
}

void myTcpServer::resend(const char *n, PDU *pdu)
{
    if (n == NULL || pdu == NULL)
    {
        return;
    }
    QString strName = n;
    for (int i = 0; i < tcpSocketList.size(); i ++)
    {
        if (strName == tcpSocketList.at(i)->getName())
        {
            tcpSocketList.at(i)->write((char*)pdu, pdu->uiPDULen);
            break;
        }
    }
}

void myTcpServer::deleteSocket(myTcpSocket *socket)
{
    int index = tcpSocketList.indexOf(socket);
    if (index != -1) {
        myTcpSocket *socketToDelete = tcpSocketList.takeAt(index);
        socketToDelete->deleteLater(); // Use deleteLater() to schedule the deletion
    }
//    QList<myTcpSocket*>::iterator iter = tcpSocketList.begin();
//    for (; iter != tcpSocketList.end(); iter ++)
//    {
//        if (socket == *iter)
//        {
//            delete *iter;
//            *iter = NULL;
//            tcpSocketList.erase(iter);
//            break;
//        }
//    }
    for (int i = 0; i < tcpSocketList.size(); ++i)
    {
        qDebug() << tcpSocketList.at(i)->getName();
    }
}
