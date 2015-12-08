{
  'includes':[
    '../build/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'xwalk_extension',
      'type': 'executable',
      'dependencies': [
        '../common/common.gyp:xwalk_tizen_common',
      ],
      'sources': [
        'common/constants.h',
        'common/constants.cc',
        'extension/xwalk_extension.h',
        'extension/xwalk_extension.cc',
        'extension/xwalk_extension_instance.h',
        'extension/xwalk_extension_instance.cc',
        'extension/xwalk_extension_adapter.h',
        'extension/xwalk_extension_adapter.cc',
        'extension/xwalk_extension_server.h',
        'extension/xwalk_extension_server.cc',
        'extension/xwalk_extension_process.cc',
      ],
      'defines': [
        'PLUGIN_LAZY_LOADING',
      ],
      'variables': {
        'packages': [
          'ecore',
        ],
      },
      'link_settings': {
        'ldflags': [
          '-ldl',
        ],
      },
    }, # end of target 'xwalk_extension'
    {
      'target_name': 'xwalk_extension_renderer',
      'type': 'static_library',
      'dependencies': [
        '../common/common.gyp:xwalk_tizen_common',
      ],
      'sources': [
        'common/constants.h',
        'common/constants.cc',
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
      'variables': {
        'packages': [
          'chromium-efl',
          'elementary',
        ],
      },
    }, # end of target 'xwalk_extension_renderer'
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
