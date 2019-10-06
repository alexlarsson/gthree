private_headers = [
  'gthreeobjectprivate.h',
  'gthreeprivate.h',
  'gthreepropertybindingprivate.h',
  'gthreepropertymixerprivate.h',
]

gtk3_headers = [
  'gthreearea.h',
]

content_files = [
  'overview.xml',
  'resources.xml',
]

expand_content_files = [
  'resources.xml',
]

graphene_prefix = dependency('graphene-1.0').get_pkgconfig_variable('prefix')
graphene_docpath = join_paths(graphene_prefix, 'share', 'gtk-doc', 'html')

glib_prefix = dependency('glib-2.0').get_pkgconfig_variable('prefix')
glib_docpath = join_paths(glib_prefix, 'share', 'gtk-doc', 'html')

gtk_prefix = dependency('gtk+-3.0').get_pkgconfig_variable('prefix')
gtk_docpath = join_paths(gtk_prefix, 'share', 'gtk-doc', 'html')

docpath = join_paths(gthree_datadir, 'gtk-doc', 'html')

gnome.gtkdoc('gthree',
  main_xml: 'gthree-docs.xml',
  src_dir: gthree_inc,
  dependencies: libgthree_dep,
  gobject_typesfile: 'gthree.types',
  scan_args: [
    '--rebuild-types',
    '--ignore-decorators=GTHREE_API',
    '--ignore-headers=' + ' '.join(private_headers + gtk3_headers),
  ],
  mkdb_args: [
    '--default-include=gthree/gthree.h',
  ],
  fixxref_args: [
    '--html-dir=@0@'.format(docpath),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'glib')),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gobject')),
    '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gio')),
    '--extra-dir=@0@'.format(join_paths(gtk_docpath, 'gtk')),
    '--extra-dir=@0@'.format(join_paths(gtk_docpath, 'gdk')),
    '--extra-dir=@0@'.format(join_paths(graphene_docpath, 'graphene')),
  ],
  content_files: content_files,
  expand_content_files: expand_content_files,
  install: true,
)

if get_option('gtk3')
  gnome.gtkdoc('gthree-gtk3',
    main_xml: 'gthree-gtk3-docs.xml',
    src_dir: gthree_inc,
    dependencies: libgthree_gtk3_dep,
    gobject_typesfile: 'gthree-gtk3.types',
    scan_args: [
      '--rebuild-types',
      '--ignore-decorators=GTHREE_API',
      '--ignore-headers=' + ' '.join(private_headers),
    ],
    mkdb_args: [
      '--default-include=gthree/gthreearea.h',
    ],
    fixxref_args: [
      '--html-dir=@0@'.format(docpath),
      '--extra-dir=@0@'.format(join_paths(glib_docpath, 'glib')),
      '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gobject')),
      '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gio')),
      '--extra-dir=@0@'.format(join_paths(gtk_docpath, 'gtk')),
      '--extra-dir=@0@'.format(join_paths(gtk_docpath, 'gdk')),
      '--extra-dir=@0@'.format(join_paths(graphene_docpath, 'graphene')),
    ],
    content_files: content_files,
    expand_content_files: expand_content_files,
    install: true,
  )
endif
