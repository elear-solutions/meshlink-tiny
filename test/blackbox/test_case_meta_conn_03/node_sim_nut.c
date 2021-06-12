/*
    node_sim.c -- Implementation of Node Simulation for Meshlink Testing
                    for meta connection test case 01 - re-connection of
                    two nodes when relay node goes down
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../common/common_handlers.h"
#include "../common/test_step.h"
#include "../common/mesh_event_handler.h"

static bool conn_status = false;

void callback_logger(meshlink_handle_t *mesh, meshlink_log_level_t level, const char *text) {
	(void)mesh;
	(void)level;

	char connection_match_msg[100];

	fprintf(stderr, "meshlink>> %s\n", text);

	if(strstr(text, "Connection") || strstr(text, "connection")) {
		assert(snprintf(connection_match_msg, sizeof(connection_match_msg),
		                "Connection with peer") >= 0);

		if(strstr(text, connection_match_msg) && strstr(text, "activated")) {
			conn_status = true;
			return;
		}

		assert(snprintf(connection_match_msg, sizeof(connection_match_msg),
		                "Already connected to peer") >= 0);

		if(strstr(text, connection_match_msg)) {
			conn_status = true;
			return;
		}

		assert(snprintf(connection_match_msg, sizeof(connection_match_msg),
		                "Connection closed by peer") >= 0);

		if(strstr(text, connection_match_msg)) {
			conn_status = false;
			return;
		}

		assert(snprintf(connection_match_msg, sizeof(connection_match_msg),
		                "Closing connection with peer") >= 0);

		if(strstr(text, connection_match_msg)) {
			conn_status = false;
			return;
		}
	}

	return;
}

int main(int argc, char *argv[]) {
	(void)argc;

	int client_id = -1;
	bool result = false;
	int i;

	if((argv[3]) && (argv[4])) {
		client_id = atoi(argv[3]);
		mesh_event_sock_connect(argv[4]);
	}

	execute_open(argv[1], argv[2]);
	meshlink_set_log_cb(mesh_handle, MESHLINK_DEBUG, callback_logger);

	if(argv[5]) {
		execute_join(argv[5]);
	}

	execute_start();
	mesh_event_sock_send(client_id, NODE_STARTED, NULL, 0);

	/* Connectivity of peer is checked using meshlink_get_node API */
	while(!conn_status) {
		sleep(1);
	}

	sleep(1);
	fprintf(stderr, "Connected with Peer\n");
	mesh_event_sock_send(client_id, META_CONN_SUCCESSFUL, NULL, 0);

	conn_status = false;
	fprintf(stderr, "Waiting 120 sec for peer to be re-connected\n");

	for(i = 0; i < 120; i++) {
		if(conn_status) {
			result = true;
			break;
		}

		sleep(1);
	}

	if(result) {
		fprintf(stderr, "Re-connected with Peer\n");
		mesh_event_sock_send(client_id, META_RECONN_SUCCESSFUL, NULL, 0);
	} else {
		fprintf(stderr, "Failed to reconnect with Peer\n");
		mesh_event_sock_send(client_id, META_RECONN_FAILURE, NULL, 0);
	}

	execute_close();
	assert(meshlink_destroy(argv[1]));

	return 0;
}
