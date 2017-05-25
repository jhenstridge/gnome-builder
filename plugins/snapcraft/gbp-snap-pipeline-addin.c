/* gbp-snap-pipeline-addin.c
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

#define G_LOG_DOMAIN "gbp-snap-pipeline-addin"

#include "gbp-snap-pipeline-addin.h"
#include "gbp-snap-build-system.h"

enum {
  EXPORT_STAGE,
  EXPORT_PRIME,
  EXPORT_SNAP,
};

static IdeBuildStage *
make_build_stage (IdeBuildPipeline  *pipeline,
                  IdeContext        *context,
                  const char        *step,
                  GError           **error)
{
  g_autoptr(IdeSubprocessLauncher) launcher = NULL;
  g_autoptr(IdeSubprocessLauncher) clean_launcher = NULL;

  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));
  g_assert (step != NULL);

  launcher = ide_build_pipeline_create_launcher (pipeline, error);
  if (!launcher)
    return NULL;

  ide_subprocess_launcher_push_argv (launcher, "snapcraft");
  ide_subprocess_launcher_push_argv (launcher, step);

  clean_launcher = ide_build_pipeline_create_launcher (pipeline, error);
  if (!clean_launcher)
    return NULL;

  ide_subprocess_launcher_push_argv (clean_launcher, "snapcraft");
  ide_subprocess_launcher_push_argv (clean_launcher, "clean");
  ide_subprocess_launcher_push_argv (clean_launcher, "--step");
  ide_subprocess_launcher_push_argv (clean_launcher, step);

  return g_object_new (IDE_TYPE_BUILD_STAGE_LAUNCHER,
                       "context", context,
                       "launcher", launcher,
                       "clean-launcher", clean_launcher,
                       NULL);
}

static gboolean
register_pull_stage (GbpSnapPipelineAddin  *self,
                     IdeBuildPipeline      *pipeline,
                     IdeContext            *context,
                     GError               **error)
{
  g_autoptr(IdeBuildStage) stage = NULL;
  guint stage_id;

  g_assert (GBP_IS_SNAP_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  stage = make_build_stage (pipeline, context, "pull", error);
  if (!stage)
    return FALSE;

  stage_id = ide_build_pipeline_connect (pipeline,
                                         IDE_BUILD_PHASE_DOWNLOADS,
                                         0,
                                         stage);
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);
  return TRUE;
}

static gboolean
register_build_stage (GbpSnapPipelineAddin  *self,
                      IdeBuildPipeline      *pipeline,
                      IdeContext            *context,
                      GError               **error)
{
  g_autoptr(IdeBuildStage) stage = NULL;
  guint stage_id;

  g_assert (GBP_IS_SNAP_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  stage = make_build_stage (pipeline, context, "build", error);
  if (!stage)
    return FALSE;

  stage_id = ide_build_pipeline_connect (pipeline,
                                         IDE_BUILD_PHASE_BUILD,
                                         0,
                                         stage);
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);
  return TRUE;
}

static gboolean
register_stage_stage (GbpSnapPipelineAddin  *self,
                      IdeBuildPipeline      *pipeline,
                      IdeContext            *context,
                      GError               **error)
{
  g_autoptr(IdeBuildStage) stage = NULL;
  guint stage_id;

  g_assert (GBP_IS_SNAP_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  stage = make_build_stage (pipeline, context, "stage", error);
  if (!stage)
    return FALSE;

  stage_id = ide_build_pipeline_connect (pipeline,
                                         IDE_BUILD_PHASE_EXPORT,
                                         EXPORT_STAGE,
                                         stage);
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);
  return TRUE;
}

static gboolean
register_prime_stage (GbpSnapPipelineAddin  *self,
                      IdeBuildPipeline      *pipeline,
                      IdeContext            *context,
                      GError               **error)
{
  g_autoptr(IdeBuildStage) stage = NULL;
  guint stage_id;

  g_assert (GBP_IS_SNAP_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  stage = make_build_stage (pipeline, context, "prime", error);
  if (!stage)
    return FALSE;

  stage_id = ide_build_pipeline_connect (pipeline,
                                         IDE_BUILD_PHASE_EXPORT,
                                         EXPORT_PRIME,
                                         stage);
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);
  return TRUE;
}

static gboolean
register_snap_stage (GbpSnapPipelineAddin  *self,
                     IdeBuildPipeline      *pipeline,
                     IdeContext            *context,
                     GError               **error)
{
  g_autoptr(IdeBuildStage) stage = NULL;
  guint stage_id;

  g_assert (GBP_IS_SNAP_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  stage = make_build_stage (pipeline, context, "snap", error);
  if (!stage)
    return FALSE;

  stage_id = ide_build_pipeline_connect (pipeline,
                                         IDE_BUILD_PHASE_EXPORT,
                                         EXPORT_SNAP,
                                         stage);
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);
  return TRUE;
}

static void
gbp_snap_pipeline_addin_load (IdeBuildPipelineAddin *addin,
                                 IdeBuildPipeline      *pipeline)
{
  GbpSnapPipelineAddin *self = (GbpSnapPipelineAddin *)addin;
  g_autoptr(GError) error = NULL;
  IdeContext *context;
  IdeBuildSystem *build_system;

  g_assert (GBP_IS_SNAP_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));

  context = ide_object_get_context (IDE_OBJECT (self));
  build_system = ide_context_get_build_system (context);

  if (!GBP_IS_SNAP_BUILD_SYSTEM (build_system))
      return;

  if (!register_pull_stage(self, pipeline, context, &error) ||
      !register_build_stage(self, pipeline, context, &error) ||
      !register_stage_stage(self, pipeline, context, &error) ||
      !register_prime_stage(self, pipeline, context, &error) ||
      !register_snap_stage(self, pipeline, context, &error))
    {
      g_assert (error != NULL);
      g_warning ("Failed to create Snapcraft launcher: %s", error->message);
    }
}

/* GObject boilerplate */

static void
build_pipeline_addin_iface_init (IdeBuildPipelineAddinInterface *iface)
{
  iface->load = gbp_snap_pipeline_addin_load;
}

struct _GbpSnapPipelineAddin { IdeObject parent_instance; };

G_DEFINE_TYPE_WITH_CODE (GbpSnapPipelineAddin, gbp_snap_pipeline_addin, IDE_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (IDE_TYPE_BUILD_PIPELINE_ADDIN,
                                                build_pipeline_addin_iface_init))

static void
gbp_snap_pipeline_addin_class_init (GbpSnapPipelineAddinClass *klass)
{
}

static void
gbp_snap_pipeline_addin_init (GbpSnapPipelineAddin *self)
{
}
