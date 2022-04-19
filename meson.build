project(
  'c_networking_examples',
  'c',
  version: '0.1.0.555',
  meson_version : '>= 0.48.0',
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
  'example_tcp_file_transfer_client',
  'example_tcp_file_transfer_server',
  ]

foreach file : example
  executable(
    file,
    file + '.c'
  )
endforeach
