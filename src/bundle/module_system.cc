// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bundle/module_system.h"

#include <v8/v8.h>
#include <algorithm>

#include "common/logger.h"
#include "bundle/extension_module.h"

namespace wrt {

namespace {

// Index used to set embedder data into v8::Context, so we can get from a
// context to its corresponding module. Index chosen to not conflict with
// WebCore::V8ContextEmbedderDataField in V8PerContextData.h.
const int kModuleSystemEmbedderDataIndex = 8;

// This is the key used in the data object passed to our callbacks to store a
// pointer back to XWalkExtensionModule.
const char* kWrtModuleSystem = "kWrtModuleSystem";

}  // namespace

ModuleSystem::ModuleSystem(v8::Handle<v8::Context> context) {
  v8::Isolate* isolate = context->GetIsolate();
  v8_context_.Reset(isolate, context);

  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::Object> function_data = v8::Object::New(isolate);
  function_data->Set(v8::String::NewFromUtf8(isolate, kWrtModuleSystem),
                     v8::External::New(isolate, this));

  function_data_.Reset(isolate, function_data);
}

ModuleSystem::~ModuleSystem() {
  DeleteExtensionModules();

  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);

  function_data_.Reset();
  v8_context_.Reset();
}

// static
ModuleSystem* ModuleSystem::GetModuleSystemFromContext(
    v8::Handle<v8::Context> context) {
  return reinterpret_cast<ModuleSystem*>(
      context->GetAlignedPointerFromEmbedderData(
          kModuleSystemEmbedderDataIndex));
}

// static
void ModuleSystem::SetModuleSystemInContext(
    std::unique_ptr<ModuleSystem> module_system,
    v8::Handle<v8::Context> context) {
  context->SetAlignedPointerInEmbedderData(kModuleSystemEmbedderDataIndex,
                                           module_system.release());
}

// static
void ModuleSystem::ResetModuleSystemFromContext(
    v8::Handle<v8::Context> context) {
  delete GetModuleSystemFromContext(context);
  SetModuleSystemInContext(std::unique_ptr<ModuleSystem>(), context);
}

void ModuleSystem::RegisterExtensionModule(
    std::unique_ptr<ExtensionModule> module,
    const std::vector<std::string>& entry_points) {
  const std::string& extension_name = module->extension_name();
  if (ContainsEntryPoint(extension_name)) {
    LOGGER(ERROR) << "Can't register Extension Module named for extension '"
                  << extension_name << "' in the Module System because name "
                  << " was already registered.";
    return;
  }

  std::vector<std::string>::const_iterator it = entry_points.begin();
  for (; it != entry_points.end(); ++it) {
    if (ContainsEntryPoint(*it)) {
      LOGGER(ERROR) << "Can't register Extension Module named for extension '"
                    << extension_name << "' in the Module System because "
                    << "another extension has the entry point '"
                    << (*it) << "'.";
      return;
    }
  }

  extension_modules_.push_back(
      ExtensionModuleEntry(extension_name, module.release(), entry_points));
}

namespace {

v8::Handle<v8::Value> EnsureTargetObjectForTrampoline(
    v8::Handle<v8::Context> context, const std::vector<std::string>& path,
    std::string* error) {
  v8::Handle<v8::Object> object = context->Global();
  v8::Isolate* isolate = context->GetIsolate();

  std::vector<std::string>::const_iterator it = path.begin();
  for (; it != path.end(); ++it) {
    v8::Handle<v8::String> part =
        v8::String::NewFromUtf8(isolate, it->c_str());
    v8::Handle<v8::Value> value = object->Get(part);

    if (value->IsUndefined()) {
      v8::Handle<v8::Object> next_object = v8::Object::New(isolate);
      object->Set(part, next_object);
      object = next_object;
      continue;
    }

    if (!value->IsObject()) {
      *error = "the property '" + *it + "' in the path is undefined";
      return v8::Undefined(isolate);
    }

    object = value.As<v8::Object>();
  }
  return object;
}

v8::Handle<v8::Value> GetObjectForPath(v8::Handle<v8::Context> context,
                                       const std::vector<std::string>& path,
                                       std::string* error) {
  v8::Handle<v8::Object> object = context->Global();
  v8::Isolate* isolate = context->GetIsolate();

  std::vector<std::string>::const_iterator it = path.begin();
  for (; it != path.end(); ++it) {
    v8::Handle<v8::String> part =
        v8::String::NewFromUtf8(isolate, it->c_str());
    v8::Handle<v8::Value> value = object->Get(part);

    if (!value->IsObject()) {
      *error = "the property '" + *it + "' in the path is undefined";
      return v8::Undefined(isolate);
    }

    object = value.As<v8::Object>();
  }
  return object;
}

}  // namespace

template <typename STR>
void SplitString(const STR& str, const typename STR::value_type s,
                 std::vector<STR>* r) {
  r->clear();
  size_t last = 0;
  size_t c = str.size();
  for (size_t i = 0; i <= c; ++i) {
    if (i == c || str[i] == s) {
      STR tmp(str, last, i - last);
      if (i != c || !r->empty() || !tmp.empty())
        r->push_back(tmp);
      last = i + 1;
    }
  }
}

bool ModuleSystem::SetTrampolineAccessorForEntryPoint(
    v8::Handle<v8::Context> context,
    const std::string& entry_point,
    v8::Local<v8::External> user_data) {
  std::vector<std::string> path;
  SplitString(entry_point, '.', &path);
  std::string basename = path.back();
  path.pop_back();

  std::string error;
  v8::Handle<v8::Value> value =
      EnsureTargetObjectForTrampoline(context, path, &error);
  if (value->IsUndefined()) {
    LOGGER(ERROR) << "Error installing trampoline for " << entry_point
                  << " : " << error;
    return false;
  }

  v8::Isolate* isolate = context->GetIsolate();
  v8::Local<v8::Array> params = v8::Array::New(isolate);
  v8::Local<v8::String> entry =
      v8::String::NewFromUtf8(isolate, entry_point.c_str());
  params->Set(v8::Integer::New(isolate, 0), user_data);
  params->Set(v8::Integer::New(isolate, 1), entry);

  // FIXME(cmarcelo): ensure that trampoline is readonly.
  value.As<v8::Object>()->SetAccessor(
      v8::String::NewFromUtf8(isolate, basename.c_str()),
      TrampolineCallback, TrampolineSetterCallback, params);
  return true;
}

// static
bool ModuleSystem::DeleteAccessorForEntryPoint(
    v8::Handle<v8::Context> context,
    const std::string& entry_point) {
  std::vector<std::string> path;
  SplitString(entry_point, '.', &path);
  std::string basename = path.back();
  path.pop_back();

  std::string error;
  v8::Handle<v8::Value> value = GetObjectForPath(context, path, &error);
  if (value->IsUndefined()) {
    LOGGER(ERROR) << "Error retrieving object for " << entry_point
                  << " : " << error;
    return false;
  }

  value.As<v8::Object>()->ForceDelete(
      v8::String::NewFromUtf8(context->GetIsolate(), basename.c_str()));
  return true;
}

bool ModuleSystem::InstallTrampoline(v8::Handle<v8::Context> context,
                                     ExtensionModuleEntry* entry) {
  v8::Local<v8::External> entry_ptr =
      v8::External::New(context->GetIsolate(), entry);
  bool ret = SetTrampolineAccessorForEntryPoint(context, entry->name,
                                                entry_ptr);
  if (!ret) {
    LOGGER(ERROR) << "Error installing trampoline for " << entry->name;
    return false;
  }

  auto it = entry->entry_points.begin();
  for (; it != entry->entry_points.end(); ++it) {
    ret = SetTrampolineAccessorForEntryPoint(context, *it, entry_ptr);
    if (!ret) {
      // TODO(vcgomes): Remove already added trampolines when it fails.
      LOGGER(ERROR) << "Error installing trampoline for " << entry->name;
      return false;
    }
  }
  return true;
}

void ModuleSystem::Initialize() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  v8::Handle<v8::Context> context = GetV8Context();

  MarkModulesWithTrampoline();

  auto it = extension_modules_.begin();
  for (; it != extension_modules_.end(); ++it) {
    if (it->use_trampoline && InstallTrampoline(context, &*it))
      continue;
    it->module->LoadExtensionCode(context);
    EnsureExtensionNamespaceIsReadOnly(context, it->name);
  }
}

v8::Handle<v8::Context> ModuleSystem::GetV8Context() {
  return v8::Local<v8::Context>::New(v8::Isolate::GetCurrent(), v8_context_);
}

bool ModuleSystem::ContainsEntryPoint(
    const std::string& entry) {
  auto it = extension_modules_.begin();
  for (; it != extension_modules_.end(); ++it) {
    if (it->name == entry)
      return true;

    auto entry_it = std::find(
        it->entry_points.begin(), it->entry_points.end(), entry);
    if (entry_it != it->entry_points.end()) {
      return true;
    }
  }
  return false;
}

void ModuleSystem::DeleteExtensionModules() {
  for (ExtensionModules::iterator it = extension_modules_.begin();
       it != extension_modules_.end(); ++it) {
    delete it->module;
  }
  extension_modules_.clear();
}

// static
void ModuleSystem::LoadExtensionForTrampoline(
    v8::Isolate* isolate,
    v8::Local<v8::Value> data) {
  v8::Local<v8::Array> params = data.As<v8::Array>();
  void* ptr = params->Get(
      v8::Integer::New(isolate, 0)).As<v8::External>()->Value();

  ExtensionModuleEntry* entry = static_cast<ExtensionModuleEntry*>(ptr);

  if (!entry)
    return;

  v8::Handle<v8::Context> context = isolate->GetCurrentContext();

  DeleteAccessorForEntryPoint(context, entry->name);

  auto it = entry->entry_points.begin();
  for (; it != entry->entry_points.end(); ++it) {
    DeleteAccessorForEntryPoint(context, *it);
  }

  ModuleSystem* module_system = GetModuleSystemFromContext(context);

  ExtensionModule* module = entry->module;
  module->LoadExtensionCode(module_system->GetV8Context());

  module_system->EnsureExtensionNamespaceIsReadOnly(context, entry->name);
}

// static
v8::Handle<v8::Value> ModuleSystem::RefetchHolder(
    v8::Isolate* isolate,
    v8::Local<v8::Value> data) {
  v8::Local<v8::Array> params = data.As<v8::Array>();
  const std::string entry_point = *v8::String::Utf8Value(
      params->Get(v8::Integer::New(isolate, 1)).As<v8::String>());

  std::vector<std::string> path;
  SplitString(entry_point, '.', &path);
  path.pop_back();

  std::string error;
  return GetObjectForPath(isolate->GetCurrentContext(), path, &error);
}

// static
void ModuleSystem::TrampolineCallback(
    v8::Local<v8::String> property,
    const v8::PropertyCallbackInfo<v8::Value>& info) {
  ModuleSystem::LoadExtensionForTrampoline(info.GetIsolate(), info.Data());
  v8::Handle<v8::Value> holder = RefetchHolder(info.GetIsolate(), info.Data());
  if (holder->IsUndefined())
    return;

  info.GetReturnValue().Set(holder.As<v8::Object>()->Get(property));
}

// static
void ModuleSystem::TrampolineSetterCallback(
    v8::Local<v8::String> property,
    v8::Local<v8::Value> value,
    const v8::PropertyCallbackInfo<void>& info) {
  ModuleSystem::LoadExtensionForTrampoline(info.GetIsolate(), info.Data());
  v8::Handle<v8::Value> holder = RefetchHolder(info.GetIsolate(), info.Data());
  if (holder->IsUndefined())
    return;

  holder.As<v8::Object>()->Set(property, value);
}

ModuleSystem::ExtensionModuleEntry::ExtensionModuleEntry(
    const std::string& name,
    ExtensionModule* module,
    const std::vector<std::string>& entry_points) :
    name(name), module(module), use_trampoline(true),
    entry_points(entry_points) {
}

ModuleSystem::ExtensionModuleEntry::~ExtensionModuleEntry() {
}

// Returns whether the name of first is prefix of the second, considering "."
// character as a separator. So "a" is prefix of "a.b" but not of "ab".
bool ModuleSystem::ExtensionModuleEntry::IsPrefix(
    const ExtensionModuleEntry& first,
    const ExtensionModuleEntry& second) {
  const std::string& p = first.name;
  const std::string& s = second.name;
  return s.size() > p.size() && s[p.size()] == '.'
      && std::mismatch(p.begin(), p.end(), s.begin()).first == p.end();
}

// Mark the extension modules that we want to setup "trampolines"
// instead of loading the code directly. The current algorithm is very
// simple: we only create trampolines for extensions that are leaves
// in the namespace tree.
//
// For example, if there are two extensions "tizen" and "tizen.time",
// the first one won't be marked with trampoline, but the second one
// will. So we'll only load code for "tizen" extension.
void ModuleSystem::MarkModulesWithTrampoline() {
  std::sort(extension_modules_.begin(), extension_modules_.end());

  ExtensionModules::iterator it = extension_modules_.begin();
  while (it != extension_modules_.end()) {
    it = std::adjacent_find(it, extension_modules_.end(),
                            &ExtensionModuleEntry::IsPrefix);
    if (it == extension_modules_.end())
      break;
    it->use_trampoline = false;
    ++it;
  }
}

void ModuleSystem::EnsureExtensionNamespaceIsReadOnly(
    v8::Handle<v8::Context> context,
    const std::string& extension_name) {
  std::vector<std::string> path;
  SplitString(extension_name, '.', &path);
  std::string basename = path.back();
  path.pop_back();

  std::string error;
  v8::Handle<v8::Value> value = GetObjectForPath(context, path, &error);
  if (value->IsUndefined()) {
    LOGGER(ERROR) << "Error retrieving object for " << extension_name << " : "
                  << error;
    return;
  }

  v8::Handle<v8::String> v8_extension_name(
      v8::String::NewFromUtf8(context->GetIsolate(), basename.c_str()));
  value.As<v8::Object>()->ForceSet(
      v8_extension_name, value.As<v8::Object>()->Get(v8_extension_name),
      v8::ReadOnly);
}

}  // namespace wrt
