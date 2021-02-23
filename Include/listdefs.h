/*
*
* Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
*
* The contents of this file are subject to the Mozilla Public License Version
* 2.0 (the "License"); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. SEe the License
* for the specific language governing rights and limitations under the
* License.
*
* The Initial Developer of the Original e is blindtiger.
*
*/

#ifndef _LISTDEFS_H_
#define _LISTDEFS_H_

#include <typesdefs.h>

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    typedef struct _list {
        struct _list * front;
        struct _list * back;
    } list;

#define __is_list_empty(head) \
            ((head)->front == (head))

    __inline
        void
        __empty_list(
            list * head
        )
    {
        head->front = head->back = head;
    }

    __inline
        u8
        __remove_node(
            list * node
        )
    {
        list * back;
        list * front;

        front = node->front;
        back = node->back;
        back->front = front;
        front->back = back;

        return (u8)(front == back);
    }

    __inline
        void
        __insert_head(
            list * head,
            list * node
        )
    {
        list * front;

        front = head->front;

        node->front = front;
        node->back = head;

        head->front = node;
        front->back = node;
    }

    __inline
        list *
        __remove_head(
            list * head
        )
    {
        list * front;
        list * node;

        node = head->front;
        front = node->front;
        head->front = front;
        front->back = head;

        return node;
    }

    __inline
        void
        __insert_tail(
            list * head,
            list * node
        )
    {
        list * back;

        back = head->back;

        node->front = head;
        node->back = back;

        back->front = node;
        head->back = node;
    }

    __inline
        list *
        __remove_tail(
            list * head
        )
    {
        list * back;
        list * node;

        node = head->back;
        back = node->back;
        head->back = back;
        back->front = head;

        return node;
    }

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_LISTDEFS_H_
