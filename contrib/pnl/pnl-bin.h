/* pnl-bin.h
 *
 * Copyright (C) 2017 Christian Hergert <chergert@redhat.com>
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

#if !defined(PNL_INSIDE) && !defined(PNL_COMPILATION)
# error "Only <pnl.h> can be included directly."
#endif

#ifndef PNL_BIN_H
#define PNL_BIN_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define PNL_TYPE_BIN (pnl_bin_get_type())

G_DECLARE_DERIVABLE_TYPE (PnlBin, pnl_bin, PNL, BIN, GtkBin)

struct _PnlBinClass
{
  GtkBinClass parent_class;
};

GtkWidget *pnl_bin_new (void);

G_END_DECLS

#endif /* PNL_BIN_H */
