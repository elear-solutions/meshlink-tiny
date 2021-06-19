#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <assert.h>

#include "meshlink-tiny.h"
#include "utils.h"

static struct sync_flag bar_reachable;

static void status_cb(meshlink_handle_t *mesh, meshlink_node_t *node, bool reachable) {
	(void)mesh;

	if(reachable && !strcmp(node->name, "bar")) {
		set_sync_flag(&bar_reachable, true);
	}
}

int main(void) {
	init_sync_flag(&bar_reachable);

	struct meshlink_node **nodes = NULL;
	size_t nnodes = 0;

	meshlink_set_log_cb(NULL, MESHLINK_DEBUG, log_cb);

	// Open new meshlink instances.

	assert(meshlink_destroy("get_all_nodes_conf.1"));
	assert(meshlink_destroy("get_all_nodes_conf.2"));
	assert(meshlink_destroy("get_all_nodes_conf.3"));

	meshlink_handle_t *mesh[3];
	mesh[0] = meshlink_open("get_all_nodes_conf.1", "foo", "get-all-nodes", DEV_CLASS_BACKBONE);
	assert(mesh[0]);

	mesh[1] = meshlink_open("get_all_nodes_conf.2", "bar", "get-all-nodes", DEV_CLASS_STATIONARY);
	assert(mesh[1]);

	mesh[2] = meshlink_open("get_all_nodes_conf.3", "baz", "get-all-nodes", DEV_CLASS_STATIONARY);
	assert(mesh[2]);

	// Check that we only know about ourself.

	nodes = meshlink_get_all_nodes(mesh[0], nodes, &nnodes);
	assert(nnodes == 1);
	assert(nodes[0] == meshlink_get_self(mesh[0]));

	// Let nodes know about each other.

	for(int i = 0; i < 3; i++) {
		assert(meshlink_set_canonical_address(mesh[i], meshlink_get_self(mesh[i]), "localhost", NULL));
		char *data = meshlink_export(mesh[i]);
		assert(data);

		for(int j = 0; j < 3; j++) {
			if(i == j) {
				continue;
			}

			assert(meshlink_import(mesh[j], data));
		}

		free(data);
	}

	// We should know about all nodes now, and their device class.

	nodes = meshlink_get_all_nodes(mesh[0], nodes, &nnodes);
	assert(nnodes == 3);

	nodes = meshlink_get_all_nodes_by_dev_class(mesh[0], DEV_CLASS_BACKBONE, nodes, &nnodes);
	assert(nnodes == 1);
	assert(nodes[0] == meshlink_get_self(mesh[0]));

	nodes = meshlink_get_all_nodes_by_dev_class(mesh[0], DEV_CLASS_STATIONARY, nodes, &nnodes);
	assert(nnodes == 2);

	// Start foo.

	time_t foo_started = time(NULL);
	assert(meshlink_start(mesh[0]));

	// Start bar and wait for it to connect.

	meshlink_set_node_status_cb(mesh[0], status_cb);

	sleep(2);
	assert(meshlink_start(mesh[1]));
	assert(wait_sync_flag(&bar_reachable, 20));
	time_t bar_started = time(NULL);

	// Stop bar.

	meshlink_stop(mesh[1]);
	sleep(2);

	// Close and restart foo, check that it remembers correctly.

	meshlink_close(mesh[0]);
	sleep(2);
	mesh[0] = meshlink_open("get_all_nodes_conf.1", "foo", "get-all_nodes", DEV_CLASS_BACKBONE);
	assert(mesh[0]);

	nodes = meshlink_get_all_nodes(mesh[0], nodes, &nnodes);
	assert(nnodes == 3);

	nodes = meshlink_get_all_nodes_by_dev_class(mesh[0], DEV_CLASS_BACKBONE, nodes, &nnodes);
	assert(nnodes == 1);
	assert(nodes[0] == meshlink_get_self(mesh[0]));

	nodes = meshlink_get_all_nodes_by_dev_class(mesh[0], DEV_CLASS_STATIONARY, nodes, &nnodes);
	assert(nnodes == 2);

	// Clean up.

	for(int i = 0; i < 3; i++) {
		meshlink_close(mesh[i]);
	}
}
