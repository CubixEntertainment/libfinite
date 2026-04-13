project(
    'textbox',
    'c',
    version: '0.1.1-dev',
    meson_version: '>=1.0'
)

cc = meson.get_compiler('c')

wayland_client = dependency('wayland-client')
wayland_protos = dependency('wayland-protocols', version: '>=1.24', default_options: ['tests=false'])
cairo = dependency('cairo')
finite = dependency('finite', version: ['>=0.8.0']) # libfinite


src = [
    'main.c',
]

deps = [
    wayland_client,
    wayland_protos,
    cairo,
    finite
]



executable(
    'finite-textbox', 
    sources: src,
    dependencies: deps
)
