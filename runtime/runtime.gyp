{
  'includes':[
    '../build/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'xwalk_runtime',
      'type': 'executable',
      'dependencies': [
        '../common/common.gyp:xwalk_tizen_common',
        'resources/resources.gyp:xwalk_runtime_resources',
      ],
      'sources': [
        'common/constants.h',
        'common/constants.cc',
        'browser/runtime_process.cc',
        'browser/runtime.h',
        'browser/runtime.cc',
        'browser/native_window.h',
        'browser/native_window.cc',
        'browser/native_app_window.h',
        'browser/native_app_window.cc',
        'browser/web_application.h',
        'browser/web_application.cc',
        'browser/web_view.h',
        'browser/web_view.cc',
        'browser/web_view_impl.h',
        'browser/web_view_impl.cc',
        'browser/popup.h',
        'browser/popup.cc',
        'browser/splash_screen.h',
        'browser/splash_screen.cc',
        'browser/popup_string.h',
        'browser/popup_string.cc',
        'browser/vibration_manager.h',
        'browser/vibration_manager.cc',
        'browser/notification_manager.h',
        'browser/notification_manager.cc',
      ],
      'defines': [
        'HAVE_WAYLAND',
      ],
      'variables': {
        'packages': [
          'capi-appfw-application',
          'chromium-efl',
          'ecore',
          'ecore-wayland',
          'efl-extension',
          'elementary',
          'deviced',
          'manifest-parser',
          'manifest-handlers',
          'notification',
        ],
      },
    }, # end of target 'xwalk_runtime'
    {
      'target_name': 'xwalk_injected_bundle',
      'type': 'shared_library',
      'dependencies': [
        '../common/common.gyp:xwalk_tizen_common',
        '../extensions/extensions.gyp:xwalk_extension_renderer',
      ],
      'sources': [
        'renderer/injected_bundle.cc',
      ],
      'cflags': [
        '-fvisibility=default',
      ],
      'variables': {
        'packages': [
          'chromium-efl',
          'elementary',
        ],
      },
    }, # end of target 'xwalk_injected_bundle'
  ],
}
