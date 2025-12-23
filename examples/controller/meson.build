project(
    'controller_test',
    'c',
)

cc = meson.get_compiler('c')

finite = dependency('finite', version: '>=0.7.0') # libfinite

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

src = [
    xdg_shell,
    layer_shell,
    'main.c'
]

deps = [
    finite
]

inc = include_directories('.')

executable(
    'controller_test', 
    sources: src,
    include_directories: inc,
    dependencies: deps
)