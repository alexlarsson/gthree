examples = [
  'cubes',
  'envmap',
  'skinning',
  'gtklogo',
  'materials',
  'properties',
  'model',
  'multi',
  'normals',
  'performance',
  'shader',
]

example_resources = gnome.compile_resources('gthree-example-resources',
                                            'examples.gresource.xml',
                                            source_dir: [
                                                # List in order of preference
                                                meson.current_build_dir(),
                                                meson.current_source_dir(),
                                            ],
                                            c_name: 'gthree_example')

libexample_sources = ['utils.c', 'prop-editor.c']  + example_resources

libexample = static_library(
  'libexample',
  libexample_sources,
  dependencies: [libgthree_dep, libm],
)

example_dep = declare_dependency(
  link_whole: libexample,
  dependencies: [libgthree_dep, libm]
)

foreach example_name : examples
  executable(example_name, ['@0@.c'.format(example_name)], dependencies: example_dep)
endforeach
