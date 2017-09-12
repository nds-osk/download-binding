/*
 * Copyright 2017 OSAKA NDS CO., LTD.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "validation.h"

static int isNumber(const char *pstr);
static int validation_id(const char *pparam, char *perr, size_t size);
static int validation_max_speed(const char *pparam, char *perr, size_t size);
static int validation_url(const char *pparam, char *perr, size_t size);
static int validation_filename(const char *pparam, char *perr, size_t size);
static int validation_crypto(const char *pparam, char *perr, size_t size);
static int validation_key_length(const char *pparam, char *perr, size_t size);
static int validation_mode(const char *pparam, char *perr, size_t size);
static int validation_key(const char *pparam, char *perr, size_t size);
static int validation_iv(const char *pparam, char *perr, size_t size);

static int isNumber(const char *pstr)
{
	int isnum = 1;		/* 1:true 0:false */
	char array[16];		/* max length:15 */
	int i;
	size_t len;

	if (pstr == NULL) {
		return 0;
	}

	len = strlen(pstr);
	if (strlen(pstr) >= sizeof(array)) {
		return 0;
	}

	(void)strcpy(array, pstr);
	for (i = 0; array[i] != '\0'; i++) {
		if (('9' < array[i]) || ('0' > array[i])) {
			isnum = 0;
			break;
		}
	}

	return isnum;
}

static int validation_id(const char *pparam, char *perr, size_t size)
{
	const char *pcomm = "id is invalid value";
	int id;

	if (pparam == NULL) {
		(void)snprintf(perr, size, "id is NULL");
		return 1;
	}
	if (isNumber(pparam) == 0) {
		(void)snprintf(perr, size, "%s", pcomm);
		return 1;
	}

	/* range: 0 - 999 */
	id = atoi(pparam);
	if ((id < 0) || (id > 999)) {
		(void)snprintf(perr, size, "%s", pcomm);
		return 1;
	}

	return 0;
}

static int validation_max_speed(const char *pparam, char *perr, size_t size)
{
	const char *pcomm = "max_speed is invalid value";
	int ival;

	if (pparam == NULL) {
		(void)snprintf(perr, size, "max_speed is NULL");
		return 1;
	}
	if (isNumber(pparam) == 0) {
		(void)snprintf(perr, size, "%s", pcomm);
		return 1;
	}

	/* range: 0(unlimited) - 104857600(100Mbps) */
	ival = atoi(pparam);
	if (ival > 104857600) {
		(void)snprintf(perr, size, "%s", pcomm);
		return 1;
	}

	return 0;
}

static int validation_url(const char *pparam, char *perr, size_t size)
{
	const char *pcomm = "url is invalid value";
	unsigned int ilen;

	if (pparam == NULL) {
		(void)snprintf(perr, size, "url is NULL");
		return 1;
	}

	/* maximum length: 2083 */
	ilen = strlen(pparam);
	if (ilen > (unsigned int)2083) {
		(void)snprintf(perr, size, "%s", pcomm);
		return 1;
	}

	return 0;
}

static int validation_filename(const char *pparam, char *perr, size_t size)
{
	const char *pcomm = "filename is invalid value";
	unsigned int ilen;

	if (pparam == NULL) {
		(void)snprintf(perr, size, "filename is NULL");
		return 1;
	}

	/* maximum length: 255 */
	ilen = strlen(pparam);
	if (ilen > (unsigned int)255) {
		(void)snprintf(perr, size, "%s", pcomm);
		return 1;
	}

	/* unusable characters */
	if (strchr(pparam, (int)'/') != NULL) {
		(void)snprintf(perr, size, "%s", pcomm);
		return 1;
	}

	return 0;
}

static int validation_crypto(const char *pparam, char *perr, size_t size)
{
	const char *pcomm = "crypto is invalid value";
	int ival;

	if (pparam == NULL) {
		(void)snprintf(perr, size, "crypto is NULL");
		return 1;
	}
	if (isNumber(pparam) == 0) {
		(void)snprintf(perr, size, "%s", pcomm);
		return 1;
	}

	/* range: 0 - 1 */
	ival = atoi(pparam);
	if ((ival != 0) && (ival != 1)) {
		(void)snprintf(perr, size, "%s", pcomm);
		return 1;
	}

	return 0;
}

static int validation_key_length(const char *pparam, char *perr, size_t size)
{
	const char *pcomm = "key_length is invalid value";
	int ival;

	if (pparam == NULL) {
		(void)snprintf(perr, size, "key_length is NULL");
		return 1;
	}
	if (isNumber(pparam) == 0) {
		(void)snprintf(perr, size, "%s", pcomm);
		return 1;
	}

	/* range: 0 - 2 */
	ival = atoi(pparam);
	if (ival > 2) {
		(void)snprintf(perr, size, "%s", pcomm);
		return 1;
	}

	return 0;
}

static int validation_mode(const char *pparam, char *perr, size_t size)
{
	const char *pcomm = "mode is invalid value";
	int ival;

	if (pparam == NULL) {
		(void)snprintf(perr, size, "mode is NULL");
		return 1;
	}
	if (isNumber(pparam) == 0) {
		(void)snprintf(perr, size, "%s", pcomm);
		return 1;
	}

	/* only 0 */
	ival = atoi(pparam);
	if (ival > 0) {
		(void)snprintf(perr, size, "%s", pcomm);
		return 1;
	}

	return 0;
}

static int validation_key(const char *pparam, char *perr, size_t size)
{
	const char *pcomm = "key is invalid value";
	unsigned int ilen;

	if (pparam == NULL) {
		(void)snprintf(perr, size, "key is NULL");
		return 1;
	}

	/* maximum length: 255 */
	ilen = strlen(pparam);
	if (ilen > (unsigned int)255) {
		(void)snprintf(perr, size, "%s", pcomm);
		return 1;
	}

	return 0;
}

static int validation_iv(const char *pparam, char *perr, size_t size)
{
	const char *pcomm = "iv is invalid value";
	unsigned int ilen;

	if (pparam == NULL) {
		(void)snprintf(perr, size, "iv is NULL");
		return 1;
	}

	/* maximum length: 255 */
	ilen = strlen(pparam);
	if (ilen > (unsigned int)255) {
		(void)snprintf(perr, size, "%s", pcomm);
		return 1;
	}

	return 0;
}

/*
 * validation
 */
int validation(const char *pname, const char *pparam, char *perr, size_t size)
{
	int ret = 0;		/* 0:valid 1:invalid */

	if (strcmp(pname, "id") == 0) {
		ret = validation_id(pparam, perr, size);
	} else if (strcmp(pname, "max_speed") == 0) {
		ret = validation_max_speed(pparam, perr, size);
	} else if (strcmp(pname, "url") == 0) {
		ret = validation_url(pparam, perr, size);
	} else if (strcmp(pname, "filename") == 0) {
		ret = validation_filename(pparam, perr, size);
	} else if (strcmp(pname, "crypto") == 0) {
		ret = validation_crypto(pparam, perr, size);
	} else if (strcmp(pname, "key_length") == 0) {
		ret = validation_key_length(pparam, perr, size);
	} else if (strcmp(pname, "mode") == 0) {
		ret = validation_mode(pparam, perr, size);
	} else if (strcmp(pname, "key") == 0) {
		ret = validation_key(pparam, perr, size);
	} else if (strcmp(pname, "iv") == 0) {
		ret = validation_iv(pparam, perr, size);
	} else {
		ret = 1;
		(void)fprintf(stderr, "%s is invalid parameter pname", pname);
	}

	return ret;
}
