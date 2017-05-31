/* gbp-snap-build-stage.c
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

#define G_LOG_DOMAIN "gbp-snap-build-stage"

#include "gbp-snap-build-stage.h"

struct _GbpSnapBuildStage
{
  IdeBuildStage          parent;

  IdeSubprocessLauncher *launcher;
  IdeSubprocessLauncher *clean_launcher;
  IdeSubprocessLauncher *chained_launcher;
  GbpSnapBuildStep       step;
};

G_DEFINE_TYPE (GbpSnapBuildStage, gbp_snap_build_stage, IDE_TYPE_BUILD_STAGE);

static const char *build_step_names[] = {
  [GBP_SNAP_BUILD_STEP_PULL]  = "pull",
  [GBP_SNAP_BUILD_STEP_BUILD] = "build",
  [GBP_SNAP_BUILD_STEP_STAGE] = "stage",
  [GBP_SNAP_BUILD_STEP_PRIME] = "prime",
  [GBP_SNAP_BUILD_STEP_SNAP]  = "snap",
};

GbpSnapBuildStage *
gbp_snap_build_stage_new (IdeContext        *context,
                          IdeBuildPipeline  *pipeline,
                          GbpSnapBuildStep   step,
                          GError           **error)
{
  g_autoptr(GbpSnapBuildStage) self = NULL;

  IDE_ENTRY;

  self = g_object_new (GBP_TYPE_SNAP_BUILD_STAGE,
                       "context", context,
                       NULL);
  self->step = step;

  self->launcher = ide_build_pipeline_create_launcher (pipeline, error);
  if (!self->launcher)
    IDE_RETURN (NULL);

  ide_subprocess_launcher_push_argv (self->launcher, "snapcraft");
  ide_subprocess_launcher_push_argv (self->launcher, build_step_names[step]);

  if (step != GBP_SNAP_BUILD_STEP_SNAP)
    {
      self->clean_launcher = ide_build_pipeline_create_launcher (pipeline, error);
      if (!self->clean_launcher)
        IDE_RETURN (NULL);

      ide_subprocess_launcher_push_argv (self->clean_launcher, "snapcraft");
      ide_subprocess_launcher_push_argv (self->clean_launcher, "clean");
      ide_subprocess_launcher_push_argv (self->clean_launcher, "--step");
      ide_subprocess_launcher_push_argv (self->clean_launcher, build_step_names[step]);
    }

  IDE_RETURN (g_steal_pointer (&self));
}

static void
gbp_snap_build_stage_finalize (GObject *object)
{
  GbpSnapBuildStage *self = (GbpSnapBuildStage *) object;

  g_clear_object (&self->launcher);
  g_clear_object (&self->clean_launcher);
  g_clear_object (&self->chained_launcher);

  G_OBJECT_CLASS (gbp_snap_build_stage_parent_class)->finalize (object);
}

static void
gbp_snap_build_stage_wait_cb (GObject *object,
                              GAsyncResult *result,
                              gpointer user_data)
{
  IdeSubprocess *subprocess = (IdeSubprocess *)object;
  GbpSnapBuildStage *self = NULL;
  g_autoptr(GTask) task = user_data;
  g_autoptr(GError) error = NULL;
  int exit_status;

  IDE_ENTRY;

  g_assert (IDE_IS_SUBPROCESS (subprocess));
  g_assert (G_IS_ASYNC_RESULT (result));
  g_assert (G_IS_TASK (task));

  self = g_task_get_source_object (task);
  g_assert (GBP_IS_SNAP_BUILD_STAGE (self));

  if (!ide_subprocess_wait_finish (subprocess, result, &error))
    {
      g_task_return_error (task, g_steal_pointer (&error));
      IDE_EXIT;
    }

  exit_status = ide_subprocess_get_exit_status (subprocess);
  if (!g_spawn_check_exit_status (exit_status, &error))
    {
      g_task_return_error (task, g_steal_pointer (&error));
      IDE_EXIT;
    }

  g_task_return_boolean (task, TRUE);
  IDE_EXIT;
}

static void
gbp_snap_build_stage_run (IdeBuildStage         *stage,
                          IdeSubprocessLauncher *launcher,
                          IdeBuildPipeline      *pipeline,
                          GCancellable          *cancellable,
                          GAsyncReadyCallback    callback,
                          gpointer               user_data)
{
  g_autoptr(GTask) task = NULL;
  g_autoptr(GError) error = NULL;
  g_autoptr(IdeSubprocess) subprocess = NULL;
  GSubprocessFlags flags;
  const char *const *argv;
  g_autofree char *message = NULL;

  IDE_ENTRY;

  g_assert (GBP_IS_SNAP_BUILD_STAGE (stage));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (!launcher || IDE_IS_SUBPROCESS_LAUNCHER (launcher));
  g_assert (!cancellable || G_IS_CANCELLABLE(cancellable));

  task = g_task_new (stage, cancellable, callback, user_data);
  g_task_set_source_tag (task, gbp_snap_build_stage_run);

  if (!launcher)
    {
      g_task_return_boolean (task, TRUE);
      IDE_EXIT;
    }

  /* Force certain flags off and others on */
  flags = ide_subprocess_launcher_get_flags (launcher);

  flags &= ~(G_SUBPROCESS_FLAGS_STDERR_SILENCE |
             G_SUBPROCESS_FLAGS_STDERR_MERGE |
             G_SUBPROCESS_FLAGS_STDIN_INHERIT);
  flags |= (G_SUBPROCESS_FLAGS_STDOUT_PIPE |
            G_SUBPROCESS_FLAGS_STDERR_PIPE);

  ide_subprocess_launcher_set_flags (launcher, flags);

  /* Log command to stdout */
  argv = ide_subprocess_launcher_get_argv (launcher);
  message = g_strjoinv (" ", (char **)argv);
  ide_build_stage_log (stage, IDE_BUILD_LOG_STDOUT, message, -1);

  subprocess = ide_subprocess_launcher_spawn (launcher, cancellable, &error);

  if (!subprocess)
    {
      g_task_return_error (task, g_steal_pointer (&error));
      IDE_EXIT;
    }
  ide_build_stage_log_subprocess (stage, subprocess);

  IDE_TRACE_MSG ("Waiting for process %s to complete, %s exit status",
                 ide_subprocess_get_identifier (subprocess),
                 priv->ignore_exit_status ? "ignoring" : "checking");

  ide_subprocess_wait_async (subprocess,
                             cancellable,
                             gbp_snap_build_stage_wait_cb,
                             g_steal_pointer (&task));

  IDE_EXIT;
}

static void
gbp_snap_build_stage_execute_async (IdeBuildStage       *stage,
                                    IdeBuildPipeline    *pipeline,
                                    GCancellable        *cancellable,
                                    GAsyncReadyCallback  callback,
                                    gpointer             user_data)
{
  GbpSnapBuildStage *self = (GbpSnapBuildStage *)stage;
  g_autoptr(IdeSubprocessLauncher) launcher = NULL;

  g_return_if_fail (GBP_IS_SNAP_BUILD_STAGE (self));

  /* If we've chained to a later build step, use its launcher */
  launcher = g_steal_pointer (&self->chained_launcher);
  if (!launcher)
    launcher = g_object_ref (self->launcher);

  gbp_snap_build_stage_run (stage, launcher, pipeline, cancellable,
                            callback, user_data);
}

static gboolean
gbp_snap_build_stage_execute_finish (IdeBuildStage  *stage,
                                     GAsyncResult   *result,
                                     GError        **error)
{
  gboolean ret;

  IDE_ENTRY;

  g_assert (GBP_IS_SNAP_BUILD_STAGE (stage));
  g_assert (G_IS_TASK (result));

  ret = g_task_propagate_boolean (G_TASK (result), error);

  IDE_RETURN (ret);
}

static void
gbp_snap_build_stage_clean_async (IdeBuildStage       *stage,
                                  IdeBuildPipeline    *pipeline,
                                  GCancellable        *cancellable,
                                  GAsyncReadyCallback  callback,
                                  gpointer             user_data)
{
  GbpSnapBuildStage *self = (GbpSnapBuildStage *)stage;

  g_return_if_fail (GBP_IS_SNAP_BUILD_STAGE (self));

  gbp_snap_build_stage_run (stage, self->clean_launcher, pipeline, cancellable,
                            callback, user_data);
}

static gboolean
gbp_snap_build_stage_clean_finish (IdeBuildStage  *stage,
                                   GAsyncResult   *result,
                                   GError        **error)
{
  gboolean ret;

  IDE_ENTRY;

  g_assert (GBP_IS_SNAP_BUILD_STAGE (stage));
  g_assert (G_IS_TASK (result));

  ret = g_task_propagate_boolean (G_TASK (result), error);

  IDE_RETURN (ret);
}

static void
gbp_snap_build_stage_query (IdeBuildStage    *stage,
                            IdeBuildPipeline *pipeline,
                            GCancellable     *cancellable)
{
  IDE_ENTRY;

  g_return_if_fail (GBP_IS_SNAP_BUILD_STAGE (stage));
  g_return_if_fail (IDE_IS_BUILD_PIPELINE (pipeline));
  g_return_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable));

  /* Always run snapcraft: it keeps track of what has been completed */
  ide_build_stage_set_completed (stage, FALSE);

  IDE_EXIT;
}

static gboolean
gbp_snap_build_stage_chain (IdeBuildStage *stage,
                            IdeBuildStage *next_stage)
{
  GbpSnapBuildStage *self = (GbpSnapBuildStage *)stage;
  GbpSnapBuildStage *next = (GbpSnapBuildStage *)next_stage;

  IDE_ENTRY;

  g_assert (GBP_IS_SNAP_BUILD_STAGE (self));

  if (!GBP_IS_SNAP_BUILD_STAGE (next))
    IDE_RETURN (FALSE);

  if (next->step > self->step)
    {
      g_clear_object (&self->chained_launcher);
      self->chained_launcher = g_object_ref (next->launcher);
      IDE_RETURN (TRUE);
    }

  IDE_RETURN (FALSE);
}

static void
gbp_snap_build_stage_class_init (GbpSnapBuildStageClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  IdeBuildStageClass *build_stage_class = IDE_BUILD_STAGE_CLASS (klass);

  object_class->finalize = gbp_snap_build_stage_finalize;

  build_stage_class->execute_async = gbp_snap_build_stage_execute_async;
  build_stage_class->execute_finish = gbp_snap_build_stage_execute_finish;
  build_stage_class->clean_async = gbp_snap_build_stage_clean_async;
  build_stage_class->clean_finish = gbp_snap_build_stage_clean_finish;
  build_stage_class->query = gbp_snap_build_stage_query;
  build_stage_class->chain = gbp_snap_build_stage_chain;
}

static void
gbp_snap_build_stage_init (GbpSnapBuildStage *self)
{
}

