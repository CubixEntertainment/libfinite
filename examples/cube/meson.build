project(
    'render',
    'c',
    version: '0.1.9',
    meson_version: '>=1.0'
)

cc = meson.get_compiler('c')

wayland_client = dependency('wayland-client')
wayland_protos = dependency('wayland-protocols', version: '>=1.24', default_options: ['tests=false'])
cairo = dependency('cairo')
vulkan = dependency('vulkan')
sndfile = dependency('sndfile', required: true)
alsa = dependency('alsa', required: true)
finite = dependency('finite', version: ['>=0.4.0']) # libfinite

xdg_shell = custom_target('xdg-shell',
    input: 'xdg-shell.xml',
    output: ['xdg-shell-client-protocol.h', 'xdg-shell-protocol.c'],
    command: [
      'wayland-scanner', 'client-header', '@INPUT@', '@OUTPUT0@',
      '&&',
      'wayland-scanner', 'private-code', '@INPUT@', '@OUTPUT1@'
    ]
)

layer_shell = custom_target('layer-shell',
    input: 'layer-shell.xml',
    output: ['layer-shell-client-protocol.h', 'layer-shell-protocol.c'],
    command: [
      'wayland-scanner', 'client-header', '@INPUT@', '@OUTPUT0@',
      '&&',
      'wayland-scanner', 'private-code', '@INPUT@', '@OUTPUT1@'
    ]
)

shaders = custom_target('shaders',
    input: ['shaders/shader_base.frag', 'shaders/shader_base.vert'],
    output: ['frag.spv', 'vert.spv'],
    command: [
        'glslc', '@INPUT0@', '-o', '@OUTPUT0@',
        '&&',
        'glslc', '@INPUT1@', '-o', '@OUTPUT1@'
    ]
)

image = custom_target('image',
    input: 'textureTest.png',
    output: 'texture.png',
    command: [
        'cp', '@INPUT@', '@OUTPUT@'
    ]
)

src = [
    'src/main.c',
    xdg_shell[1],
    layer_shell[1],
    shaders[1],
    image
]

deps = [
    wayland_client,
    wayland_protos,
    cairo,
    finite,
    sndfile,
    alsa,
    # glsl,
    vulkan
]



executable(
    'cube', 
    sources: src,
    dependencies: deps
)