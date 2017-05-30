/* gbp-snap-workbench-addin.c
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

#define G_LOG_DOMAIN "gbp-snap-workbench-addin"

#include <glib/gi18n.h>

#include "gbp-snap-workbench-addin.h"
#include "gbp-snap-build-system.h"

static void
gbp_snap_workbench_addin_export_cb (GObject      *object,
                                    GAsyncResult *result,
                                    gpointer      user_data)
{
  IdeBuildManager *manager = (IdeBuildManager *)object;
  g_autoptr(GError) error = NULL;

  g_assert (IDE_IS_BUILD_MANAGER (manager));
  g_assert (G_IS_ASYNC_RESULT (result));

  if (!ide_build_manager_execute_finish (manager, result, &error))
    {
      g_warning ("%s", error->message);
    }
}

static void
gbp_snap_workbench_addin_export (GSimpleAction *action,
                                 GVariant      *param,
                                 gpointer       user_data)
{
  IdeWorkbench *workbench = user_data;
  IdeContext *context;
  IdeBuildManager *manager;

  g_assert (G_IS_SIMPLE_ACTION (action));
  g_assert (IDE_IS_WORKBENCH (workbench));

  context = ide_workbench_get_context (workbench);
  manager = ide_context_get_build_manager (context);
  ide_build_manager_execute_async (manager,
                                   IDE_BUILD_PHASE_EXPORT,
                                   NULL,
                                   gbp_snap_workbench_addin_export_cb,
                                   NULL);
}

static void
gbp_snap_workbench_addin_load (IdeWorkbenchAddin *addin,
                               IdeWorkbench      *workbench)
{
  static const GActionEntry actions[] = {
    { "export", gbp_snap_workbench_addin_export },
  };
  g_autoptr(GSimpleActionGroup) action_group = NULL;
  IdeContext *context;
  IdeBuildSystem *build_system;

  g_assert (GBP_IS_SNAP_WORKBENCH_ADDIN (addin));
  g_assert (IDE_IS_WORKBENCH (workbench));

  context = ide_workbench_get_context (workbench);
  if (!context)
    return;

  /* If this isn't a Snapcraft project, remove the associated menu items */
  build_system = ide_context_get_build_system (context);
  if (!GBP_IS_SNAP_BUILD_SYSTEM (build_system))
    {
      GMenu *menu = ide_application_get_menu_by_id (IDE_APPLICATION_DEFAULT,
                                                    "gear-menu-snap-section");
      g_menu_remove_all (menu);
      return;
    }

  action_group = g_simple_action_group_new ();
  g_action_map_add_action_entries (G_ACTION_MAP (action_group),
                                   actions,
                                   G_N_ELEMENTS (actions),
                                   workbench);
  gtk_widget_insert_action_group (GTK_WIDGET (workbench), "snap", G_ACTION_GROUP (action_group));
}

static void
gbp_snap_workbench_addin_unload (IdeWorkbenchAddin *addin,
                                 IdeWorkbench      *workbench)
{
  g_assert (GBP_IS_SNAP_WORKBENCH_ADDIN (addin));
  g_assert (IDE_IS_WORKBENCH (workbench));

  g_message("Unloading snap workbench addin");

  gtk_widget_insert_action_group (GTK_WIDGET (workbench), "snap", NULL);
}

static void
workbench_addin_iface_init (IdeWorkbenchAddinInterface *iface)
{
  iface->load = gbp_snap_workbench_addin_load;
  iface->unload = gbp_snap_workbench_addin_unload;
}

struct _GbpSnapWorkbenchAddin
{
  GObject parent;
};

G_DEFINE_TYPE_WITH_CODE (GbpSnapWorkbenchAddin, gbp_snap_workbench_addin, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (IDE_TYPE_WORKBENCH_ADDIN,
                                                workbench_addin_iface_init))

static void
gbp_snap_workbench_addin_class_init (GbpSnapWorkbenchAddinClass *klass)
{
}

static void
gbp_snap_workbench_addin_init (GbpSnapWorkbenchAddin *self)
{
}
