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
#include "gbp-snap-build-stage.h"

enum {
  EXPORT_STAGE,
  EXPORT_PRIME,
  EXPORT_SNAP,
};

static gboolean
register_pull_stage (GbpSnapPipelineAddin  *self,
                     IdeBuildPipeline      *pipeline,
                     IdeContext            *context,
                     GError               **error)
{
  g_autoptr(GbpSnapBuildStage) stage = NULL;
  guint stage_id;

  g_assert (GBP_IS_SNAP_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  stage = gbp_snap_build_stage_new (context, pipeline, GBP_SNAP_BUILD_STEP_PULL, error);
  if (!stage)
    return FALSE;

  stage_id = ide_build_pipeline_connect (pipeline,
                                         IDE_BUILD_PHASE_DOWNLOADS,
                                         0,
                                         IDE_BUILD_STAGE (stage));
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);
  return TRUE;
}

static gboolean
register_build_stage (GbpSnapPipelineAddin  *self,
                      IdeBuildPipeline      *pipeline,
                      IdeContext            *context,
                      GError               **error)
{
  g_autoptr(GbpSnapBuildStage) stage = NULL;
  guint stage_id;

  g_assert (GBP_IS_SNAP_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  stage = gbp_snap_build_stage_new (context, pipeline, GBP_SNAP_BUILD_STEP_BUILD, error);
  if (!stage)
    return FALSE;

  stage_id = ide_build_pipeline_connect (pipeline,
                                         IDE_BUILD_PHASE_BUILD,
                                         0,
                                         IDE_BUILD_STAGE (stage));
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);
  return TRUE;
}

static gboolean
register_stage_stage (GbpSnapPipelineAddin  *self,
                      IdeBuildPipeline      *pipeline,
                      IdeContext            *context,
                      GError               **error)
{
  g_autoptr(GbpSnapBuildStage) stage = NULL;
  guint stage_id;

  g_assert (GBP_IS_SNAP_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  stage = gbp_snap_build_stage_new (context, pipeline, GBP_SNAP_BUILD_STEP_STAGE, error);
  if (!stage)
    return FALSE;

  stage_id = ide_build_pipeline_connect (pipeline,
                                         IDE_BUILD_PHASE_EXPORT,
                                         EXPORT_STAGE,
                                         IDE_BUILD_STAGE (stage));
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);
  return TRUE;
}

static gboolean
register_prime_stage (GbpSnapPipelineAddin  *self,
                      IdeBuildPipeline      *pipeline,
                      IdeContext            *context,
                      GError               **error)
{
  g_autoptr(GbpSnapBuildStage) stage = NULL;
  guint stage_id;

  g_assert (GBP_IS_SNAP_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  stage = gbp_snap_build_stage_new (context, pipeline, GBP_SNAP_BUILD_STEP_PRIME, error);
  if (!stage)
    return FALSE;

  stage_id = ide_build_pipeline_connect (pipeline,
                                         IDE_BUILD_PHASE_EXPORT,
                                         EXPORT_PRIME,
                                         IDE_BUILD_STAGE (stage));
  ide_build_pipeline_addin_track (IDE_BUILD_PIPELINE_ADDIN (self), stage_id);
  return TRUE;
}

static void
snap_notify_completed (IdeBuildStage    *stage,
                       GParamSpec       *pspec,
                       IdeBuildPipeline *pipeline)
{
  IdeContext *context;
  IdeBuildSystem *build_system;
  IdeConfiguration *configuration;
  g_autofree char *builddir = NULL;
  g_autofree char *snap_path = NULL;
  g_autoptr(GFile) snap_file = NULL;
  

  g_assert (IDE_IS_BUILD_STAGE (stage));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));

  if (!ide_build_stage_get_completed (stage))
    return;

  context = ide_object_get_context (IDE_OBJECT (stage));
  build_system = ide_context_get_build_system (context);
  configuration = ide_build_pipeline_get_configuration (pipeline);

  g_assert (GBP_IS_SNAP_BUILD_SYSTEM (build_system));

  builddir = ide_build_system_get_builddir (build_system, configuration);
  snap_path = g_build_filename (builddir,
                                gbp_snap_build_system_get_snap_name (GBP_SNAP_BUILD_SYSTEM (build_system)),
                                NULL);
  snap_file = g_file_new_for_path (snap_path);
  ide_file_manager_show (snap_file, NULL);
}

static gboolean
register_snap_stage (GbpSnapPipelineAddin  *self,
                     IdeBuildPipeline      *pipeline,
                     IdeContext            *context,
                     GError               **error)
{
  g_autoptr(GbpSnapBuildStage) stage = NULL;
  guint stage_id;

  g_assert (GBP_IS_SNAP_PIPELINE_ADDIN (self));
  g_assert (IDE_IS_BUILD_PIPELINE (pipeline));
  g_assert (IDE_IS_CONTEXT (context));

  stage = gbp_snap_build_stage_new (context, pipeline, GBP_SNAP_BUILD_STEP_SNAP, error);
  if (!stage)
    return FALSE;

  g_signal_connect_object (stage, "notify::completed",
                           G_CALLBACK (snap_notify_completed), pipeline, 0);

  stage_id = ide_build_pipeline_connect (pipeline,
                                         IDE_BUILD_PHASE_EXPORT,
                                         EXPORT_SNAP,
                                         IDE_BUILD_STAGE (stage));
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
