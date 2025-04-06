project(
  'c_networking_examples',
  ['c', 'cpp'],
  version: '0.1.0.555',
  meson_version : '>= 0.56.0',
  default_options: ['warning_level=3', 'b_sanitize=address,undefined']
  )

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')
extra_flags = [
  '-fno-common',
  '-Wformat-security',
  '-Wformat-overflow=2',
  '-Wshadow',
  ]

foreach cflag : extra_flags
  if cc.has_argument(cflag)
    add_project_arguments(cflag, language: 'c')
  endif
  if cxx.has_argument(cflag)
    add_project_arguments(cflag, language: 'cpp')
  endif
endforeach

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

sdl_example = [
  'tcp_dual_stack_sdl2_window_server',
  'tcp_dual_stack_sdl2_window_client',
  ]

_netex = static_library(
  'netex',
  'netex.c',
  install: false)

foreach file : c_example
  executable(
    file,
    file + '.c',
    link_with: _netex,
    install: false
  )
endforeach

_graphics = static_library(
  'graphics',
  'graphics.cpp',
  install: false
  )

foreach file : sdl_example
  executable(
    file,
    [file + '.cpp'],
    link_with: [_graphics, _netex],
    dependencies: [dependency('sdl2')],
    install: false
  )
endforeach

subdir ('test')
