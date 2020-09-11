#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/time.h>
#include"ssu_crond.h"

#define SECOND_TO_MICRO 1000000

//ssu_crond()함수가 실행된 시간을 측정해주는 함수, 프로그램이 종료되어도 디몬프로세스가 백그라운드로 파일들을 체크해줌
void ssu_runtime(struct timeval *begin_t, struct timeval *end_t);

int main(void)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t,NULL);

	ssu_crond();

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);

	exit(0);
}
void ssu_runtime(struct timeval *begin_t, struct timeval *end_t){
	end_t->tv_sec -= begin_t->tv_sec;

	if(end_t->tv_usec < begin_t->tv_usec){
		end_t->tv_sec--;
		end_t->tv_usec += SECOND_TO_MICRO;
	}

	end_t->tv_usec -= begin_t->tv_usec;
	printf("Runtime: %ld:%06ld(sec:usec)\n",end_t->tv_sec,end_t->tv_usec);
	return;
}
