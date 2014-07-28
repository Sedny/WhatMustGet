#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>


int check_is_dir(const char *filename);
void copy_to(char *src, char *des);

int
main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: cp SOURCE DEST");
        exit(EXIT_FAILURE);
    }

    copy_to(argv[1], argv[2]);

    exit(EXIT_SUCCESS);
}

int
check_is_dir(const char *filename)
{
    struct stat filestat;

    if (filename[strlen(filename) - 1] == '/')
        return 0;

    if (stat(filename, &filestat) == -1)
        return 1;
    else {
        if (S_ISREG(filestat.st_mode))
            return 1;
        else if(S_ISDIR(filestat.st_mode))
            return 0;
        else {
            printf("Usage: cp SOURCE DEST");
            exit(EXIT_FAILURE);
        }
    }
}

void
copy_to(char *src, char *des)
{
    int fd_src, fd_des;
    char buf[BUFSIZ];
    int read_bytes;
    int len;
    char *n_name;
    char *ptr;


    if ((fd_src = open(src, O_RDONLY)) == -1) {
        printf("Cannot open file %s: %s\n", src, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (check_is_dir(des) == 0) {
        len = strlen(des);

        ptr = strrchr(src, '/');
        if (ptr == NULL)
            ptr = src;
        else
          ptr++;

        n_name = (char *)malloc(sizeof(char) * (len + strlen(ptr) + 2));
        if (n_name == NULL) {
            printf("No memory more: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        strcat(n_name, des);
        if (des[len - 1] != '/')
            strcat(n_name, "/");
        strcat(n_name, ptr);
    } else {
        n_name = des;
    }

    if ((fd_des = open(n_name, O_RDWR | O_CREAT, 0644)) == -1) {
        printf("Cannot create file %s: %s", des, strerror(errno));
        exit(EXIT_FAILURE);
    }

    while ((read_bytes = read(fd_src, buf, BUFSIZ)) != 0) {
        if (write(fd_des, buf, read_bytes) == -1) {
            printf("write to file %s error: %s\n", des, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    free(n_name);
    close(fd_src);
    close(fd_des);
}
