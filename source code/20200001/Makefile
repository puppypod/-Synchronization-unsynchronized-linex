all : ssu_crontab ssu_crond ssu_rsync
ssu_crontab : main_crontab.o ssu_crontab.o
	gcc main_crontab.o ssu_crontab.o -o ssu_crontab

main_crontab.o : main_crontab.c ssu_crontab.h
	gcc -c main_crontab.c

ssu_crontab.o : ssu_crontab.c ssu_crontab.h
	gcc -c ssu_crontab.c

ssu_crond : main_crond.o ssu_crond.o
	gcc main_crond.o ssu_crond.o -o ssu_crond -lpthread

main_crond.o : main_crond.c ssu_crond.h
	gcc -c main_crond.c

ssu_crond.o : ssu_crond.c ssu_crond.h
	gcc -c ssu_crond.c

ssu_rsync : main_rsync.o ssu_rsync.o
	gcc main_rsync.o ssu_rsync.o -o ssu_rsync -lpthread

main_rsync.o : main_rsync.c ssu_rsync.h
	gcc -c main_rsync.c

ssu_rsync.o : ssu_rsync.c ssu_rsync.h
	gcc -c ssu_rsync.c


clean : 
	rm *.o
	rm ssu_crontab
	rm ssu_crond
	rm ssu_rsync
