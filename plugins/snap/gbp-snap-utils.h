/* gbp-snap-utils.h
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

#ifndef GBP_SNAP_UTILS_H
#define GBP_SNAP_UTILS_H

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

GFile *gbp_snap_find_snapcraft_yaml (GFile        *directory,
                                     GCancellable *cancellable);

char *gbp_snap_calculate_package_name (GFile        *snapcraft_yaml,
                                       GCancellable *cancellable);

G_END_DECLS

#endif /* GBP_SNAP_BUILD_SYSTEM_DISCOVERY_H */
