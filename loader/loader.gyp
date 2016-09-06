{
  'includes':[
    '../build/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'wrt-loader',
      'type': 'executable',
      'sources': [
        'wrt_loader.cc',
      ],
      'variables': {
        'packages': [
          'dlog',
        ],
      },
      'libraries' : [
        '-ldl',
      ],      
    }, # end of target 'wrt-loader'
  ],
}
