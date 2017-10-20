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
#ifndef DOWNLOADAPI_H
#define DOWNLOADAPI_H

#include "downloader.h"


#define API_DIR 	"download-api/"	/* relative path for download-api */
#define API_DL_DIR	API_DIR "downloads/"	/* relative path for download files */
#define API_CTX_DIR	API_DIR "contexts/"	/* relative path for session contexts */
#define API_DIR_MAX	23	/*  max length of download-api directory */

extern int is_file(const char *filepath);
#endif
