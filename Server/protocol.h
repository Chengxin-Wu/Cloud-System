#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

typedef unsigned int uint;

#define REGIST_SUCCESSED "regist success"
#define REGIST_FAILED "regist filed: name exist"

#define LOGIN_SUCCESSED "login success"
#define LOGIN_FAILED "login filed"

#define SEARCH_USR_NONE "Did not find"
#define SEARCH_USR_ONLINE "Online"
#define SEARCH_USR_OFFLINE "Offline"

#define UNKOWN_ERROR "Unknow Error"

#define EXISTED_FRIEND "Friend Exist"

#define DIR_NOT_EXIST "Current Dir not Exist"
#define DIR_NAME_EXIST "File Name Exist"
#define CREATE_DIR_SUCCESS "Create DIr Successfilly"

#define DEL_DIR_SUCCESS "delete dir successfully"
#define DEL_DIR_FAIL "fail to delete dir: this is a regular file"

#define RENAME_FILE_SUCCESS "rename file successfully"
#define RENAME_FILE_FAIL "fail to rename file"

#define ENTER_DIR_SUCCESS "enter dir successfully"
#define ENTER_DIR_FAIL "fail to enter: this is a regular file"

#define UPLOAD_FILE_SUCCESS "upload file successfully"
#define UPLOAD_FILE_FAIL "fail to upload this file"

enum ENUM_MSG_TYPE
{
    ENUM_MSG_TYPE_MIN = 0,
    ENUM_MSG_TYPE_REGIST_REQUEST,      //注册请求
    ENUM_MSG_TYPE_REGIST_RESPOND,       //注册回复
    ENUM_MSG_TYPE_LOGIN_REQUEST,
    ENUM_MSG_TYPE_LOGIN_RESPOND,
    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,   //在线用户请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,   //在线用户回复
    ENUM_MSG_TYPE_SEARCH_USR_REQUEST,   //搜索用户请求
    ENUM_MSG_TYPE_SEARCH_USR_RESPOND,   //搜索用户回复
    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,   //添加好友请求
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND,   //添加好友回复
    ENUM_MSG_TYPE_ADD_FRIEND_AGREE,     //同意添加好友
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE,    //拒绝添加好友
    ENUM_MSG_TYPE_CREATE_DIR_REQUEST,   //创建文件夹的请求
    ENUM_MSG_TYPE_CREATE_DIR_RESPOND,   //创建文件夹的回复
    ENUM_MSG_TYPE_FLUSH_FILE_REQUEST,   //刷新文件请求
    ENUM_MSG_TYPE_FLUSH_FILE_RESPOND,   //刷新文件回复
    ENUM_MSG_TYPE_DEL_DIR_REQUEST,      //刷新文件夹请求
    ENUM_MSG_TYPE_DEL_DIR_RESPOND,      //刷新文件夹回复
    ENUM_MSG_TYPE_RENAME_FILE_REQUEST,  //重命名文件请求
    ENUM_MSG_TYPE_RENAME_FILE_RESPOND,  //重命名文件回复
    ENUM_MSG_TYPE_ENTER_DIR_REQUEST,    //进入文件夹请求
    ENUM_MSG_TYPE_ENTER_DIR_RESPOND,    //进入文件夹回复
    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST,  //上传文件请求
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND,  //上传文件回复
    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST, //下载文件请求
    ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND, //下载文件回复
    ENUM_MSG_TYPE_MAX = 0x00ffffff
};

struct FileInfo
{
    char caName[32];    //文件名
    int iFileType;      //文件类型
};

struct PDU
{
    uint uiPDULen;      //协议数据单元大小
    uint uiMsgType;     //消息类型
    char caData[64];
    uint uiMsgLen;      //消息长度
    int caMsg[];        //实际消息
};

PDU *mkPDU(uint uiMsgLen);

#endif // PROTOCOL_H
