// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "extensions/renderer/xwalk_v8tools_module.h"

#include <v8.h>

#include "common/logger.h"

namespace extensions {

namespace {

void ForceSetPropertyCallback(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 3 || !info[0]->IsObject() || !info[1]->IsString()) {
    return;
  }
  info[0].As<v8::Object>()->ForceSet(info[1], info[2]);
}

void LifecycleTrackerCleanup(
    const v8::WeakCallbackData<v8::Object,
                               v8::Persistent<v8::Object> >& data) {
  v8::Isolate* isolate = data.GetIsolate();
  v8::HandleScope handle_scope(isolate);

  v8::Local<v8::Object> tracker = data.GetValue();
  v8::Handle<v8::Value> function =
      tracker->Get(v8::String::NewFromUtf8(isolate, "destructor"));

  if (function.IsEmpty() || !function->IsFunction()) {
    LOGGER(WARN) << "Destructor function not set for LifecycleTracker.";
    data.GetParameter()->Reset();
    delete data.GetParameter();
    return;
  }

  v8::Handle<v8::Context> context = v8::Context::New(isolate);

  v8::TryCatch try_catch;
  v8::Handle<v8::Function>::Cast(function)->Call(context->Global(), 0, NULL);
  if (try_catch.HasCaught())
    LOGGER(WARN) << "Exception when running LifecycleTracker destructor";

  data.GetParameter()->Reset();
  delete data.GetParameter();
}

void LifecycleTracker(const v8::FunctionCallbackInfo<v8::Value>& info) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  v8::Persistent<v8::Object>* tracker =
      new v8::Persistent<v8::Object>(isolate, v8::Object::New(isolate));
  tracker->SetWeak(tracker, &LifecycleTrackerCleanup);

  info.GetReturnValue().Set(*tracker);
}

}  // namespace

XWalkV8ToolsModule::XWalkV8ToolsModule() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::ObjectTemplate> object_template = v8::ObjectTemplate::New();

  // TODO(cmarcelo): Use Template::Set() function that takes isolate, once we
  // update the Chromium (and V8) version.
  object_template->Set(v8::String::NewFromUtf8(isolate, "forceSetProperty"),
                       v8::FunctionTemplate::New(
                          isolate, ForceSetPropertyCallback));
  object_template->Set(v8::String::NewFromUtf8(isolate, "lifecycleTracker"),
                       v8::FunctionTemplate::New(isolate, LifecycleTracker));

  object_template_.Reset(isolate, object_template);
}

XWalkV8ToolsModule::~XWalkV8ToolsModule() {
  object_template_.Reset();
}

v8::Handle<v8::Object> XWalkV8ToolsModule::NewInstance() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::EscapableHandleScope handle_scope(isolate);
  v8::Handle<v8::ObjectTemplate> object_template =
      v8::Local<v8::ObjectTemplate>::New(isolate, object_template_);
  return handle_scope.Escape(object_template->NewInstance());
}

}  // namespace extensions
