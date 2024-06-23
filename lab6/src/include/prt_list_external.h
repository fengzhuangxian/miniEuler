#ifndef PRT_LIST_EXTERNAL_H
#define PRT_LIST_EXTERNAL_H

#include "prt_typedef.h"
#include "list_types.h"

#define LIST_OBJECT_INIT(object) { \
        &(object), &(object)       \
    }

#define INIT_LIST_OBJECT(object)   \
    do {                           \
        (object)->next = (object); \
        (object)->prev = (object); \
    } while (0)

#define LIST_LAST(object) ((object)->prev)
#define LIST_FIRST(object) ((object)->next)
#define OS_LIST_FIRST(object) ((object)->next)

/* list action low level add */
OS_SEC_ALW_INLINE INLINE void ListLowLevelAdd(struct TagListObject *newNode, struct TagListObject *prev,
                                            struct TagListObject *next)
{
    newNode->next = next;
    newNode->prev = prev;
    next->prev = newNode;
    prev->next = newNode;
}

/* list action add */
OS_SEC_ALW_INLINE INLINE void ListAdd(struct TagListObject *newNode, struct TagListObject *listObject)
{
    ListLowLevelAdd(newNode, listObject, listObject->next);
}

/* list action tail add */
OS_SEC_ALW_INLINE INLINE void ListTailAdd(struct TagListObject *newNode, struct TagListObject *listObject)
{
    ListLowLevelAdd(newNode, listObject->prev, listObject);
}

/* list action lowel delete */
OS_SEC_ALW_INLINE INLINE void ListLowLevelDelete(struct TagListObject *prevNode, struct TagListObject *nextNode)
{
    nextNode->prev = prevNode;
    prevNode->next = nextNode;
}

/* list action delete */
OS_SEC_ALW_INLINE INLINE void ListDelete(struct TagListObject *node)
{
    ListLowLevelDelete(node->prev, node->next);

    node->next = NULL;
    node->prev = NULL;
}

/* list action empty */
OS_SEC_ALW_INLINE INLINE bool ListEmpty(const struct TagListObject *listObject)
{
    return (bool)((listObject->next == listObject) && (listObject->prev == listObject));
}

#define OFFSET_OF_FIELD(type, field) ((uintptr_t)((uintptr_t)(&((type *)0x10)->field) - (uintptr_t)0x10))

#define COMPLEX_OF(ptr, type, field) ((type *)((uintptr_t)(ptr) - OFFSET_OF_FIELD(type, field)))

/* 根据成员地址得到控制块首地址, ptr成员地址, type控制块结构, field成员名 */
#define LIST_COMPONENT(ptrOfList, typeOfList, fieldOfList) COMPLEX_OF(ptrOfList, typeOfList, fieldOfList)

#define LIST_FOR_EACH(posOfList, listObject, typeOfList, field)                                                    \
    for ((posOfList) = LIST_COMPONENT((listObject)->next, typeOfList, field); &(posOfList)->field != (listObject); \
        (posOfList) = LIST_COMPONENT((posOfList)->field.next, typeOfList, field))

#define LIST_FOR_EACH_SAFE(posOfList, listObject, typeOfList, field)                \
    for ((posOfList) = LIST_COMPONENT((listObject)->next, typeOfList, field);       \
        (&(posOfList)->field != (listObject))&&((posOfList)->field.next != NULL);  \
        (posOfList) = LIST_COMPONENT((posOfList)->field.next, typeOfList, field))

#endif /* PRT_LIST_EXTERNAL_H */