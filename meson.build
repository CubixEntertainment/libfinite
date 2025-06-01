project(
    'libfinite',
    'c',
    version: '0.1.1-dev'
)

cglm_dep = dependency('cglm', fallback : 'cglm', 'cglm_dep')
wayland_client = dependency('wayland-client')
gls = dependency('glslang')
wayland_protos = dependency('wayland-protocols', version: '>=1.24', default_options: ['tests=false'])
inc = include_directories('redner/include')