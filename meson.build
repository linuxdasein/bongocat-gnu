project('bongocat-gnu',
  'cpp',
  license: 'GPL-3.0',
  version: '0.1.0')

sources = files([
  'src/ctb.cpp',
  'src/custom.cpp',
  'src/data.cpp',
  'src/input.cpp',
  'src/jsoncpp.cpp',
  'src/main.cpp',
  'src/mania.cpp',
  'src/osu.cpp',
  'src/system.cpp',
  'src/taiko.cpp',
])

cpp_flags = ['-std=c++17']

inc_dirs = include_directories('include')

link_deps = [
  dependency('x11'),
  dependency('xrandr'),
  dependency('libxdo'),
  dependency('sdl2'),
  dependency('sfml-window'),
  dependency('sfml-graphics')
]

executable('bongo', sources,
  cpp_args: cpp_flags,
  dependencies: link_deps,
  include_directories: inc_dirs)
