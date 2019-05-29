gthree_sources = [
    'gthreeattribute.c',
    'gthreeambientlight.c',
    'gthreearea.c',
    'gthreebasicmaterial.c',
    'gthreecamera.c',
    'gthreecubetexture.c',
    'gthreedepthmaterial.c',
    'gthreedirectionallight.c',
    'gthreegeometry.c',
    'gthreelambertmaterial.c',
    'gthreelight.c',
    'gthreelinebasicmaterial.c',
    'gthreelinesegments.c',
    'gthreeloader.c',
    'gthreematerial.c',
    'gthreemesh.c',
    'gthreemeshmaterial.c',
    'gthreemultimaterial.c',
    'gthreenormalmaterial.c',
    'gthreeobject.c',
    'gthreeperspectivecamera.c',
    'gthreephongmaterial.c',
    'gthreepointlight.c',
    'gthreeprimitives.c',
    'gthreeprogram.c',
    'gthreerenderer.c',
    'gthreeresource.c',
    'gthreescene.c',
    'gthreeshader.c',
    'gthreeshadermaterial.c',
    'gthreetexture.c',
    'gthreeuniforms.c',
]

gthree_headers = [
    'gthree.h',
    'gthreeattribute.h',
    'gthreeambientlight.h',
    'gthreearea.h',
    'gthreebasicmaterial.h',
    'gthreecamera.h',
    'gthreecubetexture.h',
    'gthreedepthmaterial.h',
    'gthreedirectionallight.h',
    'gthreeenums.h',
    'gthreegeometry.h',
    'gthreelambertmaterial.h',
    'gthreelight.h',
    'gthreelinebasicmaterial.h',
    'gthreelinesegments.h',
    'gthreeloader.h',
    'gthreematerial.h',
    'gthreemesh.h',
    'gthreemeshmaterial.h',
    'gthreemultimaterial.h',
    'gthreenormalmaterial.h',
    'gthreeobject.h',
    'gthreeperspectivecamera.h',
    'gthreephongmaterial.h',
    'gthreepointlight.h',
    'gthreeprimitives.h',
    'gthreeprogram.h',
    'gthreerenderer.h',
    'gthreeresource.h',
    'gthreescene.h',
    'gthreeshader.h',
    'gthreeshadermaterial.h',
    'gthreetexture.h',
    'gthreetypes.h',
    'gthreeuniforms.h',
]

gthree_private_headers = [
    'gthreeobjectprivate.h',
    'gthreeprivate.h',
]

install_headers(gthree_headers, subdir: gthree_api_path)

gthree_marshalers = gnome.genmarshal('gthreemarshalers',
                                     sources: 'gthreemarshalers.list',
                                     prefix: '_gthree_marshal')
gthree_marshalers_h = gthree_marshalers[1]

gthree_typebuiltins = gnome.mkenums('gthreetypebuiltins',
                                    sources: gthree_headers,
                                    c_template: 'gthreetypebuiltins.c.template',
                                    h_template: 'gthreetypebuiltins.h.template',
                                    install_dir: join_paths(gthree_includedir, gthree_api_path),
                                    install_header: true)

gthree_typebuiltins_h = gthree_typebuiltins[1]

gthree_resources = gnome.compile_resources('gthree-resources',
                                           'gthree.gresource.xml',
                                           source_dir: [
                                               # List in order of preference
                                               meson.current_build_dir(),
                                               meson.current_source_dir(),
                                           ],
                                           c_name: 'gthree')


gthree_cflags = [
  '-DGTHREE_COMPILATION',
]

gthree_inc = include_directories('.')

gthree_deps = [glib_dep, gobject_dep, graphene_dep, gtk_dep, json_glib_dep]
gthree_private_deps = [epoxy_dep, libm]

libgthree = shared_library('gthree-1',
                           sources: gthree_sources + gthree_marshalers + gthree_typebuiltins + gthree_resources,
                           soversion: gthree_soversion,
                           version: gthree_libversion,
                           install: true,
                           dependencies: gthree_deps + gthree_private_deps,
                           include_directories: [ root_inc, gthree_inc ],
                           c_args: common_cflags + gthree_cflags,
                           link_args: common_ldflags)

gthree_dep_sources = gthree_typebuiltins_h

libgthree_dep = declare_dependency(sources: gthree_dep_sources,
                                   link_with: libgthree,
                                   include_directories: [gthree_inc, root_inc],
                                   dependencies: gthree_deps)

if not meson.is_cross_build() and get_option('introspection')
  gnome.generate_gir(libgthree,
                     sources: gthree_sources + gthree_headers + gthree_marshalers,
                     namespace: 'Gthree',
                     nsversion: gthree_api_version,
                     identifier_prefix: 'Gthree',
                     symbol_prefix: 'gthree',
                     export_packages: 'gthree',
                     includes: [ 'GObject-2.0' ],
                     install: true,
                     extra_args: [
                       '--quiet',
                       '--c-include=gthree/gthree.h',
                       '-DGTHREE_COMPILATION',
                     ])
endif
