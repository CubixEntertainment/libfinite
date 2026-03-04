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
finite = dependency('finite', version: ['>=0.7.2']) # libfinite

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