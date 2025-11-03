project(
    'libfinite',
    'c',
    default_options: 'default_library=static',
    version: '0.6.3-dev'
)

cc = meson.get_compiler('c')

cglm = dependency('cglm')
wayland_client = dependency('wayland-client', required: true)
# gls = dependency('glslang')
math = cc.find_library('m', required: true)
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
    math,
    wayland_client,
    wayland_protos,
    cairo,
    evdev,
    cglm,
    xkb,
    sndfile,
    alsa,
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
    'render/image.c',

    'core/file.c',
    'core/log.c'
    
]

libfinite = library(
    'libfinite', 
    src, 
    version : '0.6.2', 
    soversion : '1', 
    dependencies: deps,
    install: true,
    include_directories: inc
)

# install the headers
headers = [
    'include/draw.h',
    'include/input.h',
    'include/audio/audio.h',
    'include/render.h',
    'include/core.h',
    'include/log.h'
]

draw_headers = [
  'include/draw/cairo.h',
  'include/draw/window.h',
  'include/draw/wl_shm.h',
]

render_headers = [
  'include/render/render-core.h',
  'include/render/vulkan.h',
  'include/render/stb_image.h',
  'include/render/render-image.h',
]

input_headers = [
  'include/input/input-core.h',
  'include/input/keyboard.h',
]

install_headers(headers, subdir: 'finite')
install_headers(draw_headers, subdir: 'finite/draw')
install_headers(render_headers, subdir: 'finite/render')
install_headers(input_headers, subdir: 'finite/input')



# setup pkg-config
pkgconfig = import('pkgconfig')
pkgconfig.generate(
    libfinite,
    name: 'libfinite',
    description: 'The Cubix Infinite Development Library',
    version: '0.6.2',
    filebase: 'finite',
    subdirs: 'finite',
    requires: ['wayland-client', 'wayland-protocols', 'cairo', 'alsa', 'vulkan']
)