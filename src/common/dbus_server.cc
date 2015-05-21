// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/dbus_server.h"

#include "common/logger.h"

namespace wrt {

namespace {

static void OnMethodCall(GDBusConnection* connection,
                         const gchar* /*sender*/,
                         const gchar* /*object_path*/,
                         const gchar* interface_name,
                         const gchar* method_name,
                         GVariant* parameters,
                         GDBusMethodInvocation* invocation,
                         gpointer user_data) {
  DBusServer* self = reinterpret_cast<DBusServer*>(user_data);
  if (!self) {
    LOGGER(ERROR) << "DBusServer is NULL.";
    return;
  }
  auto callback = self->GetMethodCallback(interface_name);
  if (callback) {
    callback(connection, method_name, parameters, invocation);
  }
}

static GVariant* OnGetProperty(GDBusConnection* connection,
                               const gchar* /*sender*/,
                               const gchar* /*object_path*/,
                               const gchar* interface_name,
                               const gchar* property_name,
                               GError** /*error*/,
                               gpointer user_data) {
  DBusServer* self = reinterpret_cast<DBusServer*>(user_data);
  if (!self) {
    LOGGER(ERROR) << "DBusServer is NULL.";
    return NULL;
  }

  auto callback =
      self->GetPropertyGetter(interface_name);

  GVariant* ret = NULL;
  if (callback) {
    ret = callback(connection, property_name);
  }

  return ret;
}

static gboolean OnSetProperty(GDBusConnection* connection,
                              const gchar* /*sender*/,
                              const gchar* /*object_path*/,
                              const gchar* interface_name,
                              const gchar* property_name,
                              GVariant* value,
                              GError** /*error*/,
                              gpointer user_data) {
  DBusServer* self = reinterpret_cast<DBusServer*>(user_data);
  if (!self) {
    LOGGER(ERROR) << "DBusServer is NULL.";
    return FALSE;
  }

  auto callback =
      self->GetPropertySetter(interface_name);

  gboolean ret = FALSE;
  if (callback) {
    if (callback(connection, property_name, value)) {
      ret = TRUE;
    }
  }

  return ret;
}

static const GDBusInterfaceVTable kInterfaceVTable = {
  OnMethodCall,
  OnGetProperty,
  OnSetProperty
};

static void OnClosedConnection(GDBusConnection* connection,
                               gboolean /*remote_peer_vanished*/,
                               GError* /*error*/,
                               gpointer user_data) {
  g_signal_handlers_disconnect_by_func(connection,
                                       (gpointer)OnClosedConnection,
                                       user_data);
  g_object_unref(connection);
}

static gboolean OnClientRequest(GDBusServer* /*dbus_server*/,
                                GDBusConnection* connection,
                                gpointer user_data) {
  GError* err = NULL;
  DBusServer* self = reinterpret_cast<DBusServer*>(user_data);

  g_signal_connect(connection, "closed",
                   G_CALLBACK(OnClosedConnection), self);

  if (self) {
    // Check Peer Credentials
    DBusServer::PeerCredentialsCallback callback =
        self->GetPeerCredentialsCallback();
    if (callback && !callback(
        g_dbus_connection_get_peer_credentials(connection))) {
      LOGGER(WARN) << "Invalid peer credentials.";
      g_dbus_connection_close_sync(connection, NULL, NULL);
    }

    GDBusNodeInfo* node_info = self->GetIntrospectionNodeInfo();
    if (!node_info) {
      LOGGER(ERROR) << "Introspection is not set.";
      return TRUE;
    }

    // TODO(wy80.choi): register multiple interfaces
    g_object_ref(connection);
    guint reg_id = g_dbus_connection_register_object(
                          connection,
                          "/",
                          node_info->interfaces[0],
                          &kInterfaceVTable,
                          self,
                          NULL,
                          &err);
    if (reg_id == 0) {
      LOGGER(ERROR) << "Failed to register object : " << err->message;
      g_error_free(err);
    }
  }
  return TRUE;
}

}  // namespace

DBusServer::DBusServer()
    : server_(NULL),
      node_info_(NULL) {
}

DBusServer::~DBusServer() {
  if (node_info_) {
    g_dbus_node_info_unref(node_info_);
  }

  if (server_) {
    g_object_unref(server_);
  }

  if (!address_path_.empty()) {
    unlink(address_path_.c_str());
  }
}

void DBusServer::Start(const std::string& name) {
  GError* err = NULL;

  address_path_.clear();
  address_path_.append(g_get_user_runtime_dir());
  address_path_.append("/.");
  address_path_.append(name);
  // unlink existing bus address
  unlink(address_path_.c_str());

  std::string address("unix:path=");
  address.append(address_path_);

  // create new bus socket
  // TODO(wy80.choi): bus socket (Address) should be removed gracefully
  // when application is terminated.
  gchar* guid = g_dbus_generate_guid();
  server_ = g_dbus_server_new_sync(
                  address.c_str(), G_DBUS_SERVER_FLAGS_NONE,
                  guid, NULL, NULL, &err);
  g_free(guid);
  if (!server_) {
    LOGGER(ERROR) << "Failed to create dbus server : " << err->message;
    g_error_free(err);
    return;
  }

  // start server
  g_signal_connect(server_, "new-connection",
                   G_CALLBACK(OnClientRequest), this);

  g_dbus_server_start(server_);
}

std::string DBusServer::GetClientAddress() const {
  return std::string(g_dbus_server_get_client_address(server_));
}

void DBusServer::SetIntrospectionXML(const std::string& xml) {
  GError* err = NULL;
  node_info_ = g_dbus_node_info_new_for_xml(xml.c_str(), &err);
  if (!node_info_) {
    LOGGER(ERROR) << "Failed to create node info from introspection xml : "
                  << err->message;
    g_error_free(err);
  }
}

void DBusServer::SendSignal(GDBusConnection* connection,
                            const std::string& iface,
                            const std::string& signal_name,
                            GVariant* parameters) {
  GError* err = NULL;
  gboolean ret = g_dbus_connection_emit_signal(
      connection, NULL, "/",
      iface.c_str(), signal_name.c_str(),
      parameters, &err);
  if (!ret) {
    LOGGER(ERROR) << "Failed to emit signal : '"
                  << iface << '.' << signal_name << "'";
    g_error_free(err);
  }
}

void DBusServer::SetPeerCredentialsCallback(PeerCredentialsCallback func) {
  peer_credentials_callback_ = func;
}

void DBusServer::SetMethodCallback(
    const std::string& iface, MethodCallback func) {
  method_callbacks_[iface] = func;
}

void DBusServer::SetPropertyGetter(
    const std::string& iface, PropertyGetter func) {
  property_getters_[iface] = func;
}

void DBusServer::SetPropertySetter(
    const std::string& iface, PropertySetter func) {
  property_setters_[iface] = func;
}

DBusServer::PeerCredentialsCallback
DBusServer::GetPeerCredentialsCallback() const {
  return peer_credentials_callback_;
}

DBusServer::MethodCallback
DBusServer::GetMethodCallback(const std::string& iface) {
  return method_callbacks_[iface];
}

DBusServer::PropertySetter
DBusServer::GetPropertySetter(const std::string& iface) {
  return property_setters_[iface];
}

DBusServer::PropertyGetter
DBusServer::GetPropertyGetter(const std::string& iface) {
  return property_getters_[iface];
}

}  // namespace wrt
