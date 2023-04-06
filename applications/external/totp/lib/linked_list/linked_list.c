#include "linked_list.h"

ListNode* list_init_head(void* data) {
    ListNode* new = malloc(sizeof(ListNode));
    if(new == NULL) return NULL;
    new->data = data;
    new->next = NULL;
    return new;
}

ListNode* list_add(ListNode* head, void* data) {
    ListNode* new = malloc(sizeof(ListNode));
    if(new == NULL) return NULL;
    new->data = data;
    new->next = NULL;

    if(head == NULL)
        head = new;
    else {
        ListNode* it;

        for(it = head; it->next != NULL; it = it->next)
            ;

        it->next = new;
    }

    return head;
}

ListNode* list_find(ListNode* head, const void* data) {
    ListNode* it = NULL;

    for(it = head; it != NULL; it = it->next)
        if(it->data == data) break;

    return it;
}

ListNode* list_element_at(ListNode* head, uint16_t index) {
    ListNode* it;
    uint16_t i;
    for(it = head, i = 0; it != NULL && i < index; it = it->next, i++)
        ;
    return it;
}

ListNode* list_remove(ListNode* head, ListNode* ep) {
    if(head == NULL) {
        return NULL;
    }

    if(head == ep) {
        ListNode* new_head = head->next;
        free(head);
        return new_head;
    }

    ListNode* it;

    for(it = head; it->next != ep; it = it->next)
        ;

    it->next = ep->next;
    free(ep);

    return head;
}

ListNode* list_remove_at(ListNode* head, uint16_t index, void** removed_node_data) {
    if(head == NULL) {
        return NULL;
    }

    ListNode* it;
    ListNode* prev = NULL;

    uint16_t i;

    for(it = head, i = 0; it != NULL && i < index; prev = it, it = it->next, i++)
        ;

    if(it == NULL) return head;

    ListNode* new_head = head;
    if(prev == NULL) {
        new_head = it->next;
    } else {
        prev->next = it->next;
    }

    if(removed_node_data != NULL) {
        *removed_node_data = it->data;
    }

    free(it);

    return new_head;
}

ListNode* list_insert_at(ListNode* head, uint16_t index, void* data) {
    if(index == 0 || head == NULL) {
        ListNode* new_head = list_init_head(data);
        if(new_head != NULL) {
            new_head->next = head;
        }
        return new_head;
    }

    ListNode* it;
    ListNode* prev = NULL;

    uint16_t i;

    for(it = head, i = 0; it != NULL && i < index; prev = it, it = it->next, i++)
        ;

    ListNode* new = malloc(sizeof(ListNode));
    if(new == NULL) return NULL;
    new->data = data;
    new->next = it;
    prev->next = new;

    return head;
}

void list_free(ListNode* head) {
    ListNode* it = head;
    ListNode* tmp;

    while(it != NULL) {
        tmp = it;
        it = it->next;
        free(tmp);
    }
}
