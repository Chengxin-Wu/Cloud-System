#include "mytcpsocket.h"
#include <QDebug>
#include "mytcpserver.h"
#include <QDir>
#include <QFileInfoList>

myTcpSocket::myTcpSocket()
{
    connect(this, SIGNAL(readyRead()), this, SLOT(recvMsg()));
    connect(this, SIGNAL(disconnected()), this, SLOT(clientOffline()));
    m_bUpload = false;
    m_pTimer = new QTimer;
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(sendFileToClient()));
}

QString myTcpSocket::getName()
{
    return m_strName;
}

void myTcpSocket::recvMsg()
{
    if (!m_bUpload)
    {
        qDebug() << this->bytesAvailable();
        uint uiPDULen = 0;
        this->read((char*)&uiPDULen, sizeof(uint));
        uint uiMsgLen = uiPDULen - sizeof(PDU);
        PDU *pdu = mkPDU(uiMsgLen);
        this->read((char*)pdu+sizeof(uint), uiPDULen - sizeof(uint));
//        qDebug() << pdu->uiMsgType << (char*)(pdu->caMsg);
        switch (pdu->uiMsgType)
        {
        case ENUM_MSG_TYPE_REGIST_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData+32, 32);
            qDebug() << caName << caPwd << pdu->uiMsgType;
            bool ret = OpeDB::getInstance().handleRegist(caName, caPwd);
            PDU *response = mkPDU(0);
            response->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;
            if (ret)
            {
                strcpy(response->caData, REGIST_SUCCESSED);
                QDir dir;
                dir.mkdir(QString("./%1").arg(caName));
            }
            else
            {
                strcpy(response->caData, REGIST_FAILED);
            }
            write((char*)response, response->uiPDULen);
            delete response;
            response = NULL;
            break;
        }
        case ENUM_MSG_TYPE_LOGIN_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            strncpy(caName, pdu->caData, 32);
            strncpy(caPwd, pdu->caData+32, 32);
            bool ret = OpeDB::getInstance().handleLogin(caName, caPwd);
            PDU *response = mkPDU(0);
            response->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;
            if (ret)
            {
                strcpy(response->caData, LOGIN_SUCCESSED);
                m_strName = caName;
            }
            else
            {
                strcpy(response->caData, LOGIN_FAILED);
            }
            write((char*)response, response->uiPDULen);
            delete response;
            response = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
        {
            QStringList ret = OpeDB::getInstance().handleAllOnline();
            uint uiMsgLen = ret.size() * 32;
            PDU *response = mkPDU(uiMsgLen);
            response->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for (int i = 0; i < ret.size(); i ++)
            {
                memcpy((char*)(response->caMsg) + i * 32, ret.at(i).toStdString().c_str(), ret.at(i).size());
            }
            write((char*)response, response->uiPDULen);
            delete response;
            response = NULL;
            break;
        }
        case ENUM_MSG_TYPE_SEARCH_USR_REQUEST:
        {
            int ret = OpeDB::getInstance().handleSearchUsr(pdu->caData);
            PDU *response = mkPDU(0);
            response->uiMsgType = ENUM_MSG_TYPE_SEARCH_USR_RESPOND;
            if (ret == -1)
            {
                strcpy(response->caData, SEARCH_USR_NONE);
            }
            else if (ret == 1)
            {
                strcpy(response->caData, SEARCH_USR_ONLINE);
            }
            else if (ret == 0)
            {
                strcpy(response->caData, SEARCH_USR_OFFLINE);
            }
            write((char*)response, response->uiPDULen);
            delete response;
            response = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            char caPerName[32] = {'\0'};
            char caName[32] = {'\0'};
            strncpy(caPerName, pdu->caData, 32);
            strncpy(caName, pdu->caData+32, 32);
            int ret = OpeDB::getInstance().handleAddFriend(caPerName, caName);
            PDU *response = NULL;
            if (ret == -1)
            {
                response = mkPDU(0);
                response->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(response->caData, UNKOWN_ERROR);
                write((char*)response, response->uiPDULen);
                delete response;
                response = NULL;
            }
            else if (ret == 0)
            {
                response = mkPDU(0);
                response->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(response->caData, EXISTED_FRIEND);
                write((char*)response, response->uiPDULen);
                delete response;
                response = NULL;
            }
            else
            {
                myTcpServer::getInstance().resend(caPerName, pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_CREATE_DIR_REQUEST:
        {
            QDir dir;
            qDebug() << QDir::currentPath();
            QString strCurPath = QString("%1").arg((char*)(pdu->caMsg));
            bool ret = dir.exists(strCurPath);
            PDU *response = NULL;
            if (ret) //current path exist
            {
                char caNewDir[32] = {'\0'};
                memcpy(caNewDir, pdu->caData+32, 32);
                QString strNewPath = strCurPath + "/" + caNewDir;
                qDebug() << strNewPath;
                ret = dir.exists(strNewPath);
                if (ret) // new path exist
                {
                    response = mkPDU(0);
                    response->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(response->caData, DIR_NAME_EXIST);
                }
                else
                {
                    dir.mkdir(strNewPath);
                    response = mkPDU(0);
                    response->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                    strcpy(response->caData, CREATE_DIR_SUCCESS);
                }
            }
            else
            {
                response = mkPDU(0);
                response->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_RESPOND;
                strcpy(response->caData, DIR_NOT_EXIST);
            }
            write((char*)response, response->uiPDULen);
            delete response;
            response = NULL;
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
        {
            char *pCurPath = new char[pdu->uiMsgLen];
            memcpy(pCurPath, pdu->caMsg, pdu->uiMsgLen);
            qDebug() << pCurPath;
            QFileInfo fileInfo(pCurPath);
            PDU *response = NULL;
            if (fileInfo.isDir())
            {
                QDir dir(pCurPath);
                QFileInfoList fileInfoList = dir.entryInfoList();
                int iFileCount = fileInfoList.size()-2;
                response = mkPDU(sizeof(FileInfo)*iFileCount);
                strcpy(response->caData, "True");
                response->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
                FileInfo *pFileInfo = NULL;
                QString strFileName;
                for (int i = 2; i < fileInfoList.size(); i ++)
                {
                    pFileInfo = (FileInfo*)(response->caMsg)+(i-2);
                    strFileName =fileInfoList[i].fileName();

                    memcpy(pFileInfo->caName, strFileName.toStdString().c_str(), strFileName.size());
                    if (fileInfoList[i].isDir())
                    {
                        pFileInfo->iFileType = 0;
                    }
                    else if (fileInfoList[i].isFile())
                    {
                        pFileInfo->iFileType = 1;
                    }
                }
            }
            else
            {
                response = mkPDU(0);
                strcpy(response->caData, "False");
            }
            write((char*)response, response->uiPDULen);
            delete response;
            response = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DEL_DIR_REQUEST:
        {
            char caName[32] = {'\0'};
            strcpy(caName, pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caName);

            QFileInfo fileInfo(strPath);
            bool ret = false;
            if (fileInfo.isDir())
            {
                QDir dir;
                dir.setPath(strPath);
                dir.removeRecursively();
                ret = true;
            }
            else if (fileInfo.isFile())
            {
                QDir dir;
                dir.remove(strPath);
                ret = true;
            }
            else
            {
                ret = false;
            }
            PDU *response = NULL;
            if (ret)
            {
                response = mkPDU(strlen(DEL_DIR_SUCCESS)+1);
                response->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(response->caData, DEL_DIR_SUCCESS, strlen(DEL_DIR_SUCCESS));
            }
            else
            {
                response = mkPDU(strlen(DEL_DIR_FAIL)+1);
                response->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_RESPOND;
                memcpy(response->caData, DEL_DIR_FAIL, strlen(DEL_DIR_FAIL));
            }
            delete []pPath;
            pPath = NULL;
            write((char*)response, response->uiPDULen);
            delete response;
            response = NULL;
            break;
        }
        case ENUM_MSG_TYPE_RENAME_FILE_REQUEST:
        {
            char caOldName[32] = {'\0'};
            char caNewName[32] = {'\0'};
            memcpy(caOldName, pdu->caData, 32);
            memcpy(caNewName, pdu->caData+32, 32);

            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strOldPath = QString("%1/%2").arg(pPath).arg(caOldName);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(caNewName);

            qDebug() << strOldPath;
            qDebug() << strNewPath;

            QDir dir;
            bool ret = dir.rename(strOldPath, strNewPath);
            PDU *response = mkPDU(0);
            response->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_RESPOND;
            if (ret)
            {
                strcpy(response->caData, RENAME_FILE_SUCCESS);
            }
            else
            {
                strcpy(response->caData, RENAME_FILE_FAIL);
            }
            write((char*)response, response->uiPDULen);
            delete response;
            response = NULL;
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            char caName[32] = {'\0'};
            qint64 fileSize = 0;
            sscanf(pdu->caData, "%s %lld", caName, &fileSize);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caName);
            qDebug() << strPath;
            delete []pPath;
            pPath = NULL;

            m_file.setFileName(strPath);
            if (m_file.open(QIODevice::WriteOnly))
            {
                m_bUpload = true;
                m_iTotal = fileSize;
                m_iRecved = 0;
            }

            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:
        {
            char caName[32] = {'\0'};
            strcpy(caName, pdu->caData);
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath, pdu->caMsg, pdu->uiMsgLen);
            QString strPath = QString("%1/%2").arg(pPath).arg(caName);
            qDebug() << strPath;
            delete []pPath;
            pPath = NULL;

            QFileInfo FileInfo(strPath);
            qint64 fileSize = FileInfo.size();

            PDU *response = mkPDU(0);
            sprintf(response->caData, "%s %lld", caName, fileSize);
            response->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
            write((char*)response, response->uiPDULen);
            delete response;
            response = NULL;

            m_file.setFileName(strPath);
            m_file.open(QIODevice::ReadOnly);
            m_pTimer->start(1000);

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
        PDU *response = mkPDU(0);
        response->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;

        QByteArray buff = readAll();
        m_file.write(buff);
        m_iRecved += buff.size();

        if (m_iTotal == m_iRecved)
        {
            m_file.close();
            m_bUpload = false;

            strcpy(response->caData, UPLOAD_FILE_SUCCESS);
            write((char*)response, response->uiPDULen);
            delete response;
            response = NULL;
        }
        else if (m_iTotal < m_iRecved)
        {
            m_file.close();
            m_bUpload = false;

            strcpy(response->caData, UPLOAD_FILE_FAIL);
            write((char*)response, response->uiPDULen);
            delete response;
            response = NULL;
        }
    }
}

void myTcpSocket::clientOffline()
{
    OpeDB::getInstance().handleOffline(m_strName.toStdString().c_str());
    emit offline(this);
}

void myTcpSocket::sendFileToClient()
{
    m_pTimer->stop();
    char *pData = new char[4096];
    qint64 ret = 0;
    while (true)
    {
        ret = m_file.read(pData, 4096);
        if (ret > 0 && ret <= 4096)
        {
            write(pData, ret);
        }
        else if (ret == 0)
        {
            m_file.close();
            break;
        }
        else if (ret < 0)
        {
            qDebug() << "fail to send file";
            m_file.close();
            break;
        }
    }
    delete []pData;
    pData = NULL;
}
