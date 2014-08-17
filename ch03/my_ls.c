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
int list_init(List *list, void (*destroy)(void *data));

/* if element is NULL, let the new element be header */
int list_ins_next(List *list, ListElmt *element, void *data);

int list_rem_next(List *list, ListElmt *element, void **data);
int list_destroy(List *list);
void data_destroy(void *data);
void read_dir(List *list, const char *dir);
void list_sort(List *list);
void parse_command(int argc, char **argv);/*}}}*/

#define list_size(list) ((list)->size)/*{{{*/
#define list_head(list) ((list)->head)
#define list_tail(list) ((list)->tail)
#define list_is_tail(element) ((element)->next == NULL ? 1 : 0)
#define list_is_head(list, element) ((element) == (list)->head ? 1 : 0)

#define list_data(element) ((list)->data)
#define list_next(element) ((element)->next)/*}}}*/


/* Initial the link list */
int
list_init(List *list, void (*destroy)(void *data))
{/*{{{*/
    if (list == NULL || destroy == NULL)
        return -1;

    list->size = 0;
    list->head = NULL;
    list->tail = NULL;
    list->destroy = destroy;

    return 0;
}/*}}}*/

int
list_ins_next(List *list, ListElmt *element, void *data)
{/*{{{*/
    ListElmt *new_element;

    if (list == NULL)
        return -1;

    if ((new_element = (ListElmt *)malloc(sizeof(ListElmt))) == NULL)
        return -1;

    new_element->data = (void *)data;

    if (element == NULL) {
        if (list_size(list) == 0)
            list->tail = new_element;

        new_element->next = list->head;
        list->head = new_element;
    } else {
        new_element->next = element->next;
        element->next = new_element;

        if (list_tail(list) == element)
            list->tail = new_element;
    }

    return 0;
}/*}}}*/

int
list_rem_next(List *list, ListElmt *element, void **data)
{/*{{{*/
    ListElmt *rm_element;

    if (list == NULL)
        return -1;

    if (element == NULL) {
        rm_element = list_head(list);
        list->head = list->head->next;

        if (list_size(list) == 1)
            list->tail = NULL;
    } else {
        if (list_tail(list) == element)
            return -1;

        rm_element = element->next;
        element->next = rm_element->next;

        if (element->next == NULL)
            list->tail = element;
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
        if (list_rem_next(list, NULL, (void **)&data) == 0 && list->destroy != NULL)
            list->destroy(data);
    }

    free(list);
}/*}}}*/

void
data_destroy(void *data)
{/*{{{*/
    free(data);
}/*}}}*/

void
read_dir(List *list, const char *dirname)
{/*{{{*/
    DIR *dirptr;
    struct dirent *direntptr;
    ListElmt *listelmt = NULL;
    char *data;


    if (dirname == NULL)
        exit(EXIT_FAILURE);

    if ((dirptr = opendir(dirname)) == NULL) {
        printf("open directory error\n");
        exit(EXIT_FAILURE);
    }

    while ((direntptr = readdir(dirptr)) != NULL) {
        if (strncmp(direntptr->d_name, ".", 1) == 0)
            continue;

        data = (char *)malloc((strlen(direntptr->d_name) + 1) * sizeof(char));
        if (data == NULL)
            exit(EXIT_FAILURE);

        strcpy(data, direntptr->d_name);
        list_ins_next(list, listelmt, (void *)data);
        listelmt = list_tail(list);
    }

    list_sort(list);
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
            if (element1 == list_head(list))
                list->head = element2;
            if (element2 == list_tail(list))
                list->tail = element1;

            /* 重新调整element1，使之指向排序的数据 */
            prev_e2 = element1;
            element1 = element2;
        }
        prev_e1 = element1;
        element1 = element1->next;
        prev_e2 = element1;
    }

}/*}}}*/

int
main(int argc, char **argv)
{/*{{{*/
    List *list;

    list = (List *)malloc(sizeof(List));
    if (list == NULL)
        exit(EXIT_FAILURE);

    if (list_init(list, data_destroy) == -1)
        exit(EXIT_FAILURE);


    if (argc == 1)
        read_dir(list, ".");
    else
        read_dir(list, argv[1]);


    exit(EXIT_SUCCESS);
}/*}}}*/



