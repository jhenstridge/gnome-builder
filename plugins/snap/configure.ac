m4_define([yaml_required_version], [0.1.6])

# --enable-flatpak-plugin=yes/no/auto
AC_ARG_ENABLE([snap-plugin],
              [AS_HELP_STRING([--enable-snap-plugin=@<:@yes/no/auto@:>@],
                              [Build with support for snap.])],
              [enable_snap_plugin=$enableval],
              [enable_snap_plugin=auto])

AS_IF([test "$enable_snap_plugin" != no],[
	PKG_CHECK_MODULES(YAML,
	                  [yaml-0.1 >= yaml_required_version],
	                  [have_yaml=yes],
	                  [have_yaml=no])


	AS_IF([test "$enable_snap_plugin" = "yes" && test "$have_yaml" = "no"],[
	       AC_MSG_ERROR([--enable-snap-plugin requires libyaml >= yaml_required_version])
	])

	AS_IF([test "$have_yaml" = "yes"],[
		enable_snap_plugin=yes
	],[
		enable_snap_plugin=no
	])

])

AM_CONDITIONAL(ENABLE_SNAP_PLUGIN, [test "x$enable_snap_plugin" = "xyes"])
AC_CONFIG_FILES([plugins/snap/Makefile])
