project(
    'sound-demo',
    'c',
    default_options: 'default_library=static',
    version: '1.0.0'
)

finite = dependency('finite', version: '>=0.3.0') # libfinite

# move the sound fromm the src dir to the build dir
sound = custom_target('sound',
    input: ['jingle.mp3'],
    output: ['jingle.mp3'], # prefixed with build dir automatically
    command: [
        'cp', '@INPUT@', '@OUTPUT@',
    ]
)

src = [
    'audio.c',
    sound
]

deps = [
    finite,
]



executable(
    'sound', 
    sources: src,
    dependencies: deps
)