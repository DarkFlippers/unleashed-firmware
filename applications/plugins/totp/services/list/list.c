#include "list.h"

ListNode *list_init_head(void* data) {
    ListNode *new = (ListNode *) malloc(sizeof(ListNode));
    new->data = data;
    new->next = NULL;
    return new;
}

ListNode *list_add(ListNode *head, void* data) {
    ListNode *new = (ListNode *) malloc(sizeof(ListNode));
    new->data = data;
    new->next = NULL;

    if (head == NULL)
        head = new;
    else {
        ListNode *it;

        for (it = head; it->next != NULL; it = it->next)
            ;

        it->next = new;
    }

    return head;
}

ListNode *list_find(ListNode *head, void* data) {
    ListNode *it;

    for (it = head; it != NULL; it = it->next)
        if (it->data == data)
            break;

    return it;
}

ListNode *list_element_at(ListNode *head, uint16_t index) {
    ListNode *it;
    uint16_t i;
    for (it = head, i = 0; it != NULL && i < index; it = it->next, i++);
    return it;
}

ListNode *list_remove(ListNode *head, ListNode *ep) {
    if (head == ep) {
        ListNode *new_head = head->next;
        free(head);
        return new_head;
    }

    ListNode *it;

    for (it = head; it->next != ep; it = it->next)
        ;

    it->next = ep->next;
    free(ep);

    return head;
}

void list_free(ListNode *head) {
    ListNode *it = head, *tmp;

    while (it != NULL) {
        tmp = it;
        it = it->next;
        free(tmp);
    }
}
