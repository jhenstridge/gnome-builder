/* gbp-snap-build-system.c
 *
 * Copyright (C) 2017 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define G_LOG_DOMAIN "gbp-snap-build-system"

#include "gbp-snap-build-system.h"
#include "gbp-snap-utils.h"

struct _GbpSnapBuildSystem
{
  IdeObject     parent;
  GFile        *project_file;
  GFile        *project_directory;

  GFileMonitor *project_file_monitor;
  char         *snap_name;
};

enum {
  PROP_0,
  PROP_PROJECT_FILE,
  LAST_PROP
};

const char *
gbp_snap_build_system_get_snap_name (GbpSnapBuildSystem *self)
{
  return self->snap_name;
}

static void
gbp_snap_build_system_snapcraft_changed (GbpSnapBuildSystem *self,
                                         GFile *file,
                                         GFile *other_file,
                                         GFileMonitorEvent event_type,
                                         gpointer user_data)
{
  gboolean snapcraft_changed = FALSE;
  g_return_if_fail (GBP_IS_SNAP_BUILD_SYSTEM (self));
  g_return_if_fail (G_IS_FILE (file));

  switch (event_type)
    {
    case G_FILE_MONITOR_EVENT_CHANGED:
    case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT:
    case G_FILE_MONITOR_EVENT_CREATED:
    case G_FILE_MONITOR_EVENT_MOVED_IN:
      if (g_file_equal (file, self->project_file))
        snapcraft_changed = TRUE;
      break;
    case G_FILE_MONITOR_EVENT_RENAMED:
      // Has some other file been renamed over snapcraft.yaml?
      if (g_file_equal (other_file, self->project_file))
        snapcraft_changed = TRUE;
      break;

    case G_FILE_MONITOR_EVENT_DELETED:
    case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED:
    case G_FILE_MONITOR_EVENT_PRE_UNMOUNT:
    case G_FILE_MONITOR_EVENT_UNMOUNTED:
    case G_FILE_MONITOR_EVENT_MOVED:
    case G_FILE_MONITOR_EVENT_MOVED_OUT:
    default:
      break;
    }

  if (snapcraft_changed)
    {
      g_clear_pointer (&self->snap_name, g_free);
      self->snap_name = gbp_snap_calculate_package_name (self->project_file, NULL);
    }
}

static void
gbp_snap_build_system_set_project_file (GbpSnapBuildSystem *self,
                                        GFile *project_file,
                                        GCancellable *cancellable)
{
  g_autoptr(GFile) parent = NULL;
  g_autofree char *basename = NULL;
  g_autofree char *parent_basename = NULL;
  g_autoptr(GError) error = NULL;

  g_clear_object (&self->project_file);
  g_clear_object (&self->project_directory);
  g_clear_object (&self->project_file_monitor);
  g_clear_pointer (&self->snap_name, g_free);

  self->project_file = g_object_ref (project_file);

  /* Determine the project directory: if the file ends in
   * "snap/snapcraft.yaml", then the project is rooted in the
   * parent. */
  basename = g_file_get_basename (project_file);
  parent = g_file_get_parent (project_file);
  parent_basename = g_file_get_basename (parent);
  if (!g_strcmp0(basename, "snapcraft.yaml") &&
      !g_strcmp0(parent_basename, "snap"))
    self->project_directory = g_file_get_parent (parent);
  else
    self->project_directory = g_object_ref (parent);

  self->project_file_monitor = g_file_monitor_file (self->project_file,
                                                    G_FILE_MONITOR_WATCH_MOVES,
                                                    cancellable,
                                                    &error);
  if (self->project_file_monitor)
    {
      g_signal_connect_object (self->project_file_monitor, "changed",
                               G_CALLBACK (gbp_snap_build_system_snapcraft_changed),
                               self, G_CONNECT_SWAPPED);
    }
  else
    {
      g_warning ("Could not create file monitor: %s", error->message);
    }
  self->snap_name = gbp_snap_calculate_package_name (project_file, cancellable);
}

static void
gbp_snap_build_system_discover_worker (GTask        *task,
                                       gpointer      source_object,
                                       gpointer      task_data,
                                       GCancellable *cancellable)
{
  GbpSnapBuildSystem *self = source_object;
  GFile *file = task_data;
  g_autofree char *basename = NULL;
  g_autoptr(GFile) snapcraft_yaml = NULL;

  IDE_ENTRY;
  g_assert (G_IS_TASK (task));
  g_assert (GBP_IS_SNAP_BUILD_SYSTEM (self));
  g_assert (G_IS_FILE (file));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  basename = g_file_get_basename (file);
  if (!g_strcmp0 (basename, "snapcraft.yaml") ||
      !g_strcmp0 (basename, ".snapcraft.yaml"))
    {
      // We've been passed  the snapcraft.yaml file itself
      snapcraft_yaml = g_object_ref (file);
    }
  else
    {
      // Otherwise, check if it looks like we're in a Snapcraft project
      g_autoptr(GFile) directory = NULL;
      if (g_file_query_file_type (file, 0, cancellable) == G_FILE_TYPE_DIRECTORY)
        directory = g_object_ref (file);
      else
        directory = g_file_get_parent (file);

      snapcraft_yaml = gbp_snap_find_snapcraft_yaml (directory, cancellable);
    }
  g_clear_pointer (&basename, g_free);

  if (!snapcraft_yaml)
    {
      g_task_return_new_error (task,
                               G_IO_ERROR,
                               G_IO_ERROR_NOT_FOUND,
                               "Failed to locate snapcraft.yaml");
      IDE_EXIT;
    }

  gbp_snap_build_system_set_project_file (self, snapcraft_yaml, cancellable);

  g_task_return_boolean (task, TRUE);
  IDE_EXIT;
}

static void
gbp_snap_build_system_init_async (GAsyncInitable      *initable,
                                  gint                 io_priority,
                                  GCancellable        *cancellable,
                                  GAsyncReadyCallback  callback,
                                  gpointer             user_data)
{
  GbpSnapBuildSystem *self = (GbpSnapBuildSystem *)initable;
  g_autoptr(GTask) task = NULL;
  IdeContext *context;
  GFile *project_file;

  g_return_if_fail (GBP_IS_SNAP_BUILD_SYSTEM (self));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));

  task = g_task_new (initable, cancellable, callback, user_data);
  context = ide_object_get_context (IDE_OBJECT (self));
  project_file = ide_context_get_project_file (context);
  g_task_set_task_data (task, g_object_ref (project_file), g_object_unref);
  g_task_run_in_thread (task, gbp_snap_build_system_discover_worker);
}

static gboolean
gbp_snap_build_system_init_finish (GAsyncInitable  *initable,
                                   GAsyncResult    *result,
                                   GError         **error)
{
  GTask *task = (GTask *)result;

  g_return_val_if_fail (GBP_IS_SNAP_BUILD_SYSTEM (initable), FALSE);
  g_return_val_if_fail (G_IS_TASK (task), FALSE);

  return g_task_propagate_boolean (task, error);
}


static void
async_initable_iface_init (GAsyncInitableIface *iface)
{
  iface->init_async = gbp_snap_build_system_init_async;
  iface->init_finish = gbp_snap_build_system_init_finish;
}

static int
gbp_snap_build_system_get_priority (IdeBuildSystem *build_system)
{
  return 200;
}

static char *
gbp_snap_build_system_get_builddir (IdeBuildSystem   *build_system,
                                    IdeConfiguration *configuration)
{
  GbpSnapBuildSystem *self = (GbpSnapBuildSystem *)build_system;

  g_assert (GBP_IS_SNAP_BUILD_SYSTEM (self));
  g_assert (IDE_IS_CONFIGURATION (configuration));

  if (!self->project_directory || !g_file_is_native (self->project_directory))
    return NULL;

  return g_file_get_path (self->project_directory);
}

static char *
gbp_snap_build_system_get_id (IdeBuildSystem *build_system)
{
  return g_strdup ("snapcraft");
}

static char *
gbp_snap_build_system_get_display_name (IdeBuildSystem *build_system)
{
  return g_strdup ("Snapcraft");
}

static void
build_system_iface_init (IdeBuildSystemInterface *iface)
{
  iface->get_priority = gbp_snap_build_system_get_priority;
  iface->get_builddir = gbp_snap_build_system_get_builddir;
  iface->get_id = gbp_snap_build_system_get_id;
  iface->get_display_name = gbp_snap_build_system_get_display_name;
}

G_DEFINE_TYPE_WITH_CODE (GbpSnapBuildSystem,
                         gbp_snap_build_system,
                         IDE_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_ASYNC_INITABLE, async_initable_iface_init)
                         G_IMPLEMENT_INTERFACE (IDE_TYPE_BUILD_SYSTEM, build_system_iface_init))

static void
gbp_snap_build_system_finalize (GObject *object)
{
  GbpSnapBuildSystem *self = (GbpSnapBuildSystem *)object;

  g_clear_object (&self->project_file);
  g_clear_object (&self->project_directory);
  g_clear_object (&self->project_file_monitor);
  g_clear_pointer (&self->snap_name, g_free);

  G_OBJECT_CLASS (gbp_snap_build_system_parent_class)->finalize (object);
}

static void
gbp_snap_build_system_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  GbpSnapBuildSystem *self = (GbpSnapBuildSystem *)object;

  switch (prop_id)
    {
    case PROP_PROJECT_FILE:
      g_value_set_object (value, self->project_file);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gbp_snap_build_system_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  GbpSnapBuildSystem *self = (GbpSnapBuildSystem *)object;

  switch (prop_id)
    {
    case PROP_PROJECT_FILE:
      gbp_snap_build_system_set_project_file (self, g_value_get_object (value), NULL);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gbp_snap_build_system_class_init (GbpSnapBuildSystemClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gbp_snap_build_system_finalize;
  object_class->get_property = gbp_snap_build_system_get_property;
  object_class->set_property = gbp_snap_build_system_set_property;

  g_object_class_override_property (object_class, PROP_PROJECT_FILE, "project-file");
}

static void
gbp_snap_build_system_init (GbpSnapBuildSystem *self)
{
}
