wlroots_dep = dependency('wlroots-0.20', required: false)
wayland_server_dep = dependency('wayland-server', version: '>= 1.22', required: false)
compositor_xkbcommon_dep = dependency('xkbcommon', required: false)
pixman_dep = dependency('pixman-1', version: '>= 0.42.0', required: false)

compositor_egl_dep = dependency('egl', required: false)
compositor_epoxy_dep = dependency('epoxy', required: false)

compositor_deps_found = (wlroots_dep.found() and wayland_server_dep.found()
                         and compositor_xkbcommon_dep.found() and pixman_dep.found())

if compositor_deps_found and get_option('gtk4')
  compositor_deps = [wlroots_dep, wayland_server_dep, compositor_xkbcommon_dep, pixman_dep, compositor_epoxy_dep]
  compositor_c_args = ['-DUSE_GTK4', '-DWLR_USE_UNSTABLE']

  if compositor_egl_dep.found()
    compositor_deps += [compositor_egl_dep]
  endif

  executable('compositor-gtk4',
    ['compositor.c', 'wayland-source.c'],
    c_args: compositor_c_args,
    dependencies: [libgthree_gtk4_dep, libm] + compositor_deps,
    install: false)
endif
