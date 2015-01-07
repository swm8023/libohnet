#include <stdio.h>
#include <stddef.h>
#include <assert.h>

#include <fnstl/rbtree.h>
#include <fnstl/cpair.h>

/* rbtree iteartor interface */
static iterator_t _rbt_iter_next(iterator_t);
static iterator_t _rbt_iter_pre(iterator_t);
static iterator_t _rbt_iter_next_n(iterator_t, int);
static iterator_t _rbt_iter_pre_n(iterator_t, int);
static bool_t _rbt_iter_equal(iterator_t, iterator_t);
static int   _rbt_iter_distance(iterator_t, iterator_t);
static void* _rbt_iter_get_pointer(iterator_t);
static void  _rbt_iter_get_value(iterator_t, void*);
static void  _rbt_iter_set_value(iterator_t, void*);
static bool_t _rbt_iter_is_vaild(iterator_t);

_iterator_if _default_rbt_iter_if = {
    _rbt_iter_next,
    _rbt_iter_pre,
    _rbt_iter_next_n,
    _rbt_iter_pre_n,
    _rbt_iter_equal,
    _rbt_iter_distance,
    _rbt_iter_get_pointer,
    _rbt_iter_get_value,
    _rbt_iter_set_value,

    _rbt_iter_is_vaild
};

static bool_t _rbt_key_less(const void*, const void*);
static void _rbt_destroy_dfs(_rbtree_t*, _rbtreenode_t*);
static bool_t _rbt_is_vaild_dfs(_rbtree_t*, _rbtreenode_t*, int, int*);


bool_t _rbt_key_less(const void* ptr1, const void* ptr2) {
    return _pair_key_less((pair_t*)ptr1, (pair_t*)ptr2);
}


/* rbtree opertaion functions */
void _rbt_init(_rbtree_t *rbt, type_node *tpnode, bool_t comp_val, bool_t is_multi, _alloc_if *aif) {
    assert(tpnode != NULL);
    assert(rbt    != NULL);

    rbt->allocif      = aif;
    rbt->tpnode       = tpnode;
    rbt->is_multi     = is_multi;
    rbt->size         = 0;
    rbt->guard.parent = NULL;
    rbt->guard.left   = &rbt->guard;
    rbt->guard.right  = &rbt->guard;
    /* set less function, only compile first ele

    /* set, multiset, compare value */
    if (comp_val == true) {
        rbt->less = _TYPE_FUNC_LESS(_TPNODE_TYPE(tpnode));
    /* map, multimap, use pair as rbtree's element */
    } else {
        rbt->less = _rbt_key_less;
    }

    /* less function must exsit */
    assert(rbt->less != NULL);
}

void _rbt_destroy_dfs(_rbtree_t *rbt, _rbtreenode_t *node) {
    if (node != NULL) {
        _rbt_destroy_dfs(rbt, node->left);
        _rbt_destroy_dfs(rbt, node->right);
        _rbt_free_treenode(rbt, node);
    }
}

void _rbt_destroy(_rbtree_t* rbt) {
    _rbt_destroy_dfs(rbt, _RBT_ROOT(rbt));
}

_rbtreenode_t* _rbt_find_val(_rbtree_t *rbt, _rbtreenode_t **parent, const void *val) {
    assert(rbt != NULL);
    assert(val != NULL);

    _rbtreenode_t *node = _RBT_ROOT(rbt);
    if (parent) *parent = NULL;
    /* parent point to the parent of the node*/
    while (node != NULL) {
        /* decide go left or go right */
        if (_RBTNODE_VAL_LESS(rbt, val, _RBTNODE_DATAP(node))) {
            if (parent) *parent = node;
            node = node->left;
        } else if (_RBTNODE_VAL_LESS(rbt, _RBTNODE_DATAP(node), val)) {
            if (parent) *parent = node;
            node = node->right;
        /* find the node, then return it*/
        } else {
            return node;
        }
    }
    return NULL;
}

_rbtreenode_t* _rbt_insert_val(_rbtree_t *rbt, const void *val) {
    assert(rbt != NULL);
    assert(val != NULL);

    _rbtreenode_t *parent = NULL;
    _rbtreenode_t *node = _rbt_find_val(rbt, &parent, val);

    /*already exist, just update it and return */
    if (node != NULL) {
        _type_copy(_RBT_TYPE(rbt), _RBTNODE_DATAP(node), val);
        return node;
    }
    /* create new node, return NULL if failed */
    _rbtreenode_t *newnode = _rbt_new_treenode(rbt, val);
    RETURN_IF_NULL(newnode, NULL);

    /* no root node, let newnode be the root */
    if (parent == NULL) {
        _RBT_GUARD(rbt)->parent = newnode;
    } else {
        if (_RBTNODE_VAL_LESS(rbt, val, _RBTNODE_DATAP(parent))) {
            parent->left = newnode;
        } else {
            parent->right = newnode;
        }
    }
    newnode->parent = parent;

    /* increace rbtree size and fixup the tree, return 0 if insert success */
    rbt->size ++;
    _rbt_insert_node_fixup(rbt, newnode);

    /* check after delete */
    assert(_rbt_is_vaild(rbt));
    return newnode;
}

int _rbt_insert_node_fixup(_rbtree_t *rbt, _rbtreenode_t *node) {
    assert(rbt != NULL);
    assert(node != NULL);

    /* node = z, parent = p[z], uncle = u[z], p[p[z]] = z' */
    /* the p[z] is red, indicate the tree need be fixup */
    while (_RBTNODE_ISRED(node->parent)) {
        /* the p[z] is red, so the p[p[z]] must be exist beacause the root color can't be red */
        _rbtreenode_t *parent  = node->parent;
        _rbtreenode_t *gparent = parent->parent;

        /* the p[z] is p[p[z]] left child */
        if (parent == gparent->left) {
            _rbtreenode_t *uncle = gparent->right;
            /* case 1, u[z] is red */
            if (_RBTNODE_ISRED(uncle)) {
                _RBTNODE_SETBLK(parent);
                _RBTNODE_SETBLK(uncle);
                _RBTNODE_SETRED(gparent);
                node = gparent;
            } else {
                /* case 2, u[z] is black and z is p[z]'s right child, change to case 3  */
                if (node == parent->right) {
                    _rbt_left_rotate(rbt, node = parent);
                    parent = node->parent;
                }
                /* case 3, u[z] is black and z is p[z]'s left child */
                _RBTNODE_SETBLK(parent);
                _RBTNODE_SETRED(gparent);
                _rbt_right_rotate(rbt, gparent);
            }
        /* the p[z] is p[p[z]] right child */
        } else {
            _rbtreenode_t *uncle = gparent->left;
            /* case 1, u[z] is red */
            if (_RBTNODE_ISRED(uncle)) {
                _RBTNODE_SETBLK(parent);
                _RBTNODE_SETBLK(uncle);
                _RBTNODE_SETRED(gparent);
                node = gparent;
            } else {
                 /* case 2, u[z] is black and z is p[z]'s left child, change to case 3  */
                if (node == parent->left) {
                    _rbt_right_rotate(rbt, node = parent);
                    parent  = node->parent;
                }
                /* case 3, u[z] is black and z is p[z]'s right child */
                _RBTNODE_SETBLK(parent);
                _RBTNODE_SETRED(gparent);
                _rbt_left_rotate(rbt, gparent);
            }
        }
    }

    _RBTNODE_SETBLK(_RBT_ROOT(rbt));

    return 0;
}

int _rbt_erase_val(_rbtree_t* rbt, const void* val) {
    assert(rbt != NULL);
    assert(val != NULL);

    _rbtreenode_t *node = _rbt_find_val(rbt, NULL, val);

    /* return 0 if not found*/
    RETURN_IF_NULL(node, 0);

    return _rbt_erase_node(rbt, node);
}

int _rbt_erase_node(_rbtree_t *rbt, _rbtreenode_t* node) {
    assert(rbt  != NULL);
    assert(node != NULL);
    assert(node != _RBT_GUARD(rbt));

    /* case 1: l[z] and r[z] is NULL, delete directly
     * case 2: one of l[z] and r[z] is NULL, connect p[z] and the not NULL node
     * case 3: neither l[z] or r[z] is NULL, let y be the successor of z,
     *         y must in case 2, swap z and y and then delete z
     */

     /* if in case 1, delchild will be NULL
      * if in case 3, will convert to case 3, delnode point to really deleted node pos
      */
    _rbtreenode_t *delnode = ((node->left == NULL || node->right == NULL) ?
        node : _rbt_tree_successor(node));
    _rbtreenode_t *delchild =  (delnode->left ? delnode->left : delnode->right);

    if (delchild) {
        delchild->parent = delnode->parent;
    }
    if (delnode->parent == NULL) {
        _RBT_ROOT(rbt) = delchild;
    } else if (delnode == delnode->parent->left) {
        delnode->parent->left = delchild;
    } else {
        delnode->parent->right = delchild;
    }

    /* in case 3, copy from delnode to node */
    if (delnode != node) {
        _type_copy(_RBT_TYPE(rbt), _RBTNODE_DATAP(node), _RBTNODE_DATAP(delnode));
    }

    /* fix up the tree when delnode color is black, if the delnode is red, don't need fixup */
    if (_RBTNODE_ISBLK(delnode)) {
        _rbt_erase_node_fixup(rbt, delchild, delnode->parent);
    }

    /* free delnode and decrease rbtree size*/
    _rbt_free_treenode(rbt, delnode);
    rbt->size--;

    /* check after delete */
    assert(_rbt_is_vaild(rbt));
    return 0;
}

int _rbt_erase_node_fixup(_rbtree_t* rbt, _rbtreenode_t* node, _rbtreenode_t* parent) {
    assert(rbt != NULL);

    /* node = z, parent = p[z], uncle = u[z], p[p[z]] = z', brother = b[z] */
    _rbtreenode_t *brother;

    /* only go on when z is black, can consider that z is a double blcak node */
    while (node != _RBT_ROOT(rbt) && _RBTNODE_ISBLK(node)) {
        if (node == parent->left) {
            brother = parent->right;

            /* case 1: b[z] is red, so b[z] must has black child,
                change b and p[z]'s color ,then left rotate p[z], change to 2, 3 or 4*/
            if (_RBTNODE_ISRED(brother)) {
                _RBTNODE_SETBLK(brother);
                _RBTNODE_SETRED(parent);
                _rbt_left_rotate(rbt, parent);
                brother = parent->right;
            }
            /* pass case 1, b[z] must be black */
            /* case 2: both b[z]'s left child and right child are black, 
             *   let b[z] to red and z decrease to one black, then let parent be black at last
             */
            if (_RBTNODE_ISBLK(brother->left) && _RBTNODE_ISBLK(brother->right)) {
                _RBTNODE_SETRED(brother);
                node = parent; /* !!! node may be null */
                parent = node->parent;
            /* case 3: only b[z]'s right child is black, then right rotate b[z], change to case 4*/
            } else {
                if (_RBTNODE_ISBLK(brother->right)) {
                    _RBTNODE_SETBLK(brother->left);
                    _RBTNODE_SETRED(brother);
                    _rbt_right_rotate(rbt, brother);
                    brother = parent->right;
                }
                /* case 4: both b[z]'s children is red */
                _RBTNODE_SETCLR(brother, parent->color);
                _RBTNODE_SETBLK(parent);
                _RBTNODE_SETBLK(brother->right);
                _rbt_left_rotate(rbt, parent);
                node = _RBT_ROOT(rbt);
            }
        /* z is p[z]'s right child */
        } else {
            brother = parent->left;
            /* case 1: b[z] is red, so b[z] must has black child,
                change b and p[z]'s color ,then right rotate p[z], change to 2, 3 or 4*/
            if (_RBTNODE_ISRED(brother)) {
                _RBTNODE_SETBLK(brother);
                _RBTNODE_SETRED(parent);
                _rbt_right_rotate(rbt, parent);
                brother = parent->left;
            }
            /* pass case 1, b[z] must be black */
            /* case 2: both b[z]'s left child and right child are black, 
             *   let b[z] to red and z decrease to one black, then let parent be black at last
             */
            if (_RBTNODE_ISBLK(brother->left) && _RBTNODE_ISBLK(brother->right)) {
                _RBTNODE_SETRED(brother);
                node = parent; /* !!! node may be null */
                parent = node->parent;
            /* case 3: only b[z]'s left child is black, then right rotate b[z], change to case 4*/
            } else {
                if (_RBTNODE_ISBLK(brother->left)) {
                    _RBTNODE_SETBLK(brother->right);
                    _RBTNODE_SETRED(brother);
                    _rbt_left_rotate(rbt, brother);
                    brother = parent->left;
                }
                /* case 4: both b[z]'s children is red */
                _RBTNODE_SETCLR(brother, parent->color);
                _RBTNODE_SETBLK(parent);
                _RBTNODE_SETBLK(brother->left);
                _rbt_right_rotate(rbt, parent);
                node = _RBT_ROOT(rbt);
            }
        }
    }

    _RBTNODE_SETBLK(node);
    return 0;
}

void _rbt_left_rotate(_rbtree_t *rbt, _rbtreenode_t *node) {
    assert(node != NULL);
    assert(node->right != NULL);

    /*  z             a
     *   \           / \
     *    a     =>  z   c
     *   / \         \
     *  b   c         b
     */
    _rbtreenode_t *rnode = node->right;
    node->right = rnode->left;
    if (rnode->left) {
        rnode->left->parent = node;
    }
    rnode->parent = node->parent;
    if (node->parent == NULL) {
        _RBT_ROOT(rbt) = rnode;
    } else if (node == node->parent->left) {
        node->parent->left = rnode;
    } else if (node == node->parent->right) {
        node->parent->right = rnode;
    }
    rnode->left = node;
    node->parent = rnode;
}

void _rbt_right_rotate(_rbtree_t *rbt, _rbtreenode_t *node) {
    assert(node != NULL);
    assert(node->left != NULL);

    /*      z         a
     *     /         / \
     *    a     =>  b   z
     *   / \           /
     *  b   c         c
     */
    _rbtreenode_t *lnode = node->left;
    node->left = lnode->right;
    if (lnode->right) {
        lnode->right->parent = node;
    }
    lnode->parent = node->parent;
    if (node->parent == NULL) {
        _RBT_ROOT(rbt) = lnode;
    } else if (node == node->parent->left) {
        node->parent->left = lnode;
    } else if (node == node->parent->right) {
        node->parent->right = lnode;
    }
    lnode->right = node;
    node->parent = lnode;
}

_rbtreenode_t* _rbt_tree_predecessor(_rbtreenode_t* node) {
    assert(node != NULL);

    if (node->left) {
        return _rbt_tree_maximum(node->left);
    }
    while (node->parent && node == node->parent->left) {
        node = node->parent;
    }
    return node->parent;
}

_rbtreenode_t* _rbt_tree_successor(_rbtreenode_t* node) {
    assert(node != NULL);

    if (node->right) {
        return _rbt_tree_minimum(node->right);
    }
    while (node->parent && node == node->parent->right) {
        node = node->parent;
    }
    return node->parent;
}

_rbtreenode_t* _rbt_tree_minimum(_rbtreenode_t* node) {
    assert(node != NULL);

    for (; node->left; node = node->left);
    return node;
}

_rbtreenode_t* _rbt_tree_maximum(_rbtreenode_t* node) {
    assert(node != NULL);

    for (; node->right; node = node->right);
    return node;
}

_rbtreenode_t* _rbt_new_treenode(_rbtree_t* rbt, const void *valp) {
    assert(rbt != NULL);
    assert(valp != NULL);

    /* alloc and init new node, the init color is RED*/
    _type_t *type = _RBT_TYPE(rbt);
    _rbtreenode_t *newnode = (_rbtreenode_t*)_RBT_ALLOC(rbt, _RBTNODE_SIZE(type));
    memset(newnode, 0, sizeof(_rbtreenode_t));
    _type_init(type, _RBTNODE_DATAP(newnode), rbt->tpnode);
    _type_copy(type, _RBTNODE_DATAP(newnode), valp);
    return newnode;
}

void _rbt_free_treenode(_rbtree_t *rbt, _rbtreenode_t* delnode) {
    assert(delnode != NULL);
    assert(rbt != NULL);

    /* alloc and init new node, the init color is RED*/
    _type_t *type = _RBT_TYPE(rbt);
    _type_destroy(type, _RBTNODE_DATAP(delnode));
    _RBT_FREE(rbt, delnode);
}

void _rbt_tree_print_dfs(_rbtreenode_t *node, int depth) {
    if (node == NULL) return;
    int i = 0;
    for (i = 0; i < depth * 3; i++) printf(" ");
    printf("%d %c\n", *(int*)_RBTNODE_DATAP(node), node->color ? 'b' : 'r');
    _rbt_tree_print_dfs(node->left, depth + 1);
    _rbt_tree_print_dfs(node->right, depth + 1);
}

void _rbt_tree_print(_rbtree_t* rbt) {
    _rbt_tree_print_dfs(_RBT_ROOT(rbt), 0);
    printf("======\n");
}


bool_t _rbt_is_vaild(_rbtree_t *rbt) {
    //_rbt_tree_print(rbt);
    assert(rbt != NULL);

    /* property 2: root is black */
    if (_RBTNODE_ISRED(_RBT_ROOT(rbt))) {
        return false;
    }
    int maxbh = 0;
    return _rbt_is_vaild_dfs(rbt, _RBT_ROOT(rbt), 0, &maxbh);
}

static bool_t _rbt_is_vaild_dfs(_rbtree_t *rbt, _rbtreenode_t *node, int bh, int *maxbh) {
    if (node == NULL) {
        /* property 5: every path from to root to leaf has same black nodes */
        if (*maxbh == 0) *maxbh = bh + 1;
        return *maxbh == bh + 1;
    }
    if (_RBTNODE_ISBLK(node)) {
        bh += 1;
    } else if (_RBTNODE_ISRED(node)) {
        /* property 4: both children of red node is black */
        if (!_RBTNODE_ISBLK(node->left) || !_RBTNODE_ISBLK(node->right)) {
            return false;
        }
    /* property 1: a node is either red or black */
    } else {
        return false;
    }

    /* property of binary tree */
    if (node->left && !_RBTNODE_LESS(rbt, node->left, node)) {
        return false;
    }
    if (node->right && !_RBTNODE_LESS(rbt, node, node->right)) {
        return false;
    }
    if (!_rbt_is_vaild_dfs(rbt, node->left, bh, maxbh) ||
        !_rbt_is_vaild_dfs(rbt, node->right, bh, maxbh)) {
        return false;
    }
    return true;
}

/* rbtree iterator functions */
_rbtreenode_t* _rbt_begin_node(_rbtree_t* rbt) {
    return _RBT_SIZE(rbt) == 0 ? _RBT_GUARD(rbt) : _rbt_tree_minimum(_RBT_ROOT(rbt));
}

iterator_t _rbt_iter_next(iterator_t iter) {
    assert(_iter_is_vaild(iter));

    _rbtree_t *rbt      = _ITER_RBTREE(iter);
    _rbtreenode_t *node = (_rbtreenode_t*)_ITER_RBTPTR(iter);

    node = _rbt_tree_successor(node);
    _ITER_RBTPTR(iter) = (node ? (char*)node : (char*)_RBT_GUARD(rbt));

    /* check if iterator vaild after change */
    assert(_iter_is_vaild(iter));

    return iter;
}

iterator_t _rbt_iter_pre(iterator_t iter) {
    assert(_iter_is_vaild(iter));

    _rbtree_t *rbt      = _ITER_RBTREE(iter);
    _rbtreenode_t *node = (_rbtreenode_t*)_ITER_RBTPTR(iter);

    node = _rbt_tree_predecessor(node);
    _ITER_RBTPTR(iter) = (node ? (char*)node : (char*)_RBT_GUARD(rbt));

    /* check if iterator vaild after change */
    assert(_iter_is_vaild(iter));

    return iter;
}

iterator_t _rbt_iter_next_n(iterator_t iter, int step) {
    assert(_iter_is_vaild(iter));

    iter = iter_advance(iter, step);

    /* check if iterator vaild after change */
    assert(_iter_is_vaild(iter));
    return iter;
}

iterator_t _rbt_iter_pre_n(iterator_t iter, int step) {
    assert(_iter_is_vaild(iter));

    return _rbt_iter_next_n(iter, -step);
}

bool_t _rbt_iter_equal(iterator_t iter0, iterator_t iter1) {
    assert(_iter_is_same(iter0, iter1));

    return _ITER_RBTPTR(iter0)  == _ITER_RBTPTR(iter1) ? true : false;
}

int _rbt_iter_distance(iterator_t iter0, iterator_t iter1) {
    assert(_iter_is_same(iter0, iter1));


}

void* _rbt_iter_get_pointer(iterator_t iter) {
    assert(_iter_is_vaild(iter));

    return (void*)_RBTNODE_DATAP((_rbtreenode_t*)_ITER_RBTPTR(iter));
}

void  _rbt_iter_get_value(iterator_t iter, void* elmp) {
    assert(elmp != NULL);
    assert(_iter_is_vaild(iter));

    _rbtree_t *rbt      = _ITER_RBTREE(iter);
    _type_copy(_RBT_TYPE(rbt), elmp,
        _RBTNODE_DATAP((_rbtreenode_t*)_ITER_RBTPTR(iter)));
}

void  _rbt_iter_set_value(iterator_t iter, void* elmp) {
    assert(elmp != NULL);
    assert(_iter_is_vaild(iter));

    _rbtree_t *rbt = _ITER_RBTREE(iter);
    _type_copy(_RBT_TYPE(rbt), 
        _RBTNODE_DATAP((_rbtreenode_t*)_ITER_RBTPTR(iter)), elmp);
}

bool_t _rbt_iter_is_vaild(iterator_t iter) {
    /* !! can't call iter_is_vaild here*/
    _container_base *ctn = _ITER_CONTAINER_BASE(iter);
    RETURN_IF_NULL(ctn, false);
    if (_CTR_STYPE(ctn)->type_id != _typeid_set_t &&
        _CTR_STYPE(ctn)->type_id != _typeid_map_t) {
        return false;
    }
    return true;
}

