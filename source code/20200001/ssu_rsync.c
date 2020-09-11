#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<signal.h>
#include<dirent.h>
#include<utime.h>
#include<time.h>
#include "ssu_rsync.h"

int rOption = false;
int tOption = false;
int mOption = false;
int isAllSync = false;

char programDir[PATH_MAX];
char rsyncDir[PATH_MAX];
char rsyncDir_prev[PATH_MAX];
char srcDir[PATH_MAX];
char srcDir_prev[PATH_MAX];
char findDir[PATH_MAX];
char src[PATH_MAX];
char dst[PATH_MAX];
char sync_file[BUFLEN] = "ssu_rsync_log";
char cpy[PATH_MAX];
char cpy2[PATH_MAX];
char real[PATH_MAX];
char cat_str[PATH_MAX];
char m_str[BUFLEN][PATH_MAX];

struct stat statDst;	
struct stat statSrc;	

void ssu_rsync(int argc, char *argv[])
{
	

	if(argc<3){
		fprintf(stderr,"usage : %s <src> <dst>\n",argv[0]);
		exit(1);
	}
	else if(argc==3){// 옵션이 들어가지 않은 경우
		strcpy(src,argv[1]);
		strcpy(dst,argv[2]);
	}
	else if(argc==4){// 옵션이 들어간 경우
		if(strstr(argv[1],"r")!=NULL){
			rOption = true;
		}
		else if(strstr(argv[1],"t")!=NULL){
			tOption = true;
		}
		else if(strstr(argv[1],"m")!=NULL){
			mOption = true;
		}
		else{//옵션이 잘못 들어간 경우임
			fprintf(stderr,"please input \'-r\' or \'-t\' or \'-m\' option\n");
			exit(1);
		}
		strcpy(src,argv[2]);
		strcpy(dst,argv[3]);
	}
	else{// 너무 많은 인자를 넣음
		fprintf(stderr,"usage : %s [option] <src> <dst>\n",argv[0]);
		exit(1);
	}// option, src, dst 설정 완료
	
	// 존재하지 않는 파일을 인자로 주어진다면
	getcwd(programDir,PATH_MAX);
	if(access(src,F_OK)<0){
		fprintf(stderr,"please input existing files. not existing %s\n",src);
		exit(1);
	}
	if(access(dst,F_OK)<0){
		fprintf(stderr,"please input existing files. not existing %s\n",dst);
		exit(1);
	}
	// dst는 오직 디렉토리이어야만 함
	if(stat(dst,&statDst)<0){
		fprintf(stderr,"stat error for %s\n",dst);
		exit(1);
	}
	if(!S_ISDIR(statDst.st_mode)){
		fprintf(stderr,"%s는 디렉토리가 아닙니다.\n",dst);
		exit(1);
	}
	// src는 일반파일 혹은 디렉토리임
	if(stat(src,&statSrc)<0){
		fprintf(stderr,"stat error for %s\n",src);
		exit(1);
	}
	if(!S_ISREG(statSrc.st_mode) && !S_ISDIR(statSrc.st_mode)){
		fprintf(stderr,"%s는 일반파일 또는 디렉토리 파일이어야만 합니다.\n",src);
		exit(1);
	}

	/////////////////////////////////////
	//권한 check//
	
	if(access(src,R_OK)<0){//읽기 권한이 있는지 check
		fprintf(stderr,"%s에 대한 읽기 권한이 없습니다.\n",src);
		fprintf(stderr,"usage : %s [option] <src> <dst>\n",argv[0]);
		exit(1);
	}
	if(access(src,W_OK)<0){//쓰기 권한이 있는지 check
		fprintf(stderr,"%s에 대한 쓰기 권한이 없습니다.\n",src);
		fprintf(stderr,"usage : %s [option] <src> <dst>\n",argv[0]);
		exit(1);
	}

	if(access(dst,R_OK)<0){//읽기 권한이 있는지 check
		fprintf(stderr,"%s에 대한 읽기 권한이 없습니다.\n",dst);
		fprintf(stderr,"usage : %s [option] <src> <dst>\n",argv[0]);
		exit(1);
	}
	if(access(dst,W_OK)<0){//쓰기 권한이 있는지 check
		fprintf(stderr,"%s에 대한 쓰기 권한이 없습니다.\n",dst);
		fprintf(stderr,"usage : %s [option] <src> <dst>\n",argv[0]);
		exit(1);
	}
	////////////////////////////
	/////동기화 작업/////////
	exec_rsync();

	return;	
}
void exec_rsync()
{
	char *p1, *p2;
	char *del;
	char tmp[PATH_MAX];
	struct stat statbuf;
	char name[BUFLEN];
	char go_dir[PATH_MAX];

	if(strstr(dst,"/")!=NULL){// dst가 절대경로라면
		strcpy(rsyncDir, dst);
		p1 = dst;
		p2 = strstr(p1,"/");
		del = p2;
		while((p2 = strstr(p2+1,"/"))!=NULL){
			del = p2;
		}
		strncpy(findDir,p1,strlen(p1)-strlen(del));// 지정 디렉토리의 경로 위치
	}
	else{// dst가 상대경로라면
		getcwd(findDir,PATH_MAX);
		getcwd(rsyncDir,PATH_MAX);
		strcat(rsyncDir,"/");
		strcat(rsyncDir,dst);
	}
	///////////////////////////////////////////////////////////
	if(strstr(src,"/")!=NULL){// src가 절대경로라면
		p1 = src;
		p2 = strstr(p1,"/");
		del = p2;
		while((p2 = strstr(p2+1,"/"))!=NULL){
			del = p2;
		}
		strncpy(go_dir,p1,strlen(p1)-strlen(del));
		del++;
		strncpy(name,del,strlen(del));

		chdir(go_dir);
		if(stat(name, &statbuf)<0){
			fprintf(stderr,"stat error for %s\n",name);
			exit(1);
		}

		if(S_ISREG(statbuf.st_mode)){
			//일반파일
			del--;
			strncpy(srcDir, p1, strlen(p1)-strlen(del));
		}
		else if(S_ISDIR(statbuf.st_mode)){
			//디렉토리
			strcpy(srcDir,src);
		}
		chdir(programDir);
	}
	else{// src가 상대경로라면
		getcwd(tmp,PATH_MAX);
		strcat(tmp,"/");
		strcat(tmp,src);
		p1 = tmp;
		p2 = strstr(p1,"/");
		del = p2;
		while((p2 = strstr(p2+1,"/"))!=NULL){
			del = p2;
		}
		strncpy(go_dir,p1,strlen(p1)-strlen(del));
		del++;
		strncpy(name,del,strlen(del));

		chdir(go_dir);
		if(stat(name,&statbuf)<0){
			fprintf(stderr,"stat error for %s\n",name);
			exit(1);
		}
		if(S_ISREG(statbuf.st_mode)){
			//일반파일
			del--;
			strncpy(srcDir, p1, strlen(p1)-strlen(del));
		}
		else if(S_ISDIR(statbuf.st_mode)){
			//디렉토리
			strcpy(srcDir,tmp);
			p1 = srcDir;
			p2 = strstr(p1,"/");
			del = p2;
			while((p2 = strstr(p2+1,"/"))!=NULL){
				del = p2;
			}
			strncpy(srcDir_prev,p1,strlen(p1)-strlen(del));
		}
		chdir(programDir);

	}
	///////////////////////////////////////////////////////////
	option_rsync();	
}
void option_rsync()
{
	struct stat statSrc_sub;
	struct stat statDst_sub;
	struct stat tmpbuf;
	struct utimbuf time_buf;
	struct dirent *dirp, *dirp2, *dirp3;
	struct tm *today;
	DIR *dp, *dp2, *dp3;
	FILE *fp1, *fp2;
	char str_tmp[PATH_MAX];
	int length;
	int isSame;
	int isNot;

	char src_name[BUFLEN];
	char move_dir[PATH_MAX];
	char *p1, *p2;
	long int t;

	time_t current_time;
	FILE *sync;
	char input_time[PATH_MAX];
	char tmp[PATH_MAX];
	char toStr[BUFLEN];
	char filename[PATH_MAX];
	int isTrue;
	int cnt =0;
	int i;
//////////////////ssu_rsync_log 파일 생성 및 추가////////////
	chdir(programDir);

	time(&current_time);
	today = localtime(&current_time);
	sync = fopen(sync_file,"a+");
	strcpy(tmp,ctime(&current_time));
	tmp[strlen(tmp)-1] = '\0';

	if(rOption){
		sprintf(input_time,"[%s] ssu_rsync -r %s %s\n",tmp,src,dst);
	}
	else if(mOption){
		sprintf(input_time,"[%s] ssu_rsync -m %s %s\n",tmp,src,dst);
	}
	else if(tOption){
		sprintf(input_time,"[%s] ssu_rsync -t %s %s\n",tmp,src,dst);
	}
	else{
		sprintf(input_time,"[%s] ssu_rsync %s %s\n",tmp,src,dst);
	}
	fputs(input_time, sync);
////////////////////////////////////////////////////////////////
	p1 = src;

	//src_name 의 이름 구하기
	if((p2 = strstr(p1,"/"))==NULL){// src가 상대경로인 경우
		strcpy(src_name,src);
		stat(src_name,&tmpbuf);
	}
	else{// src가 절대경로인 경우
		stat(src,&tmpbuf);
		memset(src_name,'\0',sizeof(src_name));
		strncpy(src_name, p1, strlen(p1));
		while((p2 = strstr(p2+1,"/"))!=NULL){// src의 이름 src_name으로 빼내기
			p2++;	
			p1 = p2;
			memset(src_name,'\0',sizeof(src_name));
			strncpy(src_name, p1, strlen(p1));
		}
	}
	
//////////////////////////////////////////////////////////////////////////////////////	
	if(S_ISDIR(statSrc.st_mode)){// src가 디렉토리인 경우
		if((dp2 = opendir(srcDir))==NULL){
			fprintf(stderr,"opendir error\n");
			exit(1);
		}
		while((dirp2 = readdir(dp2))!=NULL){
			isSame=false;
			isNot=false;
			chdir(srcDir);
			if(!strcmp(dirp2->d_name,".") || !strcmp(dirp2->d_name,"..")){
				continue;
			}
			if(lstat(dirp2->d_name,&statSrc_sub)<0){
				fprintf(stderr,"stat error for %s\n",dirp->d_name);
				exit(1);
			}
			memset(filename,'\0',sizeof(filename));
			strcpy(filename,dirp2->d_name);
			//////////////////////////src && dst 디렉토리 비교/////////////////////////
			if((dp = opendir(rsyncDir))==NULL){
				fprintf(stderr,"opendir error\n");
				exit(1);
			}
			chdir(rsyncDir);
			if(rOption){// 만약 r옵션이라면 서브 디렉토리까지 동기화 시켜야함
				if(S_ISDIR(statSrc_sub.st_mode)){
					if(access(dirp2->d_name,F_OK)<0){// dst에 기존 서브디렉토리가 존재하지 않으면 모든 파일을 동기화함
						isAllSync = true;
						mkdir(dirp2->d_name,0755);
					}
					chdir(srcDir);
					strcpy(real,srcDir);
					strcpy(cpy2,srcDir);
					strcpy(rsyncDir_prev,rsyncDir);
					strcpy(cpy,rsyncDir);
					strcpy(srcDir_prev,srcDir);

					memset(cat_str,'\0',sizeof(cat_str));
					rOption_rsync(dirp2->d_name, sync);

					strcpy(rsyncDir,rsyncDir_prev);
					strcpy(srcDir,srcDir_prev);
					chdir(rsyncDir);
					continue;	
				}
			}
			else{
				if(S_ISDIR(statSrc_sub.st_mode)){
					continue;	
				}
			}

			isTrue = false;
			while((dirp = readdir(dp))!=NULL){
				if(!strcmp(dirp->d_name,".") || !strcmp(dirp->d_name,"..")){
					continue;
				}
				if(lstat(dirp->d_name,&statDst_sub)<0){
					fprintf(stderr,"stat error for %s\n",dirp2->d_name);
					exit(1);
				}
/////////////
				if(S_ISDIR(statSrc_sub.st_mode)){
					continue;
				}
				if(!strcmp(dirp->d_name, dirp2->d_name)){// 동일한 이름의 디렉토리 존재
					if(statDst_sub.st_size == statSrc_sub.st_size){// 동일한 파일이라면 동기화X
						if(statDst_sub.st_mtime == statSrc_sub.st_mtime){
							isTrue = true;
							isSame = true;
							break;	
						}
						else{
							isTrue = true;
							time_buf.modtime = statSrc_sub.st_mtime;
							isNot = true;	
							chdir(srcDir);

							fp1 = fopen("tmp_file","w");// 임시파일 생성, 내용을 복사할 것임
							fp2 = fopen(dirp2->d_name,"r");// 원본 파일을 읽어 임시파일에 내용 복사
							memset(str_tmp,'\0',sizeof(str_tmp));
							while(fgets(str_tmp,sizeof(str_tmp),fp2)!=NULL){
								fputs(str_tmp,fp1);
							}
							fclose(fp2);
							fclose(fp1);
		
							chdir(rsyncDir);
							if(S_ISDIR(statDst_sub.st_mode)){
								rmdir(dirp2->d_name);
							}
							else{
								remove(dirp2->d_name);
							}
							
							memset(move_dir,'\0',sizeof(move_dir));
							strcpy(move_dir,rsyncDir);
							strcat(move_dir,"/");
							strcat(move_dir,dirp2->d_name);
							utime(move_dir,&time_buf);

							chdir(srcDir);

							rename(dirp2->d_name,move_dir);

							rename("tmp_file",dirp2->d_name);
							utime(dirp2->d_name,&time_buf);

							fputs("\t",sync);
							fputs(dirp2->d_name,sync);
							fputs(" ",sync);
							sprintf(toStr,"%ldbytes\n",statSrc_sub.st_size);
							fputs(toStr,sync);

							break;
									
						}
					}
					else{// 다른 파일이라면 동기화O
						isTrue = true;
						time_buf.modtime = statSrc_sub.st_mtime;
						isNot = true;	
						chdir(srcDir);

						fp1 = fopen("tmp_file","w");// 임시파일 생성, 내용을 복사할 것임
						fp2 = fopen(dirp2->d_name,"r");// 원본 파일을 읽어 임시파일에 내용 복사
						memset(str_tmp,'\0',sizeof(str_tmp));
						while(fgets(str_tmp,sizeof(str_tmp),fp2)!=NULL){
							fputs(str_tmp,fp1);
						}
						fclose(fp2);
						fclose(fp1);
		
						chdir(rsyncDir);
						if(S_ISDIR(statDst_sub.st_mode)){
							rmdir(dirp2->d_name);
						}
						else{
							remove(dirp2->d_name);
						}
	
						memset(move_dir,'\0',sizeof(move_dir));
						strcpy(move_dir,rsyncDir);
						strcat(move_dir,"/");
						strcat(move_dir,dirp2->d_name);
				
						chdir(srcDir);
						rename(dirp2->d_name,move_dir);
						utime(move_dir,&time_buf);
						rename("tmp_file",dirp2->d_name);
						utime(dirp2->d_name,&time_buf);
						fputs("\t",sync);
						fputs(dirp2->d_name,sync);
						fputs(" ",sync);
						sprintf(toStr,"%ldbytes\n",statSrc_sub.st_size);
						fputs(toStr,sync);
						break;
					}
				}
				else{// 
					
				}

			}
			closedir(dp);
			if(isTrue){
				if(mOption){
					strcpy(m_str[cnt],dirp2->d_name);
				}
				cnt++;
			}
			if(isSame || isNot){
				//pass
			}
			else{
				time_buf.modtime = statSrc_sub.st_mtime;
				chdir(srcDir);

				fp1 = fopen("tmp_file","w");
				fp2 = fopen(dirp2->d_name,"r");// 원본 파일을 읽어 임시파일에 내용 복사
				memset(str_tmp,'\0',sizeof(str_tmp));
				while(fgets(str_tmp,sizeof(str_tmp),fp2)!=NULL){
					fputs(str_tmp,fp1);
				}
				fclose(fp2);
				fclose(fp1);

				memset(move_dir,'\0',sizeof(move_dir));
				strcpy(move_dir,rsyncDir);
				strcat(move_dir,"/");
				strcat(move_dir,dirp2->d_name);
				rename(dirp2->d_name,move_dir);
				utime(move_dir,&time_buf);

				rename("tmp_file",dirp2->d_name);
				utime(dirp2->d_name,&time_buf);

				fputs("\t",sync);
				fputs(dirp2->d_name,sync);
				fputs(" ",sync);
				sprintf(toStr,"%ldbytes\n",statSrc_sub.st_size);
				fputs(toStr,sync);
			}
			////////////////////////////////////////////////////////////////////////
		}
		closedir(dp2);

	}
	else{// src가 일반 파일인 경우 
//////////////////////////////////////////////////////////////////////////////////////	
////////////////////////////////////////////////////////////////////////////////
		if((dp = opendir(rsyncDir))==NULL){// dst 디렉토리 내에 존재하는 파일 검색
			fprintf(stderr,"opendir error for %s\n",dst);
			exit(1);
		}
		chdir(rsyncDir);
		isSame = false;
		isNot = false;
		while((dirp = readdir(dp))!=NULL){
			if(!strcmp(dirp->d_name,".") || !strcmp(dirp->d_name,"..")){
				continue;
			}
			if(stat(dirp->d_name,&statDst_sub)<0){
				fprintf(stderr,"stat error for %s\n",dirp->d_name);
				exit(1);
			}
			if(!strcmp(dirp->d_name, src_name)){//같은 이름의 파일이 파일 내에 존재할 경우
				if(statSrc.st_size == statDst_sub.st_size){//파일의 크기또한 같다면 동기화X
					if(statSrc.st_mtime == statDst_sub.st_mtime){
						isSame = true;
						if(!mOption){
							break;
						}
					}
					else{// 수정시간이 달라 동기화
						time_buf.modtime = tmpbuf.st_mtime;
						chdir(srcDir);
	
						fp1 = fopen("tmp_file","w");// 임시파일 생성, 내용을 복사할 것임
						fp2 = fopen(src_name,"r");// 원본 파일을 읽어 임시파일에 내용 복사
						while(fgets(str_tmp,sizeof(str_tmp),fp2)!=NULL){
							fputs(str_tmp,fp1);
						}
						fclose(fp2);
						fclose(fp1);
	
						chdir(rsyncDir);
						if(S_ISDIR(tmpbuf.st_mode)){
							rmdir(dirp->d_name);
						}
						else{
							remove(dirp->d_name);
						}

						strcpy(move_dir,rsyncDir);
						strcat(move_dir,"/");
						strcat(move_dir,dirp->d_name);
						
						chdir(srcDir);
						rename(src_name,move_dir);
						utime(move_dir,&time_buf);
	
						rename("tmp_file",src_name);
						utime(src_name,&time_buf);

						fputs("\t",sync);
						fputs(dirp->d_name,sync);
						fputs(" ",sync);
						sprintf(toStr,"%ldbytes\n",tmpbuf.st_size);
						fputs(toStr,sync);

						isNot = true;
						if(!mOption){
							break;
						}
					}
				}
				else{//파일의 크기가 다르다면 동기화O
					time_buf.modtime = tmpbuf.st_mtime;
					chdir(srcDir);

					fp1 = fopen("tmp_file","w");// 임시파일 생성, 내용을 복사할 것임
					fp2 = fopen(src_name,"r");// 원본 파일을 읽어 임시파일에 내용 복사
					while(fgets(str_tmp,sizeof(str_tmp),fp2)!=NULL){
						fputs(str_tmp,fp1);
					}
					fclose(fp2);
					fclose(fp1);

					chdir(rsyncDir);
					if(S_ISDIR(tmpbuf.st_mode)){
						rmdir(dirp->d_name);
					}
					else{
						remove(dirp->d_name);
					}

					strcpy(move_dir,rsyncDir);
					strcat(move_dir,"/");
					strcat(move_dir,dirp->d_name);
					
					chdir(srcDir);
					rename(src_name,move_dir);
					utime(move_dir,&time_buf);

					rename("tmp_file",src_name);
					utime(src_name,&time_buf);

					fputs("\t",sync);
					fputs(dirp->d_name,sync);
					fputs(" ",sync);
					sprintf(toStr,"%ldbytes\n",tmpbuf.st_size);
					fputs(toStr,sync);
					isNot = true;
					if(!mOption){
						break;
					}
				}
			}
			else{
				if(mOption){
					fputs("\t",sync);
					fputs(dirp->d_name,sync);
					fputs(" ",sync);
					sprintf(toStr,"delete\n");
					fputs(toStr,sync);

					if(S_ISDIR(statDst_sub.st_mode)){
						rmdir(dirp->d_name);
					}
					else{
						remove(dirp->d_name);
					}
				}
			}
		}
		closedir(dp);
		if(isSame || isNot){
			// pass	
		}
		else{
			time_buf.modtime = statDst_sub.st_mtime;
			chdir(srcDir);

			fp1 = fopen("tmp_file","w");
			fp2 = fopen(src_name,"r");// 원본 파일을 읽어 임시파일에 내용 복사
			while(fgets(str_tmp,sizeof(str_tmp),fp2)!=NULL){
				fputs(str_tmp,fp1);
			}
			fclose(fp2);
			fclose(fp1);
			strcpy(move_dir,rsyncDir);
			strcat(move_dir,"/");
			strcat(move_dir,src_name);

			rename(src_name,move_dir);
			utime(move_dir,&time_buf);

			rename("tmp_file",src_name);
			utime(src_name,&time_buf);

			fputs("\t",sync);
			fputs(src_name,sync);
			fputs(" ",sync);
			sprintf(toStr,"%ldbytes\n",tmpbuf.st_size);
			fputs(toStr,sync);
		}
	}
	
	chdir(programDir);
	fclose(sync);
	return;
}
void rOption_rsync(char dir[PATH_MAX], FILE* fp)
{
	DIR *dp, *dp2;
	struct dirent *dirp, *dirp2;
	struct stat statbuf, statbuf2;
	struct utimbuf time_buf;
	char str_tmp[PATH_MAX];
	FILE *fp1, *fp2;
	char move_dir[PATH_MAX];
	char toStr[PATH_MAX];
	char *p1, *p2;
	int length;
	char tmpFile[PATH_MAX];
	char *del;
	int isPass=false;
	char check[PATH_MAX];
	
	strcat(cpy,"/");
	strcat(cpy,dir);
	strcat(cpy2,"/");
	strcat(cpy2,dir);


	if(isAllSync){// 디렉토리가 목적 디렉토리에 존재하지 않을 때, 새로운 디렉토리를 생성해 주며 동기화 작업을 진행하는 부분
		if((dp = opendir(dir))==NULL){
			fprintf(stderr,"opendir error for %s\n",dir);
			exit(1);
		}
		while((dirp = readdir(dp))!=NULL){
			if(!strcmp(dirp->d_name,".") || !strcmp(dirp->d_name,"..")){
				continue;
			}
			chdir(dir);
			if(stat(dirp->d_name,&statbuf)<0){
				fprintf(stderr,"stat error for %s\n",dirp->d_name);
				exit(1);
			}
			if(S_ISDIR(statbuf.st_mode)){
				strcat(rsyncDir,"/");		
				strcat(rsyncDir,dir);
				chdir(rsyncDir);
				mkdir(dirp->d_name,0755);
				
				strcat(srcDir,"/");
				strcat(srcDir,dir);
				chdir(srcDir);

				p1 = srcDir;
				p2 = strstr(p1,"/");
				del = p2;
				while((p2 = strstr(p2+1,"/"))!=NULL){
					del = p2;
				}
				strncpy(srcDir,p1,strlen(p1)-strlen(del));
				
				strcat(cat_str,dir);//
				strcat(cat_str,"/");

				rOption_rsync(dirp->d_name,fp);//////////////////

				chdir(srcDir);

				p1 = rsyncDir;
				p2 = strstr(rsyncDir,"/");
				del = p2;
				while((p2 = strstr(p2+1,"/"))!=NULL){
					del = p2;
				}
				strncpy(rsyncDir,p1,strlen(p1)-strlen(del));

				
				continue;
			}
				

			fp1 = fopen("tmp_file","w");
			fp2 = fopen(dirp->d_name,"r");// 원본 파일을 읽어 임시파일에 내용 복사
			while(fgets(str_tmp,sizeof(str_tmp),fp2)!=NULL){
				fputs(str_tmp,fp1);
			}
			fclose(fp2);
			fclose(fp1);
			strcpy(move_dir,rsyncDir);
			strcat(move_dir,"/");
			strcat(move_dir,dir);

			strcat(move_dir,"/");
			strcat(move_dir,dirp->d_name);

			rename(dirp->d_name,move_dir);
			utime(move_dir,&time_buf);

			rename("tmp_file",dirp->d_name);
			utime(dirp->d_name,&time_buf);


			fputs("\t",fp);
			if(cat_str == NULL){
				//pass
			}
			else{
				fputs(cat_str,fp);
			}
			fputs(dir,fp);
			fputs("/",fp);
			fputs(dirp->d_name,fp);
			fputs(" ",fp);
			sprintf(toStr,"%ldbytes\n",statbuf.st_size);
			fputs(toStr,fp);

		}
		closedir(dp);
	}
	else{//이미 만들어진 서브 디렉토리가 존재한다면////////////////////////////////
		if((dp = opendir(dir))==NULL){
			fprintf(stderr,"opendir error for %s\n",dir);
			exit(1);
		}
		while((dirp = readdir(dp))!=NULL){
			if(!strcmp(dirp->d_name,".") || !strcmp(dirp->d_name,"..")){
				continue;
			}
			chdir(dir);
			if(stat(dirp->d_name,&statbuf)<0){
				fprintf(stderr,"stat error for %s\n",dirp->d_name);
				exit(1);
			}
			if(S_ISDIR(statbuf.st_mode)){
				strcat(rsyncDir,"/");		
				strcat(rsyncDir,dir);
				chdir(rsyncDir);
				mkdir(dirp->d_name,0755);
				
				strcat(srcDir,"/");
				strcat(srcDir,dir);
				chdir(srcDir);
				p1 = srcDir;
				p2 = strstr(p1,"/");
				del = p2;
				while((p2 = strstr(p2+1,"/"))!=NULL){
					del = p2;
				}
				strncpy(srcDir,p1,strlen(p1)-strlen(del));
				
				rOption_rsync(dirp->d_name,fp);//////////////////

				chdir(srcDir);

				p1 = rsyncDir;
				p2 = strstr(rsyncDir,"/");
				del = p2;
				while((p2 = strstr(p2+1,"/"))!=NULL){
					del = p2;
				}
				strncpy(rsyncDir,p1,strlen(p1)-strlen(del));
				continue;
			}

			chdir(cpy);
			if((dp2 = opendir(cpy))==NULL){//hs
				fprintf(stderr,"opendir error for %s\n",rsyncDir);
			}
			while((dirp2 = readdir(dp2))!=NULL){
				if(!strcmp(dirp2->d_name,".") || !strcmp(dirp2->d_name,"..")){
					continue;
				}

				isPass = false;

				chdir(cpy);
				if(stat(dirp2->d_name,&statbuf2)<0){
					fprintf(stderr,"stat error for %s\n",dirp2->d_name);
					exit(1);
				}
				///////
				if(!strcmp(dirp->d_name, dirp2->d_name)){// 동일한 이름의 디렉토리 존재
					if(statbuf.st_size == statbuf2.st_size){// 동일한 파일이라면 동기화X
						if(statbuf.st_mtime == statbuf2.st_mtime){
							isPass = true;
							break;	
						}
						else{
							time_buf.modtime = statbuf.st_mtime;
							isPass = true;	
							chdir(cpy2);

							fp1 = fopen("tmp_file","w");// 임시파일 생성, 내용을 복사할 것임
							fp2 = fopen(dirp2->d_name,"r");// 원본 파일을 읽어 임시파일에 내용 복사
							memset(str_tmp,'\0',sizeof(str_tmp));
							while(fgets(str_tmp,sizeof(str_tmp),fp2)!=NULL){
								fputs(str_tmp,fp1);
							}
							fclose(fp2);
							fclose(fp1);
		
							chdir(rsyncDir);
							if(S_ISDIR(statbuf2.st_mode)){
								rmdir(dirp2->d_name);
							}
							else{
								remove(dirp2->d_name);
							}
							
							memset(move_dir,'\0',sizeof(move_dir));
							strcpy(move_dir,cpy);
							strcat(move_dir,"/");
							strcat(move_dir,dirp2->d_name);
							utime(move_dir,&time_buf);

							chdir(cpy2);
							p1 = cpy2;
							p2 = real;//hs
							length = strlen(p2);
							p1 += length;
							strncpy(tmpFile,p1,strlen(p1));

							rename(dirp2->d_name,move_dir);

							rename("tmp_file",dirp2->d_name);
							utime(dirp2->d_name,&time_buf);

							fputs("\t",fp);
							fputs(tmpFile,fp);
							fputs("/",fp);
							fputs(dirp2->d_name,fp);
							fputs(" ",fp);
							sprintf(toStr,"%ldbytes\n",statbuf.st_size);
							fputs(toStr,fp);

							break;
									
						}
					}
					else{// 다른 파일이라면 동기화O
						time_buf.modtime = statbuf.st_mtime;
						isPass = true;	
						chdir(cpy2);
						p1 = cpy2;
						p2 = real;//hs
						length = strlen(p2);
						p1 += length;
						strncpy(tmpFile,p1,strlen(p1));

						fp1 = fopen("tmp_file","w");// 임시파일 생성, 내용을 복사할 것임
						fp2 = fopen(dirp2->d_name,"r");// 원본 파일을 읽어 임시파일에 내용 복사
						memset(str_tmp,'\0',sizeof(str_tmp));
						while(fgets(str_tmp,sizeof(str_tmp),fp2)!=NULL){
							fputs(str_tmp,fp1);
						}
						fclose(fp2);
						fclose(fp1);
		
						chdir(cpy);
						if(S_ISDIR(statbuf2.st_mode)){
							rmdir(dirp2->d_name);
						}
						else{
							remove(dirp2->d_name);
						}
	
						memset(move_dir,'\0',sizeof(move_dir));
						strcpy(move_dir,cpy);
						strcat(move_dir,"/");
						strcat(move_dir,dirp2->d_name);
				
						chdir(cpy2);
						rename(dirp2->d_name,move_dir);
						utime(move_dir,&time_buf);
						rename("tmp_file",dirp2->d_name);
						utime(dirp2->d_name,&time_buf);
						
						fputs("\t",fp);
						fputs(tmpFile,fp);
						fputs("/",fp);
						fputs(dirp2->d_name,fp);
						fputs(" ",fp);
						sprintf(toStr,"%ldbytes\n",statbuf.st_size);
						fputs(toStr,fp);
						break;
					}
				}
			}
			closedir(dp2);
			if(isPass){
				//pass
			}
			else{
				time_buf.modtime = statbuf.st_mtime;
				chdir(cpy2);
				p1 = cpy2;
				p2 = real;//hs
				length = strlen(p2);
				p1 += length;
				strncpy(tmpFile,p1,strlen(p1));

				fp1 = fopen("tmp_file","w");
				fp2 = fopen(dirp->d_name,"r");// 원본 파일을 읽어 임시파일에 내용 복사
				memset(str_tmp,'\0',sizeof(str_tmp));
				while(fgets(str_tmp,sizeof(str_tmp),fp2)!=NULL){
					fputs(str_tmp,fp1);
				}
				fclose(fp2);
				fclose(fp1);

				memset(move_dir,'\0',sizeof(move_dir));
				strcpy(move_dir,cpy);
				strcat(move_dir,"/");
				strcat(move_dir,dirp->d_name);
				rename(dirp->d_name,move_dir);
				utime(move_dir,&time_buf);

				rename("tmp_file",dirp->d_name);
				utime(dirp->d_name,&time_buf);

				fputs("\t",fp);
				fputs(tmpFile,fp);
				fputs("/",fp);
				fputs(dirp->d_name,fp);
				fputs(" ",fp);
				sprintf(toStr,"%ldbytes\n",statbuf.st_size);
				fputs(toStr,fp);
			}
			////////////////////////////////////////////////////////////////////////
			chdir(cpy2);
		}
		closedir(dp);
	}
	

	return;
}
