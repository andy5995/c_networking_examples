project(
  'c_networking_examples',
  ['c', 'cpp'],
  version: '0.1.0.555',
  meson_version : '>= 0.56.0',
  default_options: ['warning_level=3']
  )

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')
extra_flags = [
  '-fno-common',
  '-fanalyzer',
  '-Wformat-security',
  '-Wformat-overflow=2',
  '-Wshadow',
  ]

foreach cflag : extra_flags
  if cc.has_argument(cflag)
    add_project_arguments(cflag, language: 'c')
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

cpp_example = [
  'dual_stack_echo_client_cpp_base',
  'dual_stack_echo_server_cpp_base',
  ]

sdl_example = [
  'dual_stack_sdl2_window_server',
  'dual_stack_sdl2_window_client',
  ]

build_target = library(
  'netex',
  'netex.c',
  version : '0.1.0',
  soversion : '0',
  install: false)

foreach file : c_example
  executable(
    file,
    file + '.c',
    link_with: build_target,
    install: false
  )
endforeach


foreach file : cpp_example
  executable(
    file,
    file + '.cpp',
    link_with: build_target,
    install: false
  )
endforeach

foreach file : sdl_example
  executable(
    file,
    file + '.cpp',
    link_with: [build_target],
    dependencies: [dependency('sdl2')],
    install: false
  )
endforeach

subdir ('test')
