#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <utmp.h>
#include <time.h>


//#define UTMP_FILE "/var/run/utmp"
#define UTMP_SIZE sizeof(struct utmp)

void format_print(const struct utmp *tmpfile);

int
main(int argc, char **argv)
{
	struct utmp utmp_file;
	int fd;
	
	fd = open(UTMP_FILE, O_RDONLY);
	if (fd == -1) {
		printf("Open file error.\n");
		exit(-1);
	}
	
	while (read(fd, &utmp_file, UTMP_SIZE) == UTMP_SIZE) {
		if (utmp_file.ut_type == USER_PROCESS) 
			format_print(&utmp_file);
	}
	close(fd);
	exit(0);
}

void
format_print(const struct utmp *tmpfile)
{
	struct tm *ptr;
	time_t usertime;

	usertime = (time_t)tmpfile->ut_tv.tv_sec;
	ptr = localtime(&usertime);

	printf("%s    ", tmpfile->ut_user);
	printf("%-8s    ", tmpfile->ut_line);
	printf("%d-%d-%d %d:%d ", ptr->tm_year + 1900, ptr->tm_mon + 1, 
				ptr->tm_mday, ptr->tm_hour, ptr->tm_min);
	
	printf("\n");
}
