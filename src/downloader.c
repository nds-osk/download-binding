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
#include <curl/curl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "download-api.h"
#include "downloader.h"

/* Function prototypes */
static long int get_filesize(FILE * pf);
static size_t write_data(const void *ptr, size_t size, size_t nmemb, const char *filepath);
static int xferinfo(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

static long int get_filesize(FILE * pf)
{
	(void)fseek(pf, 0, SEEK_END);
	return ftell(pf);
}

static size_t write_data(const void *ptr, size_t size, size_t nmemb, const char *filepath)
{
	FILE *pf = NULL;
	size_t written;

	if (filepath == NULL) {
		return (size_t) CURLE_WRITE_ERROR;
	}
	pf = fopen(filepath, "r");

	if (pf == NULL) {	/* file is no exist */
		return (size_t) CURLE_WRITE_ERROR;
	}
	(void)fclose(pf);

	/* write data */
	pf = fopen(filepath, "a");
	if (pf != NULL) {
		written = fwrite(ptr, size, nmemb, pf);
		(void)fclose(pf);

		if (written < nmemb) {
			return (size_t) CURLE_WRITE_ERROR;
		}
	} else {
		return (size_t) CURLE_WRITE_ERROR;
	}

	return written;
}

/* this is how the CURLOPT_XFERINFOFUNCTION callback works */
static int xferinfo(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
	DL_INFO *pdl = (DL_INFO *) p;
	curl_off_t wprogress;

	if (pdl->dliTotal == (curl_off_t) 0) {
		pdl->dliTotal = dltotal;
	}
	/* calculate the progress rete */
	if ((curl_off_t) 0 < dltotal) {
		wprogress = ((curl_off_t) 100 * (pdl->dliNow + dlnow)) / pdl->dliTotal;
		pdl->dliProg = (int)wprogress;
	}

	/* check request */
	if (pdl->dliReqStop == 1) {
		/* stop to download */
		return (int)CURLE_ABORTED_BY_CALLBACK;
	}

	return 0;
}

void *dl_download(void *p)
{
	CURL *pcurl = NULL;
	FILE *pf = NULL;
	CURLcode res;
	DL_INFO *pdl = (DL_INFO *) p;
	const char *purl = pdl->pdliUrl;
	const char *pfname = pdl->pdliFileName;
	long int filesize;
	char errbuf[CURL_ERROR_SIZE];
	size_t errlen;
	int ret;

	ret = pthread_mutex_lock(pdl->pdliMutex);
	if (ret != 0) {
		(void)fprintf(stderr, "dl_download():pthread_mutex_lock() failed\n");
		return NULL;
	}

	pdl->dliState = DL_STATE_RUNNING;
	pcurl = curl_easy_init();
	if (pfname != NULL) {
		pf = fopen(pfname, "a");	/* The file is created if it does not exist */
	}

	if ((pcurl != 0) && (pf != 0)) {
		/* get current file size */
		filesize = get_filesize(pf);
		pdl->dliNow = (curl_off_t) filesize;
		(void)fclose(pf);
		pf = NULL;

		(void)curl_easy_setopt(pcurl, CURLOPT_URL, purl);
		(void)curl_easy_setopt(pcurl, CURLOPT_WRITEFUNCTION, &write_data);
		(void)curl_easy_setopt(pcurl, CURLOPT_WRITEDATA, pfname);
		(void)curl_easy_setopt(pcurl, CURLOPT_RESUME_FROM_LARGE, filesize);
		(void)curl_easy_setopt(pcurl, CURLOPT_ERRORBUFFER, errbuf);
#if LIBCURL_VERSION_NUM >= 0x072000
		(void)curl_easy_setopt(pcurl, CURLOPT_XFERINFOFUNCTION, &xferinfo);
		(void)curl_easy_setopt(pcurl, CURLOPT_XFERINFODATA, pdl);
#endif
		(void)curl_easy_setopt(pcurl, CURLOPT_NOPROGRESS, 0L);
		(void)curl_easy_setopt(pcurl, CURLOPT_FAILONERROR, 1L);
		(void)curl_easy_setopt(pcurl, CURLOPT_MAX_RECV_SPEED_LARGE, pdl->dliMaxSpeed);

		res = curl_easy_perform(pcurl);
		if (res != CURLE_OK) {
			errlen = strlen(errbuf);
			(void)fprintf(stderr, "\nlibcurl: (%d) ", (int)res);
			if (errlen != (size_t) 0) {
				(void)fprintf(stderr, "%s%s", errbuf, ((errbuf[errlen - (size_t) 1] != '\n') ? "\n" : ""));
			} else {
				(void)fprintf(stderr, "%s\n", curl_easy_strerror(res));
			}
		}
	}

	if (pdl->dliReqStop == 1) {
		pdl->dliState = DL_STATE_PAUSE;
	} else if (res != CURLE_OK) {
		/* set error info */
		pdl->dliState = DL_STATE_ERROR;
		pdl->pdliErrType = "curl";
		pdl->dliErrCode = (int)res;
	} else {		/* finish to download */
		pdl->dliState = DL_STATE_DONE;
	}

	/* always cleanup */
	if (pcurl != 0) {
		curl_easy_cleanup(pcurl);
	}
	if (pf != 0) {
		(void)fclose(pf);
	}
	ret = pthread_mutex_unlock(pdl->pdliMutex);
	if (ret != 0) {
		(void)fprintf(stderr, "dl_download():pthread_mutex_unlock() failed\n");
		return NULL;
	}
}
