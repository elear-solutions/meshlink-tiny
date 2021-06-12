/*
    ecdsa.c -- ECDSA key handling
    Copyright (C) 2014-2017 Guus Sliepen <guus@meshlink.io>

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

#include "../system.h"

#include "ed25519.h"

#define __MESHLINK_ECDSA_INTERNAL__
typedef struct {
	uint8_t private[64];
	uint8_t public[32];
} ecdsa_t;

#include "../logger.h"
#include "../ecdsa.h"
#include "../utils.h"
#include "../xalloc.h"

// Get and set ECDSA keys
//
ecdsa_t *ecdsa_set_base64_public_key(const char *p) {
	int len = strlen(p);

	if(len != 43) {
		logger(NULL, MESHLINK_ERROR, "Invalid size %d for public key!", len);
		return 0;
	}

	ecdsa_t *ecdsa = xzalloc(sizeof * ecdsa);
	len = b64decode(p, ecdsa->public, len);

	if(len != 32) {
		logger(NULL, MESHLINK_ERROR, "Invalid format of public key! len = %d", len);
		free(ecdsa);
		return 0;
	}

	return ecdsa;
}

ecdsa_t *ecdsa_set_public_key(const void *p) {
	ecdsa_t *ecdsa = xzalloc(sizeof(*ecdsa));
	memcpy(ecdsa->public, p, sizeof ecdsa->public);
	return ecdsa;
}

char *ecdsa_get_base64_public_key(ecdsa_t *ecdsa) {
	char *base64 = xmalloc(44);
	b64encode(ecdsa->public, base64, sizeof ecdsa->public);

	return base64;
}

const void *ecdsa_get_public_key(ecdsa_t *ecdsa) {
	return ecdsa->public;
}

ecdsa_t *ecdsa_set_private_key(const void *p) {
	ecdsa_t *ecdsa = xzalloc(sizeof(*ecdsa));
	memcpy(ecdsa->private, p, sizeof(*ecdsa));
	return ecdsa;
}

const void *ecdsa_get_private_key(ecdsa_t *ecdsa) {
	return ecdsa->private;
}

// Read PEM ECDSA keys

ecdsa_t *ecdsa_read_pem_public_key(FILE *fp) {
	ecdsa_t *ecdsa = xzalloc(sizeof(*ecdsa));

	if(fread(ecdsa->public, sizeof ecdsa->public, 1, fp) == 1) {
		return ecdsa;
	}

	free(ecdsa);
	return 0;
}

ecdsa_t *ecdsa_read_pem_private_key(FILE *fp) {
	ecdsa_t *ecdsa = xmalloc(sizeof * ecdsa);

	if(fread(ecdsa, sizeof * ecdsa, 1, fp) == 1) {
		return ecdsa;
	}

	free(ecdsa);
	return 0;
}

size_t ecdsa_size(ecdsa_t *ecdsa) {
	(void)ecdsa;
	return 64;
}

// TODO: standardise output format?

bool ecdsa_sign(ecdsa_t *ecdsa, const void *in, size_t len, void *sig) {
	ed25519_sign(sig, in, len, ecdsa->public, ecdsa->private);
	return true;
}

bool ecdsa_verify(ecdsa_t *ecdsa, const void *in, size_t len, const void *sig) {
	return ed25519_verify(sig, in, len, ecdsa->public);
}

bool ecdsa_active(ecdsa_t *ecdsa) {
	return ecdsa;
}

void ecdsa_free(ecdsa_t *ecdsa) {
	free(ecdsa);
}
