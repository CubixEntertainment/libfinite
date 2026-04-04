project(
    'libfinite',
    'c',
    default_options: 'default_library=static',
    version: '0.7.3-dev'
)

cc = meson.get_compiler('c')
math = cc.find_library('m', required: true)
conf = configuration_data()
finite = dependency('finite', version: ['>=0.7.3'], required: true) # libfinite
curl = dependency('libcurl', required: get_option('with-networking'))
udev = dependency('libudev')
lws = dependency('libwebsockets')

if get_option('with-finite').enabled() and finite.found()
    conf.set('USE_FINITE', true, description: 'Use libfinite debug options')
else
    conf.set('USE_FINITE', false, description: 'Don\'t use libfinite debug options')
endif

# if get_option('with-networking').enabled() and curl.found()
#     conf.set('USE_CURL', true, description: 'Use curl for networking')
# else
#     conf.set('USE_CURL', false, description: 'Don\'t use curl for networking')
# endif

configure_file(output: 'config.h', configuration: conf)

inc = include_directories('.','include')

deps = [
    math,
    finite,
    udev,
    lws,
    curl
]

src = [
    'src/core.c',
    'src/auth.c',
    'src/gamepad.c'
]   

executable(
    'mailroom', 
    sources: src,
    dependencies: deps
)