#ifndef MESHLINK_SPLAY_TREE_H
#define MESHLINK_SPLAY_TREE_H

/*
    splay_tree.h -- header file for splay_tree.c
    Copyright (C) 2014, 2017 Guus Sliepen <guus@meshlink.io>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

typedef struct splay_node_t {

	/* Linked list part */

	struct splay_node_t *next;
	struct splay_node_t *prev;

	/* Tree part */

	struct splay_node_t *parent;
	struct splay_node_t *left;
	struct splay_node_t *right;

	/* Payload */

	void *data;

} splay_node_t;

typedef int (*splay_compare_t)(const void *, const void *);
typedef void (*splay_action_t)(const void *);
typedef void (*splay_action_node_t)(const splay_node_t *);

typedef struct splay_tree_t {

	/* Linked list part */

	splay_node_t *head;
	splay_node_t *tail;

	/* Tree part */

	splay_node_t *root;

	splay_compare_t compare;
	splay_action_t delete;

	unsigned int count;

} splay_tree_t;

/* (De)constructors */

splay_tree_t *splay_alloc_tree(splay_compare_t, splay_action_t) __attribute__((__malloc__));
void splay_delete_tree(splay_tree_t *);

splay_node_t *splay_alloc_node(void) __attribute__((__malloc__));
void splay_free_node(splay_tree_t *tree, splay_node_t *);

/* Insertion and deletion */

splay_node_t *splay_insert(splay_tree_t *, void *);
splay_node_t *splay_insert_node(splay_tree_t *, splay_node_t *);

splay_node_t *splay_unlink(splay_tree_t *, void *);
void splay_unlink_node(splay_tree_t *tree, splay_node_t *);
void splay_delete(splay_tree_t *, void *);
void splay_delete_node(splay_tree_t *, splay_node_t *);

/* Searching */

void *splay_search(splay_tree_t *, const void *);
void *splay_search_closest(splay_tree_t *, const void *, int *);
void *splay_search_closest_smaller(splay_tree_t *, const void *);
void *splay_search_closest_greater(splay_tree_t *, const void *);

splay_node_t *splay_search_node(splay_tree_t *, const void *);
splay_node_t *splay_search_closest_node(splay_tree_t *, const void *, int *);
splay_node_t *splay_search_closest_node_nosplay(const splay_tree_t *, const void *, int *);
splay_node_t *splay_search_closest_smaller_node(splay_tree_t *, const void *);
splay_node_t *splay_search_closest_greater_node(splay_tree_t *, const void *);

/* Tree walking */

void splay_foreach(const splay_tree_t *, splay_action_t);
void splay_foreach_node(const splay_tree_t *, splay_action_t);

#define splay_each(type, item, tree) (type *item = (type *)1; item; item = NULL) for(splay_node_t *splay_node = (tree)->head, *splay_next; item = splay_node ? splay_node->data : NULL, splay_next = splay_node ? splay_node->next : NULL, splay_node; splay_node = splay_next)
#define inner_splay_each(type, item, tree) (type *item = (type *)1; item; item = NULL) for(splay_node_t *inner_splay_node = (tree)->head, *inner_splay_next; item = inner_splay_node ? inner_splay_node->data : NULL, inner_splay_next = inner_splay_node ? inner_splay_node->next : NULL, inner_splay_node; inner_splay_node = inner_splay_next)

#endif
