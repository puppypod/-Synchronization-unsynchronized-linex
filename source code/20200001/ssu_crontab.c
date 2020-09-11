#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<signal.h>
#include<pthread.h>
#include<syslog.h>
#include<string.h>
#include"ssu_crontab.h"

char command_filename[BUFLEN] = "ssu_crontab_file";
char temp_file[BUFLEN] = "ssu_crontab_tempfile";
char log_file[BUFLEN] = "ssu_crontab_log";
char programDir[PATH_MAX];
char add_line[L_MAX];
char timeCycle_token[10][BUFLEN];
int remove_num;

int isExit = false;
int isAdd = false;
int isRemove = false;

void ssu_crontab() 
{
	char buf[L_MAX];
	int buf_cnt;
	FILE *fp;
	char cpy[PATH_MAX];
	char cpy2[PATH_MAX];
	char cpy3[PATH_MAX];
	char timeCycle[BUFLEN];
	char remark[BUFLEN];
	char rest[BUFLEN];
	char command[BUFLEN]="";
	char *id;
	char *p, *p2, *p3, *p4;
	char scope_p[L_MAX];
	int scope_num;
	char input[L_MAX] = "";
	char input_cpy[L_MAX];
	char *tmp, *tmp2;
	int isEnd;
	int two_token;
	////////////////////////
	char *tp1, *tp2;
	int num;
	char tmp_num[L_MAX];
		
	getcwd(programDir, PATH_MAX);//프로그램 작업 경로 초기화

	while(1){
		if(access(command_filename,F_OK)<0){//ssu_crontab_file이 존재하지 않을 경우, 생성하기
			fp = fopen(command_filename,"a+");
			fclose(fp);
		}
		fp = fopen(command_filename,"r");
		buf_cnt=0;
		while(fgets(buf,L_MAX,fp)!=NULL){// ssu_crontab_file 내에 명령어 출력
			printf("%d. %s",buf_cnt++,buf);
			memset(buf,'\0',sizeof(buf));
		}
		printf("\n");
		fclose(fp);
		strcpy(cpy,programDir);
		p = strtok(cpy,"/");
		while((p = strtok(NULL,"/"))!=NULL){// 해당 학번 프롬프트 출력
			id = p;	
		}
		isExit = false;
		isAdd = false;
		isRemove = false;
		while(1){
			printf("%s> ",id);// 프롬프트 출력
			memset(input,'\0',sizeof(input));
			fgets(input,L_MAX,stdin);
			input[strlen(input)-1]='\0';
			if(strlen(input) == 0){
				continue;
			}

			strcpy(cpy2,input);//input 값들을 token으로 분리

			if(!strncmp(input,"add",3)){// add 명령어를 수행할 때 주기를 저장해줌
				p2 = strstr(cpy2, " ");
				p2 = strstr(p2+1, " ");
				p2 = strstr(p2+1, " ");
				p2 = strstr(p2+1, " ");
				p2 = strstr(p2+1, " ");
				p2 = strstr(p2+1, " ");
				strncpy(timeCycle,input,strlen(input)-strlen(p2));
			}
			else if(!strncmp(input,"remove",6)){// remove 명령어를 수행할 때
				strcpy(timeCycle,input);
			}
			else if(!strncmp(input,"exit",4)){// exit 명령어를 수행할 때
				isExit = true;
				break;
			}
			else{
				fprintf(stderr,"%s란 명령어는 존재하지 않습니다.\n",input);
				continue;
			}

			memset(command,'\0',sizeof(command));
			p3 = strtok(timeCycle," ");
			strcpy(command,p3);//명령어 입력

			if(!strncmp(command,"add",3)){//추가 명령어
				for(int i=0 ; i<5 ; i++){//add 실행주기 토큰 분,시,일,월,요일 로 나뉨
					if((p3 = strtok(NULL," "))!=NULL){
						strcpy(timeCycle_token[i], p3);
						if(strstr(timeCycle_token[i],"*")!=NULL){// *이 포함된 경우
							if(!strcmp(timeCycle_token[i],"*")){// *이 단독인 경우
								//printf("%s\n","* 단독");
							}
							else{// *이 단독이 아닌경우
								if(strstr(timeCycle_token[i],"/")!=NULL){
									if(strstr(timeCycle_token[i],",")!=NULL){
										//printf("%s\n","*/숫자 가 복수개");// */숫자 가 복수개임
											
									}
									else{
										//printf("%s\n","*/숫자 가 단독");// */숫자 가 단독임
										tp1 = timeCycle_token[i];
										tp2 = strstr(timeCycle_token[i],"/");
										if(strncmp(tp1,"*",1)){
											fprintf(stderr,"(*/숫자)실행주기를 잘못 입력하셨습니다.\n");
											isExit = true;
											break;
										}
									}
								}
							}
						}
						else{// *이 없고 숫자가 포함된 경우
							if(strstr(timeCycle_token[i],",")!=NULL){// ,가 포함된 경우
								if(strstr(timeCycle_token[i],"-")==NULL){// ,만 포함된 경우
									//printf("%s\n",",가 포함된 경우");
									tmp = timeCycle_token[i];
									tmp2 = tmp = strstr(tmp,",");
									memset(scope_p,'\0',sizeof(scope_p));
									strncpy(scope_p,timeCycle_token[i],strlen(timeCycle_token[i])-strlen(tmp));
									tmp2++;
									scope_num = atoi(scope_p);
									//범위를 벗어났을 경우 재입력 요구해야함
									isEnd = false;
									while(1){
										if(i == 0){//분
											if((scope_num < 0 )||(scope_num > 59) ){
												fprintf(stderr,"분 범위를 잘못 입력하셨습니다.\n");
												isExit = true;
												break;
											}
										}
										else if(i == 1){//시
											if((scope_num < 0 )||(scope_num > 23) ){
												fprintf(stderr,"시간 범위를 잘못 입력하셨습니다.\n");
												isExit = true;
												break;
											}
										}
										else if(i == 2){//일
											if((scope_num < 1 )||(scope_num > 31) ){
												fprintf(stderr,"일 범위를 잘못 입력하셨습니다.\n");
												isExit = true;
												break;
											}
										}
										else if(i == 3){//월
											if((scope_num < 1 )||(scope_num > 12) ){
												fprintf(stderr,"월 범위를 잘못 입력하셨습니다.\n");
												isExit = true;
												break;
											}
										}
										else if(i == 4){//요일
											if((scope_num < 0 )||(scope_num > 6) ){
												fprintf(stderr,"요일 범위를 잘못 입력하셨습니다.\n");
												isExit = true;
												break;
											}
										}
										if(isEnd){
											break;
										}
										if((tmp = strstr(tmp+1,","))==NULL){
											memset(scope_p,'\0',sizeof(scope_p));
											strncpy(scope_p,tmp2,strlen(tmp2));
											scope_num = atoi(scope_p);
											isEnd = true;
											continue;
										}
										memset(scope_p,'\0',sizeof(scope_p));
										strncpy(scope_p,tmp2,strlen(tmp2)-strlen(tmp));
										scope_num = atoi(scope_p);
										tmp2 = tmp;
										tmp2++;
									}
									if(isExit){
										break;
									}
								}
								else if(strstr(timeCycle_token[i],"-")!=NULL){//, 와 -가 포함된 경우
									tmp = timeCycle_token[i];
									tmp2 = strstr(tmp,"-");
									memset(scope_p,'\0',sizeof(scope_p));
									strncpy(scope_p,tmp,strlen(tmp)-strlen(tmp2));
									tmp2++;
									tmp = tmp2;
									scope_num = atoi(scope_p);
									//범위를 벗어났을 경우 재입력 요구해야함
									isEnd = false;
									two_token = 0;
									while(1){
										if(i == 0){//분
											if((scope_num < 0 )||(scope_num > 59) ){
												fprintf(stderr,"분 범위를 잘못 입력하셨습니다.\n");
												isExit = true;
												break;
											}
										}
										else if(i == 1){//시
											if((scope_num < 0 )||(scope_num > 23) ){
												fprintf(stderr,"시간 범위를 잘못 입력하셨습니다.\n");
												isExit = true;
												break;
											}
										}
										else if(i == 2){//일
											if((scope_num < 1 )||(scope_num > 31) ){
												fprintf(stderr,"일 범위를 잘못 입력하셨습니다.\n");
												isExit = true;
												break;
											}
										}
										else if(i == 3){//월
											if((scope_num < 1 )||(scope_num > 12) ){
												fprintf(stderr,"월 범위를 잘못 입력하셨습니다.\n");
												isExit = true;
												break;
											}
										}
										else if(i == 4){//요일
											if((scope_num < 0 )||(scope_num > 6) ){
												fprintf(stderr,"요일 범위를 잘못 입력하셨습니다.\n");
												isExit = true;
												break;
											}
										}
										if(isEnd){
											break;
										}
										if(two_token%2 == 0){
											if((tmp2 = strstr(tmp2+1,","))==NULL){
												memset(scope_p,'\0',sizeof(scope_p));
												strncpy(scope_p,tmp,strlen(tmp));
												scope_num = atoi(scope_p);
												isEnd = true;
												continue;
											}
											memset(scope_p,'\0',sizeof(scope_p));
											strncpy(scope_p,tmp,strlen(tmp)-strlen(tmp2));
											scope_num = atoi(scope_p);
											tmp2++;
											tmp = tmp2;
											two_token++;
										}
										else{
											if((tmp2 = strstr(tmp2+1,"-"))==NULL){
												memset(scope_p,'\0',sizeof(scope_p));
												strncpy(scope_p,tmp,strlen(tmp));
												scope_num = atoi(scope_p);
												isEnd = true;
												continue;
											}
											memset(scope_p,'\0',sizeof(scope_p));
											strncpy(scope_p,tmp,strlen(tmp)-strlen(tmp2));
											tmp2++;
											tmp = tmp2;
											scope_num = atoi(scope_p);
											two_token++;
										}
									}
									if(isExit){
										break;
									}

								}
							}
							else if(strstr(timeCycle_token[i],"-")!=NULL){// -가 포함된 경우
								//printf("%s\n","-가 포함된 경우");
								tmp = timeCycle_token[i];
								tmp2 = tmp = strstr(tmp,"-");
								memset(scope_p,'\0',sizeof(scope_p));
								strncpy(scope_p,timeCycle_token[i],strlen(timeCycle_token[i])-strlen(tmp));
								tmp2++;
								scope_num = atoi(scope_p);
								//범위를 벗어났을 경우 재입력 요구해야함
								isEnd = false;
								while(1){
									if(i == 0){//분
										if((scope_num < 0 )||(scope_num > 59) ){
											fprintf(stderr,"분 범위를 잘못 입력하셨습니다.\n");
											isExit = true;
											break;
										}
									}
									else if(i == 1){//시
										if((scope_num < 0 )||(scope_num > 23) ){
											fprintf(stderr,"시간 범위를 잘못 입력하셨습니다.\n");
											isExit = true;
											break;
										}
									}
									else if(i == 2){//일
										if((scope_num < 1 )||(scope_num > 31) ){
											fprintf(stderr,"일 범위를 잘못 입력하셨습니다.\n");
											isExit = true;
											break;
										}
									}
									else if(i == 3){//월
										if((scope_num < 1 )||(scope_num > 12) ){
											fprintf(stderr,"월 범위를 잘못 입력하셨습니다.\n");
											isExit = true;
											break;
										}
									}
									else if(i == 4){//요일
										if((scope_num < 0 )||(scope_num > 6) ){
											fprintf(stderr,"요일 범위를 잘못 입력하셨습니다.\n");
											isExit = true;
											break;
										}
									}
									if(isEnd){
										break;
									}
									if((tmp = strstr(tmp+1,"-"))==NULL){
										memset(scope_p,'\0',sizeof(scope_p));
										strncpy(scope_p,tmp2,strlen(tmp2));
										scope_num = atoi(scope_p);
										isEnd = true;
										continue;
									}
									memset(scope_p,'\0',sizeof(scope_p));
									strncpy(scope_p,tmp2,strlen(tmp2)-strlen(tmp));
									scope_num = atoi(scope_p);
									tmp2 = tmp;
									tmp2++;
								}
								if(isExit){
									break;
								}
							}
							else{// 숫자만 단독으로 포함된 경우
								//숫자는 최대 59까지 이므로 2자리 수까지 확인하면 됨
								if(strlen(timeCycle_token[i])==2){
									//printf("%s\n","숫자는 두자리수");
								}
								else if(strlen(timeCycle_token[i])==1){
									//printf("%s\n","숫자는 한자리수");
								}
								else{//에러인 경우
									fprintf(stderr,"(숫자)실행주기를 잘못 입력하셨습니다.\n");
									isExit = true;
									break;
								}
							}
						}
					}
					else{
						fprintf(stderr,"실행주기 개수를 잘못 입력하셨습니다.\n");
						isExit = true;
						break;
					}
				}
				if(isExit){
					isExit = false;
					continue;
				}
				isAdd = true;
			}
			else if(!strncmp(command,"remove",6)){//삭제 명령어
				strcpy(cpy3,input);
				p3 = strtok(cpy3," ");
				if((p3 = strtok(NULL," "))==NULL){//숫자를 입력하지 않았을 때 에러처리
					fprintf(stderr,"숫자를 입력해 주세요.\n");
					continue;
				}
				if(strlen(p3)==3){//세자리수
					if((p3[0]<48 || p3[0]>57) || (p3[1]<48 || p3[1]>57) ||(p3[2]<48 || p3[2]>57)){
						fprintf(stderr,"문자를 입력했습니다.숫자를 입력하세요.\n");
						continue;
					}
					remove_num = (p3[0]-48)*100 + (p3[1]-48)*10 + p3[2]-48;//문자를 숫자로 변환
					isRemove = true;
				}
				else if(strlen(p3)==2){//두자리수
					if((p3[0]<48 || p3[0]>57) || (p3[1]<48 || p3[1]>57)){
						fprintf(stderr,"문자를 입력했습니다.숫자를 입력하세요.\n");
						continue;
					}
					remove_num = (p3[0]-48)*10 + p3[1]-48;//문자를 숫자로 변환
					isRemove = true;
				}
				else if(strlen(p3)==1){//한자리수
					if((p3[0]<48 || p3[0]>57)){
						fprintf(stderr,"문자를 입력했습니다.숫자를 입력하세요.\n");
						continue;
					}
					remove_num = p3[0]-48;//문자를 숫자로 변환
					isRemove = true;
				}
				else if(strlen(p3)==0){//입력이 없는경우
					fprintf(stderr,"숫자를 입력하지 않았습니다.\n");
					continue;
				}
				else{//p3 길이가 4이상일때
					fprintf(stderr,"숫자를 입력하지 않았습니다.\n");
					continue;
				}
			}
			else if(!strncmp(command, "exit", 4)){//종료 명령어
				isExit = true;
				break;
			}
			else{//add, remove, exit 명령어 이외의 것
				fprintf(stderr,"\'%s\'란 명령어는 없습니다.\n",command);
				continue;
			}

			if(isAdd){//add 명령어 시작
				memset(input_cpy,'\0',L_MAX);
				memset(add_line,'\0',L_MAX);
				strcpy(input_cpy,input);
				p = strtok(input_cpy," ");
				while((p = strtok(NULL," "))!=NULL){
					strcat(add_line,p);
					strcat(add_line," ");
				}
				command_add();
				break;
			}
			else if(isRemove){//remove 명령어 시작
				command_remove();
				break;
			}

		}
		if(isExit){
			break;
		}

	}

	return;
}
void command_add(){
	FILE *fp;

	fp = fopen(command_filename,"a+");// add 명령어 수행 시 ssu_crontab_file에 저장
	
	fputs(add_line,fp);
	fputs("\n",fp);
	fclose(fp);
	return;	
}
void command_remove(){
	FILE *fp, *fp2;
	char buf[L_MAX];
	int skip_line = 0;
	int isScope = false;
	

	fp = fopen(command_filename,"a+");//원래 파일
	fp2 = fopen(temp_file,"w");//임시 파일

	while(fgets(buf,L_MAX,fp)!=NULL){
		if(skip_line == remove_num){
			skip_line++;
			isScope = true;
			continue;
		}
		fputs(buf,fp2);
		skip_line++;
	}

	fclose(fp);
	fclose(fp2);	
	if(!isScope){
		fprintf(stderr,"입력한 숫자는 삭제 가능한 인덱스 범위에 존재하지 않습니다.\n\n");
	}
	
	unlink(command_filename);
	rename(temp_file,command_filename);
	return;
}
