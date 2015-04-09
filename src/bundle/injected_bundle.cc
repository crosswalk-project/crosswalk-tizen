// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <v8.h>
#include <ewk_ipc_message.h>
#include <string>

extern "C" void DynamicSetWidgetInfo(int widget_id) {
}

extern "C" void DynamicPluginStartSession(int widget_id,
                                          v8::Handle<v8::Context> context,
                                          int routing_handle,
                                          double scale,
                                          const char* encoded_bundle,
                                          const char* theme,
                                          const char* base_url) {
}

extern "C" void DynamicPluginStopSession(
    int widget_id, v8::Handle<v8::Context> context) {
}

extern "C" void DynamicUrlParsing(
    std::string* old_url, std::string* new_url, int widget_id) {
}

extern "C" void DynamicDatabaseAttach(int attach) {
}

extern "C" void DynamicOnIPCMessage(const Ewk_IPC_Wrt_Message_Data& data) {
}

extern "C" void DynamicPreloading() {
}
