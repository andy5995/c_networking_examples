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
