project(
    'draw',
    'c',
    default_options: 'default_library=static'
)

cc = meson.get_compiler('c')

# build xdg
xdg_shell = custom_target('xdg-shell',
    input: 'xdg-shell.xml',
    output: ['xdg-shell-client-protocol.h', 'xdg-shell-protocol.c'],
    command: [
      'wayland-scanner', 'client-header', '@INPUT@', '@OUTPUT0@',
      '&&',
      'wayland-scanner', 'private-code', '@INPUT@', '@OUTPUT1@'
    ],
    install: true,
    install_dir: [
      join_paths(get_option('includedir'), 'finite/protocols'),
      get_option('datadir')
    ]
)

# build layer-shell
layer_shell = custom_target('layer-shell',
    input: 'layer-shell.xml',
    output: ['layer-shell-client-protocol.h', 'layer-shell-protocol.c'],
    command: [
      'wayland-scanner', 'client-header', '@INPUT@', '@OUTPUT0@',
      '&&',
      'wayland-scanner', 'private-code', '@INPUT@', '@OUTPUT1@'
    ],
    install: true,
    install_dir: [
      join_paths(get_option('includedir'), 'finite/protocols'),
      get_option('datadir') 
    ]
)

finite = dependency('finite', version: '>=0.7.0') # libfinite

src = [
    xdg_shell,
    layer_shell,
    'main.c'
]

deps = [
    finite
]

executable(
    'draw_test', 
    sources: src,
    dependencies: deps
)
