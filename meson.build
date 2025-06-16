project(
    'libfinite',
    'c',
    version: '0.1.1-dev'
)

cc = meson.get_compiler('c')

# cglm_dep = dependency('cglm', fallback : 'cglm', 'cglm_dep')
wayland_client = dependency('wayland-client')
# gls = dependency('glslang')
wayland_protos = dependency('wayland-protocols', version: '>=1.24', default_options: ['tests=false'])
cairo = dependency('cairo')
# inc = include_directories('redner/include')

# build xdg
xdg_shell = custom_target('xdg-shell',
    input: 'xdg-shell.xml',
    output: ['xdg-shell-client-protocol.h', 'xdg-shell-protocol.c'],
    command: [
      'wayland-scanner', 'client-header', '@INPUT@', '@OUTPUT0@',
      '&&',
      'wayland-scanner', 'private-code', '@INPUT@', '@OUTPUT1@'
    ],
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
)

inc = include_directories('include')

deps = [
    wayland_client,
    wayland_protos,
    cairo
]

src = [
    xdg_shell[1],
    layer_shell[1],
    'draw/window.c',
    'draw/wl_shm.c',
    'draw/listen.c',
    'draw/draw.c',
]

libfinite = library(
    'libfinite', 
    src, 
    version : '0.1.1', 
    soversion : '1', 
    dependencies: deps,
    install: true,
    include_directories: inc
)

# install the draw headers
headers = [
    'include/draw.h',
    'include/window.h',
    'include/wl_shm.h'
]

install_headers(headers, subdir: 'finite')

# setup pkg-config
pkgconfig = import('pkgconfig')
pkgconfig.generate(
    libfinite,
    name: 'libfinite',
    description: 'The Cubix Infinite Development Library',
    version: '0.1.1',
    filebase: 'finite',
    subdirs: 'finite',
    requires: ['wayland-client', 'wayland-protocols', 'cairo']
)