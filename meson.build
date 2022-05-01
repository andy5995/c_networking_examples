project(
  'c_networking_examples',
  'c',
  version: '0.1.0.555',
  meson_version : '>= 0.56.0',
  default_options: ['warning_level=3']
  )

cc = meson.get_compiler('c')
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

example = [
  'tcp_file_transfer_client',
  'tcp_file_transfer_server',
  'tcp_chat_client',
  'tcp_chat_server',
  'udp_echo_client',
  'udp_echo_server'
  ]

buildtarget = library(
  'lib',
  'lib.c',
  # include_directories : inc,
  version : '0.1.0',
  soversion : '0',
  install: false)

# How to use in a superproject and other info
# https://mesonbuild.com/Subprojects.html
dep_lib = declare_dependency(link_with : buildtarget)

foreach file : example
  executable(
    file,
    file + '.c',
    dependencies: dep_lib,
    install: false
  )
endforeach

subdir ('test')
