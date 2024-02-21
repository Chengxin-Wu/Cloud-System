#include "book.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>

Book::Book(QWidget *parent)
    : QWidget{parent}
{
    m_pTimer = new QTimer;
    m_bDownload = false;

    m_pBookListW = new QListWidget;
    m_pReturnPB = new QPushButton("Return");
    m_pCreateDirPB = new QPushButton("Create Directory");
    m_pDelDirPB = new QPushButton("Delete Directory");
    m_pRenamePB = new QPushButton("Rename");
    m_pFlushPB = new QPushButton("Reload");

    QVBoxLayout *pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturnPB);
    pDirVBL->addWidget(m_pCreateDirPB);
    pDirVBL->addWidget(m_pDelDirPB);
    pDirVBL->addWidget(m_pRenamePB);
    pDirVBL->addWidget(m_pFlushPB);

    m_pUplaodPB = new QPushButton("Upload File");
    m_pDownLoadPB = new QPushButton("Download File");
    m_pDelFilePB = new QPushButton("Delete File");
    m_pShareFilePB = new QPushButton("Share File");

    QVBoxLayout *pFileVBL = new QVBoxLayout;
    pFileVBL->addWidget(m_pUplaodPB);
    pFileVBL->addWidget(m_pDownLoadPB);
    pFileVBL->addWidget(m_pDelFilePB);
    pFileVBL->addWidget(m_pShareFilePB);

    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget((m_pBookListW));
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);

    setLayout(pMain);

    connect(m_pCreateDirPB,SIGNAL(clicked(bool)), this, SLOT(createDir()));
    connect(m_pFlushPB, SIGNAL(clicked(bool)), this, SLOT(flushFile()));
    connect(m_pDelDirPB, SIGNAL(clicked(bool)), this, SLOT(delDir()));
    connect(m_pRenamePB, SIGNAL(clicked(bool)), this, SLOT(renameFile()));
    connect(m_pBookListW, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(enterDir(QModelIndex)));
    connect(m_pReturnPB, SIGNAL(clicked(bool)), this, SLOT(returnPev()));
    connect(m_pUplaodPB, SIGNAL(clicked(bool)), this, SLOT(uploadFile()));
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(uploadFileData()));
    connect(m_pDelFilePB, SIGNAL(clicked(bool)), this, SLOT(delFile()));
    connect(m_pDownLoadPB, SIGNAL(clicked(bool)), this, SLOT(downloadFile()));
}

void Book::updateFileList(const PDU *pdu)
{
    if (pdu == NULL)
    {
        return;
    }
    m_pBookListW->clear();

    FileInfo *pfileInfo = NULL;
    int iCount = pdu->uiMsgLen/sizeof(FileInfo);
    for (int i = 0; i < iCount; i ++)
    {
        pfileInfo = (FileInfo*)(pdu->caMsg) + i;
        QListWidgetItem *pItem = new QListWidgetItem;
        if (pfileInfo->iFileType == 0)
        {
            pItem->setIcon(QIcon(QPixmap(":/Folder_Icon.svg")));
        }
        else if (pfileInfo->iFileType == 1)
        {
            pItem->setIcon(QIcon(QPixmap(":/file.png")));
        }
        pItem->setText(pfileInfo->caName);
        m_pBookListW->addItem(pItem);
    }
}

void Book::setDownloadStatus(bool status)
{
    m_bDownload = status;
}

bool Book::getDownloadStatus()
{
    return m_bDownload;
}

QString Book::getSaveFilePath()
{
    return m_strSaveFilePath;
}

void Book::createDir()
{
    QString strNewDir = QInputDialog::getText(this, "Create Dir", "Dir Name");
    if(!strNewDir.isEmpty())
    {
        if (strNewDir.size() > 32)
        {
            QMessageBox::warning(this, "Dir Name", "Dir Name too Long");
        }
        else
        {
            QString strName = TcpClient::getInstance().loginName();
            QString strCurPath = TcpClient::getInstance().curPath();
            PDU *pdu = mkPDU(strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_CREATE_DIR_REQUEST;
            strncpy(pdu->caData, strName.toStdString().c_str(), strName.size());
            strncpy(pdu->caData+32, strNewDir.toStdString().c_str(), strNewDir.size());
            memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());

            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            delete pdu;
            pdu = NULL;
        }
    }
    else
    {
        QMessageBox::warning(this, "Dir Name", "Dir Name Could Not be Empty");
    }

}

void Book::flushFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    newCurPath = strCurPath;
    PDU *pdu = mkPDU(strCurPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strncpy((char*)(pdu->caMsg), strCurPath.toStdString().c_str(), strCurPath.size());
    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    delete pdu;
    pdu = NULL;
}

void Book::delDir()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (pItem == NULL)
    {
        return;
    }
    else
    {
        QString strDelName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        strncpy(pdu->caData, strDelName.toStdString().c_str(), strDelName.size());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        delete pdu;
        pdu = NULL;
    }
}

void Book::renameFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (pItem == NULL)
    {
        return;
    }
    else
    {
        QString strOldName = pItem->text();
        QString strNewName = QInputDialog::getText(this, "Rename", "Input the new name");
        if (!strNewName.isEmpty())
        {
            PDU *pdu = mkPDU(strCurPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_RENAME_FILE_REQUEST;
            strncpy(pdu->caData, strOldName.toStdString().c_str(), strOldName.size());
            strncpy(pdu->caData+32, strNewName.toStdString().c_str(), strNewName.size());
            memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());

            TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
            delete pdu;
            pdu = NULL;
        }
        else
        {
            QMessageBox::warning(this, "Rename", "New Name Cannot be Empty");
        }
    }
}

void Book::enterDir(const QModelIndex &index)
{
    QString dirName = index.data().toString();
    QString strCurPath = TcpClient::getInstance().curPath() + '/' + dirName;
    newCurPath = strCurPath;
    PDU *pdu = mkPDU(strCurPath.size()+1);
//    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_REQUEST;
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strncpy(pdu->caData, dirName.toStdString().c_str(), dirName.size());
    memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());

    TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
    delete pdu;
    pdu = NULL;
}

void Book::returnPev()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QString strRootPath = "./" + TcpClient::getInstance().loginName();
    if (strCurPath == strRootPath)
    {
        QMessageBox::warning(this, "Return", "FAIL: this is the root path");
    }
    else
    {
        int index = strCurPath.lastIndexOf('/');
        strCurPath.remove(index, strCurPath.size() - index);
        newCurPath = strCurPath;
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());

        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        delete pdu;
        pdu = NULL;
    }
}

void Book::uploadFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    if (!m_strUploadFilePath.isEmpty())
    {
        int index = m_strUploadFilePath.lastIndexOf('/');
        QString strFileName = m_strUploadFilePath.right(m_strUploadFilePath.size()-index-1);

        QFile file(m_strUploadFilePath);
        qint64 fileSize = file.size();

        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        sprintf(pdu->caData, "%s %lld", strFileName.toStdString().c_str(), fileSize);

        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        delete pdu;
        pdu = NULL;

        m_pTimer->start(1000);
    }
    else
    {
        QMessageBox::warning(this, "Upload File", "file cannot be empty");
    }
}

void Book::delFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (pItem == NULL)
    {
        return;
    }
    else
    {
        QString strDelName = pItem->text();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DEL_DIR_REQUEST;
        strncpy(pdu->caData, strDelName.toStdString().c_str(), strDelName.size());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());
        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        delete pdu;
        pdu = NULL;
    }
}

void Book::downloadFile()
{
    QString strCurPath = TcpClient::getInstance().curPath();
    QListWidgetItem *pItem = m_pBookListW->currentItem();
    if (pItem == NULL)
    {
        return;
    }
    else
    {
        QString strSaveFilePath = QFileDialog::getSaveFileName();
        if (strSaveFilePath.isEmpty())
        {
            QMessageBox::warning(this, "Dowload File", "Set the download path");
            m_strSaveFilePath.clear();
        }
        else
        {
            m_strSaveFilePath = strSaveFilePath;
        }

        QString strCurPath = TcpClient::getInstance().curPath();
        PDU *pdu = mkPDU(strCurPath.size()+1);
        pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
        QString strFileName = pItem->text();
        strcpy(pdu->caData, strFileName.toStdString().c_str());
        memcpy(pdu->caMsg, strCurPath.toStdString().c_str(), strCurPath.size());

        TcpClient::getInstance().getTcpSocket().write((char*)pdu, pdu->uiPDULen);
        delete pdu;
        pdu = NULL;
    }
}

void Book::uploadFileData()
{
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, "Upload File", "Fail to Open the File");
        return;
    }

    char *pBuffer = new char[4096];
    qint64 ret = 0;
    while(true)
    {
        ret = file.read(pBuffer, 4096);
        if (ret > 0 && ret <= 4096)
        {
            TcpClient::getInstance().getTcpSocket().write(pBuffer, ret);
        }
        else if (ret == 0)
        {
            break;
        }
        else
        {
            QMessageBox::warning(this, "Uploaf File", "Fail to Upload the File");
            break;
        }
    }
    file.close();
    delete []pBuffer;
    pBuffer = NULL;
}

QString Book::getNewPath()
{
    return newCurPath;
}
