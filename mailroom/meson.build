project(
    'libfinite',
    'c',
    default_options: 'default_library=static',
    version: '0.7.3-dev'
)

cc = meson.get_compiler('c')
math = cc.find_library('m', required: true)
conf = configuration_data()
finite = dependency('finite', version: ['>=0.7.3'], required: get_option('with-finite')) # libfinite
curl = dependency('curl', required: true)

if get_option('with-finite').enabled() and finite.found()
    conf.set('USE_FINITE', true, description: 'Use libfinite debug options')
else
    conf.set('USE_FINITE', false, description: 'Don\'t use libfinite debug options')
endif

configure_file(output: 'config.h', configuration: conf)

inc = include_directories('.','include')

deps = [
    math,
    finite,
    curl
]

src = [
    'src/core.c'
]

executable(
    'mailroom', 
    sources: src,
    dependencies: deps
)