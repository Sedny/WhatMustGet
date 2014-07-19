#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <errno.h>




int
main(int argc, char **argv)
{
    struct passwd *user_info;
    uid_t userid;

    userid = geteuid();
    if ((user_info = getpwuid(userid)) == NULL) {
        perror("Can not get user's information");
        exit(EXIT_FAILURE);
    }

    printf("%s\n", user_info->pw_name);

    exit(EXIT_SUCCESS);
}
