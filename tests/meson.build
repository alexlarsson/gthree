if get_option('gtk4')
  test_sources = [
    'test-main.c',
    'test-common.c',
    'test-basic.c',
    'test-materials.c',
    'test-lights.c',
    'test-features.c',
    'test-pbr.c',
    'test-advanced.c',
  ]

  gthree_visual_tests = executable('gthree-visual-tests',
    test_sources,
    dependencies: [libgthree_dep, gtk4_dep, epoxy_dep, libm],
    link_with: [libgthree, libgthree_gtk4],
    c_args: ['-DSRCDIR="@0@"'.format(meson.current_source_dir())],
    install: false,
  )

  configure_file(
    input: 'run-tests.sh',
    output: 'run-tests.sh',
    copy: true,
  )

  run_target('visual-test',
    command: [find_program('bash'), meson.current_build_dir() / 'run-tests.sh'],
  )

  run_target('visual-test-update-ref',
    command: [find_program('bash'), meson.current_build_dir() / 'run-tests.sh', '--update-ref'],
  )
endif
