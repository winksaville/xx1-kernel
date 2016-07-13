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

/**
 * A ac_msg_pool contains msgs which can be used for sending
 * information between components.
 */

#ifndef SADIE_LIBS_AC_COMP_MGR_INCS_AC_COMP_MGR_H
#define SADIE_LIBS_AC_COMP_MGR_INCS_AC_COMP_MGR_H

#include <ac_inttypes.h>
#include <ac_msg.h>

typedef struct AcCompMgr AcCompMgr;

/**
 * Forward declaration of an asynchronous component
 */
typedef struct AcComp AcComp;

/**
 * A opaque info for an AcComp
 */
typedef struct AcCompInfo AcCompInfo;

/**
 * Process a message.
 *
 * @return AC_FALSE if the message was NOT fully handled in which
 * case the parent component will be called, otherwise processing
 * is complete. TODO: Add support for Hierarchical Processing
 */
typedef ac_bool (*AcCompMsgProcessor)(AcComp* this, AcMsg* pmsg);

/**
 * An Asynchronouse Component
 */
typedef struct AcComp {
  ac_u8* name;                     // Name of component, must be unique
  AcCompMsgProcessor process_msg;  // Process a message
} AcComp;

/**
 * Add a component to be managed
 *
 * @param: comp is an initialized component type
 *
 * @return: AcCompInfo or AC_NULL if an error
 */
AcCompInfo* AcCompMgr_add_comp(AcCompMgr* mgr, AcComp* comp);

/**
 * Remove a component being managed
 *
 * @param: info an AcCompInfo returned by AcCompMgr_add_comp.
 *
 * @return: AcComp passed to AcCompMgr_add_comp.
 */
AcComp* AcCompMgr_rmv_comp(AcCompMgr* mgr, AcCompInfo* comp_info);

/**
 * Send a message to the comp
 */
void AcCompMgr_send_msg(AcCompMgr* mgr, AcCompInfo* info, AcMsg* msg);

/**
 * Deiniti AcCompMsg
 */
void AcCompMgr_deinit(AcCompMgr* mgr);

/**
 * Initialize the component manager, may only be called once.
 *
 * @param: max_component_threads is the maximum number of threads to manage
 * @param: max_components_thread is the maximum number of components per thread
 * @param: stack_size is number of bytes for a threads stack, 0 will provide the default
 *
 * @return: AcCompMgr* or AC_NULL if an error
 */
AcCompMgr* AcCompMgr_init(ac_u32 max_component_threads, ac_u32 max_components_per_thread,
    ac_u32 stack_size);

#endif
