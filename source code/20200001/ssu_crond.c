#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<signal.h>
#include<pthread.h>
#include<unistd.h>
#include<string.h>
#include<syslog.h>
#include<time.h>
#include "ssu_crond.h"

struct thread_data{// 쓰레드 함수를 수행시 필요한 변수
	char week[CYCLE_LEN];
	char month[CYCLE_LEN];
	char day[CYCLE_LEN];
	char hour[CYCLE_LEN];
	char minute[CYCLE_LEN];
	char command_cycle[L_MAX];
	FILE *log;
};
struct thread_data thread_data_ary[COMMAND_MAX];

void *thread_time_cycle(void *arg);

char command_filename[BUFLEN] = "ssu_crontab_file";
char log_file[BUFLEN] = "ssu_crontab_log";
char programDir[PATH_MAX];
char save_command[COMMAND_MAX][L_MAX];
char save_remove_ary[COMMAND_MAX][L_MAX];

void ssu_crond()
{
	pid_t pid;

	getcwd(programDir, PATH_MAX);//프로그램 작업 경로 초기화

	if((pid = fork())<0){
		fprintf(stderr,"fork error\n");
		exit(1);
	}
	else if(pid == 0){//자식 프로세스
		if(exec_crond()<0){//디몬 프로세스
			fprintf(stderr,"ssu_daemon_init() error\n");
			exit(1);
		}
		exit(0);//디몬 프로세스 종료
	}
	return;
}
int exec_crond()
{
	int fd,maxfd;
	pid_t pid;
	FILE *command_fp, *log_fp;//command_fp -> ssu_crontab_file, log_fp -> ssu_crontab_log
	char *p;
	char buf_line[L_MAX];
	struct stat statbuf;
	char cpy[L_MAX];
	int prev_line_num;
	int pres_line_num;
	char add_line_daemon[L_MAX];
	char add_cat_line[L_MAX];
	time_t current_time;
	char input_time[L_MAX];
	char input_time_cpy[L_MAX];
	////////////ssu_crontab_file에 내용을 배열에 저장하는 변수////////////////
	//char save_command[][] -> 전역변수 선언
	//char save_remove_ary[][] -> 전역변수 선언
	char *remove_name;
	int count;
	int i;
	////////////시간 주기 배열, 명령어 배열////////////////
	char time_cycle[COMMAND_MAX][L_MAX];
	char command_cycle[COMMAND_MAX][L_MAX];
	char tmp_cycle[L_MAX];
	char *cp1, *cp2;//cycle pointer
	///////////쓰레드 관련 변수//////////////////////
	char *tp1, *tp2;
	pthread_t tid[COMMAND_MAX];
	int status;
	struct tm *prev_time;
	struct tm *pres_time;
	int prev_min, pres_min;
	time_t current_time_thread;
	int isFirst;

	if((pid = fork())<0){
		fprintf(stderr,"fork error\n");
		exit(1);
	}
	else if(pid != 0){//부모 프로세스죽이기
		exit(0);
	}
	pid = getpid();
	setsid();//자식 프로세스를 세션의 리더로 만듬

	//표준 입출력, 에러 filedescription 무시
	signal(SIGTTIN,SIG_IGN);
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTSTP,SIG_IGN);

	maxfd = getdtablesize(); 
	for(fd=0 ; fd<maxfd ; fd++){
		close(fd);
	}

	umask(0);//접근 허가
	chdir("/");//루트 권한으로 설정
	fd = open("/dev/null",O_RDWR);
	dup(0);
	dup(0);
////////////////////////////////////////////////////////////////////////////
	//디몬 프로세스 동작 부분
	chdir(programDir);
	if(access(command_filename,F_OK)<0){
		command_fp = fopen(command_filename,"a+");//로그 파일
		fclose(command_fp);
	}
	command_fp = fopen(command_filename,"r");//명령어 파일
	prev_line_num = 0;

	while(fgets(buf_line,L_MAX,command_fp)!=NULL){//명령어 라인의 개수를 확인함
		prev_line_num++;//명령어 파일이 몇개있는지 알 수 있음 
	}
	fclose(command_fp);
	///////////////////ssu_crontab_file 의 내용을 배열(save_command)에 저장//////////////////

	if((command_fp = fopen(command_filename,"r"))==NULL){
		fprintf(stderr,"fopen error\n");	
		exit(1);
	}
	count=0;
	while((fgets(save_command[count++],sizeof(save_command),command_fp))!=NULL){
		;	
	}
	count--;
	fclose(command_fp);

	//////////////////////////////////////////////////////////////////////
	time(&current_time_thread);
	prev_time = localtime(&current_time_thread);
	prev_min = prev_time->tm_min;
	isFirst = true;
	log_fp = fopen(log_file,"a+");//로그 파일
	fclose(log_fp);

	while(1){

		command_fp = fopen(command_filename,"r");//명령어 파일

		pres_line_num = 0;
		while(fgets(buf_line,L_MAX,command_fp)!=NULL){//명령어 라인의 개수를 확인함
			pres_line_num++;//명령어 파일이 몇개있는지 알 수 있음 
		}
		sleep(1);
		fclose(command_fp);



		if(prev_line_num < pres_line_num){// add 명령어, 로그에 추가
			log_fp = fopen(log_file,"a+");//로그 파일
			//명령어 마지막 줄에 있는것이 추가된 명령어임	
			command_fp = fopen(command_filename,"r");
			while((fgets(buf_line,L_MAX,command_fp))!=NULL){
				memset(add_line_daemon,'\0',sizeof(buf_line));
				strcpy(add_line_daemon,buf_line);
			}//마지막에 나온 add_line_daemon이 추가된 값임
			memset(add_cat_line,'\0',sizeof(add_cat_line));
			time(&current_time);// 현재 시간 구하기 == 추가한 시간
			strcpy(input_time_cpy,ctime(&current_time));
			input_time_cpy[strlen(input_time_cpy)-1] = '\0';//ctime 을 통해 출력한
			sprintf(input_time,"[%s]",input_time_cpy);	
			sprintf(add_cat_line,"%s add %s",input_time,add_line_daemon);
			fputs(add_cat_line,log_fp);//ssu_crontab_log 파일에 add 추가
			sleep(1);
			fclose(log_fp);
			fclose(command_fp);
			//ssu_crontab_file 초기화	
			if((command_fp = fopen(command_filename,"r"))==NULL){
				fprintf(stderr,"fopen error\n");	
				exit(1);
			}
			count=0;
			while((fgets(save_command[count++],sizeof(save_command),command_fp))!=NULL){
				;	
			}
			count--;
			fclose(command_fp);
			prev_line_num = pres_line_num;
		}
		else if(prev_line_num > pres_line_num){//remove 명령어, 로그에 추가
			log_fp = fopen(log_file,"a+");//로그 파일
			if((command_fp = fopen(command_filename,"r"))==NULL){
				fprintf(stderr,"fopen error\n");	
				exit(1);
			}
			count=0;
			while((fgets(save_remove_ary[count++],sizeof(save_remove_ary),command_fp))!=NULL){
				;	
			}
			count--;
			fclose(command_fp);
		/////////////////////////////////////////////////////	
			remove_name = NULL;
			remove_name = compare_command_remove(prev_line_num, pres_line_num);

			time(&current_time);// 현재 시간 구하기 == 삭제한 시간
			strcpy(input_time_cpy,ctime(&current_time));
			input_time_cpy[strlen(input_time_cpy)-1] = '\0';//ctime 을 통해 출력한
			sprintf(input_time,"[%s]",input_time_cpy);	
			sprintf(add_cat_line,"%s remove %s",input_time,remove_name);
			fputs(add_cat_line,log_fp);//ssu_crontab_log 파일에 add 추가
			memset(save_command, '\0', sizeof(save_command));

			for(int i=0 ; i<pres_line_num ; i++){
				strcpy(save_command[i], save_remove_ary[i]);	
			}
			sleep(1);
			fclose(log_fp);

			prev_line_num = pres_line_num;
		}
		else{ //실제 ssu_crond 실행
			time(&current_time_thread);
			pres_time = localtime(&current_time_thread);
			pres_min = pres_time->tm_min;
			if(prev_min != pres_min || isFirst){//매 분단위로 checking
				prev_min = pres_min;// 과거 시간 저장
				isFirst = false;
				for(i=0 ; i<pres_line_num ; i++){
					strcpy(tmp_cycle, save_command[i]);

					cp1 = strtok(tmp_cycle," ");
					memset(time_cycle[i],0,sizeof(time_cycle[i]));
					strcat(time_cycle[i], cp1);
					for(int j=0 ; j<4 ; j++){
						strcat(time_cycle[i]," ");
						cp1 = strtok(NULL," ");
						strcat(time_cycle[i], cp1);
					}
					//////////////////////////////////////////////////////////
					cp2 = strstr(save_command[i]," ");
					for(int j=0 ; j<4 ; j++){
						cp2 = strstr(cp2+1," ");
					}
					cp2++;
					strcpy(command_cycle[i],cp2);
					command_cycle[i][strlen(command_cycle[i])-1] = '\0';
				}
				// time_cycle[][] -> 시간 주기
				// command_cycle[][] -> 명령어, 즉 system()에 넣을 것임
				for(i=0 ; i<pres_line_num ; i++){
					log_fp = fopen(log_file,"a+");//로그 파일
					tp1 = time_cycle[i];
					tp2 = strstr(time_cycle[i], " ");
					strncpy(thread_data_ary[i].minute,tp1,strlen(tp1)-strlen(tp2));
					tp2++;
					tp1 = tp2;
					tp2 = strstr(tp2+1, " ");

					strncpy(thread_data_ary[i].hour,tp1,strlen(tp1)-strlen(tp2));
					tp2++;
					tp1 = tp2;
					tp2 = strstr(tp2+1, " ");

					strncpy(thread_data_ary[i].day,tp1,strlen(tp1)-strlen(tp2));
					tp2++;
					tp1 = tp2;
					tp2 = strstr(tp2+1, " ");

					strncpy(thread_data_ary[i].month,tp1,strlen(tp1)-strlen(tp2));
					tp2++;
					tp1 = tp2;

					strncpy(thread_data_ary[i].week,tp1,strlen(tp1));
					strcpy(thread_data_ary[i].command_cycle,command_cycle[i]);
					thread_data_ary[i].log = log_fp;

					// 쓰레드 작업 시작
	
					if(pthread_create(&tid[i],NULL,thread_time_cycle,(void *)&thread_data_ary[i])!=0){
						fprintf(stderr,"pthread_create error\n");
						exit(1);
					}
				}
			}


		}

	}//while 문 종료 부분


	return 0;
	
}
void *thread_time_cycle(void *arg)// 명령어 실행 주기를 check하여 ssu_crontab_log에 저장 및 명령어 수행해주는 함수
{
	struct thread_data *data;
	char command_cycle[L_MAX];
	char time_arr[5][CYCLE_LEN];
	int i,j,k;
	FILE *fp;
	char input_time_cpy[L_MAX];
	char input_time[L_MAX];
	char add_cat_line[L_MAX];
	char cat_str[L_MAX];

	struct tm* today;
	time_t current_time;
	char *p1, *p2, *comma_p, *slide_p, *bar_p;

	int parse_num;
	int scope1, scope2;
	char xscope[L_MAX], yscope[L_MAX];
	int tnum;
	int comma_num;
	int isPass;
	char scope[L_MAX];
	char time_comma[L_MAX];
	char test[10];

	data = (struct thread_data *)arg;
	strcpy(time_arr[4], data->minute);
	strcpy(time_arr[3], data->hour);
	strcpy(time_arr[2], data->day);
	strcpy(time_arr[1], data->month);
	strcpy(time_arr[0], data->week);
	strcpy(command_cycle, data->command_cycle);
	fp = data->log;
////////////////////////본격적인 실행 주기에 따른 명령어 수행 시작/////////////////////	

// ssu_crontab()에서 예외 처리를 하였기에 정상적인 값들만을 검사할 수 있음/////////////

	//tm_min(0~59), tm_hour(0~23), tm_mday(1~31), tm_mon(0~11), tm_wday(0~6)

	time(&current_time);
	today = localtime(&current_time);
	for(i=0 ; i<5 ; i++){
		if(strstr(time_arr[i],"*")!=NULL){// *가 존재한다면
			if(strstr(time_arr[i], "/")!=NULL){// *와 /가 존재한다면
				if(strstr(time_arr[i],",")!=NULL){// */2, */4와 같은 형식이 들어옴
					//1	
					comma_num=1;
					comma_p = strstr(time_arr[i],",");
					while((comma_p = strstr(comma_p+1,","))!=NULL){// ,가 몇개인지 check
						comma_num++;
					}
					/*memset(test,0,sizeof(test));
					test[0] = comma_num+48;
					fputs(test,fp);*/
					comma_p = strstr(time_arr[i],",");
					p1 = time_arr[i];
					memset(time_comma,0,sizeof(time_comma));
					strncpy(time_comma,p1,strlen(p1)-strlen(comma_p));

					if(strstr(time_comma,"*")!=NULL && strlen(time_comma)==1){
						continue;
					}

					comma_p++;
					p1 = comma_p;
					for(j=0 ; j<=comma_num ; j++){
						isPass = false;


						if(strstr(time_comma,"/")!=NULL){// */숫자

							p2 = time_comma;
							slide_p = strstr(time_comma,"/");
							slide_p++;
							//fputs(p2,fp);
							p2 = slide_p;
							parse_num = atoi(p2);//숫자 추출
							if(i == 0){//요일
								for(k=1 ; k<=7 ; k++){
									if(k%parse_num==0){//해당 요일 check
										if(today->tm_wday == k-1){//해당 요일이 되었다는 뜻
											isPass = true;	
											break;
										}
									}
								}
								if(isPass){
									break;
								}
							}
							else if(i == 1){//월
								for(k=1 ; k<=12 ; k++){
									if(k%parse_num==0){//해당 월 check
										if(today->tm_mon == k-1){//해당 월이 되었다는 뜻
											isPass = true;	
											break;
										}
									}
								}
								if(isPass){
									break;
								}
							}
							else if(i == 2){//일
								for(k=1 ; k<=31 ; k++){
									if(k%parse_num==0){//해당 일 check
										if(today->tm_mday == k){//해당 일이 되었다는 뜻
											isPass = true;	
											break;
										}
									}
								}
								if(isPass){
									break;
								}
							}
							else if(i == 3){//시간
								for(k=1 ; k<=24 ; k++){
									if(k%parse_num==0){//해당 시간 check
										if(today->tm_hour == k-1){//해당 시간이 되었다는 뜻
											isPass = true;	
											break;
										}
									}
								}
								if(isPass){
									break;
								}
							}
							else if(i == 4){//분
								for(k=1 ; k<=60 ; k++){
									if(k%parse_num==0){//해당 시간 check
										if(today->tm_min == k-1){//해당 시간이 되었다는 뜻
											isPass = true;	
											break;
										}
									}
								}
								if(isPass){
									break;
								}
							}
						}
						else{// 숫자 또는 *(time_comma)

							if(strstr(time_comma,"*")!=NULL){
								isPass = true;	
								break;		
							}

							parse_num = atoi(time_comma);//숫자 추출
							if(i == 0){//요일
								if(today->tm_wday == parse_num){//해당 요일이 되었다는 뜻
									isPass = true;	
									break;
								}
							}
							else if(i == 1){//월
								if(today->tm_mon+1 == parse_num){//해당 월이 되었다는 뜻
									isPass = true;	
									break;
								}
							}
							else if(i == 2){//일
								if(today->tm_mday == parse_num){//해당 일이 되었다는 뜻
									isPass = true;	
									break;
								}
							}
							else if(i == 3){//시간
								if(today->tm_hour == parse_num){//해당 시간이 되었다는 뜻
									isPass = true;	
									break;
								}
							}
							else if(i == 4){//분
								if(today->tm_min == parse_num){//해당 분이 되었다는 뜻

									isPass = true;	
									break;
								}
							}
						}
						if(j<comma_num-1){
							comma_p = strstr(comma_p+1,",");
							memset(time_comma,0,sizeof(time_comma));
							strncpy(time_comma,p1,strlen(p1)-strlen(comma_p));

							comma_p++;
							p1 = comma_p;
						}
						else{
							memset(time_comma,0,sizeof(time_comma));
							strncpy(time_comma,p1,strlen(p1));
						}
					}
					if(isPass){
						continue;//통과
					}
					else{
						pthread_exit(NULL);
						return NULL;
					}
				}
				else{// */4가 단독으로 들어옴
					//2
					p1 = time_arr[i];
					p2 = strstr(p1,"/");
					p2++;
					p1 = p2;// p1은 숫자가 될 것임
					//memset(scope,'\0',sizeof(scope));
					//strcpy(scope,p1);
					parse_num = atoi(p1);
					isPass = false;
					if(i == 0){//요일
						for(j=1 ; j<=7 ; j++){
							if(j%parse_num==0){//해당 요일 check
								if(today->tm_wday == j-1){//해당 요일이 되었다는 뜻
									isPass = true;	
									break;
								}
							}
						}
						if(isPass){
							continue;
						}
						else{
							pthread_exit(NULL);
							return NULL;
						}
					}
					else if(i == 1){//월
						for(j=1 ; j<=12 ; j++){
							if(j%parse_num==0){//해당 월 check
								if(today->tm_mon+1 == j){//해당 월이 되었다는 뜻
									isPass = true;	
									break;
								}
							}
						}
						if(isPass){
							continue;
						}
						else{
							pthread_exit(NULL);
							return NULL;
						}

					}
					else if(i == 2){//일
						for(j=1 ; j<=31 ; j++){
							if(j%parse_num==0){//해당 일 check
								if(today->tm_mday == j){//해당 일이 되었다는 뜻
									isPass = true;	
									break;
								}
							}
						}
						if(isPass){
							continue;
						}
						else{
							pthread_exit(NULL);
							return NULL;
						}

					}
					else if(i == 3){//시간
						for(j=1 ; j<=24 ; j++){
							if(j%parse_num==0){//해당 시간 check
								if(today->tm_hour == j-1){//해당 시간이 되었다는 뜻
									isPass = true;	
									break;
								}
							}
						}
						if(isPass){
							continue;
						}
						else{
							pthread_exit(NULL);
							return NULL;
						}
					}
					else if(i == 4){//분
						for(j=1 ; j<=60 ; j++){
							if(j%parse_num==0){//해당 분 check
								if(today->tm_min == j-1){//해당 분이 되었다는 뜻
									isPass = true;	
									break;
								}
							}
						}
						if(isPass){
							continue;
						}
						else{
							pthread_exit(NULL);
							return NULL;
						}

					}
				}
			}
			else{// *가 단독으로 존재한다면
				//3
				continue;//통과
			}
		}
		else if(strstr(time_arr[i],",")!=NULL){// ,가 존재한다면
			if(strstr(time_arr[i],"-")!=NULL){// ,와 -가 존재한다면
				if(strstr(time_arr[i],"/")!=NULL){// 2-5/2,1-10/2,3,1-5 와 같은 형식이 들어옴
					//4   	
					p1 = time_arr[i];
					comma_p = strstr(p1,",");
					comma_num = 1;
					while((comma_p = strstr(comma_p+1,","))!=NULL){
						comma_num++;
					}
					comma_p = strstr(p1, ",");
					strncpy(time_comma,p1, strlen(p1)-strlen(comma_p));
					if(strstr(time_comma,"-")==NULL){// 7
						parse_num = atoi(time_comma);
					}
					else{
						if(strstr(time_comma,"/")!=NULL){// 2-5/2
							p2 = time_comma;
							bar_p = strstr(p2,"-");
							strncpy(xscope,p2,strlen(p2)-strlen(bar_p));
							bar_p++;
							p2 = bar_p;
							slide_p = strstr(p2,"/");
							strncpy(yscope,p2,strlen(p2)-strlen(slide_p));
							slide_p++;
							p2 = slide_p;
							strncpy(scope,p2,strlen(p2));

							scope1 = atoi(xscope);
							scope2 = atoi(yscope);
							if(scope1 > scope2){
								tnum = scope1;
								scope1 = scope2;
								scope2 = tnum;
							}
							parse_num = atoi(scope);
						}
						else{// 1-5
							p2 = time_comma;
							bar_p = strstr(p2,"-");
							strncpy(xscope,p2,strlen(p2)-strlen(bar_p));
							bar_p++;
							p2 = bar_p;
							strncpy(yscope,p2,strlen(p2));

							scope1 = atoi(xscope);
							scope2 = atoi(yscope);
							if(scope1 > scope2){
								tnum = scope1;
								scope1 = scope2;
								scope2 = tnum;
							}
						}
					}
					comma_p++;
					p1 = comma_p;
					for(j=0 ; j<=comma_num ; j++){
						isPass = false;
						if(i == 0){//요일
							if(strstr(time_comma,"-")==NULL){// 7
								if(today->tm_wday == parse_num){
									isPass = true;
									break;
								}
							}
							else{
								if(strstr(time_comma,"/")!=NULL){// 2-5/2
									tnum=0;
									for(k=scope1 ; k<=scope2 ; k++){
										tnum++;
										if(tnum%parse_num == 0){
											if(today->tm_wday == k){
												isPass = true;
												break;
											}
										}
									}
									if(isPass){
										break;
									}
								}
								else{// 1-5
									for(k=scope1 ; k<=scope2 ; k++){
										if(today->tm_wday == k){
											isPass = true;
											break;
										}
									}
									if(isPass){
										break;
									}
								}
							}
						}
						if(i == 1){//월
							if(strstr(time_comma,"-")==NULL){// 7
								if(today->tm_mon+1 == parse_num){
									isPass = true;
									break;
								}
							}
							else{
								if(strstr(time_comma,"/")!=NULL){// 2-5/2
									tnum=0;
									for(k=scope1 ; k<=scope2 ; k++){
										tnum++;
										if(tnum%parse_num == 0){
											if(today->tm_mon+1 == k){
												isPass = true;
												break;
											}
										}
									}
									if(isPass){
										break;
									}
								}
								else{// 1-5
									for(k=scope1 ; k<=scope2 ; k++){
										if(today->tm_mon+1 == k){
											isPass = true;
											break;
										}
									}
									if(isPass){
										break;
									}
								}
							}
						}
						if(i == 2){//일
							if(strstr(time_comma,"-")==NULL){// 7
								if(today->tm_mday == parse_num){
									isPass = true;
									break;
								}
							}
							else{
								if(strstr(time_comma,"/")!=NULL){// 2-5/2
									tnum=0;
									for(k=scope1 ; k<=scope2 ; k++){
										tnum++;
										if(tnum%parse_num == 0){
											if(today->tm_mday == k){
												isPass = true;
												break;
											}
										}
									}
									if(isPass){
										break;
									}
								}
								else{// 1-5
									for(k=scope1 ; k<=scope2 ; k++){
										if(today->tm_mday == k){
											isPass = true;
											break;
										}
									}
									if(isPass){
										break;
									}
								}
							}
						}
						if(i == 1){//시
							if(strstr(time_comma,"-")==NULL){// 7
								if(today->tm_hour == parse_num){
									isPass = true;
									break;
								}
							}
							else{
								if(strstr(time_comma,"/")!=NULL){// 2-5/2
									tnum=0;
									for(k=scope1 ; k<=scope2 ; k++){
										tnum++;
										if(tnum%parse_num == 0){
											if(today->tm_hour == k){
												isPass = true;
												break;
											}
										}
									}
									if(isPass){
										break;
									}
								}
								else{// 1-5
									for(k=scope1 ; k<=scope2 ; k++){
										if(today->tm_hour == k){
											isPass = true;
											break;
										}
									}
									if(isPass){
										break;
									}
								}
							}
						}
						if(i == 4){//분
							if(strstr(time_comma,"-")==NULL){// 7
								if(today->tm_min == parse_num){
									isPass = true;
									break;
								}
							}
							else{
								if(strstr(time_comma,"/")!=NULL){// 2-5/2
									tnum=0;
									for(k=scope1 ; k<=scope2 ; k++){
										tnum++;
										if(tnum%parse_num == 0){
											if(today->tm_min == k){
												isPass = true;
												break;
											}
										}
									}
									if(isPass){
										break;
									}
								}
								else{// 1-5
									for(k=scope1 ; k<=scope2 ; k++){
										if(today->tm_min == k){
											isPass = true;
											break;
										}
									}
									if(isPass){
										break;
									}
								}
							}
						}
						
						if(j<comma_num-1){
							comma_p = strstr(comma_p+1,",");
							memset(time_comma,'\0',sizeof(time_comma));
							strncpy(time_comma,p1,strlen(p1)-strlen(comma_p));
							comma_p++;
							p1 = comma_p;
							if(strstr(time_comma,"-")==NULL){// 7
								parse_num = atoi(time_comma);
							}
							else{
								if(strstr(time_comma,"/")!=NULL){// 2-5/2
									p2 = time_comma;
									bar_p = strstr(p2,"-");
									strncpy(xscope,p2,strlen(p2)-strlen(bar_p));
									bar_p++;
									p2 = bar_p;
									slide_p = strstr(p2,"/");
									strncpy(yscope,p2,strlen(p2)-strlen(slide_p));
									slide_p++;
									p2 = slide_p;
									strncpy(scope,p2,strlen(p2));
									scope1 = atoi(xscope);
									scope2 = atoi(yscope);
									if(scope1 > scope2){
										tnum = scope1;
										scope1 = scope2;
										scope2 = tnum;
									}
									parse_num = atoi(scope);
								}
								else{// 1-5
									p2 = time_comma;
									bar_p = strstr(p2,"-");
									strncpy(xscope,p2,strlen(p2)-strlen(bar_p));
									bar_p++;
									p2 = bar_p;
									strncpy(yscope,p2,strlen(p2));
									scope1 = atoi(xscope);
									scope2 = atoi(yscope);
									if(scope1 > scope2){
										tnum = scope1;
										scope1 = scope2;
										scope2 = tnum;
									}
								}
							}
						}
						else{
							memset(time_comma,'\0',sizeof(time_comma));
							strncpy(time_comma,p1,strlen(p1));
							if(strstr(time_comma,"-")==NULL){// 7
								parse_num = atoi(time_comma);
							}
							else{
								if(strstr(time_comma,"/")!=NULL){// 2-5/2
									p2 = time_comma;
									bar_p = strstr(p2,"-");
									strncpy(xscope,p2,strlen(p2)-strlen(bar_p));
									bar_p++;
									p2 = bar_p;
									slide_p = strstr(p2,"/");
									strncpy(yscope,p2,strlen(p2)-strlen(slide_p));
									slide_p++;
									p2 = slide_p;
									strncpy(scope,p2,strlen(p2));
									scope1 = atoi(xscope);
									scope2 = atoi(yscope);
									if(scope1 > scope2){
										tnum = scope1;
										scope1 = scope2;
										scope2 = tnum;
									}
									parse_num = atoi(scope);
								}
								else{// 1-5
									p2 = time_comma;
									bar_p = strstr(p2,"-");
									strncpy(xscope,p2,strlen(p2)-strlen(bar_p));
									bar_p++;
									p2 = bar_p;
									strncpy(yscope,p2,strlen(p2));
									scope1 = atoi(xscope);
									scope2 = atoi(yscope);
									if(scope1 > scope2){
										tnum = scope1;
										scope1 = scope2;
										scope2 = tnum;
									}
								}
							}
						}
					}//for 문 종료
					if(isPass){
						continue;
					}
					else{
						pthread_exit(NULL);
						return NULL;
					}
				}
				else{// 2-5,6-10,5 와 같은 형식이 들어옴
					//5
					comma_num=1;
					p1 = time_arr[i];
					comma_p = strstr(p1,",");
					while((comma_p = strstr(comma_p+1,","))!=NULL){
						comma_num++;
					}
					
					comma_p = strstr(p1,",");
					strncpy(time_comma,p1,strlen(p1)-strlen(comma_p));	
					if(strstr(time_comma,"-")!=NULL){// 1-5
						p2 = time_comma;
						bar_p = strstr(p2,"-");
						strncpy(xscope,p2,strlen(p2)-strlen(bar_p));	
						bar_p++;
						p2 = bar_p;
						strncpy(yscope,p2,strlen(p2));
						scope1 = atoi(xscope);
						scope2 = atoi(yscope);
						if(scope1 > scope2){
							tnum = scope1;
							scope1 = scope2;
							scope2 = tnum;
						}
					}
					else{// 숫자
						parse_num = atoi(time_comma);
					}
					comma_p++;
					p1 = comma_p;
					for(j=0 ; j<=comma_num ; j++){
						isPass = false;
						if(strstr(time_comma,"-")!=NULL){// 1-5
							if(i == 0){//요일
								for(k=scope1 ; k<=scope2 ; k++){
									if(today->tm_wday == k){
										isPass = true;
										break;
									}
								}
							}
							if(i == 1){//월
								for(k=scope1 ; k<=scope2 ; k++){
									if(today->tm_mon+1 == k){
										isPass = true;
										break;
									}
								}
							}
							if(i == 2){//일
								for(k=scope1 ; k<=scope2 ; k++){
									if(today->tm_mday == k){
										isPass = true;
										break;
									}
								}
							}
							if(i == 3){//시간
								for(k=scope1 ; k<=scope2 ; k++){
									if(today->tm_hour == k){
										isPass = true;
										break;
									}
								}
							}
							if(i == 4){//분
								for(k=scope1 ; k<=scope2 ; k++){
									if(today->tm_min == k){
										isPass = true;
										break;
									}
								}
							}

							if(isPass){
								break;
							}
						}
						else{//숫자
							if(i==0){//요일
								if(today->tm_wday == parse_num){
									isPass = true;
									break;
								}
							}
							else if(i==1){//월
								if(today->tm_mon+1 == parse_num){
									isPass = true;
									break;
								}
							}
							else if(i==2){//일
								if(today->tm_mday == parse_num){
									isPass = true;
									break;
								}
							}
							else if(i==3){//시간
								if(today->tm_hour == parse_num){
									isPass = true;
									break;
								}
							}
							else if(i==4){
								if(today->tm_min == parse_num){
									isPass = true;
									break;
								}
							}
						}

						if(j<comma_num-1){
							comma_p = strstr(comma_p+1,",");
							memset(time_comma,'\0',sizeof(time_comma));
							strncpy(time_comma,p1,strlen(p1)-strlen(comma_p));
							if(strstr(time_comma,"-")!=NULL){//1-5일 경우
								p2 = time_comma;
								bar_p = strstr(p2,"-");
								strncpy(xscope,p2,strlen(p2)-strlen(bar_p));
								bar_p++;
								p2 = bar_p;
								strncpy(yscope,p2,strlen(p2));
								scope1 = atoi(xscope);
								scope2 = atoi(yscope);
								if(scope1 > scope2){
									tnum = scope1;
									scope1 = scope2;
									scope2 = tnum;
								}
							}
							else{// 숫자일 경우
								parse_num = atoi(time_comma);
							}
							comma_p++;
							p1 = comma_p;
						}
						else{
							memset(time_comma,'\0',sizeof(time_comma));
							strncpy(time_comma,p1,strlen(p1));
							if(strstr(time_comma,"-")!=NULL){// 1-5
								p2 = time_comma;
								bar_p = strstr(p2,"-");
								strncpy(xscope,p2,strlen(p2)-strlen(bar_p));
								bar_p++;
								p2 = bar_p;
								strncpy(yscope,p2,strlen(p2));
								scope1 = atoi(xscope);
								scope2 = atoi(yscope);
								if(scope1 > scope2){
									tnum = scope1;
									scope1 = scope2;
									scope2 = tnum;
								}
							}
							else{//숫자
								parse_num = atoi(time_comma);
							}
						}
					}//for 문 종료
					if(isPass){
						continue;
					}
					else{
						pthread_exit(NULL);
						return NULL;
					}
				}
			
			}
			else{// ,가 단독으로 존재한다면, 3,4,5 와 같은 형식 
				//6
				comma_p = strstr(time_arr[i],",");
				comma_num = 1;
				while((comma_p = strstr(comma_p+1, ","))!=NULL){
					comma_num++;
				}
				p1 = time_arr[i];
				comma_p = strstr(p1,",");
				memset(time_comma,0,sizeof(time_comma));
				strncpy(time_comma,p1,strlen(p1)-strlen(comma_p));
				comma_p++;
				p1 = comma_p;
				parse_num = atoi(time_comma);

				isPass = false;
				for(j=0 ; j<=comma_num ; j++){
					if(i == 0){//요일
						if(today->tm_wday == parse_num){
							isPass = true;
							break;
						}
					}
					else if(i == 1){//월
						if(today->tm_mon+1 == parse_num){
							isPass = true;
							break;
						}
					}
					else if(i == 2){//일
						if(today->tm_mday == parse_num){
							isPass = true;
							break;
						}
					}
					else if(i == 3){//시간
						if(today->tm_hour == parse_num){
							isPass = true;
							break;
						}
					}
					else if(i == 4){//분
						if(today->tm_min == parse_num){
							isPass = true;
							break;
						}
					}

					if(j<comma_num-1){
						comma_p = strstr(comma_p+1,",");
						memset(time_comma,'\0',sizeof(time_comma));
						strncpy(time_comma,p1,strlen(p1)-strlen(comma_p));
						parse_num = atoi(time_comma);
						comma_p++;
						p1 = comma_p;
					}
					else{
						memset(time_comma,'\0',sizeof(time_comma));
						strncpy(time_comma,p1,strlen(p1));
						parse_num = atoi(time_comma);
					}
				}
				if(isPass){
					continue;//통과
				}
				else{
					pthread_exit(NULL);
					return NULL;
				}
			}
			
		}
		else if(strstr(time_arr[i],"-")!=NULL){// -가 단독으로 존재한다면
			//7	
			if(strstr(time_arr[i],"/")==NULL){
				p1 = time_arr[i];
				p2 = strstr(p1,"-");
				strncpy(xscope,p1,strlen(p1)-strlen(p2));
				p2++;
				p1 = p2;
				strncpy(yscope,p1,strlen(p1));
				scope1 = atoi(xscope);
				scope2 = atoi(yscope);
				if(scope1 > scope2){
					tnum = scope1;	
					scope1 = scope2;
					scope2 = tnum;
				}
				
				if(i == 0){//요일
					isPass = false;
					for(j=scope1 ; j<=scope2 ; j++){
						if(today->tm_wday == j){
							isPass = true;	
							break;
						}
					}
					if(isPass){
						continue;
					}
					else{
						pthread_exit(NULL);
						return NULL;
					}
				}
				else if(i == 1){//월
					isPass = false;
					for(j=scope1 ; j<=scope2 ; j++){
						if(today->tm_mon+1 == j){
							isPass = true;	
							break;
						}
					}
					if(isPass){
						continue;
					}
					else{
						pthread_exit(NULL);
						return NULL;
					}
	
				}
				else if(i == 2){//일
					isPass = false;
					for(j=scope1 ; j<=scope2 ; j++){
						if(today->tm_mday == j){
							isPass = true;	
							break;
						}
					}
					if(isPass){
						continue;
					}
					else{
						pthread_exit(NULL);
						return NULL;
					}
	
				}
				else if(i == 3){//시간
					isPass = false;
					for(j=scope1 ; j<=scope2 ; j++){
						if(today->tm_hour == j){
							isPass = true;	
							break;
						}
					}
					if(isPass){
						continue;
					}
					else{
						pthread_exit(NULL);
						return NULL;
					}

				}
				else if(i == 4){//분
					isPass = false;
					for(j=scope1 ; j<=scope2 ; j++){
						if(today->tm_min == j){
							isPass = true;	
							break;
						}
					}
					if(isPass){
						continue;
					}
					else{
						pthread_exit(NULL);
						return NULL;
					}

				}
			}
			else{// 1-5/2 와 같은 형식
				p1 = time_arr[i];
				p2 = strstr(p1,"-");
				strncpy(xscope,p1,strlen(p1)-strlen(p2));
				p2++;
				p1 = p2;
				p2 = strstr(p1,"/");

				strncpy(yscope,p1,strlen(p1)-strlen(p2));
				p2++;
				p1 = p2;

				strncpy(scope, p1, strlen(p1));

				scope1 = atoi(xscope);
				scope2 = atoi(yscope);
				parse_num = atoi(scope);
				if(scope1 > scope2){
					tnum = scope1;	
					scope1 = scope2;
					scope2 = tnum;
				}
				tnum = 0;
				if(i==0){//요일
					isPass = false;
					for(j=scope1 ; j<=scope2 ; j++){
						tnum++;	
						if(tnum%parse_num==0){
							if(today->tm_wday == j){
								isPass = true;
								break;
							}
						}
					}
					if(isPass){
						continue; //통과
					}
					else{
						pthread_exit(NULL);
						return NULL;
					}
				}
				else if(i==1){//월
					isPass = false;
					for(j=scope1 ; j<=scope2 ; j++){
						tnum++;	
						if(tnum%parse_num==0){
							if(today->tm_mon+1 == j){
								isPass = true;
								break;
							}
						}
					}
					if(isPass){
						continue; //통과
					}
					else{
						pthread_exit(NULL);
						return NULL;
					}
				}
				else if(i==2){//일
					isPass = false;
					for(j=scope1 ; j<=scope2 ; j++){
						tnum++;	
						if(tnum%parse_num==0){
							if(today->tm_mday == j){
								isPass = true;
								break;
							}
						}
					}
					if(isPass){
						continue; //통과
					}
					else{
						pthread_exit(NULL);
						return NULL;
					}
				}
				else if(i==3){//시
					isPass = false;
					for(j=scope1 ; j<=scope2 ; j++){
						tnum++;	
						if(tnum%parse_num==0){
							if(today->tm_hour == j){
								isPass = true;
								break;
							}
						}
					}
					if(isPass){
						continue; //통과
					}
					else{
						pthread_exit(NULL);
						return NULL;
					}
				}
				else if(i==4){//분
					isPass = false;
					for(j=scope1 ; j<=scope2 ; j++){
						tnum++;	
						if(tnum%parse_num==0){
							if(today->tm_min == j){
								isPass = true;
								break;
							}
						}
					}
					if(isPass){
						continue; //통과
					}
					else{
						pthread_exit(NULL);
						return NULL;
					}
				}
				
			}
		}
		else{// 숫자만 단독으로 존재한다면
			//8
			parse_num = atoi(time_arr[i]);
			if(i == 0){//요일
				if(today->tm_wday == parse_num){
					continue;	
				}
				else{
					pthread_exit(NULL);
					return NULL;
				}
			}
			else if(i == 1){//월
				if(today->tm_mon+1 == parse_num){
					continue;	
				}
				else{
					pthread_exit(NULL);
					return NULL;
				}
			}
			else if(i == 2){//일
				if(today->tm_mday == parse_num){
					continue;	
				}
				else{
					pthread_exit(NULL);
					return NULL;
				}
			}
			else if(i == 3){//시간
				if(today->tm_hour == parse_num){
					continue;	
				}
				else{
					pthread_exit(NULL);
					return NULL;
				}
			}
			else if(i == 4){//분
				if(today->tm_min == parse_num){
					/*fputs(time_arr[i],fp);
					fputs("\n",fp);
					fclose(fp);
					fp = fopen(log_file,"a+");*/
					continue;	
				}
				else{
					pthread_exit(NULL);
					return NULL;
				}
			}
		}

	}
	// 위 모든 과정을 통과했다면 system() 함수 동작하기
	if(system(command_cycle)==0){//system() 함수가 정상적으로 동작했다는 것임
		strcpy(input_time_cpy,ctime(&current_time));
		input_time_cpy[strlen(input_time_cpy)-1] = '\0';//ctime 을 통해 출력한
		sprintf(input_time,"[%s]",input_time_cpy);	

		memset(cat_str,0,sizeof(cat_str));
		strcat(cat_str,data->minute);
		strcat(cat_str," ");
		strcat(cat_str,data->hour);
		strcat(cat_str," ");
		strcat(cat_str,data->day);
		strcat(cat_str," ");
		strcat(cat_str,data->month);
		strcat(cat_str," ");
		strcat(cat_str,data->week);
		strcat(cat_str," ");
		strcat(cat_str,data->command_cycle);

		sprintf(add_cat_line,"%s run %s\n",input_time,cat_str);
		memset(data->minute,'\0',sizeof(data->minute));
		memset(data->hour,'\0',sizeof(data->hour));
		memset(data->day,'\0',sizeof(data->day));
		memset(data->month,'\0',sizeof(data->month));
		memset(data->week,'\0',sizeof(data->week));


		fputs(add_cat_line,fp);//ssu_crontab_log 파일에 add 추가
		sleep(1);
		fclose(fp);
	}

	pthread_exit(NULL);
	return NULL;
}
char *compare_command_remove(int prev, int pres)// ssu_crontab_file에서 명령어가 삭제 되었을 때 어떤 명령어가 삭제 되었는지 알려주는 함수
{
	int i;
	for(i=0 ; i<pres ; i++){
		if(strcmp(save_command[i], save_remove_ary[i])){
			return save_command[i];
		}
	}
	return save_command[i];
}
