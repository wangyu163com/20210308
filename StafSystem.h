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

#ifndef STAFSYSTEM_H__
#define STAFSYSTEM_H__
/**********************************************************/
//注册协议
#define USER_LOGIN 0	//普通用户注册
#define VIP_LOGIN 1		//管理员用户注册
#define LOGIN_SUC 2		//注册成功
#define USER_EXIST 3    //普通用户存在
#define VIP_EXIST 4     //管理员用户存在

/*-------------------------------------------------------*/
//登录协议
#define USER_ENTER 8	//普通用户登录
#define VIP_ENTER 9		//管理员用户登录
#define ENTER_SUC 10	//用户登录成功
#define USER_FAIL 11	//此普通用户已登录
#define VIP_FAIL 12		//用户名或者密码错误

/*-------------------------------------------------------*/
//查看协议
#define USER_FIND 64	//普通用户查看
#define USER_F_SUC 65	//查看成功

#define VIP_INS 512		//管理员增加
#define INS_SUC 513		//管理员增加成功
#define INS_FAIL 514	//增加失败

#define VIP_DEL 515		//管理员删除
#define DEL_SUC 516		//管理员删除成功
#define DEL_FAIL 517	//删除失败

#define VIP_CHANGE 518	//管理员修改
#define VHANGE_SUC 519	//修改成功
#define CHANGE_FAIL 520 //修改失败

#define VIP_FIND_ONE 521 //管理员查看一个
#define VIP_FIND_ALL 522 //管理员查看所有
#define FIND_SUC 523	//查看成功
#define FIND_FAIL 524	//查看失败

/*-------------------------------------------------------*/
//退出登录
#define EXIT 99		//退出登录
#define EXIT 99		//退出成功

/**********************************************************/

typedef struct INFOPROCOTOL{
	int Type;
	char Id[20];
	char Password[20];
	struct INFO {
		char Name[20];
		char WorkNum[4];
		char Sex;
		char TelNum[15];
		float Salary;
		char Email[20];
	};
	char Res[20];//备用
}__attribute__((packed)) InfoP;

#endif //STAFSYSTEM_H__

