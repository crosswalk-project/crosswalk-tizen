{
  'includes':[
    '../build/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'xwalk_extension_shared',
      'type': 'shared_library',
      'dependencies': [
        '../common/common.gyp:xwalk_tizen_common',
      ],
      'sources': [
        'common/constants.h',
        'common/constants.cc',
        'common/xwalk_extension.h',
        'common/xwalk_extension.cc',
        'common/xwalk_extension_instance.h',
        'common/xwalk_extension_instance.cc',
        'common/xwalk_extension_adapter.h',
        'common/xwalk_extension_adapter.cc',
        'common/xwalk_extension_manager.h',
        'common/xwalk_extension_manager.cc',
        'common/xwalk_extension_server.h',
        'common/xwalk_extension_server.cc',
        'renderer/xwalk_extension_client.h',
        'renderer/xwalk_extension_client.cc',
        'renderer/xwalk_extension_module.h',
        'renderer/xwalk_extension_module.cc',
        'renderer/xwalk_extension_renderer_controller.h',
        'renderer/xwalk_extension_renderer_controller.cc',
        'renderer/xwalk_module_system.h',
        'renderer/xwalk_module_system.cc',
        'renderer/xwalk_v8tools_module.h',
        'renderer/xwalk_v8tools_module.cc',
        'renderer/widget_module.h',
        'renderer/widget_module.cc',
        'renderer/object_tools_module.h',
        'renderer/object_tools_module.cc',
        'renderer/runtime_ipc_client.h',
        'renderer/runtime_ipc_client.cc',
      ],
      'cflags': [
        '-fvisibility=default',
      ],
      'variables': {
        'packages': [
          'v8',
          'chromium-efl',
          'elementary',
        ],
      },
      'direct_dependent_settings': {
        'libraries': [
          '-lxwalk_extension_shared',
        ],
        'variables': {
          'packages': [
            'v8',
            'jsoncpp',
          ],
        },
      },
    }, # end of target 'xwalk_extension_static'
    {
      'target_name': 'widget_plugin',
      'type': 'shared_library',
      'dependencies': [
        '../common/common.gyp:xwalk_tizen_common',
      ],
      'sources': [
        'internal/widget/widget_api.js',
        'internal/widget/widget_extension.cc',
      ],
      'copies': [
        {
          'destination': '<(SHARED_INTERMEDIATE_DIR)',
          'files': [
            'internal/widget/widget.json'
          ],
        },
      ],
    }, # end of target 'widget_plugin'
    {
      'target_name': 'splash_screen_plugin',
      'type': 'shared_library',
      'dependencies': [
        '../common/common.gyp:xwalk_tizen_common',
      ],
      'sources': [
        'internal/splash_screen/splash_screen_api.js',
        'internal/splash_screen/splash_screen_extension.cc',
      ],
      'copies': [
        {
          'destination': '<(SHARED_INTERMEDIATE_DIR)',
          'files': [
            'internal/splash_screen/splash_screen.json'
          ],
        },
      ],
    }, # end of target 'splash_screen_plugin'
  ], # end of targets
}
