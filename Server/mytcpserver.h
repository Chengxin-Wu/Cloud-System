#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include "mytcpsocket.h"

class myTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    myTcpServer();

    static myTcpServer &getInstance();
    void incomingConnection(qintptr socketDescriptor);

    void resend(const char *n, PDU *pdu);

public slots:
    void deleteSocket(myTcpSocket *socket);

private:
    QList<myTcpSocket*> tcpSocketList;
};

#endif // MYTCPSERVER_H
