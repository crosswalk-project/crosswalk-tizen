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
          'appsvc',
          'aul',
          'capi-appfw-application',
          'capi-appfw-app-manager',
          'capi-appfw-package-manager',
          'capi-system-system-settings',
          'dlog',
          'uuid',
          'libwebappenc',
          'manifest-parser',
          'wgt-manifest-handlers',
          'pkgmgr-info',
          'glib-2.0',
          'ttrace',
        ],
      },
      'conditions': [
        ['tizen_feature_web_ime_support == 1', {
          'defines': ['IME_FEATURE_SUPPORT'],
        }],
        ['tizen_feature_watch_face_support == 1', {
          'defines': ['WATCH_FACE_FEATURE_SUPPORT'],
        }],
      ],
      'direct_dependent_settings': {
        'libraries': [
          '-lxwalk_tizen_common',
        ],
        'variables': {
          'packages': [
            'dlog',
          ],
        },
      },
    },
  ],
}
