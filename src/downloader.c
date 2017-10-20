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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "download-api.h"
#include "downloader.h"

/* Function prototypes */
static off_t get_filesize(const char *pfname);
static size_t write_data(const void *ptr, size_t size, size_t nmemb, DL_INFO * pdl);
static int xferinfo(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

/* remove downloaded file */
int remove_dlfile(DL_INFO * pdl)
{
	int ret = 0;
	int isfile = 0;

	ret = pthread_mutex_lock(pdl->pdliMutex);
	if (ret != 0) {
		(void)fprintf(stderr, "remove_dlfile():pthread_mutex_lock() failed\n");
		return ret;
	}

	isfile = is_file(pdl->pdliFileName);
	if (isfile == 1) {
		ret = remove((const char *)pdl->pdliFileName);
		if (ret != 0) {
			pthread_mutex_unlock(pdl->pdliMutex);
			return ret;
		}
	}

	ret = pthread_mutex_unlock(pdl->pdliMutex);
	if (ret != 0) {
		(void)fprintf(stderr, "remove_dlfile():pthread_mutex_unlock() failed\n");
		return ret;
	}

	return ret;
}

static off_t get_filesize(const char *pfname)
{
	int fd;
	struct stat stbuf;

	fd = open(pfname, O_RDONLY);
	if (fd < 0) {
		perror("get_filesize() error");
		return -1;
	}

	if (fstat(fd, &stbuf) < 0) {
		perror("get_filesize() error");
		return -1;
	}

	if (close(fd) < 0) {
		perror("get_filesize() error");
		return -1;
	}

	return stbuf.st_size;
}

static size_t write_data(const void *ptr, size_t size, size_t nmemb, DL_INFO * pdl)
{
	FILE *pf = NULL;
	size_t written;
	const char *filepath = pdl->pdliFileName;
	off_t write_offset = pdl->dliWriteOffset;

	if (filepath == NULL) {
		return (size_t) CURLE_WRITE_ERROR;
	}
	pf = fopen(filepath, "r+");

	if (pf == NULL) {	/* file is no exist */
		return (size_t) CURLE_WRITE_ERROR;
	}

	/* move to writing start position */
	fseeko(pf, write_offset, SEEK_SET);

	written = fwrite(ptr, size, nmemb, pf);
	(void)fclose(pf);

	if (written < nmemb) {
		return (size_t) CURLE_WRITE_ERROR;
	}
	write_offset = write_offset + (size * nmemb);
	pdl->dliWriteOffset = write_offset;

	return written;
}

/* this is how the CURLOPT_XFERINFOFUNCTION callback works */
static int xferinfo(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
	DL_INFO *pdl = (DL_INFO *) p;
	curl_off_t wprogress;

	if (dltotal <= 0) {	/* for the first few times, total = 0 */
		return 0;
	}
	if (pdl->dliTotal < (curl_off_t) 0) {	/* no set ? */
		pdl->dliTotal = dltotal;
	}

	/* calculate the progress rete */
	pdl->dliSize = pdl->dliTotal - (dltotal - dlnow);
	wprogress = ((curl_off_t) 100 * pdl->dliSize) / pdl->dliTotal;
	pdl->dliProg = (int)wprogress;

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
	char errbuf[CURL_ERROR_SIZE];
	size_t errlen;
	int ret;
	long res_code;

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
		(void)fclose(pf);
		pf = NULL;

		pdl->dliWriteOffset = pdl->dliSize;
		(void)curl_easy_setopt(pcurl, CURLOPT_URL, purl);
		(void)curl_easy_setopt(pcurl, CURLOPT_WRITEFUNCTION, &write_data);
		(void)curl_easy_setopt(pcurl, CURLOPT_WRITEDATA, pdl);
		(void)curl_easy_setopt(pcurl, CURLOPT_RESUME_FROM_LARGE, pdl->dliWriteOffset);
		(void)curl_easy_setopt(pcurl, CURLOPT_ERRORBUFFER, errbuf);
#if LIBCURL_VERSION_NUM >= 0x072000
		(void)curl_easy_setopt(pcurl, CURLOPT_XFERINFOFUNCTION, &xferinfo);
		(void)curl_easy_setopt(pcurl, CURLOPT_XFERINFODATA, pdl);
#endif
		(void)curl_easy_setopt(pcurl, CURLOPT_NOPROGRESS, 0L);
		(void)curl_easy_setopt(pcurl, CURLOPT_FAILONERROR, 1L);
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
		/* get the last response code */
		(void)curl_easy_getinfo(pcurl, CURLINFO_RESPONSE_CODE, &res_code);
		pdl->dliResCode = res_code;
	}

	if (res == CURLE_OK) {
		/* in case of 0 byte file */
		if (pdl->dliTotal < (curl_off_t) 0) {	/* no set ? */
			pdl->dliTotal = (curl_off_t) 0;
			pdl->dliSize = (curl_off_t) 0;
			pdl->dliProg = 100;
		}

		pdl->dliState = DL_STATE_DONE;
	} else if (res == CURLE_ABORTED_BY_CALLBACK && pdl->dliReqStop == 1) {
		pdl->dliState = DL_STATE_PAUSE;
	} else {		/* ERROR */
		/* set error info */
		pdl->dliState = DL_STATE_ERROR;
		pdl->pdliErrType = "curl";
		pdl->dliErrCode = (int)res;
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
