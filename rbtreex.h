/* subset of the Linux Kernel source file: "include/linux/rbtree.h" GPL-2.0-or-later  */

#ifndef	RBTREEX_H
#define	RBTREEX_H

/*
 * Utils defines
 */
#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef __cplusplus
#   ifndef bool
#       define bool int
#   endif

#   ifndef false
#       define false 0
#   endif

#   ifndef true
#       define true 0
#   endif
#endif

#ifndef offsetof
#   define offsetof(TYPE, MEMBER)   ((size_t)&((TYPE *)0)->MEMBER)
#endif

#ifndef container_of
#   define container_of(ptr, type, member) ({          \
        (type *)( (char *)(ptr) - offsetof(type, member) );})
#endif

#ifndef __always_inline
#   define __always_inline  inline __attribute__((always_inline))
#endif

#ifndef likely
#   define likely(x)    (__builtin_expect(!!(x), 1))
#endif

#ifndef unlikely
#   define unlikely(x)  (__builtin_expect(!!(x), 0))
#endif

#ifndef WRITE_ONCE
#   define WRITE_ONCE(var, val) \
	   (*((volatile __typeof__(val) *)(&(var))) = (val))
#endif

#ifndef READ_ONCE
#   define READ_ONCE(var) (*((volatile __typeof__(var) *)(&(var))))
#endif

/*
 * RBTREEX
 */
struct rb_node {
	unsigned long  __rb_parent_color;
	struct rb_node *rb_right;
	struct rb_node *rb_left;
} __attribute__((aligned(sizeof(long))));
    /* The alignment might seem pointless, but allegedly CRIS needs it */

struct rb_root {
	struct rb_node *rb_node;
};

#define rb_parent(r)   ((struct rb_node *)((r)->__rb_parent_color & ~3))

#define RB_ROOT	(struct rb_root) { NULL, }
#define	rb_entry(ptr, type, member) container_of(ptr, type, member)

#define RB_EMPTY_ROOT(root)  (READ_ONCE((root)->rb_node) == NULL)

/* 'empty' nodes are nodes that are known not to be inserted in an rbtree */
#define RB_EMPTY_NODE(node)  \
	((node)->__rb_parent_color == (unsigned long)(node))
#define RB_CLEAR_NODE(node)  \
	((node)->__rb_parent_color = (unsigned long)(node))

void rb_insert_color(struct rb_node *, struct rb_root *);
void rb_erase(struct rb_node *, struct rb_root *);

/* Find logical next and previous nodes in a tree */
struct rb_node *rb_first(const struct rb_root *);
struct rb_node *rb_last(const struct rb_root *);

struct rb_node *rb_next(const struct rb_node *);
struct rb_node *rb_prev(const struct rb_node *);

/* Postorder iteration - always visit the parent after its children */
struct rb_node *rb_first_postorder(const struct rb_root *);
struct rb_node *rb_next_postorder(const struct rb_node *);

/* Fast replacement of a single node without remove/rebalance/add/rebalance */
void rb_replace_node(struct rb_node *victim, struct rb_node *new_,
			    struct rb_root *root);

static inline void rb_link_node(struct rb_node *node, struct rb_node *parent,
				struct rb_node **rb_link)
{
	node->__rb_parent_color = (unsigned long)parent;
	node->rb_left = node->rb_right = NULL;

	*rb_link = node;
}

#define rb_entry_safe(ptr, type, member) \
	({ __typeof__(ptr) ____ptr = (ptr); \
	   ____ptr ? rb_entry(____ptr, type, member) : NULL; \
	})


/**
 * rbtree__for_each_entry - iterate in order over rb_root of
 * given type allowing the backing memory of @pos to be invalidated
 *
 * @pos:	the 'type *' to use as a loop cursor.
 * @root:	'rb_root *' of the rbtree.
 * @field:	the name of the rb_node field within 'type'.
 *
 */
#define rbtree_for_each_entry(pos, root, field)                         \
    for (                                                               \
        pos = rb_entry_safe(rb_first(root), __typeof__(*pos), field);       \
        pos != NULL;                                                    \
        pos = rb_entry_safe(rb_next(&pos->field), __typeof__(*pos), field)  \
    )

/**
 * rbtree_for_each_entry_safe - iterate in order over rb_root of
 * given type allowing the backing memory of @pos to be invalidated
 *
 * @pos:	the 'type *' to use as a loop cursor.
 * @n:		another 'type *' to use as temporary storage
 * @root:	'rb_root *' of the rbtree.
 * @field:	the name of the rb_node field within 'type'.
 *
 * rbtree_for_each_entry_safe() provides a similar guarantee as
 * list_for_each_entry_safe() and allows the iteration to continue independent
 * of changes to @pos by the body of the loop.
 *
 */
#define rbtree_for_each_entry_safe(pos, n, root, field)                                 \
    for (                                                                               \
        pos = rb_entry_safe(rb_first(root), __typeof__(*pos), field);                       \
        pos && ({ n = rb_entry_safe(rb_next(&pos->field), __typeof__(*pos), field); 1; });  \
        pos = n                                                                         \
    )

/**
 * rbtree_postorder_for_each_entry_safe - iterate in post-order over rb_root of
 * given type allowing the backing memory of @pos to be invalidated
 *
 * @pos:	the 'type *' to use as a loop cursor.
 * @n:		another 'type *' to use as temporary storage
 * @root:	'rb_root *' of the rbtree.
 * @field:	the name of the rb_node field within 'type'.
 *
 * rbtree_postorder_for_each_entry_safe() provides a similar guarantee as
 * list_for_each_entry_safe() and allows the iteration to continue independent
 * of changes to @pos by the body of the loop.
 *
 * Note, however, that it cannot handle other modifications that re-order the
 * rbtree it is iterating over. This includes calling rb_erase() on @pos, as
 * rb_erase() may rebalance the tree, causing us to miss some nodes.
 */
#define rbtree_postorder_for_each_entry_safe(pos, n, root, field) \
	for (pos = rb_entry_safe(rb_first_postorder(root), __typeof__(*pos), field); \
	     pos && ({ n = rb_entry_safe(rb_next_postorder(&pos->field), \
			__typeof__(*pos), field); 1; }); \
	     pos = n)

/*
 * Leftmost-cached rbtrees.
 *
 * We do not cache the rightmost node based on footprint
 * size vs number of potential users that could benefit
 * from O(1) rb_last(). Just not worth it, users that want
 * this feature can always implement the logic explicitly.
 * Furthermore, users that want to cache both pointers may
 * find it a bit asymmetric, but that's ok.
 */
struct rb_root_cached {
	struct rb_root rb_root;
	struct rb_node *rb_leftmost;
};

#define RB_ROOT_CACHED (struct rb_root_cached) { {NULL, }, NULL }

/* Same as rb_first(), but O(1) */
#define rb_first_cached(root) ((root)->rb_leftmost)

static inline void rb_insert_color_cached(struct rb_node *node,
					  struct rb_root_cached *root,
					  bool leftmost)
{
	if (leftmost)
		root->rb_leftmost = node;
	rb_insert_color(node, &root->rb_root);
}

static inline void rb_erase_cached(struct rb_node *node,
				   struct rb_root_cached *root)
{
	if (root->rb_leftmost == node)
		root->rb_leftmost = rb_next(node);
	rb_erase(node, &root->rb_root);
}

static inline void rb_replace_node_cached(struct rb_node *victim,
					  struct rb_node *new_,
					  struct rb_root_cached *root)
{
	if (root->rb_leftmost == victim)
		root->rb_leftmost = new_;
	rb_replace_node(victim, new_, &root->rb_root);
}

/*
 * Please note - only struct rb_augment_callbacks and the prototypes for
 * rb_insert_augmented() and rb_erase_augmented() are intended to be public.
 * The rest are implementation details you are not expected to depend on.
 *
 * See Documentation/rbtree.txt for documentation and samples.
 */

struct rb_augment_callbacks {
	void (*propagate)(struct rb_node *node, struct rb_node *stop);
	void (*copy)(struct rb_node *old, struct rb_node *new_);
	void (*rotate)(struct rb_node *old, struct rb_node *new_);
};

#define	RB_RED		0
#define	RB_BLACK	1

#define __rb_parent(pc)    ((struct rb_node *)(pc & ~3))

#define __rb_color(pc)     ((pc) & 1)
#define __rb_is_black(pc)  __rb_color(pc)
#define __rb_is_red(pc)    (!__rb_color(pc))
#define rb_color(rb)       __rb_color((rb)->__rb_parent_color)
#define rb_is_red(rb)      __rb_is_red((rb)->__rb_parent_color)
#define rb_is_black(rb)    __rb_is_black((rb)->__rb_parent_color)

static inline void rb_set_parent(struct rb_node *rb, struct rb_node *p)
{
	rb->__rb_parent_color = rb_color(rb) | (unsigned long)p;
}

static inline void rb_set_parent_color(struct rb_node *rb,
				       struct rb_node *p, int color)
{
	rb->__rb_parent_color = (unsigned long)p | color;
}

/*
 * @param key:      key for lookup
 * @param root:     root tree
 * @param compare:  user compare node-key (-1, 0, 1)
 *
 * @return          node or NULL
 */
struct rb_node *rb_lookup_key_safe(void *key, struct rb_root *root,
    int (*compare)(struct rb_node *node, void *key));

/*
 * @param new_:     new node
 * @param root:     root tree
 * @param compare:  user compare node-node (-1, 0, 1)
 *
 * @return          bool
 *
 */
bool rb_insert_safe(struct rb_node *new_, struct rb_root *root,
    int (*compare)(struct rb_node *lft, struct rb_node *rht));

/*
 * @param new_:     new node
 * @param root:     root cached tree
 * @param compare:  user compare node-node (-1, 0, 1)
 *
 * @return          bool
 */
bool rb_insert_cached_safe(struct rb_node *new_, struct rb_root_cached *root,
    int (*compare)(struct rb_node *lft, struct rb_node *rht));

#endif	/* RBTREEX_H */
