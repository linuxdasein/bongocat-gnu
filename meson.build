project('bongocat-gnu',
  'cpp',
  license: 'GPL-3.0',
  version: '0.1.0')

sources = files([
  'src/custom.cpp',
  'src/data.cpp',
  'src/input.cpp',
  'src/logger.cpp',
  'src/main.cpp',
  'src/mouse.cpp',
  'src/math.cpp',
  'src/mousepaw.cpp',
  'src/system.cpp',
  'src/cats.cpp',
  'src/config.cpp',
])

ld_flags = []
cpp_flags = ['-std=c++17', '-Werror']

cpp = meson.get_compiler('cpp')

if cpp.get_id() == 'gcc' and cpp.version().version_compare('<9.0.0')
  # GCC 8 requires a specific flag to use std::filesystem
  ld_flags += ['-lstdc++fs']
endif

inc_dirs = include_directories('include')

link_deps = [
  dependency('x11'),
  dependency('xrandr'),
  dependency('libxdo'),
  dependency('jsoncpp'),
  dependency('sfml-window'),
  dependency('sfml-graphics')
]

executable('bongo', sources,
  cpp_args: cpp_flags,
  link_args: ld_flags,
  dependencies: link_deps,
  include_directories: inc_dirs,
  install: true)
