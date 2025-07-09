project(
    'libfinite',
    'c',
    version: '0.5.0-dev'
)

cc = meson.get_compiler('c')

# cglm_dep = dependency('cglm', fallback : 'cglm', 'cglm_dep')
wayland_client = dependency('wayland-client', required: true)
# gls = dependency('glslang')
wayland_protos = dependency('wayland-protocols', version: '>=1.24', default_options: ['tests=false'], required: true)
cairo = dependency('cairo', required: true)
evdev = dependency('libevdev', required: true)
sndfile = dependency('sndfile', required: true)
alsa = dependency('alsa', required: true)
vulkan = dependency('vulkan', required: true)
# use 1.6.0 for now...
xkb = dependency('xkbcommon', version: '>=1.6.0', fallback: 'libxkbcommon', required: true)

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

inc = include_directories('include')

deps = [
    wayland_client,
    wayland_protos,
    cairo,
    evdev,
    xkb,
    sndfile,
    alsa.
    vulkan
]

src = [
    xdg_shell[1],
    layer_shell[1],
    'draw/window.c',
    'draw/wl_shm.c',
    'draw/listen.c',
    'draw/draw.c',
    'draw/btn.c',

    'input/input.c',
    'input/listen.c',

    'audio/audio.c',

    'render/render.c',
    'render/shaders.c',
    'render/vulkan.c',

    'core/file.c'
    
]

libfinite = library(
    'libfinite', 
    src, 
    version : '0.5.0', 
    soversion : '1', 
    dependencies: deps,
    install: true,
    include_directories: inc
)

# install the draw headers
headers = [
    'include/draw/draw.h',
    'include/draw/cairo.h',
    'include/draw/window.h',
    'include/draw/wl_shm.h',

    'include/input/input.h',
    'include/input/input-core.h',
    'include/input/keyboard.h',

    'include/audio/audio.h',
    
    'include/render/render-core.h',
    'include/render/vulkan.h',
    'include/render/render.h',

    'include/core.h'
]

install_headers(headers, subdir: 'finite')

# setup pkg-config
pkgconfig = import('pkgconfig')
pkgconfig.generate(
    libfinite,
    name: 'libfinite',
    description: 'The Cubix Infinite Development Library',
    version: '0.5.0',
    filebase: 'finite',
    subdirs: 'finite',
    requires: ['wayland-client', 'wayland-protocols', 'cairo', 'alsa', 'vulkan']
)