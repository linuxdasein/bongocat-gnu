project('bongocat-gnu',
  'cpp',
  license: 'GPL-3.0',
  meson_version: '>=1.1.0',
  version: '0.3.1')

sources = files([
  'src/cat.cpp',
  'src/data.cpp',
  'src/input.cpp',
  'src/logger.cpp',
  'src/main.cpp',
  'src/mouse.cpp',
  'src/math.cpp',
  'src/mousepaw.cpp',
  'src/system.cpp',
  'src/config.cpp',
  'src/settings.cpp',
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
  dependency('cxxopts'),
  dependency('sfml-window'),
  dependency('sfml-graphics')
]

executable('bongo', sources,
  cpp_args: cpp_flags,
  link_args: ld_flags,
  dependencies: link_deps,
  include_directories: inc_dirs,
  install: true)

# Install app resources
install_subdir('img', install_dir : '')
install_subdir('share', install_dir : '')
icon_dir = get_option('icondir')

# Install icons in os specific directory
install_data('ico/bongo.ico',         install_dir : icon_dir / '128x128/apps', rename : 'io.github.linuxdasein.bongocat_gnu.ico' )
install_data('ico/bongo-16x16.png',   install_dir : icon_dir / '16x16/apps',   rename : 'io.github.linuxdasein.bongocat_gnu.png' )
install_data('ico/bongo-24x24.png',   install_dir : icon_dir / '24x24/apps',   rename : 'io.github.linuxdasein.bongocat_gnu.png' )
install_data('ico/bongo-32x32.png',   install_dir : icon_dir / '32x32/apps',   rename : 'io.github.linuxdasein.bongocat_gnu.png' )
install_data('ico/bongo-48x48.png',   install_dir : icon_dir / '48x48/apps',   rename : 'io.github.linuxdasein.bongocat_gnu.png' )
install_data('ico/bongo-170x170.png', install_dir : icon_dir / '170x170/apps', rename : 'io.github.linuxdasein.bongocat_gnu.png' )
