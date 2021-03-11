/*================================================================
 *   Copyright (C) 2021 hqyj Ltd. All rights reserved.
 *   
 *   文件名称：Server.c
 *   创 建 者：cym
 *   创建日期：2021年03月09日
 *   描    述：
 *
 ================================================================*/

#include "StafSystem.h"

int main(int argc, char *argv[])
{
	InfoP Recvmsg;
	InfoP Sendmsg;
	ret = sqlite3_open("./sq.qb", &db);
	if(ret){
		printf("服务器打开账号信息数据库失败\n");
		return ret;
	}

	//导入账号信息表
	Import(db);
	fputs("导入账号信息表成功\n", stderr);
	//导入员工信息表	
	ImportInfo(db);
	fputs("导入账号信息表成功\n", stderr);
	//申请文件描述符
	int sfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sfd<0){
		perror("socket");
		return -1;
	}
	//设置端口复用
	int duankou=1;
	setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &duankou, sizeof(int));
	//设置服务器网络信息
	bzero(&Send, sizeof(Send));
	Send.sin_family = AF_INET;
	Send.sin_port = htons(PORT);
	Send.sin_addr.s_addr = inet_addr(IP);
	//绑定服务器网络信息
	ret = bind(sfd, (void*)&Send, sizeof(Send));
	if(ret <0){
		perror("bind");
		return -1;
	}
	//监听客户端连接
	ret = listen(sfd, 10);
	if(ret){
		perror("listen");
		return -1;
	}
	//接受客户端信息的变量
	socklen_t socklen = sizeof(Recv);
	int epfd = epoll_create(1);
	if(epfd<0){
		perror("epoll_create");
		return -1;
	}

	events.events = EPOLLIN;
	events.data.fd = sfd;
	if(epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &events)<0){
		perror("epoll_ctl");
		return -1;
	}

	while(1){
		ret = epoll_wait(epfd, revents, num, -1);
		if(ret == -1){
			perror("epoll_wait");
			return -1;
		}
		for(i=0;i<ret;i++){
			if(revents[i].data.fd == sfd){
				if((fd = accept(sfd, (void*)&Recv, (void*)&socklen))<0){
					perror("accept");
					continue;
				}
				num++;
				bzero(&events, sizeof(events));
				events.events = EPOLLIN;
				events.data.fd = fd;
				if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &events)<0){
					perror("epoll_ctl");
					continue;
				}
				//打印客户端连接信息
				printf_info(Recv);
			}else if(revents[i].data.fd>sfd){
				//接受信息
				bzero(&Recvmsg, sizeof(Recvmsg));
				do{
					ret = recv(revents[i].data.fd, (void*)&Recvmsg, sizeof(Recvmsg), 0);
				}while(ret < 0 && errno==EINTR );
				if(ret <= 0){
					if(exit_(epfd, &events, revents[i].data.fd, Recvmsg, db))
						close(revents[i].data.fd);
					continue;
				}
				switch(Recvmsg.Type){
					/*	case USER_LOGIN:
						UserLogin(db, Recvmsg, Sendmsg, revents[i].data);
						breakt@github.com:wangyu163com/20210308.git;
						*/
				case VIP_LOGIN:
					VipLogin(db, Recvmsg, Sendmsg, revents[i].data.fd);
					break;
				case ID_ENTER:
					IdEnter(db, Recvmsg, Sendmsg, revents[i].data.fd);
					break;
				case VIP_INS:
					InsertMsg(db, Recvmsg, Sendmsg, revents[i].data.fd);
					break;
				case VIP_DEL:
					DelMsg(db, Recvmsg, Sendmsg, revents[i].data.fd);
					break;
				case VIP_CHANGE:
					fprintf(stderr, "%s:%s:%d\n", __FILE__, __func__, __LINE__);
					if(!strcmp(Recvmsg.Res, "1"))
						ChangeMsg(db, Recvmsg, Sendmsg, revents[i].data.fd, 1);
					else
						ChangeMsg(db, Recvmsg, Sendmsg, revents[i].data.fd, 0);
					break;
				case VIP_FIND:
					if(!strcmp(Recvmsg.Res, "1"))
						LookMsg(db, Recvmsg, Sendmsg, revents[i].data.fd, 1);
					else
						LookMsg(db, Recvmsg, Sendmsg, revents[i].data.fd, 0);
					break;
				case EXIT1:
					exit_out(db, Recvmsg);
					break;
				case EXIT:
					exit_(epfd, &events, revents[i].data.fd, Recvmsg, db);
					break;

				}
			}
		}
	}
	close(sfd);
	sqlite3_close(db);
	return 0;
}

int Import(sqlite3* db){
	int i=1;
	char** pazRes=NULL;		//查询数据库返回的指针
	char *errmsg=NULL;
	bzero(cmdbuf, sizeof(cmdbuf));
	sprintf(cmdbuf, "create table if not exists AcInfo (Type int, Id char primary key, Password char, Online int)");
	ret = sqlite3_exec(db, cmdbuf, NULL, NULL, &Errmsg);
	if(ret){
		printf("%s line:%d\n", sqlite3_errmsg(db), __LINE__);
		return ret;
	}
	fprintf(stderr, "AcInfo表创建成功！\n");
	bzero(cmdbuf, sizeof(cmdbuf));
	sprintf(cmdbuf, "select * from AcInfo ");
	sqlite3_get_table(db, cmdbuf, &pazRes, &Row, &Colum, &errmsg);
	while(i<=Row){
		bzero(cmdbuf, sizeof(cmdbuf));
		sprintf(cmdbuf, "update AcInfo set Online = 0 where Id = \"%s\"", pazRes[i*4+1]);
		sqlite3_exec(db, cmdbuf, NULL, NULL, &errmsg);
		i++;
	}
	sqlite3_free_table(pazRes);
	return 0;
}

int ImportInfo(sqlite3 *db){
	char *errmsg=NULL;
	bzero(cmdbuf, sizeof(cmdbuf));
	sprintf(cmdbuf, "create table if not exists StaffInfo (Name char, WorkNum char primary key, Sex char, TelNum char, Salary float, Email char)");
	ret = sqlite3_exec(db, cmdbuf, NULL, NULL, &errmsg);
	if(ret){
		printf("%s line:%d\n", sqlite3_errmsg(db), __LINE__);
		return ret;
	}
	fprintf(stderr, "StaffInfo表创建成功！\n");
	return 0;
}

void printf_info(struct sockaddr_in Recv){
	fprintf(stderr, "[%s:%d]:连接成功！\n", inet_ntoa(Recv.sin_addr), ntohs(Recv.sin_port));
}

int exit_(int epfd, struct epoll_event* events, int fd, InfoP Recvmsg, sqlite3* db){
	bzero(cmdbuf, sizeof(cmdbuf));//更改账户状态
	sprintf(cmdbuf, "update AcInfo set Online = 0 where  Id = \"%s\"", Recvmsg.Id);
	sqlite3_exec(db, cmdbuf, NULL, NULL, &errmsg);
	events->events = EPOLLIN;
	events->data.fd = fd;
	if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, events)){
		perror("epoll_ctl");
		return -1;
	}
	num--;
	fprintf(stderr, "%s用户退出登录！\n", Recvmsg.Id);
	return 1;
}
/*
   int UserLogin(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd){
   char *errmsg=NULL;
   bzero(cmdbuf, sizeof(cmdbuf));
   bzero(&Sendmsg, sizeof(Sendmsg));
   sprintf(cmdbuf, "select * from AcInfo where Id = \'%s\'", Recvmsg.Id);
   while(sqlite3_get_table(db, cdmbuf, &pazRes, Row, Colum, &Errmsg)){
   perror("sqlite3_get_table");
   }
   if(Row == 1){
   Sendmsg.Type = USER_EXIST;
   while(send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0)<0)
   perror("send");
   fprintf(stderr, "%s普通用户已存在！\n", Recvmsg.Id);
   }else if(Row == 0){
   bzero(cmdbuf, sizeof(cmdbuf));
   sprintf(cmdbuf, "insert into AcInfo values(0, \'%s\', \'%s\', 0)", Recvmsg.Id, Recvmsg.Password);
   while(sqlite3_exec(db, NULL, NULL, &errmsg))
   perror(sqlite3_exec);
   Sendmsg.Type = LOGIN_SUC;
   while(send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0)<0)
   perror("send");
   fprintf(stderr, "%s普通用户注册成功\n", Recvmsg.Id);
   }
   sqlite3_free_table(pazRes);
   return 0;
   }
   */

int VipLogin(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd){
	char** pazRes=NULL;		//查询数据库返回的指针
	char *errmsg=NULL;
	bzero(cmdbuf, sizeof(cmdbuf));
	bzero(&Sendmsg, sizeof(Sendmsg));
	sprintf(cmdbuf, "select * from AcInfo where Type = 1");
	while(sqlite3_get_table(db, cmdbuf, &pazRes, &Row, &Colum, &Errmsg)){
		perror("sqlite3_get_table");
	}
	if(Row == 1){
		Sendmsg.Type = VIP_EXIST;
		do{
			ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
		}while(ret<0 && errno == EINTR);
		if(ret<0)
			perror("send");
		fprintf(stderr, "%s管理员用户已被注册！\n", Recvmsg.Id);
	}else if(Row == 0){
		bzero(cmdbuf, sizeof(cmdbuf));
		sprintf(cmdbuf, "insert into AcInfo values(1, \'%s\', \'%s\', 0)", Recvmsg.Id, Recvmsg.Password);
		while(sqlite3_exec(db, cmdbuf, NULL, NULL, &errmsg))
			perror("sqlite3_exec");
		Sendmsg.Type = LOGIN_SUC;
		do{
			ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
		}while(ret<0 && errno == EINTR);
		if(ret<0)
			perror("send");
		fprintf(stderr, "%s管理员用户注册成功\n", Recvmsg.Id);
	}
	sqlite3_free_table(pazRes);
	return 0;
}

int IdEnter(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd){
	Row = 0;
	char** pazRes=NULL;		//查询数据库返回的指针
	int flag =0;
	bzero(cmdbuf, sizeof(cmdbuf));
	bzero(&Sendmsg, sizeof(Sendmsg));
	sprintf(cmdbuf, "select * from AcInfo where Id = \'%s\'", Recvmsg.Id);
	while(sqlite3_get_table(db, cmdbuf, &pazRes, &Row, &Colum, &Errmsg))
		perror("sqlite3_get_table");
	if(Row == 1){
		Row=0;
		bzero(cmdbuf, sizeof(cmdbuf));
		sprintf(cmdbuf, "select * from AcInfo where Id = \'%s\' and Password = \'%s\'", Recvmsg.Id, Recvmsg.Password);
		while(sqlite3_get_table(db, cmdbuf, &pazRes, &Row, &Colum, &Errmsg))
			perror("sqlite3_get_table");
		if(Row == 1){
			Row=0;
			bzero(cmdbuf, sizeof(cmdbuf));
			sprintf(cmdbuf, "select * from AcInfo where Id = \'%s\' and Online = 0", Recvmsg.Id);
			while(sqlite3_get_table(db, cmdbuf, &pazRes, &Row, &Colum, &Errmsg))
				perror("sqlite3_get_table");
			if(Row == 1){
				Row=0;
				Sendmsg.Type = ENTER_SUC;
				bzero(cmdbuf, sizeof(cmdbuf));
				sprintf(cmdbuf, "update AcInfo set Online = 1 where Id = \"%s\"", Recvmsg.Id);
				if(sqlite3_exec(db, cmdbuf, NULL, NULL, &errmsg)<0)
					perror("sqlite3_exec");
				fprintf(stderr, "账户状态更新成功！\n");
				bzero(cmdbuf, sizeof(cmdbuf));
				sprintf(cmdbuf, "select * from AcInfo where Id = \'%s\' and Type = 0", Recvmsg.Id);
				if(sqlite3_get_table(db, cmdbuf, &pazRes, &Row, &Colum, &Errmsg)<0)
					perror("sqlite3_get_table");
				if(Row == 1)
					strcpy(Sendmsg.Res, "0");
				else
					strcpy(Sendmsg.Res, "1");
				do{
					fprintf(stderr, "%s账号为%s\n", Recvmsg.Id, Sendmsg.Res);//调试
					ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
				}while(ret<0 && errno==EINTR);
				if(ret<0)
					perror("send");
				fprintf(stderr, "%s用户登录成功！line:%d\n", Recvmsg.Id, __LINE__);//调试
			}else{
				Sendmsg.Type = USER_FAIL;
				do{
					ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
				}while(ret<0 && errno==EINTR);
				if(ret<0)
					perror("send");
				fprintf(stderr, "%s用户已登录！\n", Recvmsg.Id);
			}
		}else{
			Sendmsg.Type = VIP_FAIL;
			do{
				ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
			}while(ret<0 && errno==EINTR);
			if(ret<0)
				perror("send");
			fprintf(stderr, "%s用户名或者密码错误！\n", Recvmsg.Id);
		}
	}else if(Row == 0){
		Sendmsg.Type = USER_UFIND;
		do{
			ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
		}while(ret<0 && errno==EINTR);
		if(ret<0)
			perror("send");
		fprintf(stderr, "%s用户不存在！\n", Recvmsg.Id);
	}
	sqlite3_free_table(pazRes);
	return 0;
}

void InsertMsg(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd){
	char** pazRes=NULL;		//查询数据库返回的指针
	bzero(cmdbuf, sizeof(cmdbuf));
	bzero(&Sendmsg, sizeof(Sendmsg));
	sprintf(cmdbuf, "select * form StaffInfo where WorkNum = \'%s\'", Recvmsg.INFO.WorkNum);
	if(sqlite3_get_table(db, cmdbuf, &pazRes, &Row, &Colum, &Errmsg))
		perror("sqlite3_get_table");
	if(Row == 1){
		Sendmsg.Type = INS_FAIL;
		do{
			ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
		}while(ret<0 && errno==EINTR);
		fprintf(stderr, "%s该员工信息已存在！\n", Recvmsg.INFO.WorkNum);
		sqlite3_free_table(pazRes);
		return;
	}
	bzero(cmdbuf, sizeof(cmdbuf));
	sprintf(cmdbuf, "insert into StaffInfo values (\'%s\', \'%s\', \'%c\', \'%s\', \'%f\', \'%s\')", 
			Recvmsg.INFO.Name, Recvmsg.INFO.WorkNum, Recvmsg.INFO.Sex, Recvmsg.INFO.TelNum, Recvmsg.INFO.Salary,
			Recvmsg.INFO.Email);
	if(sqlite3_exec(db, cmdbuf, NULL, NULL, &errmsg))
		perror("sqlite3_exec");
	fprintf(stderr, "信息添加完成！\n");
	bzero(cmdbuf, sizeof(cmdbuf));
	sprintf(cmdbuf, "insert into AcInfo values (0, \'%s\', \'%s\', 0)", Recvmsg.INFO.TelNum, Recvmsg.INFO.WorkNum);
	if(sqlite3_exec(db, cmdbuf, NULL, NULL, &Errmsg))
		perror("sqlite3_exec");
	fprintf(stderr, "%s该员工账号已创建成功！\n", Recvmsg.INFO.Name);

	Sendmsg.Type = INS_SUC;
	do{
		ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
	}while(ret<0 && errno == EINTR);
	fprintf(stderr, "添加信息反馈客户端成功！\n");
	sqlite3_free_table(pazRes);
}

void DelMsg(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd){
	char** pazRes=NULL;		//查询数据库返回的指针
	bzero(cmdbuf, sizeof(cmdbuf));
	bzero(&Sendmsg, sizeof(Sendmsg));
	sprintf(cmdbuf, "select * from StaffInfo where WorkNum = \"%s\"", Recvmsg.INFO.WorkNum);
	if(sqlite3_get_table(db, cmdbuf, &pazRes, &Row, &Colum, &Errmsg))
		perror("sqlite3_get_table");
	if(Row == 1){
		bzero(cmdbuf, sizeof(cmdbuf));
		sprintf(cmdbuf, "delete from StaffInfo where WorkNum = \"%s\"", Recvmsg.INFO.WorkNum);
		if(sqlite3_exec(db, cmdbuf, NULL, NULL, &errmsg))
			perror("sqlite3_exec");
		fprintf(stderr, "%s员工信息删除成功!\n", Recvmsg.INFO.WorkNum);
		bzero(cmdbuf, sizeof(cmdbuf));
		sprintf(cmdbuf, "delete from AcInfo where Id = \"%s\"", Recvmsg.INFO.TelNum);
		if(sqlite3_exec(db, cmdbuf, NULL, NULL, &errmsg))
			perror("sqlite3_exec");
		fprintf(stderr, "%s员工账号已删除！\n", Recvmsg.INFO.WorkNum);
		Sendmsg.Type = DEL_SUC;
		do{
			ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
		}while(ret<0 && errno == EINTR);
		fprintf(stderr, "删除成功信息反馈客户端成功！line:%d\n", __LINE__);
	}else{
		Sendmsg.Type = DEL_FAIL;
		do{
			ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
		}while(ret<0 && errno == EINTR);
		fprintf(stderr, "删除失败信息反馈客户端成功！line:%d\n", __LINE__);
	}
	sqlite3_free_table(pazRes);
}

void ChangeMsg(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd, int flag){
	j=k=0;
	char** pazRes=NULL;		//查询数据库返回的指针
	bzero(cmdbuf, sizeof(cmdbuf));
	bzero(&Sendmsg, sizeof(Sendmsg));
	sprintf(cmdbuf, "select * from StaffInfo where WorkNum = \"%s\"", Recvmsg.INFO.WorkNum);
	if(sqlite3_get_table(db, cmdbuf, &pazRes, &Row, &Colum, &Errmsg))
		perror("sqlite3_get_table");
	if(!(Row)){
		Sendmsg.Type = CHANGE_FAIL;
		strcpy(Sendmsg.Res, "不存在！");
		do{
			ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
		}while(ret<0 && errno == EINTR);
		fprintf(stderr, "修改失败信息反馈客户端成功！\n");
		return;
	}
	if(flag){
		//管理员用户更改 除了性别和工号
		bzero(cmdbuf, sizeof(cmdbuf));
		if(!strlen(Recvmsg.INFO.Name) == 0){
			sprintf(cmdbuf, "update StaffInfo set Name = \"%s\" where WorkNum = \"%s\"", Recvmsg.INFO.Name, Recvmsg.INFO.WorkNum);
			sqlite3_exec(db, cmdbuf, NULL, NULL, &errmsg);
		}
		if(!strlen(Recvmsg.INFO.TelNum)==0){
			sprintf(cmdbuf, "update StaffInfo set TelNum = \"%s\" where  WorkNum = \"%s\"", Recvmsg.INFO.TelNum, Recvmsg.INFO.WorkNum);
			sqlite3_exec(db, cmdbuf, NULL, NULL, &errmsg);
		}
		if(Recvmsg.INFO.Salary){
			sprintf(cmdbuf, "update StaffInfo set Salary = \"%f\" where  WorkNum = \"%s\"", Recvmsg.INFO.Salary, Recvmsg.INFO.WorkNum);
			sqlite3_exec(db, cmdbuf, NULL, NULL, &errmsg);
		}
		if(!strlen(Recvmsg.INFO.Email)==0){
			sprintf(cmdbuf, "update StaffInfo set Email = \"%s\" where WorkNum = \"%s\"", Recvmsg.INFO.Email, Recvmsg.INFO.WorkNum);
			sqlite3_exec(db, cmdbuf, NULL, NULL, &errmsg);
		}
		Sendmsg.Type = CHANGE_SUC;
		do{
			ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
		}while(ret<0 && errno == EINTR);
		fprintf(stderr, "%s:员工信息更新完成！\n", Recvmsg.INFO.WorkNum);
	}else{
		//普通用户更改
		if(!strlen(Recvmsg.INFO.TelNum)==0){
			sprintf(cmdbuf, "update StaffInfo set TelNum = \"%s\" where  WorkNum = \"%s\"", Recvmsg.INFO.TelNum, Recvmsg.INFO.WorkNum);
			sqlite3_exec(db, cmdbuf, NULL, NULL, &errmsg);
			k++;
		}
		if(!strlen(Recvmsg.INFO.Email)==0){
			sprintf(cmdbuf, "update StaffInfo set  Email = \"%s\" where WorkNum = \"%s\"", Recvmsg.INFO.Email, Recvmsg.INFO.WorkNum);
			sqlite3_exec(db, cmdbuf, NULL, NULL, &errmsg);
			k++;
		}
		if(!strlen(Recvmsg.INFO.Name)==0){
			l++;
		}
		if(Recvmsg.INFO.Salary){
			l++;
		}
		if(k>0||l==0){
			Sendmsg.Type = CHANGE_SUC;
		}else if(k==0 && l>0){
			Sendmsg.Type = CHANGE_FAIL_WU;
			strcpy(Sendmsg.Res, "无权限！");
		}
		do{
			ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
		}while(ret<0 && errno == EINTR);
		fprintf(stderr, "%s:员工信息更新完成！\n", Recvmsg.INFO.WorkNum);
	}
	sqlite3_free_table(pazRes);
}

void LookMsg(sqlite3* db, InfoP Recvmsg, InfoP Sendmsg, int fd, int flag){
	j=0;
	k=1;
	char** pazRes=NULL;		//查询数据库返回的指针
	bzero(cmdbuf, sizeof(cmdbuf));
	bzero(&Sendmsg, sizeof(Sendmsg));
	if(flag){
		//管理员用户
		if(strlen(Recvmsg.INFO.WorkNum) == 0){
			sprintf(cmdbuf, "select * from StaffInfo ");		
			if(sqlite3_get_table(db, cmdbuf, &pazRes, &Row, &Colum, &Errmsg))
				perror("sqlite3_get_table");
			if(!Row){
				Sendmsg.Type = FIND_FAIL;
				do{
					ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
				}while(ret <0 && errno == EINTR);
				fprintf(stderr, "无信息可查看！line:%d", __LINE__);
				goto FREE;
			}
			while(k<=Row){
				Sendmsg = SendAll(pazRes, Sendmsg, k);
				Sendmsg.Type = FIND_SUC;
				if(++k >Row)
					strcpy(Sendmsg.Res, "10");
				do{
					ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
				}while(ret <0 && errno ==EINTR);
				fprintf(stderr, "全部信息发送成功！line:%d\n", __LINE__);
			}
		}else{
			bzero(cmdbuf, sizeof(cmdbuf));
			sprintf(cmdbuf, "select * from StaffInfo where WorkNum = \"%s\"", Recvmsg.INFO.WorkNum);
			if(sqlite3_get_table(db, cmdbuf, &pazRes, &Row, &Colum, &Errmsg))
				perror("sqlite3_get_table");
			if(Row){
				Sendmsg = SendAll(pazRes, Sendmsg, 1);
				Sendmsg.Type = FIND_SUC;
				strcpy(Sendmsg.Res, "10");
				do{
					ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
				}while(ret <0 && errno ==EINTR);
				fprintf(stderr, "%s信息发送成功！\n", Recvmsg.INFO.WorkNum);
			}else{
				Sendmsg.Type = FIND_FAIL;
				do{
					ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
				}while(ret <0 && errno ==EINTR);
				fprintf(stderr, "%s信息未找到！\n", Recvmsg.INFO.WorkNum);
			}
		}
		return;
	}else{
		//普通用户
		if(strlen(Recvmsg.INFO.WorkNum) != 0){
			bzero(cmdbuf, sizeof(cmdbuf));
			sprintf(cmdbuf, "select * from StaffInfo ");		
			if(sqlite3_get_table(db, cmdbuf, &pazRes, &Row, &Colum, &Errmsg))
				perror("sqlite3_get_table");
			if(Row){
				Sendmsg = SendAll(pazRes, Sendmsg, 1);
				Sendmsg.INFO.Salary = 0.0;
				Sendmsg.Type = FIND_SUC;
				strcpy(Sendmsg.Res, "10");
				do{
					ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
				}while(ret <0 && errno ==EINTR);
				fprintf(stderr, "%s信息发送成功！\n", Recvmsg.INFO.WorkNum);
			}else{
				Sendmsg.Type = FIND_FAIL;
				do{
					ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
				}while(ret <0 && errno ==EINTR);
				fprintf(stderr, "%s信息未找到！\n", Recvmsg.INFO.WorkNum);
			}		
		}else{
			bzero(cmdbuf, sizeof(cmdbuf));
			sprintf(cmdbuf, "select * from StaffInfo");
			if(sqlite3_get_table(db, cmdbuf, &pazRes, &Row, &Colum, &Errmsg))
				perror("sqlite3_get_table");
			if(!Row){
				Sendmsg.Type = FIND_FAIL;
				do{
					ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
				}while(ret<0 && errno == EINTR);
				fprintf(stderr, "无信息可查看！line:%d\n", __LINE__);
				goto FREE;
			}
			while(k<=Row){
				Sendmsg = SendAll(pazRes, Sendmsg, k);
				Sendmsg.INFO.Salary = 0.0;
				Sendmsg.Type = FIND_SUC;
				if(++k>Row)
					strcpy(Sendmsg.Res, "10");
				do{
					ret = send(fd, (void*)&Sendmsg, sizeof(Sendmsg), 0);
				}while(ret <0 && errno ==EINTR);
				fprintf(stderr, "全部信息发送成功！\n");
			}
		}
	}
FREE:
	sqlite3_free_table(pazRes);
}

InfoP SendAll(char** pazRes, InfoP Sendmsg, int k){
	strcpy(Sendmsg.INFO.Name, pazRes[k*6]);
	strcpy(Sendmsg.INFO.WorkNum, pazRes[k*6+1]);
	Sendmsg.INFO.Sex = pazRes[k*6+2][0];
	strcpy(Sendmsg.INFO.TelNum, pazRes[k*6+3]);
	Sendmsg.INFO.Salary = atof(pazRes[k*6+4]);
	strcpy(Sendmsg.INFO.Email, pazRes[k*6+5]);
	return Sendmsg;
}

void exit_out(sqlite3* db, InfoP Recvmsg){
	bzero(cmdbuf, sizeof(cmdbuf));
	sprintf(cmdbuf, "update AcInfo set Online = 0 where Id = \"%s\"", Recvmsg.Id);
	sqlite3_exec(db, cmdbuf, NULL, NULL, &errmsg);
	fprintf(stderr, "%s用户退出登录！line:%d\n", Recvmsg.Id, __LINE__);
}
