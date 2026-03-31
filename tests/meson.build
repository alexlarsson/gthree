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

test_c_args = ['-DSRCDIR="@0@"'.format(meson.current_source_dir())]

if get_option('gtk4')
  executable('gthree-visual-tests',
    test_sources,
    dependencies: [libgthree_dep, gtk4_dep, epoxy_dep, libm],
    link_with: [libgthree, libgthree_gtk4],
    c_args: test_c_args + ['-DUSE_GTK4'],
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

if get_option('gtk3')
  executable('gthree-visual-tests-gtk3',
    test_sources,
    dependencies: [libgthree_dep, gtk3_dep, epoxy_dep, libm],
    link_with: [libgthree, libgthree_gtk3],
    c_args: test_c_args + ['-DUSE_GTK3'],
    install: false,
  )
endif
