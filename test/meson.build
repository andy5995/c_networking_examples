vg = find_program('valgrind', required : false)

if vg.found()
  add_test_setup(
    'valgrind',
    exe_wrapper : [join_paths(meson.current_source_dir(), 'valgrind_wrap.sh')]
  )
endif

scripts = [
  'test_tcp_file_transfer_server.sh',
  'test_tcp_file_transfer_client.sh',
  ]

foreach s : scripts
  test(s,
    files(s),
    env : [
      'MESON_BUILD_ROOT=' + meson.project_build_root(),
      'MESON_SOURCE_ROOT=' + meson.project_source_root(),
      ])
endforeach
