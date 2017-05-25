/* gbp-flatpak-plugin.c
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

#include <libpeas/peas.h>
#include <ide.h>

#include "gbp-snap-build-system.h"
#include "gbp-snap-pipeline-addin.h"

void
peas_register_types (PeasObjectModule *module)
{
  /* these should only be ignored at the top level */
  ide_vcs_register_ignored ("parts");
  ide_vcs_register_ignored ("stage");
  ide_vcs_register_ignored ("prime");

  peas_object_module_register_extension_type (module,
                                              IDE_TYPE_BUILD_SYSTEM,
                                              GBP_TYPE_SNAP_BUILD_SYSTEM);
  peas_object_module_register_extension_type (module,
                                              IDE_TYPE_BUILD_PIPELINE_ADDIN,
                                              GBP_TYPE_SNAP_PIPELINE_ADDIN);
}
