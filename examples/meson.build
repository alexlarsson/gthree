examples = [
  'cubes',
  'envmap',
  'gtklogo',
  'materials',
  'model',
  'multi',
  'normals',
  'performance',
  'shader',
]

foreach example_name : examples
  executable(example_name, ['@0@.c'.format(example_name), 'utils.c'],
             dependencies: [libgthree_dep, libm])
endforeach

