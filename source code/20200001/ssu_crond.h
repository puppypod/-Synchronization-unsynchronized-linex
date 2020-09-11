#ifndef CROND_H
#define CROND_H


#ifndef true
	#define true 1
#endif
#ifndef false
	#define false 0
#endif
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif
#ifndef COMMAND_MAX
	#define COMMAND_MAX 512
#endif
#ifndef BUF_MAX
	#define BUF_MAX 512
#endif
#ifndef BUFLEN
	#define BUFLEN 64
#endif
#ifndef L_MAX
	#define L_MAX 256
#endif
#ifndef CYCLE_LEN
	#define CYCLE_LEN 32
#endif
#ifndef ADD
	#define ADD 0
#endif
#ifndef REMOVE
	#define REMOVE 1
#endif
#ifndef	EXIT 
	#define EXIT 2
#endif

void ssu_crond();// 디몬 프로그램을 실행 시켜주는 함수
int exec_crond();// 실질적으로 ssu_crond()함수를 실행시켜 주는 함수
char *compare_command_remove(int prev, int pres);// 어떤 명령어가 삭제되었는지 check하여 ssu_crontab_log에 기록을 남겨주는 함수

#endif
