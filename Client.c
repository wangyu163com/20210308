/*================================================================
 *   Copyright (C) 2021 hqyj Ltd. All rights reserved.
 *   
 *   文件名称：Client.c
 *   创 建 者：cym
 *   创建日期：2021年03月10日
 *   描    述：
 *
 ================================================================*/

#include "StafSystem.h"

int main(int argc, char *argv[])
{
	InfoP Recvmsg;
	InfoP Sendmsg;
	bzero(&Recvmsg, sizeof(Recvmsg));
	bzero(&Sendmsg, sizeof(Sendmsg));
	int cfd = socket(AF_INET, SOCK_STREAM, 0);
	if(cfd<0){
		perror("socket");
		return -1;
	}

	struct sockaddr_in Send;
	Send.sin_family = AF_INET;
	Send.sin_port   = htons(PORT);
	Send.sin_addr.s_addr=inet_addr(IP);

	int duankou = 1;
	setsockopt(cfd, SOL_SOCKET, SO_REUSEADDR, &duankou, sizeof(int));

	ret = connect(cfd, (void*)&Send, sizeof(Send));
	if(ret<0){
		perror("connect");
		return -1;
	}
	fprintf(stderr, "服务器连接成功！  行号：%d\n", __LINE__);//调试代码

	while(1){
		system("clear");
		fprintf(stderr, "************************************************\n");
		fprintf(stderr, "      1.注册       2.登录        3.退出         \n");
		fprintf(stderr, "************************************************\n");
		fprintf(stderr, "请输入：");
SCANF:		
		num=0;
		scanf("%d", &num);
		while(getchar()!='\n');
		if(!(num<4 && num>0)){
			fprintf(stderr, "输入有误，请重新输入:\n");
			goto SCANF;
		}
		switch(num){
		case LOGIN:
CliLogin1:
			ret = CliLogin(cfd, Sendmsg, Recvmsg);
			if(ret<0){
				fprintf(stderr, "请输入任意键继续！\n");
				while(getchar()!='\n');
				goto CliLogin1;
			}else if(ret >0){
				fprintf(stderr, "请输入任意键返回上一级！\n");
				while(getchar()!='\n');
				break;
			}
			break;
		case ENTER:
			ret = CliEnter(cfd, Sendmsg, Recvmsg);
			if(ret!=0){
				fprintf(stderr, "请输入任意键继续！\n");
				while(getchar()!='\n');
			}else if(ret == 0){
				fprintf(stderr, "请输入任意键返回上一级！\n");
				while(getchar()!='\n');
			}
			break;
		case EXITOUT:
			close(cfd);
			return 0;
		}
	}
	close(cfd);	
	return 0;
}

int CliLogin(int cfd, InfoP Sendmsg, InfoP Recvmsg){
	bzero(&Sendmsg, sizeof(Sendmsg));
	bzero(&Recvmsg, sizeof(Recvmsg));
	while(1){
		system("clear");
		fprintf(stderr, "************************************************\n");
		fprintf(stderr, "           1.注册            2.上一级           \n");
		fprintf(stderr, "************************************************\n");
		fprintf(stderr, "请输入：");
SCANF1:
		num=0;
		scanf("%d", &num);
		while(getchar()!='\n');
		if(num>3||num<1){
			fprintf(stderr, "输入错误，请重新输入：");
			goto SCANF1;
		}
		switch(num){
		case LOGIN1:
			fprintf(stderr, "请输入ID(请输入数字或者字母)：");
			scanf("%s", Sendmsg.Id);
			while(getchar()!='\n');
			fprintf(stderr, "请输入Password(请输入数字或者字母)：");
			scanf("%s", Sendmsg.Password);
			while(getchar()!='\n');
			Sendmsg.Type = VIP_LOGIN;
			do{
				ret = send(cfd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
			}while(ret<0 && errno == EINTR);
			if(ret<0){
				fprintf(stderr, "网络不佳，请重新注册！\n");
				return -1;//网络不好
			}
			do{
				ret = recv(cfd, (void*)&Recvmsg, sizeof(Recvmsg), 0);
			}while(ret<0 && errno ==EINTR);
			if(Recvmsg.Type == VIP_EXIST){
				fprintf(stderr, "管理员账户只能注册一个！\n");
				return -2;
			}else if(Recvmsg.Type == LOGIN_SUC){
				fprintf(stderr, "用户注册成功！\n");
				return 1;
			}
			break;
		case EXITOUT1:
			return 0;
		}
	}
	return 0;
}

int CliEnter(int cfd, InfoP Sendmsg, InfoP Recvmsg){
	bzero(&Sendmsg, sizeof(Sendmsg));
	bzero(&Recvmsg, sizeof(Recvmsg));
	while(1){
		system("clear");
		fprintf(stderr, "************************************************\n");
		fprintf(stderr, "           1.登录            2.上一级           \n");
		fprintf(stderr, "************************************************\n");
		fprintf(stderr, "请输入：");
SCANF2:
		num = 0;
		scanf("%d", &num);
		while(getchar()!='\n');
		if(num>3||num<1){
			fprintf(stderr, "输入错误，请重新输入：");
			goto SCANF2;
		}
		switch(num){
		case ENTER1:
			fprintf(stderr, "请输入ID：");
			scanf("%s", Sendmsg.Id);
			while(getchar()!='\n');
			fprintf(stderr, "请输入Password：");
			scanf("%s", Sendmsg.Password);
			while(getchar()!='\n');
			Sendmsg.Type = ID_ENTER;
			do{
				ret = send(cfd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
			}while(ret <0 && errno == EINTR);
			if(ret<0)
				perror("send");
			do{
				ret = recv(cfd, (void*)&Recvmsg, sizeof(Recvmsg), 0);
			}while(ret<0 && errno == EINTR);
			if(ret<0)
				perror("recv");
			if(Recvmsg.Type == ENTER_SUC){
				CliInter(cfd, Sendmsg, Recvmsg);
				return 1;
			}else if(Recvmsg.Type == USER_FAIL){
				fprintf(stderr, "用户已登录！\n");
				return -1;
			}else if(Recvmsg.Type == VIP_FAIL){
				fprintf(stderr, "用户名或密码错误！\n");
				return -2;
			}else if(Recvmsg.Type == USER_UFIND){
				fprintf(stderr, "用户不存在！\n");
				return -3;
			}
			break;
		case EXITOUT2:
			return 0;
		}
	}
	return 0;
}

int CliInter(int cfd, InfoP Sendmsg, InfoP Recvmsg){
	bzero(&Sendmsg, sizeof(Sendmsg));
	int quanxian=0;
	char tmp[20]="";
	if(strcpy(Recvmsg.Res, "0"))
		quanxian=0;
	else
		quanxian=1;
	while(1){
		system("clear");
		fprintf(stderr, "************************************************************\n");
		fprintf(stderr, "1.添加信息  2.修改信息  3.删除信息  4.查看信息  5.退出登录  \n");
		fprintf(stderr, "************************************************************\n");
		fprintf(stderr, "请输入：");
SCANF3:		
		num=0;
		scanf("%d", &num);
		while(getchar()!='\n');
		if(num>5||num<1){
			fprintf(stderr, "输入有误，请重新输入：");
			goto SCANF3;
		}
		switch(num){
		case 1:
			if(quanxian==0){
				//普通用户操作
				fprintf(stderr, "此用户无权限！请重新输入：\n");
				goto SCANF3;
			}else{
				//管理员用户操作
				fprintf(stderr, "请输入员工姓名：");
				scanf("%s", Sendmsg.INFO.Name);
				while(getchar()!='\n');
				fprintf(stderr, "请输入员工工号：");
				scanf("%s", Sendmsg.INFO.WorkNum);
				while(getchar()!='\n');
				fprintf(stderr, "请输入员工性别：");
				scanf("%c", Sendmsg.INFO.Sex);
				while(getchar()!='\n');
				fprintf(stderr, "请输入员工电话：");
				scanf("%s", Sendmsg.INFO.TelNum);
				while(getchar()!='\n');
				fprintf(stderr, "请输入员工工资：");
				scanf("%f", Sendmsg.INFO.Salary);
				while(getchar()!='\n');
				fprintf(stderr, "请输入员工邮箱：");
				scanf("%s", Sendmsg.INFO.Email);
				while(getchar()!='\n');
				Sendmsg.Type = VIP_INS;
				do{
					ret = send(cfd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
				}while(ret<0 && ret ==EINTR);
				do{
					ret = recv(cfd, (void*)&Recvmsg, sizeof(Recvmsg), 0);
				}while(ret<0 && errno == EINTR);
				if(Recvmsg.Type == INS_FAIL){
					fprintf(stderr, "该员工信息添加失败，信息已存在！输入任意键继续！\n");
				}else if(Recvmsg.Type == INS_SUC){
					fprintf(stderr, "该员工信息添加成功！账号为电话号码，密码为工号。输入任意键继续！\n");
				}
			}
			while(getchar()!='\n');
			break;
		case 2:
			break;
		case 3:
			if(quanxian==0){
				//普通用户操作
				fprintf(stderr, "此用户无权限！请重新输入：\n");
				goto SCANF3;
			}else{
				//管理员用户操作
			}
			break;
		case 4:
			if(quanxian == 0){
				strcpy(Sendmsg.Res, "0");
				Findmsg(cfd, Sendmsg, Recvmsg);
			}else{
				strcpy(Sendmsg.Res, "1");
				Findmsg(cfd, Sendmsg, Recvmsg);
			}
			while(getchar()!='\n');
			break;
		case 5:
			return 0;
		}
	}
}

void Findmsg(int cfd, InfoP Sendmsg, InfoP Recvmsg){
	fprintf(stderr, "请输入要查找的员工工号(查看所有请输入999)：");
TEM:
	tem = 0;
	scanf("%s", tmp);
	while(getchar()!='\n');
	if(strcmp(tmp, "10000")>0 || strcmp(tmp, "999")<0){
		fprintf(stderr, "输入错误，请重新输入：");
		goto TEM;
	}else if(strcmp(tmp, "999")){
		bzero(Sendmsg.INFO.WorkNum, sizeof(Sendmsg.INFO.WorkNum));
		Sendmsg.Type = VIP_FIND;
		do{
			ret = send(cfd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
		}while(ret<0 && errno == EINTR);
	}else{
		Sendmsg.Type = VIP_FIND;
		sprintf(Sendmsg.INFO.WorkNum, tmp);
		do{
			ret = send(cfd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
		}while(ret<0 && errno == EINTR);
	}
	while(strcmp(Recvmsg.Res, "10")){
		do{
			ret = recv(cfd, (void*)&Recvmsg, sizeof(Recvmsg), 0);
		}while(ret<0 && errno == EINTR);
		fprintf(stderr, "姓名：%s  工号：%s  性别：%s  工资：%g  电话：%s  邮箱：%s\n", 
				Recvmsg.INFO.Name, Recvmsg.INFO.WorkNum, Recvmsg.INFO.Sex,
				Recvmsg.INFO.Salary, Recvmsg.INFO.TelNum, Recvmsg.INFO.Email);
	}

}
