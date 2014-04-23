{
  'targets': [
    {
      'target_name': 'gluv',
      'product_name': 'gluv',
      'type': 'executable',

      'sources': [
        '<!@(find src -name "*.c" -or -name "*.h")',
      ],

      'variables': {
        'cflags': [
          '<@(glfw3_cflags)',
          '<@(uv_cflags)',
        ],
        'ldflags': [
          '<@(glfw3_ldflags)',
          '<@(uv_ldflags)',
        ],
      },

      'cflags': ['<@(cflags)'],

      'link_settings': {
        'ldflags': ['<@(ldflags)'],
        'libraries': [
          '<@(glfw3_libs)',
          '<@(uv_libs)',
        ]
      },

      'xcode_settings': {
        'OTHER_CFLAGS': ['<@(cflags)'],
        'OTHER_LDFLAGS': ['<@(ldflags)'],
      },
    }
  ],
}
