{
  'includes': [
    '../build/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'wrt-upgrade',
      'type': 'executable',
      'dependencies': [
        '../common/common.gyp:xwalk_tizen_common',
      ],
      'sources': [
        'wrt-upgrade.cc',
        'wrt-upgrade-info.cc',
      ],
      'variables': {
        'packages': [
          'sqlite3',
          'dlog',
        ],
      },
      'libraries' : [
        '-ldl',
      ],
      'copies': [
        {
          'destination': '<(SHARED_INTERMEDIATE_DIR)',
          'files': [
            '720.wrt.upgrade.sh'
          ],
        },
      ],
    }, # end of target 'wrt-upgrade'
  ], # end of targets
}
