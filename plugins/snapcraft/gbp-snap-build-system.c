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
  IdeObject parent;
  GFile *project_file;
};

enum {
  PROP_0,
  PROP_PROJECT_FILE,
  LAST_PROP
};

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

  if (snapcraft_yaml)
    {
      g_task_return_pointer (task, g_steal_pointer (&snapcraft_yaml),
                             g_object_unref);
    }
  else
    {
      g_task_return_new_error (task,
                               G_IO_ERROR,
                               G_IO_ERROR_NOT_FOUND,
                               "Failed to locate snapcraft.yaml");
    }
  IDE_EXIT;
}

static void
gbp_snap_build_system_init_async (GAsyncInitable      *initable,
                                  gint                 io_priority,
                                  GCancellable        *cancellable,
                                  GAsyncReadyCallback  callback,
                                  gpointer             user_data)
{
  GbpSnapBuildSystem *system = (GbpSnapBuildSystem *)initable;
  g_autoptr(GTask) task = NULL;
  IdeContext *context;
  GFile *project_file;

  g_return_if_fail (GBP_IS_SNAP_BUILD_SYSTEM (system));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));

  task = g_task_new (initable, cancellable, callback, user_data);
  context = ide_object_get_context (IDE_OBJECT (system));
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
  g_autoptr(GFile) project_file = NULL;

  g_return_val_if_fail (GBP_IS_SNAP_BUILD_SYSTEM (initable), FALSE);
  g_return_val_if_fail (G_IS_TASK (task), FALSE);

  project_file = g_task_propagate_pointer (task, error);
  if (project_file)
    {
      g_object_set (initable, "project-file", project_file, NULL);
    }

  return !!project_file;
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
  iface->get_id = gbp_snap_build_system_get_id;
  iface->get_display_name = gbp_snap_build_system_get_display_name;
}

G_DEFINE_TYPE_WITH_CODE (GbpSnapBuildSystem,
                         gbp_snap_build_system,
                         IDE_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_ASYNC_INITABLE, async_initable_iface_init)
                         G_IMPLEMENT_INTERFACE (IDE_TYPE_BUILD_SYSTEM, build_system_iface_init))

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
      g_clear_object (&self->project_file);
      self->project_file = g_value_dup_object (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
gbp_snap_build_system_class_init (GbpSnapBuildSystemClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = gbp_snap_build_system_get_property;
  object_class->set_property = gbp_snap_build_system_set_property;

  g_object_class_override_property (object_class, PROP_PROJECT_FILE, "project-file");
}

static void
gbp_snap_build_system_init (GbpSnapBuildSystem *self)
{
}
