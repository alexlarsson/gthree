examples = [
  'cubes',
  'envmap',
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

foreach example_name : examples
  executable(example_name, ['@0@.c'.format(example_name), 'utils.c', 'prop-editor.c'] + example_resources,
             dependencies: [libgthree_dep, libm])
endforeach
