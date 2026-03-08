project(
    'controller_test',
    'c',
)

cc = meson.get_compiler('c')

finite = dependency('finite', version: '>=0.7.2') # libfinite
src = [
    'main.c'
]

deps = [
    finite
]

inc = include_directories('.')

executable(
    'controller_test', 
    sources: src,
    include_directories: inc,
    dependencies: deps
)