/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef WRT_EXTENSION_EXTENSION_ADAPTER_H_
#define WRT_EXTENSION_EXTENSION_ADAPTER_H_

#include <map>

#include "extension/extension.h"
#include "extension/extension_instance.h"

#include "extension/xwalk/XW_Extension.h"
#include "extension/xwalk/XW_Extension_SyncMessage.h"
#include "extension/xwalk/XW_Extension_EntryPoints.h"
#include "extension/xwalk/XW_Extension_Runtime.h"
#include "extension/xwalk/XW_Extension_Permissions.h"

namespace wrt {

class ExtensionAdapter {
 public:
  typedef std::map<XW_Extension, Extension*> ExtensionMap;
  typedef std::map<XW_Instance, ExtensionInstance*> InstanceMap;

  static ExtensionAdapter* GetInstance();

  XW_Extension GetNextXWExtension();
  XW_Instance GetNextXWInstance();

  void RegisterExtension(Extension* extension);
  void UnregisterExtension(Extension* extension);

  void RegisterInstance(ExtensionInstance* instance);
  void UnregisterInstance(ExtensionInstance* instance);

  // Returns the correct struct according to interface asked. This is
  // passed to external extensions in XW_Initialize() call.
  static const void* GetInterface(const char* name);

 private:
  ExtensionAdapter();
  virtual ~ExtensionAdapter();

  static Extension* GetExtension(XW_Extension xw_extension);
  static ExtensionInstance* GetExtensionInstance(XW_Instance xw_instance);

  static void CoreSetExtensionName(XW_Extension xw_extension, const char* name);
  static void CoreSetJavaScriptAPI(XW_Extension xw_extension,
                                   const char* javascript_api);
  static void CoreRegisterInstanceCallbacks(XW_Extension xw_extension,
                                     XW_CreatedInstanceCallback created,
                                     XW_DestroyedInstanceCallback destroyed);
  static void CoreRegisterShutdownCallback(XW_Extension xw_extension,
                                    XW_ShutdownCallback shutdown);
  static void CoreSetInstanceData(XW_Instance xw_instance, void* data);
  static void* CoreGetInstanceData(XW_Instance xw_instance);
  static void MessagingRegister(XW_Extension xw_extension,
                                XW_HandleMessageCallback handle_message);
  static void MessagingPostMessage(XW_Instance xw_instance,
                                   const char* message);
  static void SyncMessagingRegister(XW_Extension xw_extension,
                             XW_HandleSyncMessageCallback handle_sync_message);
  static void SyncMessagingSetSyncReply(XW_Instance xw_instance,
                                        const char* reply);
  static void EntryPointsSetExtraJSEntryPoints(XW_Extension xw_extension,
                                               const char** entry_points);
  static void RuntimeGetStringVariable(XW_Extension xw_extension,
                                       const char* key,
                                       char* value,
                                       unsigned int value_len);
  static int PermissionsCheckAPIAccessControl(XW_Extension xw_extension,
                                              const char* api_name);
  static int PermissionsRegisterPermissions(XW_Extension xw_extension,
                                            const char* perm_table);

  ExtensionMap extension_map_;
  InstanceMap instance_map_;

  XW_Extension next_xw_extension_;
  XW_Instance next_xw_instance_;
};

}  // namespace wrt

#endif  // WRT_EXTENSION_EXTENSION_ADAPTER_H_
