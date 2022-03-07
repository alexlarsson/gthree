if not meson.is_cross_build() and get_option('introspection')
  extra_gir_args = [
    '--quiet',
    '--c-include=gthree/gthree.h',
    '-DGTHREE_COMPILATION',
  ]
  if graphene_is_subproject
    extra_gir_args = extra_gir_args + ['--add-include-path', graphene_subproject_typelib_dir]
  endif
  if gtk4_is_subproject
    extra_gir_args = extra_gir_args + ['--add-include-path', gtk4_subproject_typelib_dir]
  endif
endif

metadatadir = meson.current_source_dir() / 'metadata'

gthree_sources = [
    'gthreeattribute.c',
    'gthreeambientlight.c',
    'gthreemeshbasicmaterial.c',
    'gthreebone.c',
    'gthreeskeleton.c',
    'gthreegroup.c',
    'gthreecamera.c',
    'gthreecubetexture.c',
    'gthreeeffectcomposer.c',
    'gthreefog.c',
    'gthreepass.c',
    'gthreemeshdepthmaterial.c',
    'gthreemeshdistancematerial.c',
    'gthreedirectionallight.c',
    'gthreedirectionallightshadow.c',
    'gthreegeometry.c',
    'gthreemeshlambertmaterial.c',
    'gthreelight.c',
    'gthreelightshadow.c',
    'gthreelinebasicmaterial.c',
    'gthreelinesegments.c',
    'gthreeline.c',
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
    'gthreemeshtoonmaterial.c',
    'gthreemeshstandardmaterial.c',
    'gthreemeshspecglosmaterial.c',
    'gthreepointlight.c',
    'gthreespotlight.c',
    'gthreespotlightshadow.c',
    'gthreeprimitives.c',
    'gthreehelpers.c',
    'gthreeprogram.c',
    'gthreeraycaster.c',
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
    'gthreehemispherelight.c',
]

gthree_headers = [
    'gthree.h',
    'gthreeattribute.h',
    'gthreeambientlight.h',
    'gthreemeshbasicmaterial.h',
    'gthreebone.h',
    'gthreeskeleton.h',
    'gthreecamera.h',
    'gthreecubetexture.h',
    'gthreeeffectcomposer.h',
    'gthreefog.h',
    'gthreepass.h',
    'gthreemeshdepthmaterial.h',
    'gthreemeshdistancematerial.h',
    'gthreedirectionallight.h',
    'gthreedirectionallightshadow.h',
    'gthreeenums.h',
    'gthreegroup.h',
    'gthreegeometry.h',
    'gthreemeshlambertmaterial.h',
    'gthreelight.h',
    'gthreelightshadow.h',
    'gthreelinebasicmaterial.h',
    'gthreelinesegments.h',
    'gthreeline.h',
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
    'gthreemeshtoonmaterial.h',
    'gthreemeshstandardmaterial.h',
    'gthreemeshspecglosmaterial.h',
    'gthreepointlight.h',
    'gthreespotlight.h',
    'gthreespotlightshadow.h',
    'gthreeprimitives.h',
    'gthreehelpers.h',
    'gthreeprogram.h',
    'gthreeraycaster.h',
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
    'gthreehemispherelight.h',
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
                                           extra_args: '--manual-register',
                                           c_name: 'gthree')


gthree_cflags = [
  '-DGTHREE_COMPILATION',
]

gthree_inc = include_directories('.')

gthree_deps = [glib_dep, gobject_dep, graphene_dep, gdkpixbuf_dep, cairo_dep, json_glib_dep]
gthree_private_deps = [epoxy_dep, libm]

if get_option('shared_lib')
  libtype = 'shared_library'
else
  libtype = 'static_library'
endif

libgthree = build_target('gthree-1',
                           sources: gthree_sources + gthree_marshalers + gthree_typebuiltins + gthree_resources,
                           soversion: gthree_soversion,
                           version: gthree_libversion,
                           install: true,
                           dependencies: gthree_deps + gthree_private_deps,
                           include_directories: [ root_inc, gthree_inc ],
                           c_args: common_cflags + gthree_cflags,
                           link_args: common_ldflags,
                           target_type: libtype)
gthree_dep_sources = [gthree_typebuiltins_h]

if not meson.is_cross_build() and get_option('introspection')
   libgthree_gir = gnome.generate_gir(libgthree,
                                      sources: gthree_sources + gthree_headers + gthree_marshalers + [gthree_typebuiltins_h],
                                      namespace: 'Gthree',
                                      nsversion: gthree_api_version,
                                      identifier_prefix: 'Gthree',
                                      symbol_prefix: 'gthree',
                                      export_packages: 'gthree',
                                      includes: [ 'GObject-2.0', 'Graphene-1.0', 'GdkPixbuf-2.0', 'cairo-1.0' ],
                                      install: true,
                                      extra_args: extra_gir_args,
                		      header: 'gthree/gthree.h')
   gthree_dep_sources += libgthree_gir
   libgthree_gir_dep = declare_dependency(sources: libgthree_gir)

   if get_option('vapi')
     libgthree_vapi = gnome.generate_vapi('gthree-' + gthree_api_version,
                                          sources: libgthree_gir[0],
                                          packages: [ 'gio-2.0', 'cairo-gobject', 'graphene-1.0', 'gdk-pixbuf-2.0' ],
                                          install: true,
                                          metadata_dirs: metadatadir,)
   endif
endif

libgthree_dep = declare_dependency(sources: gthree_dep_sources,
                                   link_with: libgthree,
                                   include_directories: [gthree_inc, root_inc],
                                   dependencies: gthree_deps)

if get_option('gtk3')
  gthree_gtk3_sources = [
    'gthreearea.c',
  ]

  gthree_gtk3_headers = [
    'gthreearea.h',
  ]

  install_headers(gthree_gtk3_headers, subdir: gthree_gtk3_api_path)

  gthree_gtk3_deps = [libgthree_dep, gtk3_dep]

  libgthree_gtk3 = build_target('gthree-gtk3-1',
                                  sources: gthree_gtk3_sources,
                                  soversion: gthree_soversion,
                                  version: gthree_libversion,
                                  install: true,
                                  dependencies: gthree_gtk3_deps,
                                  include_directories: [ root_inc, gthree_inc ],
                                  c_args: common_cflags + gthree_cflags,
                                  link_args: common_ldflags,
                                  target_type: libtype)
  gthree_gtk3_dep_sources = []

  # This failed to build for some reason with static libs, ignore for now
  if not meson.is_cross_build() and get_option('shared_lib') and get_option('introspection')
    libgthree_gtk3_gir = gnome.generate_gir(libgthree_gtk3,
                                            sources: gthree_gtk3_sources + gthree_gtk3_headers,
                                            namespace: 'GthreeGtk3',
                                            nsversion: gthree_api_version,
                                            identifier_prefix: 'Gthree',
                                            symbol_prefix: 'gthree',
                                            export_packages: 'gthree',
                                            includes: [ libgthree_gir[0], 'Gtk-3.0' ],
                                            install: true,
                                            dependencies: libgthree_gir_dep,
                                            extra_args: extra_gir_args,
                           		    header: 'gthree/gthreearea.h')
     gthree_gtk3_dep_sources += libgthree_gtk3_gir
     libgthree_gtk3_gir_dep = declare_dependency(sources: libgthree_gtk3_gir)

    if get_option('vapi')
      libgthree_gtk3_vapi = gnome.generate_vapi('gthree-gtk3-' + gthree_api_version,
                                           sources: libgthree_gtk3_gir[0],
                                           packages: [ 'gtk+-3.0', libgthree_vapi ],
                                           install: true,
                                           metadata_dirs: [ meson.current_source_dir() ],
                                           vapi_dirs: meson.current_build_dir(),
                                           metadata_dirs: metadatadir,)
    endif
  endif

  libgthree_gtk3_dep = declare_dependency(sources: gthree_gtk3_dep_sources,
                                          link_with: libgthree_gtk3,
                                          include_directories: [gthree_inc, root_inc],
                                          dependencies: gthree_gtk3_deps)
endif

if get_option('gtk4')
  gthree_gtk4_sources = [
    'gthreearea.c',
  ]

  gthree_gtk4_headers = [
    'gthreearea.h',
  ]

  install_headers(gthree_gtk4_headers, subdir: gthree_gtk4_api_path)

  gthree_gtk4_deps = [libgthree_dep, gtk4_dep]

  libgthree_gtk4 = build_target('gthree-gtk4-1',
                                  sources: gthree_gtk4_sources,
                                  soversion: gthree_soversion,
                                  version: gthree_libversion,
                                  install: true,
                                  dependencies: gthree_gtk4_deps,
                                  include_directories: [ root_inc, gthree_inc ],
                                  c_args: common_cflags + gthree_cflags,
                                  link_args: common_ldflags,
                                  target_type: libtype)
  gthree_gtk4_dep_sources = []

  # This failed to build for some reason with static libs, ignore for now
  if not meson.is_cross_build() and get_option('shared_lib') and get_option('introspection')
    libgthree_gtk4_gir = gnome.generate_gir(libgthree_gtk4,
                                            sources: gthree_gtk4_sources + gthree_gtk4_headers,
                                            namespace: 'GthreeGtk4',
                                            nsversion: gthree_api_version,
                                            identifier_prefix: 'Gthree',
                                            symbol_prefix: 'gthree',
                                            export_packages: 'gthree',
                                            includes: [ libgthree_gir[0], 'Gtk-4.0' ],
                                            install: true,
                                            dependencies: libgthree_gir_dep,
                                            extra_args: extra_gir_args,
                           		    header: 'gthree/gthreearea.h')
     gthree_gtk4_dep_sources += libgthree_gtk4_gir
     libgthree_gtk4_gir_dep = declare_dependency(sources: libgthree_gtk4_gir)

     if get_option('vapi')
       libgthree_gtk4_vapi = gnome.generate_vapi('gthree-gtk4-' + gthree_api_version,
                                            sources: libgthree_gtk4_gir[0],
                                            packages: [ 'gtk4', libgthree_vapi ],
                                            install: true,
                                            metadata_dirs: [ meson.current_source_dir() ],
                                            vapi_dirs: meson.current_build_dir(),
                                            metadata_dirs: metadatadir,)
     endif
  endif

  libgthree_gtk4_dep = declare_dependency(sources: gthree_gtk4_dep_sources,
                                          link_with: libgthree_gtk4,
                                          include_directories: [gthree_inc, root_inc],
                                          dependencies: gthree_gtk4_deps)
endif
