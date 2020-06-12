examples = [
  'cairo',
  'cubes',
  'effects',
  'envmap',
  'gtklogo',
  'interactive',
  'materials',
  'model',
  'morphtargets',
  'multi',
  'normals',
  'performance',
  'points',
  'properties',
  'rendertarget',
  'shader',
  'shadow',
  'skinning',
  'sprites',
  'testanimation',
  'clipping',
]

example_resources = gnome.compile_resources('gthree-example-resources',
                                            'examples.gresource.xml',
                                            source_dir: [
                                                # List in order of preference
                                                meson.current_build_dir(),
                                                meson.current_source_dir(),
                                            ],
                                            c_name: 'gthree_example')

libexample_sources = [
  'prop-editor.c',
  'utils.c',
  'orbitcontrols.c',
] + example_resources

if get_option('gtk3')
  libexample_gtk3 = static_library(
    'libexample-gtk3',
    libexample_sources,
    c_args: [ '-DUSE_GTK3' ],
    dependencies: [
      libgthree_gtk3_dep, libm ],
    install: false,
  )

  libexample_gk3_dep = declare_dependency(
    link_whole: libexample_gtk3,
    dependencies: [
      libgthree_gtk3_dep, libm ],
  )

  foreach example : examples
    executable(
      '@0@-gtk3'.format(example),
      '@0@.c'.format(example),
      c_args: [ '-DUSE_GTK3' ],
      dependencies: [
        libexample_gk3_dep ],
      install: false)
  endforeach
endif

if get_option('gtk4')
  libexample_gtk4 = static_library(
    'libexample-gtk4',
    libexample_sources,
    c_args: [ '-DUSE_GTK4' ],
    dependencies: [
      libgthree_gtk4_dep, libm ],
    install: false,
  )

  libexample_gk4_dep = declare_dependency(
    link_whole: libexample_gtk4,
    dependencies: [
      libgthree_gtk4_dep, libm ],
  )

  foreach example : examples
    executable(
      '@0@-gtk4'.format(example),
      '@0@.c'.format(example),
      c_args: [ '-DUSE_GTK4' ],
      dependencies: [
        libexample_gk4_dep],
      install: false)
  endforeach
endif
