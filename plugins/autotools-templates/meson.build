if get_option('with_autotools_templates')

autotools_template_datadir = join_paths(plugindatadir, 'autotools_templates')
autotools_template_dir = join_paths(plugindir, 'autotools_templates')

install_data('autotools-templates.plugin', install_dir: plugindir)
install_data('autotools_templates/__init__.py', install_dir: autotools_template_dir)
install_subdir('autotools_templates/resources', install_dir: autotools_template_datadir)

endif
