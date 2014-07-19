#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#define LINE 4096

void get_tty_size(struct winsize *size);
int check_is_a_file(char *pathfile);
void do_more(char *pathfile);
int see_more(char *filename);


const char *help_string[] = {
	"Usage: more [options] file...\n",
	"Options:",
	"-d        display help instead of ring bell",
	"-f        count logical, rather thann screen lines",
	"-l        suppess pause after form feed",
	"-p        suppress scroll, clean screen and display text",
	"-c        suppress scroll, display text and clean line ends",
	"-u        suppress underlining",
	"-s        squeeze multiple blank of lines per screenful",
	"-Num      specify the number of lines per screenful",
	"+Num      display file beginning from line number NUM",
	"+/STRING  display file beginning from search string match",
	"-V        output version information and exit",
	NULL
};

struct winsize ttysize;
unsigned int rows;
unsigned int cols;



int
main(int argc, char **argv)
{
    const char **ptr_string;

	if (argc < 2) {
        ptr_string = help_string;
		while (*ptr_string != NULL) {
			printf("%s\n", *ptr_string);
            ptr_string++;
		}
		exit(EXIT_FAILURE);
	}

	while (*++argv != NULL) {
		do_more(*argv);
	}

	exit(EXIT_SUCCESS);
}

void
do_more(char *pathfile)
{
	unsigned int rows, cols;
	char buf[LINE];
	FILE *fptr;
    int fullscreen = 0;
    int read_size;
    int reply;

	if (check_is_a_file(pathfile) == -1) {
		printf("%s is not a file.\n", pathfile);
		exit(EXIT_FAILURE);
	}

	get_tty_size(&ttysize);
	rows = ttysize.ws_row;
	cols = ttysize.ws_col;
	memset(buf, '\0', (size_t)sizeof(buf));
#ifdef TEST
	printf("tty rows = %d, tty cols = %d\n",
				ttysize.ws_row, ttysize.ws_col);
#endif

	fptr = fopen(pathfile, "r");
	if (fptr == NULL) {
		perror("Cannot open file");
		exit(EXIT_FAILURE);
	}

	while(fgets(buf, cols, fptr) != NULL) {
		if (fullscreen == rows) {
            reply = see_more(pathfile);
			fullscreen -= reply;
		}
		fputs(buf, stdout);
		fullscreen++;
	}
    fclose(fptr);
}

//get the rows' and cols' sizes of tty window
void
get_tty_size(struct winsize *size)
{
	if (isatty(STDOUT_FILENO) == 0) {
		perror("Not a terminal");
		exit(EXIT_FAILURE);
	}

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, (char *)size) == -1) {
		perror("Can't get the terminal's size");
		exit(EXIT_FAILURE);
	}
}

//make sure that the ARG is a regular file
int
check_is_a_file(char *pathfile)
{
	struct stat statbuf;
	if (stat(pathfile, &statbuf) == -1) {
		perror("Can not get file's information");
		exit(EXIT_FAILURE);
	}

	return (S_ISREG(statbuf.st_mode));
}

//when full screen, what do?
int
see_more(char *filename)
{
	int ch;

    while ((ch = getchar()) != EOF) {
        fflush(stdin);
        fflush(stdout);
	    switch (ch) {
            case '\n' || 'j':
                return 1;
            case ' ':
                return rows;
            case 'q':
                exit(EXIT_SUCCESS);
            default:
                continue;
        }
    }
}
