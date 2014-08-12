#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>


typedef struct _ListElmt {
    void *data;
    struct _ListElmt *next;
}ListElmt;

typedef struct _List {
    uintmax_t size;
    ListElmt *head;
    ListElmt *tail;

    void (*destroy)(void *data);
}List;


int list_init(List *list, void (*destroy)(void *data));

/* if element is NULL, let the new element be header */
int list_ins_next(List *list, ListElmt *element, void *data);

int list_rem_next(List *list, ListElmt *element, void **data);
int list_destroy(List *list);
void data_destroy(void *data);



void read_dir(const char *dir);


#define list_size(list) ((list)->size)
#define list_head(list) ((list)->head)
#define list_tail(list) ((list)->tail)
#define list_is_tail(element) ((element)->next == NULL ? 1 : 0)
#define list_is_head(list, element) ((element) == (list)->head ? 1 : 0)

#define list_data(element) ((list)->data)
#define list_next(element) ((element)->next)

/* Initial the link list */
int
list_init(List *list, void (*destroy)(void *data))
{
    if (list == NULL || destroy == NULL)
        return -1;

    list->size = 0;
    list->head = NULL;
    list->tail = NULL;
    list->destroy = destroy;

    return 0;
}

int
list_ins_next(List *list, ListElmt *element, void *data)
{
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
}

int
list_rem_next(List *list, ListElmt *element, void **data)
{
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
}

int
list_destroy(List *list)
{
    void *data;

    while (list_size(list) > 0) {
        if (list_rem_next(list, NULL, (void **)&data) == 0 && list->destroy != NULL)
            list->destroy(data);
    }
}

void
data_destroy(void *data)
{
    free(data);
}

int
main(int argc, char **argv)
{


}



