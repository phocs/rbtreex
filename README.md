# rbtreex
It's subset of the Linux 's red-black tree implementation.

To use it, `#include <rbtreex.h>`.

Creating a new rbtreex
---------------------

Data nodes in an rbtree tree are structures containing a struct rb_node member:

```
typedef struct mydata {
    struct rb_node node;
    int key;

    char attr_a;
    char attr_b;
    char attr_c;
    char attr_d;
} mydata_t;
```

At the root of each rbtreex is an rb_root structure, which is initialized to be
empty via:
```
struct rb_root tree = RB_ROOT;
```
Searching for a key in an rbtreex, example:
-------------------------------------------
```
/* define compare functions */
int key_compare(struct rb_node *node, void *key) {
    mydata_t *entry = rb_entry_safe(node, mydata_t, node);
    return memcmp((void *) &(entry->key), key, sizeof(int));
}

int node_compare(struct rb_node *lft, struct rb_node *rht) {
    mydata_t *erht = rb_entry_safe(rht, mydata_t, node);
    return key_compare(lft, &(erht->key));
}

mydata_t *search(struct rb_root *root, int key) {
    struct rb_node *found = rb_lookup_key_safe(&key, root, key_compare);
    return rb_entry_safe(found, mydata_t, node);
}
```
Inserting data into an rbtreex, example:
--------------------------------------
```
bool insert(struct rb_root *root, mydata_t *new_) {
    return rb_insert_safe(&(new_->node), root, node_compare);
}

```
Removing or replacing existing data in an rbtreex, example:
---------------------------------------------------------
```
void delete(struct rb_root *root, mydata_t *old) {
    rb_erase(&(old->node), root);
}

void replace(mydata_t *victim, mydata_t *new_, struct rb_root *root) {
    rb_replace_node(&(victim->node), &(new_->node), root);
}
```

Install:
---------------------------------------------------------
```
mkdir build
cd build
cmake ..
make
make install
```

Uninstall:
---------------------------------------------------------
```
make uninstall
```

References:
---------------
Linux [Source](https://github.com/torvalds/linux)

Linux List [include/linux/rbtreex.h](https://github.com/torvalds/linux/blob/master/include/linux/rbtree.h)
