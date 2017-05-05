/* pnl-dock-bin-bin.h
 *
 * Copyright (C) 2016 Christian Hergert <chergert@redhat.com>
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

#ifndef PNL_DOCK_BIN_H
#define PNL_DOCK_BIN_H

#include "pnl-dock-types.h"

G_BEGIN_DECLS

#define PNL_DOCK_BIN_STYLE_CLASS_PINNED "pinned"

struct _PnlDockBinClass
{
  GtkContainerClass parent;

  GtkWidget *(*create_edge) (PnlDockBin *self);

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
  gpointer _reserved5;
  gpointer _reserved6;
  gpointer _reserved7;
  gpointer _reserved8;
};

GtkWidget *pnl_dock_bin_new               (void);
GtkWidget *pnl_dock_bin_get_center_widget (PnlDockBin   *self);
GtkWidget *pnl_dock_bin_get_top_edge      (PnlDockBin   *self);
GtkWidget *pnl_dock_bin_get_left_edge     (PnlDockBin   *self);
GtkWidget *pnl_dock_bin_get_bottom_edge   (PnlDockBin   *self);
GtkWidget *pnl_dock_bin_get_right_edge    (PnlDockBin   *self);

G_END_DECLS

#endif /* PNL_DOCK_BIN_H */
