#ifndef _LIST_TYPES_H
#define _LIST_TYPES_H

struct TagListObject {
    struct TagListObject *prev;
    struct TagListObject *next;
};

#endif  /* end _LIST_TYPES_H */