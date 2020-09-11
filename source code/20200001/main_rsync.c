#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/time.h>
#include"ssu_rsync.h"

#define SECOND_TO_MICRO 1000000

//ssu_rsync() 프로그램의 실행 시간을 측정하는 함수
void ssu_runtime(struct timeval *begin_t, struct timeval *end_t);

int main(int argc, char *argv[])
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t,NULL);

	ssu_rsync(argc, argv);

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
