project('gthree', 'c',
        version: '0.2.0',
        default_options: [
          'buildtype=debugoptimized',
          'warning_level=1',
          'c_std=gnu99',
        ],
        meson_version : '>= 0.50.0',
        license: 'MIT')

version = meson.project_version().split('.')
gthree_major_version = version[0].to_int()
gthree_minor_version = version[1].to_int()
gthree_micro_version = version[2].to_int()

gthree_interface_age = 1
gthree_binary_age = 10000 * gthree_major_version + 100 * gthree_minor_version + 10 + gthree_micro_version

gthree_api_version = '1.0'
gthree_api_path = 'gthree-@0@/gthree'.format(gthree_api_version)
gthree_gtk3_api_path = 'gthree-gtk3-@0@/gthree'.format(gthree_api_version)
gthree_gtk4_api_path = 'gthree-gtk4-@0@/gthree'.format(gthree_api_version)

add_project_arguments('-D_GNU_SOURCE', language: 'c')

cc = meson.get_compiler('c')
libm = cc.find_library('m', required: false)

glib_dep       = dependency('glib-2.0', version: '>= 2.43.2')
gobject_dep    = dependency('gobject-2.0', version: '>= 2.43.2')
graphene_dep   = dependency('graphene-gobject-1.0', version: '>= 1.10.0',
                            fallback: ['graphene', 'graphene_dep'])
gdkpixbuf_dep  = dependency('gdk-pixbuf-2.0')
cairo_dep      = dependency('cairo-gobject')
gtk3_dep       = dependency('gtk+-3.0', version: '>= 3.22',
                            required: get_option('gtk3'))
gtk4_dep       = dependency('gtk4', version: '>= 3.96',
                            required: get_option('gtk4'),
                            fallback: ['gtk', 'libgtk_dep'],
                            default_options: ['media=none', 'print-backends=none',
                                              'demos=false',
                                              'build-examples=false', 'build-tests=false'])
epoxy_dep      = dependency('epoxy', version: '>= 1.4')
json_glib_dep  = dependency('json-glib-1.0', version: '>= 1.2.0')

gthree_prefix = get_option('prefix')
gthree_libdir = join_paths(gthree_prefix, get_option('libdir'))
gthree_includedir = join_paths(gthree_prefix, get_option('includedir'))
gthree_datadir = join_paths(gthree_prefix, get_option('datadir'))

if graphene_dep.type_name() == 'internal'
  graphene_is_subproject = true
  graphene_subproject_typelib_dir = join_paths(meson.build_root(), 'subprojects', 'graphene', 'src')
else
  graphene_is_subproject = false
  graphene_subproject_typelib_dir = ''
endif

if gtk4_dep.type_name() == 'internal'
  gtk4_is_subproject = true
  gtk4_subproject_typelib_dir = join_paths(meson.build_root(), 'subprojects', 'gtk', 'src')
else
  gtk4_is_subproject = false
  gtk4_subproject_typelib_dir = ''
endif

# Maintain version scheme with libtool
gthree_soversion = 0
current = gthree_binary_age - gthree_interface_age
gthree_libversion = '@0@.@1@.@2@'.format(gthree_soversion, current, gthree_interface_age)

# Compiler and linker flags
common_cflags = []
common_ldflags = []

gthree_conf = configuration_data()
gthree_conf.set_quoted('VERSION', meson.project_version())

gthree_packages = ' '.join([ 'glib-2.0', 'gobject-2.0', 'graphene-gobject-1.0' ])
gthree_private_packages = ' '.join([ 'epoxy', 'json-glib-1.0' ])
gthree_gtk3_packages = ' '.join([ 'gthree-1.0', 'gtk+-3.0' ])
gthree_gtk4_packages = ' '.join([ 'gthree-1.0', 'gtk4' ])

# Compat variables for pkgconfig
pkgconf = configuration_data()
pkgconf.set('prefix', gthree_prefix)
pkgconf.set('exec_prefix', gthree_prefix)
pkgconf.set('libdir', gthree_libdir)
pkgconf.set('includedir', gthree_includedir)
pkgconf.set('VERSION', meson.project_version())
pkgconf.set('srcdir', '.')
pkgconf.set('GTHREE_PACKAGES', gthree_packages)
pkgconf.set('GTHREE_PRIVATE_PACKAGES', gthree_private_packages)
pkgconf.set('GTHREE_GTK3_PACKAGES', gthree_gtk3_packages)
pkgconf.set('GTHREE_GTK4_PACKAGES', gthree_gtk4_packages)

pkgfiles = [ 'gthree-1.0.pc' ]
if get_option('gtk3')
  pkgfiles += [ 'gthree-gtk3-1.0.pc' ]
endif
if get_option('gtk4')
  pkgfiles += [ 'gthree-gtk4-1.0.pc' ]
endif

foreach pkgfile: pkgfiles
  configure_file(input: pkgfile + '.in',
                 output: pkgfile,
                 configuration: pkgconf,
                 install_dir: join_paths(gthree_libdir, 'pkgconfig'))
endforeach

gnome = import('gnome')

configure_file(output: 'config.h', configuration: gthree_conf)

root_inc = include_directories('.')

subdir('gthree')
subdir('examples')

if get_option('gtk_doc')
  subdir('docs')
endif

summary = [
  '',
  '------',
  'Gthree @0@ (@1@)'.format(meson.project_version(), gthree_api_version),
  '',
  '  GTK 3 widgetry: @0@'.format(get_option('gtk3')),
  '  GTK 4 widgetry: @0@'.format(get_option('gtk4')),
  '   Introspection: @0@'.format(get_option('introspection')),
  '   Documentation: @0@'.format(get_option('gtk_doc')),
  'Directories:',
  '          prefix: @0@'.format(gthree_prefix),
  '      includedir: @0@'.format(gthree_includedir),
  '          libdir: @0@'.format(gthree_libdir),
  '         datadir: @0@'.format(gthree_datadir),
  '------',
  ''
]

message('\n'.join(summary))

