/*
 * Copyright 2015 Wink Saville
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

#ifndef SADIE_LIBS_AC_DISPATCHER_H
#define SADIE_LIBS_AC_DISPATCHER_H

#include <ac.h>

#include <ac_mpscfifo.h>

// The opaque ac_dipatcher
typedef struct _ac_dispatcher ac_dispatcher;

/**
 * Get a dispatcher able to support max_count ac's.
 */
ac_dispatcher* ac_dispatcher_get(ac_u32 max_count);

/**
 * Return dispatcher to the system.
 */
void ac_dispatcher_ret(ac_dispatcher* pd);

/**
 * Add the ac and its queue to this dispatcher.
 *
 * return the ac or AC_NULL if this ac was not added
 * this might occur if there are to many ac's registered
 * or the the ac has already been registered.
 */
ac* ac_dispatcher_add_acq(ac_dispatcher* pd, ac* pac, ac_mpscfifo* pq);

/**
 * Remove all instances of this ac assoicated with this dispatcher
 *
 * return the ac or AC_NULL if this ac was not added.
 */
ac* ac_dispatcher_rmv_ac(ac_dispatcher* pd, ac* pac);

#endif
