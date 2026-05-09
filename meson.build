project('slgro', 'c',
  version: '1.3',
  default_options: ['c_std=c99', 'warning_level=2'],
)

add_project_arguments('-D_POSIX_C_SOURCE=200809L', language: 'c')

deps = [
  dependency('swc'),
  dependency('wayland-server'),
  dependency('xkbcommon'),
  dependency('libinput'),
  dependency('pixman-1'),
  dependency('libdrm'),
  dependency('wld'),
  dependency('libudev'),
  dependency('xcb'),
  dependency('xcb-composite'),
  dependency('xcb-ewmh'),
  dependency('xcb-icccm'),
  dependency('luajit'),
]

src = [
  'source/slgro.c',
  'source/util.c',
  'source/lua-config.c',
]

install_data('source/config.lua',
  install_dir: '/usr/share/slgro',
)

executable('slgro', src,
  dependencies: deps,
  include_directories: include_directories('source'),
  install: true,
  install_dir: get_option('bindir'),
)


