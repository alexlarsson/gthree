gthree_sources = [
    'gthreeattribute.c',
    'gthreeambientlight.c',
    'gthreearea.c',
    'gthreemeshbasicmaterial.c',
    'gthreebone.c',
    'gthreeskeleton.c',
    'gthreegroup.c',
    'gthreecamera.c',
    'gthreecubetexture.c',
    'gthreeeffectcomposer.c',
    'gthreepass.c',
    'gthreemeshdepthmaterial.c',
    'gthreedirectionallight.c',
    'gthreegeometry.c',
    'gthreemeshlambertmaterial.c',
    'gthreelight.c',
    'gthreelightshadow.c',
    'gthreelinebasicmaterial.c',
    'gthreelinesegments.c',
    'gthreeloader.c',
    'gthreematerial.c',
    'gthreemesh.c',
    'gthreeskinnedmesh.c',
    'gthreemeshmaterial.c',
    'gthreemeshnormalmaterial.c',
    'gthreeobject.c',
    'gthreeperspectivecamera.c',
    'gthreeorthographiccamera.c',
    'gthreemeshphongmaterial.c',
    'gthreemeshstandardmaterial.c',
    'gthreepointlight.c',
    'gthreeprimitives.c',
    'gthreeprogram.c',
    'gthreerenderer.c',
    'gthreerendertarget.c',
    'gthreeresource.c',
    'gthreescene.c',
    'gthreeshader.c',
    'gthreeshadermaterial.c',
    'gthreesprite.c',
    'gthreespritematerial.c',
    'gthreepoints.c',
    'gthreepointsmaterial.c',
    'gthreetexture.c',
    'gthreeuniforms.c',
    'gthreeinterpolant.c',
    'gthreelinearinterpolant.c',
    'gthreediscreteinterpolant.c',
    'gthreecubicinterpolant.c',
    'gthreequaternioninterpolant.c',
    'gthreekeyframetrack.c',
    'gthreevectorkeyframetrack.c',
    'gthreecolorkeyframetrack.c',
    'gthreenumberkeyframetrack.c',
    'gthreequaternionkeyframetrack.c',
    'gthreeanimationclip.c',
    'gthreeanimationaction.c',
    'gthreeanimationmixer.c',
    'gthreepropertymixer.c',
    'gthreepropertybinding.c',
]

gthree_headers = [
    'gthree.h',
    'gthreeattribute.h',
    'gthreeambientlight.h',
    'gthreearea.h',
    'gthreemeshbasicmaterial.h',
    'gthreebone.h',
    'gthreeskeleton.h',
    'gthreecamera.h',
    'gthreecubetexture.h',
    'gthreeeffectcomposer.h',
    'gthreepass.h',
    'gthreemeshdepthmaterial.h',
    'gthreedirectionallight.h',
    'gthreeenums.h',
    'gthreegroup.h',
    'gthreegeometry.h',
    'gthreemeshlambertmaterial.h',
    'gthreelight.h',
    'gthreelightshadow.h',
    'gthreelinebasicmaterial.h',
    'gthreelinesegments.h',
    'gthreeloader.h',
    'gthreematerial.h',
    'gthreemesh.h',
    'gthreeskinnedmesh.h',
    'gthreemeshmaterial.h',
    'gthreemeshnormalmaterial.h',
    'gthreeobject.h',
    'gthreeperspectivecamera.h',
    'gthreeorthographiccamera.h',
    'gthreemeshphongmaterial.h',
    'gthreemeshstandardmaterial.h',
    'gthreepointlight.h',
    'gthreeprimitives.h',
    'gthreeprogram.h',
    'gthreerenderer.h',
    'gthreerendertarget.h',
    'gthreeresource.h',
    'gthreescene.h',
    'gthreeshader.h',
    'gthreeshadermaterial.h',
    'gthreesprite.h',
    'gthreespritematerial.h',
    'gthreepoints.h',
    'gthreepointsmaterial.h',
    'gthreetexture.h',
    'gthreetypes.h',
    'gthreeuniforms.h',
    'gthreeinterpolant.h',
    'gthreelinearinterpolant.h',
    'gthreediscreteinterpolant.h',
    'gthreecubicinterpolant.h',
    'gthreequaternioninterpolant.h',
    'gthreevectorkeyframetrack.h',
    'gthreecolorkeyframetrack.h',
    'gthreenumberkeyframetrack.h',
    'gthreequaternionkeyframetrack.h',
    'gthreeanimationclip.h',
    'gthreeanimationaction.h',
    'gthreeanimationmixer.h',
    'gthreekeyframetrack.h',
]

gthree_private_headers = [
    'gthreepropertymixerprivate.h',
    'gthreepropertybindingprivate.h',
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
                     includes: [ 'GObject-2.0', 'Graphene-1.0', 'GdkPixbuf-2.0', 'Gdk-3.0' ],
                     install: true,
                     extra_args: [
                       '--quiet',
                       '--c-include=gthree/gthree.h',
                       '-DGTHREE_COMPILATION',
                     ])
endif
