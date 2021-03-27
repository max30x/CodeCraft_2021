#define lnode(type) \
struct{ \
    type* next; \
    type* prev; \
}

#define lnodes(type) \
struct{ \
    type* head; \
    type* tail; \
}

#define lnodes_init(lnodes) \
do{ \
    (lnodes)->head=nullptr; \
    (lnodes)->tail=nullptr; \
}while (0)

#define lnode_init(lnode) \
do{ \
    (lnode)->next=nullptr; \
    (lnode)->prev=nullptr; \ 
}while (0)

#define lappend(lnodes,lelem,lfield) \
do{ \
if ((lnodes)->tail==nullptr){ \
    (lnodes)->head=(lelem); \
    (lnodes)->tail=(lelem); \
    break; \
} \
(lelem)->lfield.prev=(lnodes)->tail; \
(lnodes)->tail->lfield.next=(lelem); \
(lnodes)->tail=(lelem); \
}while(0)

#define ldelete(lnodes,lelem,lfield) \
do{ \
if ((lelem)->lfield.prev==nullptr){ \
    (lnodes)->head=(lelem)->lfield.next; \
    if ((lnodes)->head!=nullptr) \
        (lnodes)->head->lfield.prev = nullptr; \
} \
if ((lelem)->lfield.next==nullptr){ \
    (lnodes)->tail=(lelem)->lfield.prev; \
    if ((lnodes)->tail!=nullptr) \
        (lnodes)->tail->lfield.next = nullptr; \
} \
if ((lelem)->lfield.prev!=nullptr&&(lelem)->lfield.next!=nullptr){ \
    (lelem)->lfield.prev->lfield.next=(lelem)->lfield.next; \
    (lelem)->lfield.next->lfield.prev=(lelem)->lfield.prev; \
} \
(lelem)->lfield.prev=nullptr; \
(lelem)->lfield.next=nullptr; \
}while(0)

#define ltraverse(type,lnodes,lfield,now) \
for (type* now=(lnodes)->head;now!=nullptr;now=now->lfield.next) \
