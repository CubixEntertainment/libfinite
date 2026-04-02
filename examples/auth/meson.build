project(
    'draw',
    'c',
    default_options: 'default_library=static'
)

cc = meson.get_compiler('c')


finite = dependency('finite', version: '>=0.7.2') # libfinite

src = [
    'main.c'
]

deps = [
    finite
]

executable(
    'auth_test', 
    sources: src,
    dependencies: deps
)
