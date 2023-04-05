#pragma once

#include <stdlib.h>
#include <inttypes.h>

/**
 * @brief Single linked list node
 */
typedef struct ListNode {
    /**
     * @brief Pointer to the data assigned to the current list node
     */
    void* data;

    /**
     * @brief Pointer to the next list node
     */
    struct ListNode* next;
} ListNode;

/**
 * @brief Initializes a new list node head
 * @param data data to be assigned to the head list node
 * @return Head list node
 */
ListNode* list_init_head(void* data);

/**
 * @brief Adds new list node to the end of the list
 * @param head head list node
 * @param data data to be assigned to the newly added list node
 * @return Head list node
 */
ListNode* list_add(
    ListNode* head,
    void* data); /* adds element with specified data to the end of the list and returns new head node. */

/**
 * @brief Searches list node with the given assigned \p data in the list
 * @param head head list node
 * @param data data to be searched
 * @return List node containing \p data if there is such a node in the list; \c NULL otherwise
 */
ListNode* list_find(ListNode* head, const void* data);

/**
 * @brief Searches list node with the given \p index in the list
 * @param head head list node
 * @param index desired list node index
 * @return List node with the given \p index in the list if there is such a list node; \c NULL otherwise
 */
ListNode* list_element_at(ListNode* head, uint16_t index);

/**
 * @brief Removes list node from the list
 * @param head head list node
 * @param ep list node to be removed
 * @return Head list node
 */
ListNode* list_remove(ListNode* head, ListNode* ep);

/**
 * @brief Removes list node with the given \p index in the list from the list
 * @param head head list node
 * @param index index of the node to be removed
 * @param[out] removed_node_data data which was assigned to the removed list node
 * @return Head list node
 */
ListNode* list_remove_at(ListNode* head, uint16_t index, void** removed_node_data);

/**
 * @brief Inserts new list node at the given index
 * @param head head list node
 * @param index index in the list where the new list node should be inserted
 * @param data data to be assgned to the new list node
 * @return Head list node
 */
ListNode* list_insert_at(ListNode* head, uint16_t index, void* data);

/**
 * @brief Disposes all the list nodes in the list
 * @param head head list node
 */
void list_free(ListNode* head);

#define TOTP_LIST_INIT_OR_ADD(head, item, assert) \
    if(head == NULL) {                            \
        head = list_init_head(item);              \
        assert(head != NULL);                     \
    } else {                                      \
        assert(list_add(head, item) != NULL);     \
    }

#define TOTP_LIST_FOREACH(head, node, action) \
    ListNode* node = head;                    \
    while(node != NULL) {                     \
        action node = node->next;             \
    }
