{
  'includes':[
    '../build/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'xwalk_tizen_common',
      'type': 'shared_library',
      'sources': [
        'command_line.h',
        'command_line.cc',
        'dbus_client.h',
        'dbus_client.cc',
        'dbus_server.h',
        'dbus_server.cc',
        'file_utils.h',
        'file_utils.cc',
        'string_utils.h',
        'string_utils.cc',
        'logger.h',
        'picojson.h',
        'profiler.h',
        'profiler.cc',
        'url.h',
        'url.cc',
        'app_control.h',
        'app_control.cc',
        'app_db.h',
        'app_db.cc',
        'app_db_sqlite.h',
        'application_data.h',
        'application_data.cc',
        'locale_manager.h',
        'locale_manager.cc',
        'resource_manager.h',
        'resource_manager.cc',
      ],
      'cflags': [
        '-fvisibility=default',
      ],
      'variables': {
        'packages': [
          'dlog',
          'gio-2.0',
          'uuid',
          'aul',
          'appsvc',
          'manifest-parser',
          'manifest-handlers',
          'capi-appfw-package-manager',
          'capi-appfw-application',
          'capi-system-system-settings',
          'libcurl'
        ],
      },
      'direct_dependent_settings': {
        'libraries': [
          '-lxwalk_tizen_common',
        ],
        'variables': {
          'packages': [
            'dlog',
            'gio-2.0',
          ],
        },
      },
    },
  ],
}
