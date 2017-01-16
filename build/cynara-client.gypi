{
    'variables': {
        'pkg-config': 'pkg-config',
    },
    'cflags': [
        '<!@(<(pkg-config) --cflags cynara-client)'
    ],
    'link_settings': {
        'ldflags': [
            '<!@(<(pkg-config) --libs-only-L --libs-only-other cynara-client)',
        ],
        'libraries': [
            '<!@(<(pkg-config) --libs-only-l cynara-client)',
        ],
    },
} # cynara-client
