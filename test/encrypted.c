#ifdef NDEBUG
#undef NDEBUG
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <assert.h>
#include <dirent.h>

#include "meshlink.h"
#include "utils.h"

int main(void) {
	meshlink_set_log_cb(NULL, MESHLINK_DEBUG, log_cb);

	// Open a new meshlink instance.

	assert(meshlink_destroy("encrypted_conf"));
	meshlink_handle_t *mesh = meshlink_open_encrypted("encrypted_conf", "foo", "encrypted", DEV_CLASS_BACKBONE, "right", 5);
	assert(mesh);

	// Close the mesh and open it again, now with a different key.

	meshlink_close(mesh);

	mesh = meshlink_open_encrypted("encrypted_conf", "foo", "encrypted", DEV_CLASS_BACKBONE, "wrong", 5);
	assert(!mesh);

	// Open it again, now with the right key.

	mesh = meshlink_open_encrypted("encrypted_conf", "foo", "encrypted", DEV_CLASS_BACKBONE, "right", 5);
	assert(mesh);

	// That's it.

	meshlink_close(mesh);

	// Destroy the mesh.

	assert(meshlink_destroy("encrypted_conf"));

	DIR *dir = opendir("encrypted_conf");
	assert(dir);
	struct dirent *ent;

	while((ent = readdir(dir))) {
		fprintf(stderr, "%s\n", ent->d_name);
		assert(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."));
	}

	closedir(dir);
}
