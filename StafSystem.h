/*================================================================
 *   Copyright (C) 2021 hqyj Ltd. All rights reserved.
 *   
 *   文件名称：StafSystem.h
 *   创 建 者：cym
 *   创建日期：2021年03月08日
 *   描    述：
 *
 ================================================================*/
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

#ifndef STAFSYSTEM_H__
#define STAFSYSTEM_H__
/**********************************************************/
//注册协议
#define USER_LOGIN 0	//普通用户注册
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
#define VHANGE_SUC 519	//修改成功
#define CHANGE_FAIL 520 //修改失败

#define VIP_FIND 521 //管理员查看
#define FIND_SUC 523	//查看成功
#define FIND_FAIL 524	//查看失败

/*-------------------------------------------------------*/
//退出登录
#define EXIT 99		//退出登录
#define EXIT 99		//退出成功

/**********************************************************/
//网络地址信息
#define PORT 8080
#define IP "192.168.199.246"
/**********************************************************/
typedef struct Info{
	char Name[20];
	char WorkNum[4];
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
struct epoll_event *revents=NULL;  //epoll监控返回的事件
int num=1;
char cmdbuf[50]="";
int ret=0;
int i=0;
int fd=0;
char** pazRes=NULL;		//查询数据库返回的指针
int *Row=NULL;
int *Colum=NULL;
char *Errmsg=NULL;
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

//用户退出
int exit_(int epfd, struct epoll_event* revents, int fd, struct epoll_event events);

//普通用户注册
int UserLogin(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd);

//管理员用户注册
int VipLogin(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd);

//用户登录
int  IdEnter(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd);

//添加信息
void InsertMsg(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd);

//删除信息
void DelMsg(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd);

//修改信息
void ChangeMsg(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd, int flag);

//查看信息
void LookMsg(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd);
/**********************************************************/
#endif //STAFSYSTEM_H__

