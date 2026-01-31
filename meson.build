project(
    'libfinite',
    'c',
    default_options: 'default_library=static',
    version: '0.7.1-dev'
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
xkb = dependency('xkbcommon', version: '>=1.6.0', fallback: 'libxkbcommon', required: true)

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
    'protocol/xdg-shell-protocol.c',
    'protocol/layer-shell-protocol.c',
    'protocol/virtual-keyboard-protocol.c',

    'draw/window.c',
    'draw/wl_shm.c',
    'draw/listen.c',
    'draw/draw.c',
    'draw/btn.c',

    'input/input.c',
    'input/listen.c',
    'input/gamepad.c',

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
    version : '0.7.1', 
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

proto_headers = [
  'include/protocol/xdg-shell-client-protocol.h',
  'include/protocol/layer-shell-client-protocol.h',
  'include/protocol/virtual-keyboard-client-protocol.h',
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
  'include/input/gamepad.h',
]

install_headers(headers, subdir: 'finite')
install_headers(proto_headers, subdir: 'finite/protocol')
install_headers(draw_headers, subdir: 'finite/draw')
install_headers(render_headers, subdir: 'finite/render')
install_headers(input_headers, subdir: 'finite/input')



# setup pkg-config
pkgconfig = import('pkgconfig')
pkgconfig.generate(
    libfinite,
    name: 'libfinite',
    description: 'The Cubix Infinite Development Library',
    version: '0.7.1',
    filebase: 'finite',
    subdirs: 'finite',
    requires: ['wayland-client', 'wayland-protocols', 'cairo', 'alsa', 'vulkan']
)