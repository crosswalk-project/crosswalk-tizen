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
        '../extensions/extensions.gyp:xwalk_extension_shared',
        'resources/resources.gyp:xwalk_runtime_resources',
      ],
      'sources': [
        'common/constants.h',
        'common/constants.cc',
        'browser/runtime_process.cc',
        'browser/runtime.h',
        'browser/runtime.cc',
        'browser/ui_runtime.h',
        'browser/ui_runtime.cc',
        'browser/native_window.h',
        'browser/native_window.cc',
        'browser/native_app_window.h',
        'browser/native_app_window.cc',
        'browser/notification_window.h',
        'browser/notification_window.cc',
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
        'browser/prelauncher.h',
        'browser/prelauncher.cc',
        'browser/preload_manager.h',
        'browser/preload_manager.cc',
      ],
      'ldflags': [
        '-pie',
      ],
      'variables': {
        'packages': [
          'capi-appfw-application',
          'capi-ui-efl-util',
          'chromium-efl',
          'ecore',
          'ecore-wayland',
          'elementary',
          'efl-extension',
          'deviced',
          'manifest-parser',
          'wgt-manifest-handlers',
          'notification',
          'launchpad',
        ],
      },
      'conditions': [
        ['profile == "mobile"', {
          'defines': ['PROFILE_MOBILE'],
        }],
        ['profile == "wearable"', {
          'defines': ['PROFILE_WEARABLE'],
        }],
        ['profile == "tv"', {
          'defines': ['PROFILE_TV'],
        }],
        ['tizen_model_formfactor == "circle"', {
          'defines': ['MODEL_FORMFACTOR_CIRCLE'],
        }],
        ['tizen_feature_rotary_event_support == 1', {
          'defines': ['ROTARY_EVENT_FEATURE_SUPPORT'],
        }],
        ['tizen_feature_web_ime_support == 1', {
          'defines': ['IME_FEATURE_SUPPORT'],
          'sources': [
            'browser/ime_runtime.h',
            'browser/ime_runtime.cc',
            'browser/ime_application.h',
            'browser/ime_application.cc',
            'browser/native_ime_window.h',
            'browser/native_ime_window.cc',
          ],
          'variables': {
            'packages': [
              'capi-ui-inputmethod',
            ],
          },
        }],
        ['tizen_feature_watch_face_support == 1', {
          'defines': ['WATCH_FACE_FEATURE_SUPPORT'],
          'sources': [
            'browser/watch_runtime.h',
            'browser/watch_runtime.cc',
            'browser/native_watch_window.h',
            'browser/native_watch_window.cc',
          ],
          'variables': {
            'packages': [
              'capi-appfw-watch-application',
              'appcore-watch',
            ],
          },
        }],
        ['tizen_feature_manual_rotate_support == 1', {
          'defines': ['MANUAL_ROTATE_FEATURE_SUPPORT'],
        }],
      ],
    }, # end of target 'xwalk_runtime'
    {
      'target_name': 'xwalk_injected_bundle',
      'type': 'shared_library',
      'dependencies': [
        '../common/common.gyp:xwalk_tizen_common',
        '../extensions/extensions.gyp:xwalk_extension_shared',
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
