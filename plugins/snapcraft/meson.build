if get_option('with_snapcraft')

snapcraft_resources = gnome.compile_resources(
  'gbp-snap-resources',
  'gbp-snap-resources.gresource.xml',
  c_name: 'gbp_snap'
)

snapcraft_sources = [
  snapcraft_resources,
  'gbp-snap-plugin.c',
  'gbp-snap-utils.c',
  'gbp-snap-utils.h',
  'gbp-snap-build-system.c',
  'gbp-snap-build-system.h',
  'gbp-snap-pipeline-addin.c',
  'gbp-snap-pipeline-addin.h',
  'gbp-snap-workbench-addin.c',
  'gbp-snap-workbench-addin.h',
]

snapcraft_deps = plugin_deps + [
  dependency('yaml-0.1', version: '>= 0.1.6'),
]

shared_module('snapcraft-plugin', snapcraft_sources,
  dependencies: snapcraft_deps,
  link_args: plugin_link_args,
  link_depends: plugin_link_deps,
  install: true,
  install_dir: plugindir,
)

configure_file(
  input: 'snapcraft.plugin',
  output: 'snapcraft.plugin',
  configuration: configuration_data(),
  install: true,
  install_dir: plugindir,
)

endif
