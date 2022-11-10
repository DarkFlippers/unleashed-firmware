#pragma once

#include <stdlib.h>
#include <inttypes.h>

typedef struct ListNode {
    void* data;
    struct ListNode* next;
} ListNode;

ListNode* list_init_head(void* data);
ListNode* list_add(
    ListNode* head,
    void* data); /* adds element with specified data to the end of the list and returns new head node. */
ListNode* list_find(
    ListNode* head,
    const void* data); /* returns pointer of element with specified data in list. */
ListNode* list_element_at(
    ListNode* head,
    uint16_t index); /* returns pointer of element with specified index in list. */
ListNode* list_remove(
    ListNode* head,
    ListNode* ep); /* removes element from the list and returns new head node. */
void list_free(ListNode* head); /* deletes all elements of the list. */

#define TOTP_LIST_INIT_OR_ADD(head, item, assert) \
    do {                                          \
        if(head == NULL) {                        \
            head = list_init_head(item);          \
            assert(head != NULL);                 \
        } else {                                  \
            assert(list_add(head, item) != NULL); \
        }                                         \
    } while(false)
