project(
  'c_networking_examples',
  ['c'],
  version: '0.1.0.555',
  meson_version : '>= 0.56.0',
  default_options: ['warning_level=3', 'b_sanitize=address,undefined', 'b_lundef=false', 'c_std=c99'],
  )

cc = meson.get_compiler('c')

extra_flags = [
  '-fno-common',
  '-fstack-protector-strong',
  '-Wformat-security',
  '-Wshadow',
  '-Wstrict-overflow=5',
  '-Werror=odr',
  '-Werror=lto-type-mismatch',
  '-Werror=strict-aliasing',
  ]

add_project_arguments(cc.get_supported_arguments(extra_flags), language: 'c')

definitions = [
  '-D_DEFAULT_SOURCE',
  ]
add_project_arguments(definitions, language: 'c')

c_example = [
  'dual_stack_echo_client',
  'dual_stack_echo_server',
  'tcp_file_transfer_client',
  'tcp_file_transfer_server',
  'tcp_chat_client',
  'tcp_chat_server',
  'tcp_multiple_connections',
  'udp_echo_client',
  'udp_echo_server'
  ]

_netex = static_library(
  'netex',
  ['netex.c', 'util.c'],
  install: false)

foreach file : c_example
  executable(
    file,
    file + '.c',
    link_with: _netex,
    install: false
  )
endforeach

sdl_dep = dependency('sdl2', required: false)
if sdl_dep.found()
  sdl_example = [
    'tcp_sdl_window_updater_server',
    'tcp_sdl_window_updater_client',
    ]

  _graphics = static_library(
    'graphics',
    'graphics.c',
    dependencies: sdl_dep,
    install: false
    )

  foreach file : sdl_example
    executable(
      file,
      [file + '.c'],
      link_with: [_graphics, _netex],
      dependencies: sdl_dep,
      install: false
    )
  endforeach
endif

subdir ('test')
