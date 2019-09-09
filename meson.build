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

add_project_arguments('-D_GNU_SOURCE', language: 'c')

cc = meson.get_compiler('c')
libm = cc.find_library('m', required : false)

glib_dep       = dependency('glib-2.0', version: '>= 2.43.2')
gobject_dep    = dependency('gobject-2.0', version: '>= 2.43.2')
graphene_dep   = dependency('graphene-gobject-1.0', version: '>= 1.10.0',
                            fallback: ['graphene', 'graphene_dep'])
gtk_dep        = dependency('gtk+-3.0', version: '>= 3.22')
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

# Maintain version scheme with libtool
gthree_soversion = 0
current = gthree_binary_age - gthree_interface_age
gthree_libversion = '@0@.@1@.@2@'.format(gthree_soversion, current, gthree_interface_age)

# Compiler and linker flags
common_cflags = []
common_ldflags = []

gthree_conf = configuration_data()
gthree_conf.set_quoted('VERSION', meson.project_version())

gthree_packages = ' '.join([ 'glib-2.0', 'gobject-2.0', 'graphene-gobject-1.0', 'gtk+-3.0' ])
gthree_private_packages = ' '.join([ 'epoxy', 'json-glib-1.0' ])

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

foreach pkg: [ 'gthree-1.0.pc', ]
  configure_file(input: pkg + '.in',
                 output: pkg,
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
