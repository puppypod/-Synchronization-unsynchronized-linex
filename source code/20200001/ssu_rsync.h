#ifndef RSYNC_H
#define RSYNC_H


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

void ssu_rsync(int argc, char *argv[]);// 입력 인자의 에러를 잡아주며, 접근 권한을 check해 주는 함수
void exec_rsync();// 동기화 작업을 위해 src,dst의 절대경로, 상대경로를 check하여 프로그램을 실행하기에 알맞도록 변수 값의 호환을 유지시켜줌

void option_rsync();// 동기화 작업을 실행하며, 옵션에 따라 다르게 동작함
void rOption_rsync(char dir[PATH_MAX], FILE *fp);// r옵션이 실행되면 서브 디렉토리 또한 동기화 시켜줘야하므로 함수를 만들어 재귀적으로 동작해 서브 디렉토리 동기화 작업을 수행함

#endif
