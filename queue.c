#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *q = malloc(sizeof(struct list_head));
    if (!q)
        return NULL;
    INIT_LIST_HEAD(q);
    return q;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l || list_empty(l)) {
        free(l);
        return;
    }

    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, l, list) {
        q_release_element(entry);
    }
    free(l);
    return;
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;

    element_t *entry = malloc(sizeof(element_t));
    if (!entry)
        return false;
    size_t len = strlen(s) + 1;
    entry->value = malloc(len);
    if (!entry->value) {
        free(entry);
        return false;
    }

    memcpy(entry->value, s, len);
    list_add(&entry->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head || !s)
        return false;

    element_t *entry = malloc(sizeof(element_t));
    if (!entry)
        return false;
    size_t len = strlen(s) + 1;
    entry->value = malloc(len);
    if (!entry->value) {
        free(entry);
        return false;
    }

    memcpy(entry->value, s, len);
    list_add_tail(&entry->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *tmp = list_first_entry(head, element_t, list);
    list_del(head->next);
    if (sp && bufsize != 0) {
        strncpy(sp, tmp->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return tmp;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *tmp = list_last_entry(head, element_t, list);
    list_del(head->prev);
    if (sp && bufsize != 0) {
        strncpy(sp, tmp->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return tmp;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;

    int size = 0;
    struct list_head *node;
    list_for_each (node, head) {
        size++;
    }
    return size;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/

    if (!head || list_empty(head))
        return false;

    struct list_head *fast = head->next, *slow = head->next;
    while (fast != head && fast->next != head) {
        fast = fast->next->next;
        slow = slow->next;
    }
    list_del(slow);
    q_release_element(list_entry(slow, element_t, list));
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/

    if (!head)
        return false;

    element_t *cur, *next, *tmp;
    list_for_each_entry_safe (cur, next, head, list) {
        if ((&next->list != head) && (strcmp(cur->value, next->value) == 0)) {
            do {
                tmp = next;
                next = list_entry(next->list.next, element_t, list);
                q_release_element(tmp);
            } while ((&next->list != head) &&
                     (strcmp(cur->value, next->value) == 0));

            cur->list.prev->next = &next->list;
            next->list.prev = cur->list.prev;
            q_release_element(cur);
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/

    if (!head || list_empty(head))
        return;

    struct list_head *cur, *safe, *temp;
    for (cur = (head)->next, safe = cur->next;
         (cur != (head) && cur->next != (head)); cur = safe, safe = cur->next) {
        /* save safe->next to temp*/
        temp = safe->next;
        /* next link */
        cur->prev->next = safe;
        cur->next = safe->next;
        safe->next = cur;
        /* prev link */
        temp->prev = cur;
        safe->prev = cur->prev;
        cur->prev = safe;
        /* reset safe to temp */
        safe = temp;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *cur, *next, *temp;
    list_for_each_safe (cur, next, head) {
        temp = next;
        cur->next = cur->prev;
        cur->prev = temp;
    }
    temp = head->next;
    head->next = head->prev;
    head->prev = temp;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/

    if (!head || q_size(head) <= 1 || k <= 1 || q_size(head) < k)
        return;

    struct list_head *node = head->next;
    struct list_head *safe;
    for (int n = q_size(head); n >= k; n -= k) {
        /* use dummy head to store the temporary reverseK result */
        struct list_head dummy_head = {&(dummy_head), &(dummy_head)};

        for (int i = 0; i < k; i++) {
            safe = node->next;
            list_del(node);
            list_add(node, &dummy_head);
            node = safe;
        }
        list_splice_tail(&dummy_head, node);
    }
}

/**
 * merge_two_q() - Sort elements of two sub-queue into one queue in
 * ascending/descending order.
 * @head: header of queue after merged two queue
 * @left: header of left sub-queue
 * @right: header of right sub-queue
 * @descend: whether or not to sort in descending order
 *
 * This function is used to help q_sort().
 */
void merge_two_q(struct list_head *head,
                 struct list_head *left,
                 struct list_head *right,
                 bool descend)
{
    switch ((int) descend) {
    case 1:
        while (!list_empty(left) && !list_empty(right)) {
            if (strcmp(list_entry(left->next, element_t, list)->value,
                       list_entry(right->next, element_t, list)->value) > 0) {
                list_move_tail(left->next, head);
            } else {
                list_move_tail(right->next, head);
            }
        }
        break;
    case 0:
        while (!list_empty(left) && !list_empty(right)) {
            if (strcmp(list_entry(left->next, element_t, list)->value,
                       list_entry(right->next, element_t, list)->value) < 0) {
                list_move_tail(left->next, head);
            } else {
                list_move_tail(right->next, head);
            }
        }
        break;
    default:
        break;
    }

    /* link the remaining nodes */
    if (!list_empty(left)) {
        list_splice_tail_init(left, head);
    } else {
        list_splice_tail_init(right, head);
    }
    return;
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || q_size(head) < 2)
        return;

    struct list_head *fast = head, *slow = head;
    do {
        fast = fast->next->next;
        slow = slow->next;
    } while (fast != head && fast->next != head);

    LIST_HEAD(left);
    LIST_HEAD(right);
    list_splice_tail_init(head, &right);
    list_cut_position(&left, &right, slow);
    q_sort(&left, descend);
    q_sort(&right, descend);
    merge_two_q(head, &left, &right, descend);
    return;
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/

    if (!head || list_empty(head) || head->next == head->prev)
        return q_size(head);

    element_t *cur = list_entry(head->prev, element_t, list);
    element_t *safe = list_entry(cur->list.prev, element_t, list);
    char *mins = cur->value;

    while (&cur->list != head) {
        if (strcmp(cur->value, mins) > 0) {
            list_del(&cur->list);
            q_release_element(cur);
        } else {
            mins = cur->value;
        }
        cur = safe;
        safe = list_entry(cur->list.prev, element_t, list);
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/

    if (!head || q_size(head) <= 1)
        return q_size(head);

    element_t *cur = list_entry(head->prev, element_t, list);
    element_t *safe = list_entry(cur->list.prev, element_t, list);
    char *maxs = cur->value;

    while (&cur->list != head) {
        if (strcmp(cur->value, maxs) < 0) {
            list_del(&cur->list);
            q_release_element(cur);
        } else {
            maxs = cur->value;
        }
        cur = safe;
        safe = list_entry(cur->list.prev, element_t, list);
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/

    if (!head || list_empty(head))
        return 0;
    queue_contex_t *first = list_first_entry(head, queue_contex_t, chain),
                   *ctx = NULL;
    int sum = first->size;
    list_for_each_entry (ctx, head, chain) {
        if (ctx == first)
            continue;
        sum += ctx->size;
        list_splice_tail_init(ctx->q, first->q);
    }
    q_sort(first->q, descend);
    return sum;
}
