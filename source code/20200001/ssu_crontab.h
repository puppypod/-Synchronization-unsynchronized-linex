#ifndef CRONTAB_H
#define CRONTAB_H

#ifndef true
	#define true 1
#endif
#ifndef false
	#define false 0
#endif
#ifndef PATH_MAX
	#define PATH_MAX 1024
#endif
#ifndef BUFLEN
	#define BUFLEN 64
#endif
#ifndef L_MAX
	#define L_MAX 128
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

void ssu_crontab();// ssu_crontab_file을 읽어 명령 주기를 check, 시간 주기 입력이 잘못 되었다면 다시 입력 받을 수 있도록 해줌
void command_add();// add 명령어를 수행한 후 ssu_crontab_file에 저장해줌
void command_remove();// remove 명령어를 수행해주는 함수

#endif
