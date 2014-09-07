#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <dirent.h>
#include <getopt.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>


typedef void (*destroy_t)(void *);
typedef struct _ListElmt {/*{{{*/
    void *data;
    struct _ListElmt *next;
}ListElmt;/*}}}*/

typedef struct _List {/*{{{*/
    uintmax_t size;
    ListElmt *head;
    ListElmt *tail;
    void (*destroy)(void *data);
}List;/*}}}*/

/*{{{*/
int list_init(List **list, destroy_t destroy);
/* if element is NULL, let the new element be header */
int list_ins_next(List *list, ListElmt *element, void *data);

int list_rem_next(List *list, ListElmt *element, void **data);
int list_destroy(List *list);
void data_destroy(void *data);
void read_dir(const char *dir);
void list_sort(List *list);
void parse_command(int argc, char **argv);/*}}}*/
void help_funtion(void);
void print_version(void);
void print_long_format(const char *filename);
void get_file_mode(const struct stat statbuf, char *file_mode);

#define list_size(list) ((list)->size)/*{{{*/
#define list_head(list) ((list)->head)
#define list_tail(list) ((list)->tail)
#define list_is_tail(element) ((element)->next == NULL ? 1 : 0)
#define list_is_head(list, element) ((element) == (list)->head ? 1 : 0)

#define list_data(element) ((list)->data)
#define list_next(element) ((element)->next)/*}}}*/

#define CURRENT_DIR "."
/* Global lable */
int show_all_file = 0;          /* option -a */
int unsort = 0;              /* option -U, do not sort the files */
int long_list_format = 0;       /* option -l */

char dir_name[BUFSIZ];
List *list;

/* Initial the link list */
int
list_init(List **list, destroy_t destroy)
{/*{{{*/
    if (destroy == NULL) {
        return -1;
    }

    *list = (List *)malloc(sizeof(List));
    if (list == NULL) {
        return -1;
    }

    (*list)->size = 0;
    (*list)->head = NULL;
    (*list)->tail = NULL;
    (*list)->destroy = destroy;

    return 0;
}/*}}}*/

int
list_ins_next(List *list, ListElmt *element, void *data)
{/*{{{*/
    ListElmt *new_element;

    if (list == NULL) {
        return -1;
    }

    if ((new_element = (ListElmt *)malloc(sizeof(ListElmt))) == NULL) {
        return -1;
    }

    new_element->data = (void *)data;

    if (element == NULL) {
        if (list_size(list) == 0) {
            list->tail = new_element;
        }

        new_element->next = list->head;
        list->head = new_element;
    } else {
        new_element->next = element->next;
        element->next = new_element;

        if (list_tail(list) == element) {
            list->tail = new_element;
        }
    }

    list->size++;
    return 0;
}/*}}}*/

int
list_rem_next(List *list, ListElmt *element, void **data)
{/*{{{*/
    ListElmt *rm_element;

    if (list == NULL) {
        return -1;
    }

    if (element == NULL) {
        rm_element = list_head(list);
        list->head = list->head->next;

        if (list_size(list) == 1) {
            list->tail = NULL;
        }
    } else {
        if (list_tail(list) == element) {
            return -1;
        }

        rm_element = element->next;
        element->next = rm_element->next;

        if (element->next == NULL) {
            list->tail = element;
        }
    }

    *data = rm_element->data;
    free(rm_element);
    list->size--;

    return 0;
}/*}}}*/

int
list_destroy(List *list)
{/*{{{*/
    void *data;

    while (list_size(list) > 0) {
        if (list_rem_next(list, NULL, (void **)&data) == 0 && list->destroy != NULL) {
            list->destroy(data);
        }
    }

    free(list);

    return 0;
}/*}}}*/

void
data_destroy(void *data)
{/*{{{*/
    free(data);
}/*}}}*/

void
read_dir(const char *dirname)
{/*{{{*/
    DIR *dirptr;
    struct dirent *direntptr;
    ListElmt *listelmt = NULL;
    char *data;


    if (dirname == NULL) {
        exit(EXIT_FAILURE);
    }

    /* Initialize link list */
    if (list_init(&list, data_destroy) == -1) {
        exit(EXIT_FAILURE);
    }

    if ((dirptr = opendir(dirname)) == NULL) {
        printf("open directory error\n");
        exit(EXIT_FAILURE);
    }

    while ((direntptr = readdir(dirptr)) != NULL) {
        if (show_all_file == 0) {
            if (strncmp(direntptr->d_name, ".", 1) == 0) {
                continue;
            }
        }

        data = (char *)malloc((strlen(direntptr->d_name) + 1) * sizeof(char));
        if (data == NULL) {
            exit(EXIT_FAILURE);
        }

        strcpy(data, direntptr->d_name);
        list_ins_next(list, listelmt, (void *)data);
        listelmt = list_tail(list);
    }

    if (unsort == 0) {
        list_sort(list);
    }

    if (long_list_format == 1) {
        listelmt = list_head(list);
        while (listelmt != NULL) {
            print_long_format(listelmt->data);
            listelmt = listelmt->next;
        }
    }
#ifdef DEBUG
    int i = 0;
    listelmt = list_head(list);
    while (listelmt != NULL) {
        printf("%d: %s\n", i++, (char *)listelmt->data);
        listelmt = listelmt->next;
    }
#endif
    list_destroy(list);
}/*}}}*/

void
list_sort(List *list)
{/*{{{*/
    ListElmt *element1, *element2;
    ListElmt *prev_e1;      /* 指向element1的前一结点 */
    ListElmt *prev_e2;      /* 指向element2的前一结点 */
    ListElmt *tmp;

    element1 = list_head(list);
    prev_e2 = element1;
    prev_e1 = NULL;

    while (element1 != NULL) {
        while ((element2 = prev_e2->next) != NULL) {
            if (strcasecmp((char *)element1->data, (char *)element2->data) < 0) {
                prev_e2 = element2;
                continue;
            }

            if (prev_e1 == NULL) {
                /* e1为head结点时所做交换 */
                if (prev_e2 == element1) {
                    /* e1->next为e2时所做交换方法 */
                    element1->next = element2->next;
                    element2->next = element1;
                } else {
                    /* e1->next不是e2时所做交换 */
                    tmp = element1->next;
                    element1->next = element2->next;
                    element2->next = tmp;
                    prev_e2->next = element1;
                }
            } else {
                if (prev_e2 == element1) {
                    element1->next = element2->next;
                    prev_e1->next = element2;
                    element2->next = element1;
                } else {
                    tmp = element1->next;
                    element1->next = element2->next;
                    prev_e2->next = element1;
                    prev_e1->next = element2;
                    element2->next = tmp;
                }
            }

            /*  若交换双方为head或tail结点，重置head/tail */
            if (element1 == list_head(list)) {
                list->head = element2;
            }
            if (element2 == list_tail(list)) {
                list->tail = element1;
            }

            /* 重新调整element1，使之指向排序的数据 */
            prev_e2 = element1;
            element1 = element2;
        }
        prev_e1 = element1;
        element1 = element1->next;
        prev_e2 = element1;
    }

}/*}}}*/

void
help_funtion(void)
{/*{{{*/

}/*}}}*/

void
print_version(void)
{/*{{{*/

}/*}}}*/

void
parse_command(int argc, char **argv)
{/*{{{*/
    const char *optstrings = "afUl";
    const struct option longopts[] = {
        {"all", no_argument, NULL, 'a'},
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {NULL, 0, NULL, 0}
    };

    int longindex;
    int optarg;
    int index;

    while ((optarg = getopt_long(argc, argv, optstrings,
                        longopts,&longindex)) != -1) {
        switch (optarg) {
            case 'h':
                help_funtion();
                exit(EXIT_SUCCESS);
            case 'v':
                print_version();
                exit(EXIT_SUCCESS);
            case '?':
                printf("Try 'ls --help' for more information.\n");
                exit(EXIT_FAILURE);
            case 'a':
                show_all_file = 1;
                break;
            case 'l':
                long_list_format = 1;
                break;
            case 'U':
                unsort = 1;
                break;
            case 'f':
                show_all_file = 1;
                unsort = 1;
                break;
        }
    }

    if (optind == argc) {
        read_dir(CURRENT_DIR);
    } else {
        for (index = optind; index < argc; index++) {
            read_dir(argv[index]);
        }
    }
}/*}}}*/

void
print_long_format(const char *filename)
{/*{{{*/
    struct stat statbuf;
    char file_mode[11] = "----------";
    char file_date[25];
    struct passwd *pwptr;
    struct group *grptr;


    if (lstat(filename, &statbuf) == -1) {
        printf("Can not get %s's status\n", filename);
        exit(EXIT_FAILURE);
    }

    pwptr = getpwuid(statbuf.st_uid);
    grptr = getgrgid(statbuf.st_gid);
    get_file_mode(statbuf, file_mode);
    strftime(file_date, 25, "%b %e %H:%M", localtime(&statbuf.st_mtime));

    printf("%s ", file_mode);
    printf("%d ", (int)statbuf.st_nlink);
    printf("%s ", pwptr->pw_name);
    printf("%s ", grptr->gr_name);
    printf("%5d ", (int)statbuf.st_size);
    printf("%s ", file_date);
    printf("%s ", filename);
    printf("\n");
}/*}}}*/

void
get_file_mode(const struct stat statbuf, char *file_mode)
{/*{{{*/
    mode_t mode = statbuf.st_mode;

    if (S_ISDIR(mode)) {
        file_mode[0] = 'd';
    } else if (S_ISCHR(mode)) {
        file_mode[0] = 'c';
    } else if (S_ISBLK(mode)) {
        file_mode[0] = 'b';
    } else if (S_ISFIFO(mode)) {
        file_mode[0] = 'p';
    } else if (S_ISLNK(mode)) {
        file_mode[0] = 'l';
    } else if (S_ISSOCK(mode)) {
        file_mode[0] = 's';
    }

    /* owner's permission */
    if ((mode & S_IRUSR) != 0) {
        file_mode[1] = 'r';
    }
    if ((mode & S_IWUSR) != 0) {
        file_mode[2] = 'w';
    }
    if ((mode & S_IXUSR) != 0) {
        if ((mode & S_ISUID) != 0) {
            file_mode[3] = 's';
        } else {
            file_mode[3] = 'x';
        }
    } else if ((mode & S_ISUID) != 0) {
            file_mode[3] = 'S';
    }

    /* groups' permission */
    if ((mode & S_IRGRP) != 0) {
        file_mode[4] = 'r';
    }
    if ((mode & S_IWGRP) != 0) {
        file_mode[5] = 'w';
    }
    if ((mode & S_IXGRP) != 0) {
        if ((mode & S_ISGID) != 0) {
            file_mode[6] = 's';
        } else {
            file_mode[6] = 'x';
        }
    } else {
        if ((mode & S_ISGID) != 0) {
            file_mode[6] = 'S';
        }
    }

    /* Others' permission */
    if ((mode & S_IROTH) != 0) {
        file_mode[7] = 'r';
    }
    if ((mode & S_IWOTH) != 0) {
        file_mode[8] = 'w';
    }
    if ((mode & S_IXOTH) != 0) {
        if ((mode & S_ISVTX) != 0) {
            file_mode[9] = 't';
        } else {
            file_mode[9] = 'x';
        }
    } else {
        if ((mode & S_ISVTX) != 0) {
            file_mode[9] = 'T';
        }
    }
}/*}}}*/

int
main(int argc, char **argv)
{/*{{{*/

    /* read the command line option */
    parse_command(argc, argv);

    exit(EXIT_SUCCESS);
}/*}}}*/



