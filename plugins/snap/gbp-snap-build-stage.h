/* gbp-snap-build-stage.h
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

#ifndef GBP_SNAP_BUILD_STAGE_H
#define GBP_SNAP_BUILD_STAGE_H

#include <ide.h>

G_BEGIN_DECLS

typedef enum {
  GBP_SNAP_BUILD_STEP_PULL,
  GBP_SNAP_BUILD_STEP_BUILD,
  GBP_SNAP_BUILD_STEP_STAGE,
  GBP_SNAP_BUILD_STEP_PRIME,
  GBP_SNAP_BUILD_STEP_SNAP,
} GbpSnapBuildStep;

#define GBP_TYPE_SNAP_BUILD_STAGE (gbp_snap_build_stage_get_type())

G_DECLARE_FINAL_TYPE (GbpSnapBuildStage, gbp_snap_build_stage, GBP, SNAP_BUILD_STAGE, IdeBuildStage)

GbpSnapBuildStage *gbp_snap_build_stage_new (IdeContext        *context,
                                             IdeBuildPipeline  *pipeline,
                                             GbpSnapBuildStep   step,
                                             GError           **error);

G_END_DECLS

#endif /* GBP_SNAP_BUILD_STAGE_H */
