#include "tcpclient.h"
#include "./ui_tcpclient.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>

TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    ui->setupUi(this);
    loadConfig();

    connect(&tcpSocket, &QTcpSocket::connected, this, &TcpClient::showConnect);
    connect(&tcpSocket, &QTcpSocket::readyRead, this, &TcpClient::recvMsg);

    tcpSocket.connectToHost(QHostAddress(IP_address), port);
}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray data = file.readAll();
        QString strData = data.toStdString().c_str();
        file.close();

        strData.replace("\n", " ");
        QStringList data_list = strData.split(" ");

        IP_address = data_list.at(0);
        port = data_list.at(1).toUShort();
    }
    else
    {
        QMessageBox::critical(this, "Open Config", "Fail");
    }
}

TcpClient &TcpClient::getInstance()
{
    static TcpClient instance;
    return instance;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return tcpSocket;
}

QString TcpClient::loginName()
{
    return m_strLoginName;
}

QString TcpClient::curPath()
{
    return m_strCurPath;
}

void TcpClient::updateCurPath(QString path)
{
    m_strCurPath = path;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this, "Connected to Server", "Success");
}

void TcpClient::recvMsg()
{
    if (!OpeWidget::getInstance().getBook()->getDownloadStatus())
    {
    //    qDebug() << tcpSocket.bytesAvailable();
        uint uiPDULen = 0;
        tcpSocket.read((char*)&uiPDULen, sizeof(uint));
        uint uiMsgLen = uiPDULen - sizeof(PDU);
        PDU *pdu = mkPDU(uiMsgLen);
        tcpSocket.read((char*)pdu+sizeof(uint), uiPDULen - sizeof(uint));
    //        qDebug() << pdu->uiMsgType << (char*)(pdu->caMsg);
    //    qDebug() << pdu->uiMsgType;
        switch (pdu->uiMsgType)
        {
        case ENUM_MSG_TYPE_REGIST_RESPOND:
        {
            if (0 == strcmp(pdu->caData, REGIST_SUCCESSED))
            {
                QMessageBox::information(this, "Regist", REGIST_SUCCESSED);
            }
            else
            {
                QMessageBox::warning(this, "Regist", REGIST_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_RESPOND:
        {
            if (0 == strcmp(pdu->caData, LOGIN_SUCCESSED))
            {
                m_strCurPath = QString("./%1").arg(m_strLoginName);
                QMessageBox::information(this, "Login", LOGIN_SUCCESSED);
                OpeWidget::getInstance().show();
                this->hide();
            }
            else
            {
                QMessageBox::warning(this, "Login", LOGIN_FAILED);
            }
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
        {
            OpeWidget::getInstance().getFriend()->showAllOnlineUsr(pdu);
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_RESPOND:
        {
            if (0 == strcmp(pdu->caData, SEARCH_USR_NONE))
            {
                QMessageBox::information(this, "Search", QString("%1: Not exist").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
            }
            else if (0 == strcmp(pdu->caData, SEARCH_USR_ONLINE))
            {

            }
            else if (0 == strcmp(pdu->caData, SEARCH_USR_OFFLINE))
            {
                QMessageBox::information(this, "Search", QString("%1: Offline").arg(OpeWidget::getInstance().getFriend()->m_strSearchName));
            }
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char caName[32] = {'\0'};
            strncpy(caName, pdu->caData+32, 32);
            int ret = QMessageBox::information(this, "Add Friend", QString("%1 want to add you as friend").arg(caName), QMessageBox::Yes, QMessageBox::No);
            PDU *response = mkPDU(0);
            memcpy(response->caData, pdu->caData, 32);
            if (ret == QMessageBox::Yes)
            {
                response->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGREE;
            }
            else
            {
                response->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
            }
            tcpSocket.write((char*)response, response->uiPDULen);
            delete response;
            response = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
        {
            QMessageBox::information(this, "Add Friend", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_RESPOND:
        {
            QMessageBox::information(this, "Create Folder", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND:
        {
            if (strcmp(pdu->caData, "True") == 0)
            {
                TcpClient::getInstance().updateCurPath(OpeWidget::getInstance().getBook()->getNewPath());
                OpeWidget::getInstance().getBook()->updateFileList(pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_RESPOND:
        {
            QMessageBox::information(this, "Del folder", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_RESPOND:
        {
            QMessageBox::information(this, "RENAME FILE", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:
        {
            QMessageBox::information(this, "Upload File", pdu->caData);
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:
        {
            qDebug() << pdu->caData;
            char caFileName[32] = {'\0'};
            sscanf(pdu->caData, "%s %lld", caFileName, &(OpeWidget::getInstance().getBook()->m_iTotal));
            qDebug() << 1;
            if (strlen(caFileName) > 0 && OpeWidget::getInstance().getBook()->m_iTotal > 0)
            {
                OpeWidget::getInstance().getBook()->setDownloadStatus(true);
                m_file.setFileName(OpeWidget::getInstance().getBook()->getSaveFilePath());
                qDebug() << 2;
                if (!m_file.open(QIODevice::WriteOnly))
                {
                    QMessageBox::warning(this, "Download File", "wrong download path");
                }
                else
                {
                    qDebug() << "start dowloand";
                }
            }
            break;
        }
        default:
            break;
        }
        delete pdu;
        pdu = NULL;
    }
    else
    {
        QByteArray buffer = tcpSocket.readAll();
        m_file.write(buffer);

        Book *pBook = OpeWidget::getInstance().getBook();
        pBook->m_iRecved += buffer.size();
        if (pBook->m_iTotal == pBook->m_iRecved)
        {
            m_file.close();
            pBook->m_iTotal = 0;
            pBook->m_iRecved = 0;
            QMessageBox::information(this, "Download FIle", "Finished");
            pBook->setDownloadStatus(false);
        }
        else if (pBook->m_iTotal <= pBook->m_iRecved)
        {
            m_file.close();
            pBook->m_iTotal = 0;
            pBook->m_iRecved = 0;
            pBook->setDownloadStatus(false);

            QMessageBox::critical(this, "Download File", "fail to download file");
        }

    }
}

/*
void TcpClient::on_send_bt_clicked()
{
    QString strMsg = ui->lineEdit->text();
    if (!strMsg.isEmpty())
    {
        PDU *pdu = mkPDU(strMsg.size());
        pdu->uiMsgType = 8888;
        memcpy(pdu->caMsg, strMsg.toStdString().c_str(), strMsg.size());
        tcpSocket.write((char*)pdu, pdu->uiPDULen);
        delete[] pdu;
        pdu = NULL;
    }
    else
    {
        QMessageBox::warning(this, "send message", "Message Cannot Be Empty");
    }
}
*/


void TcpClient::on_login_bt_clicked()
{
    QString strName = ui->name_le->text();
    QString strPws = ui->pwd_le->text();
    if (!strName.isEmpty() && !strPws.isEmpty())
    {
        m_strLoginName = strName;
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32);
        strncpy(pdu->caData+32, strPws.toStdString().c_str(), 32);
        tcpSocket.write((char*)pdu, pdu->uiPDULen);
        delete pdu;
        pdu = NULL;
    }
    else
    {
        QMessageBox::critical(this, "LOGIN", "User Name or Password cannot be Empty");
    }
}


void TcpClient::on_regist_bt_clicked()
{
    QString strName = ui->name_le->text();
    QString strPws = ui->pwd_le->text();
    if (!strName.isEmpty() && !strPws.isEmpty())
    {
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        strncpy(pdu->caData, strName.toStdString().c_str(), 32);
        strncpy(pdu->caData+32, strPws.toStdString().c_str(), 32);
        tcpSocket.write((char*)pdu, pdu->uiPDULen);
        delete pdu;
        pdu = NULL;
    }
    else
    {
        QMessageBox::critical(this, "Register", "User Name or Password cannot be Empty");
    }
}


void TcpClient::on_Logout_bt_clicked()
{

}

