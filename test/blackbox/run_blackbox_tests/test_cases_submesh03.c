/*
    test_cases_submesh03.c -- Execution of specific meshlink black box test cases
    Copyright (C) 2018  Guus Sliepen <guus@meshlink.io>

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

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <assert.h>
#include "execute_tests.h"
#include "test_cases_submesh03.h"
#include "pthread.h"
#include "../common/containers.h"
#include "../common/test_step.h"
#include "../common/common_handlers.h"
#include "../common/mesh_event_handler.h"

#define CORENODE1_ID  "0"
#define APP1NODE1_ID  "1"
#define APP1NODE2_ID  "2"

#define INIT_ST         0

static bool test_case_status = false;

static void test_case_submesh_03(void **state);
static bool test_steps_submesh_03(void);

static char event_node_name[][10] = {"CORENODE1", "APP1NODE1", "APP1NODE2"};
static const char *node_ids[] = { "corenode1", "app1node1", "app1node2" };

static mesh_event_t core_node1[] = { NODE_STARTED, CHANNEL_OPENED, CHANNEL_DATA_RECIEVED};

static mesh_event_t app1_node1[] = { NODE_STARTED, NODE_JOINED, CHANNEL_OPENED, CHANNEL_DATA_RECIEVED};

static mesh_event_t app1_node2[] = { NODE_STARTED, NODE_JOINED, CHANNEL_OPENED, CHANNEL_DATA_RECIEVED, CHANNEL_OPENED, CHANNEL_DATA_RECIEVED, MESH_EVENT_COMPLETED};

static node_status_t node_status[3] = {
	{core_node1, 0, 3},
	{app1_node1, 0, 4},
	{app1_node2, 0, 7},
};

/* State structure for SubMesh Test Case #3 */
static char *test_case_submesh_3_nodes[] = { "corenode1", "app1node1", "app1node2" };
static black_box_state_t test_case_submesh_3_state = {
	.test_case_name =  "test_cases_submesh03",
	.node_names =  test_case_submesh_3_nodes,
	.num_nodes =  3
};

static int black_box_group0_setup(void **state) {
	(void)state;

	const char *nodes[] = { "corenode1", "app1node1", "app1node2" };
	int num_nodes = sizeof(nodes) / sizeof(nodes[0]);

	PRINT_TEST_CASE_MSG("Creating Containers\n");
	destroy_containers();
	create_containers(nodes, num_nodes);

	return 0;
}

static int black_box_group0_teardown(void **state) {
	(void)state;

	PRINT_TEST_CASE_MSG("Destroying Containers\n");
	destroy_containers();

	return 0;
}

static void restart_all_nodes(char *import) {
	int i;

	for(i = 0; i < 3; i++) {
		node_step_in_container(node_ids[i], "SIGTERM");
		node_status[i].current_index = 0;
	}

	sleep(5);

	node_sim_in_container_event("corenode1", "1", NULL, CORENODE1_ID, import);
	node_sim_in_container_event("app1node1", "1", NULL, APP1NODE1_ID, import);
	node_sim_in_container_event("app1node2", "1", NULL, APP1NODE2_ID, import);
}

static bool event_cb(mesh_event_payload_t payload) {

	fprintf(stderr, "%s(%lu) : %s\n", event_node_name[payload.client_id], time(NULL), event_status[payload.mesh_event]);
	assert(change_state(&node_status[payload.client_id], payload.mesh_event));

	if(payload.mesh_event == NODE_JOINED) {
		signal_node_start(node_status, 1, 2, (char **)node_ids);
	}

	if(check_nodes_finished(node_status, 3)) {
		test_case_status = true;
		return true;
	}

	return false;
}

/* Execute SubMesh Test Case # 3 */
static void test_case_submesh_03(void **state) {
	execute_test(test_steps_submesh_03, state);
}

/* Test Steps for SubMesh Test Case # 3

    Test Steps:
    1. Run corenode1, app1node1, and app1node2
    2. Generate invites to app1node1, and app1node2
        from corenode1 to join corenode1.
    3. After Join is successful start channels from all nodes and exchange data on channels
    4. Try to restart all the nodes at the same time.

    Expected Result:
    Channels should be formed between nodes of sub-mesh & coremesh, nodes with in sub-mesh
    and should be able to exchange data even after restart.
*/
static bool test_steps_submesh_03(void) {
	char *invite_app1node1, *invite_app1node2;
	char *import;

	import = mesh_event_sock_create(eth_if_name);
	invite_app1node1 = invite_in_container("corenode1", "app1node1");
	invite_app1node2 = invite_in_container("corenode1", "app1node2");

	node_sim_in_container_event("corenode1", "1", NULL, CORENODE1_ID, import);
	node_sim_in_container_event("app1node1", "1", invite_app1node1, APP1NODE1_ID, import);
	node_sim_in_container_event("app1node2", "1", invite_app1node2, APP1NODE2_ID, import);

	PRINT_TEST_CASE_MSG("Waiting for nodes to get connected with corenode1\n");

	assert(wait_for_event(event_cb, 120));
	assert(test_case_status);

	test_case_status = false;

	restart_all_nodes(import);
	PRINT_TEST_CASE_MSG("Waiting for nodes to get restarted\n");

	assert(wait_for_event(event_cb, 120));
	assert(test_case_status);

	free(invite_app1node1);
	free(invite_app1node2);

	mesh_event_destroy();

	return true;
}

int test_cases_submesh03(void) {
	const struct CMUnitTest blackbox_group0_tests[] = {
		cmocka_unit_test_prestate_setup_teardown(test_case_submesh_03, setup_test, teardown_test,
		                (void *)&test_case_submesh_3_state)
	};
	total_tests += sizeof(blackbox_group0_tests) / sizeof(blackbox_group0_tests[0]);

	return cmocka_run_group_tests(blackbox_group0_tests, black_box_group0_setup, black_box_group0_teardown);
}
