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
#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

#include <afb/afb-binding.h>

#include <unistd.h>
#include <pthread.h>
#include <glib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "download-api.h"
#include "downloader.h"
#include "validation.h"
/*
 * the interface to afb-daemon
 */
static const struct afb_binding_interface *pitf;


/*
 * definition of a download
 */
typedef struct {
	GSList *pdlcList;
} DL_CTX;

/* Function prototypes */
static void make_dir(const char *pdir);
static void init_dirs(void);
static int search_downloading(const DL_INFO * pdl, const int *pid);
static DL_INFO *find_downloading(DL_CTX * pctx, int id);
static DL_INFO *get_new_downloading(DL_CTX * pctx);
static void release_downloading(DL_INFO * pdl);
static void release_download(DL_CTX * pctx);
static DL_CTX *get_new_download(void);
static DL_CTX *download_of_req(struct afb_req req);
static struct json_object *describe_downloading(const DL_INFO * pdl);
static struct json_object *describe(DL_CTX * pctx, int id);
static int start_download(struct afb_req req, DL_INFO * pdl);
static int remove_dlinfo(DL_CTX * pctx, DL_INFO * pdl);
void get_dl_pathname(char *ppath, size_t size, const char *pfname);
static void cb_download(struct afb_req req);
static void cb_info(struct afb_req req);
static void cb_stop(struct afb_req req);
static void cb_resume(struct afb_req req);
static void cb_cancel(struct afb_req req);
static void cb_delete(struct afb_req req);

/*
 * make directory
 */
static void make_dir(const char *pdir)
{
	int ret;

	if (pdir == NULL) {
		(void)fprintf(stderr, "make_dir():argument is NULL\n");
		return;
	}
	ret = mkdir(pdir, ACCESSPERMS);
	if (ret == -1) {
		if (errno != EEXIST) {
			perror("make_dir():mkdir() error");
		}
	}
}

/*
 * init directory
 */
static void init_dirs(void)
{
	make_dir(API_DIR);
	make_dir(API_DL_DIR);
	make_dir(API_CTX_DIR);
}

/*
 * Searchs a downloading having the 'id'.
 * Returns it if found or NULL otherwise.
 */
static int search_downloading(const DL_INFO * pdl, const int *pid)
{
	if (pdl == NULL || pid == NULL) {
		return 1;
	}
	if (pdl->dliId == *pid) {
		return 0;
	}
	return 1;
}

/*
 * find a downloading having the 'id'.
 */
static DL_INFO *find_downloading(DL_CTX * pctx, int id)
{
	GSList *plist;
	DL_INFO *pdl = NULL;

	plist = g_slist_find_custom(pctx->pdlcList, &id, (GCompareFunc) & search_downloading);
	if (plist != NULL) {
		pdl = (DL_INFO *) plist->data;
	}
	return pdl;
}

/*
 * Create a new downloading and returns it.
 */
static DL_INFO *get_new_downloading(DL_CTX * pctx)
{
	int ret;
	int id;

	/* allocation */
	DL_INFO *pdl = calloc((size_t) 1, sizeof *pdl);

	/* initialisation */
	pdl->dliState = DL_STATE_RUNNING;
	pdl->dliProg = 0;
	pdl->dliReqStop = 0;
	pdl->dliTotal = (curl_off_t) - 1;
	pdl->dliSize = (curl_off_t) 0;
	pdl->dliResCode = 0;
	pdl->pdliMutex = calloc((size_t) 1, sizeof(pthread_mutex_t));
	ret = pthread_mutex_init(pdl->pdliMutex, NULL);
	if (ret != 0) {
		(void)fprintf(stderr, "get_new_downloading():pthread_mutex_init() failed\n");
		release_downloading(pdl);
		return NULL;
	}
	/* set default option */
	pdl->pdliUrl = NULL;
	pdl->pdliFileName = NULL;

	do {
		id = (rand() >> 2) % 1000;
	} while ((g_slist_find_custom(pctx->pdlcList, &id, (GCompareFunc) & search_downloading) != NULL) || (id == 0));
	pdl->dliId = id;

	/* link */
	pctx->pdlcList = g_slist_append(pctx->pdlcList, pdl);

	return pdl;
}

/*
 * Release a download
 */
static void release_downloading(DL_INFO * pdl)
{
	free(pdl->pdliUrl);
	free(pdl->pdliFileName);
	free(pdl->pdliMutex);
	free(pdl);
}

static void release_download(DL_CTX * pctx)
{
	(void)g_slist_free_full(pctx->pdlcList, (GDestroyNotify) & release_downloading);
	(void)g_slist_free(pctx->pdlcList);
	free(pctx);
}

/*
 * Creates a new download and returns it.
 */
static DL_CTX *get_new_download(void)
{
	/* allocation */
	DL_CTX *pctx = calloc((size_t) 1, sizeof *pctx);

	return pctx;
}

/*
 * retrieves the download of the request
 */
static DL_CTX *download_of_req(struct afb_req req)
{
	DL_CTX *pctx;

	pctx = afb_req_context(req, (void *)&get_new_download, (void *)&release_download);
	return pctx;
}

/*
 * get the downloading description
 */
static struct json_object *describe_downloading(const DL_INFO * pdl)
{
	struct json_object *parr;

	parr = json_object_new_object();
	(void)json_object_object_add(parr, "id", json_object_new_int(pdl->dliId));
	(void)json_object_object_add(parr, "state", json_object_new_int((int)pdl->dliState));
	(void)json_object_object_add(parr, "progress", json_object_new_int(pdl->dliProg));
	(void)json_object_object_add(parr, "url", json_object_new_string(pdl->pdliUrl));
	(void)json_object_object_add(parr, "filename", json_object_new_string(pdl->pdliFileName));
	(void)json_object_object_add(parr, "size_total", json_object_new_int64(pdl->dliTotal));
	(void)json_object_object_add(parr, "size", json_object_new_int64(pdl->dliSize));
	if (pdl->dliState == DL_STATE_ERROR) {
		(void)json_object_object_add(parr, "error_type", json_object_new_string(pdl->pdliErrType));
		(void)json_object_object_add(parr, "error_code", json_object_new_int((int)pdl->dliErrCode));
	}
	if (pdl->dliState == DL_STATE_DONE || pdl->dliState == DL_STATE_ERROR) {
		(void)json_object_object_add(parr, "response_code", json_object_new_int((int)pdl->dliResCode));
	}
	return parr;
}

/*
 * get the download description
 */
static struct json_object *describe(DL_CTX * pctx, int id)
{
	struct json_object *presu, *parr;
	DL_INFO *pdl;
	GSList *plist;

	presu = json_object_new_object();

	/* downloading for id */
	if (id > 0) {
		plist = g_slist_find_custom(pctx->pdlcList, &id, (GCompareFunc) & search_downloading);
		if (plist != 0) {
			pdl = (DL_INFO *) plist->data;
			(void)json_object_object_add(presu, "downloading", describe_downloading(pdl));
		}
		/* all downloading */
	} else {
		plist = pctx->pdlcList;
		parr = json_object_new_array();

		while (plist != NULL) {
			pdl = (DL_INFO *) plist->data;
			(void)json_object_array_add(parr, describe_downloading(pdl));
			plist = g_slist_next(plist);
		}
		(void)json_object_object_add(presu, "downloadings", parr);
	}

	return presu;
}

/*
 * start to download
 */
static int start_download(struct afb_req req, DL_INFO * pdl)
{
	pthread_t thread_id;
	char buf[1024];
	int ret = 0;		/* error number (success:0) */

	pdl->dliReqStop = 0;
	ret = pthread_create(&thread_id, NULL, &dl_download, (void *)pdl);
	if (ret != 0) {
		(void)strerror_r(ret, buf, sizeof(buf));
		(void)afb_req_fail_f(req, "failed", "pthread_create : %s\n", buf);
		return ret;
	}

	ret = pthread_detach(thread_id);
	if (ret != 0) {
		(void)strerror_r(ret, buf, sizeof(buf));
		(void)afb_req_fail_f(req, "failed", "pthread_detach : %s\n", buf);
		return ret;
	}

	return ret;
}

/*
 * is file existed
 */
int is_file(const char *filepath)
{
	FILE *pf = NULL;
	int isfile = 0;

	pf = fopen(filepath, "r");
	if (pf != NULL) {
		isfile = 1;
		(void)fclose(pf);
	}

	return isfile;
}

/* remove downloaded file and information */
static int remove_dlinfo(DL_CTX * pctx, DL_INFO * pdl)
{
	int ret;

	ret = remove_dlfile(pdl);
	if (ret != 0) {
		return 1;
	}
	release_downloading(pdl);
	pctx->pdlcList = g_slist_remove(pctx->pdlcList, pdl);

	return 0;
}

/* get download file path name */
void get_dl_pathname(char *ppath, size_t size, const char *pfname)
{
	if (ppath == NULL || pfname == NULL) {
		return;
	}
	(void)snprintf(ppath, size, "%s%s", API_DL_DIR, pfname);
}


/*
 * download
 */
static void cb_download(struct afb_req req)
{
	DL_CTX *pctx;
	DL_INFO *pdl;
	struct json_object *pjresp;
	pthread_t thread_id;
	int ret;
	char buf[1024];
	char err[128];
	const char *purl, *pfile;
	size_t pathsize;
	char pfpath[API_DIR_MAX + DLAPI_FILENAME_MAX + 1];

	INFO(pitf, "method 'download' called");

	/* retrieves the arguments */
	purl = afb_req_value(req, "url");
	pfile = afb_req_value(req, "filename");

	/* validation */
	if (validation("url", purl, err, sizeof(err)) == 1) {
		(void)afb_req_fail(req, "failed", (const char *)err);
		return;
	}
	if (validation("filename", pfile, err, sizeof(err)) == 1) {
		(void)afb_req_fail(req, "failed", (const char *)err);
		return;
	}

	/* file check */
	get_dl_pathname(pfpath, sizeof(pfpath), pfile);
	if (is_file(pfpath) == 1) {
		(void)afb_req_fail(req, "failed", "filename already exists");
		return;
	}

	/* retrieves the context for the session */
	pctx = download_of_req(req);

	/* make new download */
	pdl = get_new_downloading(pctx);

	/* set required parameters */
	pdl->pdliUrl = strdup(purl);
	pdl->pdliFileName = strdup(pfpath);

	/* start download */
	if (start_download(req, pdl) != 0) {
		return;
	}
	INFO(pitf, "start download id=%d", pdl->dliId);

	/* make response */
	pjresp = json_object_new_object();
	(void)json_object_object_add(pjresp, "id", json_object_new_int(pdl->dliId));

	/* send response */
	(void)afb_req_success(req, pjresp, NULL);
}

/*
 * info
 */
static void cb_info(struct afb_req req)
{
	DL_CTX *pctx;
	struct json_object *pjresp;
	const char *pid;
	char err[128];
	int iid;

	INFO(pitf, "method 'info' called");

	/* retrieves the arguments */
	pid = afb_req_value(req, "id");

	/* validation */
	if (pid != NULL) {	/* option */
		if (validation("id", pid, err, sizeof(err)) == 1) {
			(void)afb_req_fail(req, "failed", (const char *)err);
			return;
		}
	}

	/* retrieves the context for the session */
	pctx = download_of_req(req);

	/* find a downloading */
	if (pid != NULL) {
		iid = atoi(pid);
		if (find_downloading(pctx, iid) == NULL) {
			(void)afb_req_fail_f(req, "failed", "id=%d is no exist", iid);
			return;
		}
	}

	/* describe the download */
	if (pid != NULL) {
		pjresp = describe(pctx, iid);
	} else {
		pjresp = describe(pctx, 0);
	}

	/* send the download's description */
	(void)afb_req_success(req, pjresp, NULL);
}

/*
 * stop
 */
static void cb_stop(struct afb_req req)
{
	DL_CTX *pctx;
	DL_INFO *pdl;
	const char *pid;
	int iId;
	char err[128];

	INFO(pitf, "method 'stop' called");

	/* retrieves the arguments */
	pid = afb_req_value(req, "id");

	/* validation */
	if (validation("id", pid, err, sizeof(err)) == 1) {
		(void)afb_req_fail(req, "failed", (const char *)err);
		return;
	}

	/* retrieves the context for the session */
	pctx = download_of_req(req);


	/* find a downloading */
	iId = atoi(pid);
	pdl = find_downloading(pctx, iId);
	if (pdl == NULL) {
		(void)afb_req_fail_f(req, "failed", "id=%d is no exist", iId);
		return;
	}

	/* check state */
	if (pdl->dliState != DL_STATE_RUNNING) {
		(void)afb_req_fail(req, "failed", "must call when state is 1:running");
		return;
	}

	/* stop to download */
	pdl->dliReqStop = 1;

	/* send response */
	(void)afb_req_success(req, NULL, NULL);
}

/*
 * resume
 */
static void cb_resume(struct afb_req req)
{
	DL_CTX *pctx;
	DL_INFO *pdl;
	const char *pid;
	int iid;
	char err[128];

	INFO(pitf, "method 'resume' called");

	/* retrieves the arguments */
	pid = afb_req_value(req, "id");

	/* validation */
	if (validation("id", pid, err, sizeof(err)) == 1) {
		(void)afb_req_fail(req, "failed", (const char *)err);
		return;
	}

	/* retrieves the context for the session */
	pctx = download_of_req(req);


	/* find a downloading */
	iid = atoi(pid);
	pdl = find_downloading(pctx, iid);
	if (pdl == NULL) {
		(void)afb_req_fail_f(req, "failed", "id=%d is no exist", iid);
		return;
	}

	/* check state */
	if (pdl->dliState != DL_STATE_PAUSE) {
		(void)afb_req_fail(req, "failed", "must call when state is 0:pause");
		return;
	}

	/* resume to download */
	if (start_download(req, pdl) != 0) {
		return;
	}

	/* send response */
	(void)afb_req_success(req, NULL, NULL);
}

/*
 * cancel
 */
static void cb_cancel(struct afb_req req)
{
	DL_CTX *pctx;
	DL_INFO *pdl;
	const char *pid;
	int iid;
	char err[128];
	int ret;

	INFO(pitf, "method 'cancel' called");

	/* retrieves the arguments */
	pid = afb_req_value(req, "id");

	/* validation */
	if (validation("id", pid, err, sizeof(err)) == 1) {
		(void)afb_req_fail(req, "failed", (const char *)err);
		return;
	}

	/* retrieves the context for the session */
	pctx = download_of_req(req);


	/* find a downloading */
	iid = atoi(pid);
	pdl = find_downloading(pctx, iid);
	if (pdl == NULL) {
		(void)afb_req_fail_f(req, "failed", "id=%d is no exist", iid);
		return;
	}

	/* check state */
	if (pdl->dliState > DL_STATE_RUNNING) {
		(void)afb_req_fail(req, "failed", "must call when state is 0:pause or 1:running");
		return;
	}

	/* stop to download */
	pdl->dliReqStop = 1;

	/* remove file */
	ret = remove_dlinfo(pctx, pdl);
	if (ret != 0) {
		(void)afb_req_fail(req, "failed", "failed to delete file");
		return;
	}
	/* send response */
	(void)afb_req_success(req, NULL, NULL);
}

/*
 * delete
 */
static void cb_delete(struct afb_req req)
{
	DL_CTX *pctx;
	DL_INFO *pdl;
	const char *pid;
	int iid;
	char err[128];
	int ret;

	INFO(pitf, "method 'delete' called");

	/* retrieves the arguments */
	pid = afb_req_value(req, "id");

	/* validation */
	if (validation("id", pid, err, sizeof(err)) == 1) {
		(void)afb_req_fail(req, "failed", (const char *)err);
		return;
	}

	/* retrieves the context for the session */
	pctx = download_of_req(req);


	/* find a downloading */
	iid = atoi(pid);
	pdl = find_downloading(pctx, iid);
	if (pdl == NULL) {
		(void)afb_req_fail_f(req, "failed", "id=%d is no exist", iid);
		return;
	}

	/* check state */
	if (pdl->dliState < DL_STATE_DONE) {
		(void)afb_req_fail(req, "failed", "must call when state is 2:done or 3:error");
		return;
	}

	/* remove file */
	ret = remove_dlinfo(pctx, pdl);
	if (ret != 0) {
		(void)afb_req_fail(req, "failed", "failed to delete file");
		return;
	}
	/* send response */
	(void)afb_req_success(req, NULL, NULL);
}

/*
 * array of the verbs exported to afb-daemon
 */
static const struct afb_verb_desc_v1 binding_verbs[] = {

	/* *INDENT-OFF* */
	{"download" ,AFB_SESSION_CHECK ,&cb_download ,"download a file"            },
	{"info"     ,AFB_SESSION_CHECK ,&cb_info     ,"get info of the downloading"},
	{"stop"     ,AFB_SESSION_CHECK ,&cb_stop     ,"stop downloading"           },
	{"resume"   ,AFB_SESSION_CHECK ,&cb_resume   ,"resume downloading"         },
	{"cancel"   ,AFB_SESSION_CHECK ,&cb_cancel   ,"cancel downloading"         },
	{"delete"   ,AFB_SESSION_CHECK ,&cb_delete   ,"delete a file downloaded"   },
	{NULL}			/* marker for end of the array */
	/* *INDENT-ON* */
};

/*
 * description of the binding for afb-daemon
 */
static const struct afb_binding binding_description = {
	/* description conforms to VERSION 1 */
	.type = AFB_BINDING_VERSION_1,
	.v1 = {			/* fills the v1 field of the union when AFB_BINDING_VERSION_1 */
	       .prefix = "download",	/* the API name (or binding name or prefix) */
	       .info = "download file",	/* short description of of the binding */
	       .verbs = binding_verbs	/* the array describing the verbs of the API */
	       }
};

/*
 * activation function for registering the binding called by afb-daemon
 */
const struct afb_binding *afbBindingV1Register(const struct
					       afb_binding_interface *interface)
{
	pitf = interface;	/* records the interface for accessing afb-daemon */

	init_dirs();		/* init directories */

	return &binding_description;	/* returns the description of the binding */
}
