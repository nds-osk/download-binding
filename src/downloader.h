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
#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <curl/curl.h>
/*
 * state of download
 */
typedef enum {
	DL_STATE_PAUSE,		/* pause to download   */
	DL_STATE_RUNNING,	/* downloading         */
	DL_STATE_DONE,		/* downloaded          */
	DL_STATE_ERROR,		/* occur error         */
	DL_STATE_NUM		/* end mark */
} DL_STATE;

/*
 * definition of a downloading
 */
typedef struct {
	/* setting value */
	int dliId;		/* downloading id */
	char *pdliUrl;		/* URL of the downloading file */
	char *pdliFileName;	/* name of the downloading file */
	curl_off_t dliMaxSpeed;	/* max download speed (bytes per second) */

	/* status of downloader */
	DL_STATE dliState;	/* state of download  */
	int dliProg;		/* progress of the downloading [%] */
	curl_off_t dliTotal;	/* total size of file to be downloaded [byte] */
	curl_off_t dliNow;	/* size downloaded before resume() is called [byte] */
	char *pdliErrType;	/* error type */
	int dliErrCode;		/* error code */

	/* other */
	int dliReqStop;		/* request to stop download */
	pthread_mutex_t *pdliMutex;	/* for synchronization of download file and DL_INFO */
} DL_INFO;

extern void *dl_download(void *p);
#endif
