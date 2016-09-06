{
  'targets': [
    {
      'target_name': 'xwalk_tizen_all_targets',
      'type': 'none',
      'dependencies': [
        'common/common.gyp:*',
        'extensions/extensions.gyp:*',
        'runtime/runtime.gyp:*',
        'loader/loader.gyp:*',
      ],
    }, # end of target 'xwalk_tizen'
  ], # end of targets
}
