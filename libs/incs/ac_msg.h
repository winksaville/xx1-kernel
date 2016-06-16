/*
 * Copyright 2016 Wink Saville
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

#ifndef SADIE_LIBS_INCS_AC_MSG_H
#define SADIE_LIBS_INCS_AC_MSG_H

#include <ac_inttypes.h>

typedef struct AcMsgPool AcMsgPool;

typedef struct AcMsg AcMsg;

typedef struct AcMsg {
  AcMsg *pnext;          // Next message
  AcMsgPool* pool;       // Pool that this message belongs too
  ac_u32 cmd;            // Command to perform
  ac_u32 arg;            // argument u32
  ac_u64 arg_u64;        // argument u64
} AcMsg;

#endif
