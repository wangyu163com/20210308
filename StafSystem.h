/*================================================================
 *   Copyright (C) 2021 hqyj Ltd. All rights reserved.
 *   
 *   文件名称：StafSystem.h
 *   创 建 者：cym
 *   创建日期：2021年03月08日
 *   描    述：
 *
 ================================================================*/

#ifndef STAFSYSTEM_H__
#define STAFSYSTEM_H__

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sqlite3.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <signal.h>

/**********************************************************/
//注册协议
//#define USER_LOGIN 0	//普通用户注册
#define VIP_LOGIN 1		//管理员用户注册
#define LOGIN_SUC 2		//注册成功
#define USER_EXIST 3    //普通用户存在
#define VIP_EXIST 4     //管理员用户已被注册

/*-------------------------------------------------------*/
//登录协议
#define ID_ENTER 8	//用户登录
#define ENTER_SUC 9	//用户登录成功
#define USER_FAIL 10	//此用户已登录
#define VIP_FAIL 11		//用户名或者密码错误
#define USER_UFIND 12	//用户不存在

/*-------------------------------------------------------*/
//查看协议
#define USER_FIND 64	//普通用户查看
#define USER_F_FAIL 65	//无权限

#define VIP_INS 512		//管理员增加
#define INS_SUC 513		//管理员增加成功
#define INS_FAIL 514	//增加失败

#define VIP_DEL 515		//管理员删除
#define DEL_SUC 516		//管理员删除成功
#define DEL_FAIL 517	//删除失败

#define VIP_CHANGE 518	//管理员修改
#define CHANGE_SUC 519	//修改成功
#define CHANGE_FAIL 520 //修改失败
#define CHANGE_FAIL_WU 521 //无权限

#define VIP_FIND 521 //管理员查看
#define FIND_SUC 523	//查看成功
#define FIND_FAIL 524	//查看失败

/*--------------------------------------------------------*/
//退出登录
#define EXIT 99		//退出登录
#define EXIT1 999		//退出成功

/**********************************************************/
//网络地址信息
#define PORT 8080
#define IP "192.168.199.246"
/**********************************************************/
//客户端使用
#define LOGIN 1
#define ENTER 2
#define EXITOUT  3

#define LOGIN1 1
#define EXITOUT1 2

#define ENTER1 1
#define EXITOUT2 2

/**********************************************************/
typedef struct Info{
	char Name[20];
	char WorkNum[5];
	char Sex;
	char TelNum[15];
	float Salary;
	char Email[20];
}INfo;

INfo INFO;

typedef struct INFOPROCOTOL{
	int Type;
	char Id[20];
	char Password[20];
	struct Info INFO;
	char Res[20];//备用
}__attribute__((packed)) InfoP;

sqlite3* db=NULL;
struct sockaddr_in Recv;	//接受
struct sockaddr_in Send;	//发送
struct epoll_event events;  //epoll监控的事件
struct epoll_event revents[20];  //epoll监控返回的事件
int num=1;
char cmdbuf[200]="";
int ret=0;
int i=0;
int fd=0;
char** pazRes=NULL;		//查询数据库返回的指针
int Row;
int Colum;
char *Errmsg=NULL;
char *errmsg=NULL;
char ID[20]="";
int j=0;
int k=0;
int l=0;

/**********************************************************/
//服务器函数
//导入账号信息表
int Import(sqlite3 *db);

//导入员工信息表
int ImportInfo(sqlite3 *db);

//打印客户端连接信息
void printf_info(struct sockaddr_in Recv);

//用户退出客户端
int exit_(int epfd, struct epoll_event* events, int fd, InfoP Recvmsg, sqlite3 *db);

//普通用户注册
int UserLogin(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd);

//管理员用户注册
int VipLogin(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd);

//用户登录
int IdEnter(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd);

//添加信息
void InsertMsg(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd);

//删除信息
void DelMsg(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd);

//修改信息
void ChangeMsg(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd, int flag);

//查看信息
void LookMsg(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd, int flag);

//发送信息
InfoP SendAll(char** pazRes, InfoP Sendmsg, int k);

//用户退出登录
void exit_out(sqlite3* db, InfoP Recvmsg);
/**********************************************************/
//客户端注册函数
int CliLogin(int cfd, InfoP Sendmsg, InfoP Recvmsg);

//客户端登录函数
int CliEnter(int cfd, InfoP Sendmsg, InfoP Recvmsg);

//客户端交互画面
int CliInter(int cfd, InfoP Sendmsg, InfoP Recvmsg);

//查看信息
void Findmsg(int cfd, InfoP Sendmsg, InfoP Recvmsg);

//修改信息
void Changeinfo(int cfd, InfoP Sendmsg, InfoP Recvmsg);
/**********************************************************/
#endif //STAFSYSTEM_H__

