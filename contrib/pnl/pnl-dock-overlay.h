/* pnl-dock-overlay.h
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

#ifndef PNL_DOCK_OVERLAY_H
#define PNL_DOCK_OVERLAY_H

#include "pnl-dock.h"
#include "pnl-dock-overlay-edge.h"

G_BEGIN_DECLS

struct _PnlDockOverlayClass
{
  GtkEventBoxClass parent;

  void (*hide_edges) (PnlDockOverlay *self);

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
  gpointer _reserved5;
  gpointer _reserved6;
  gpointer _reserved7;
  gpointer _reserved8;
};

GtkWidget              *pnl_dock_overlay_new                   (void);
void                    pnl_overlay_add_child                  (PnlDockOverlay  *self,
                                                                GtkWidget       *child,
                                                                const gchar     *type);
PnlDockOverlayEdge     *pnl_dock_overlay_get_edge              (PnlDockOverlay  *self,
                                                                GtkPositionType  position);
GtkAdjustment          *pnl_dock_overlay_get_edge_adjustment   (PnlDockOverlay  *self,
                                                                GtkPositionType  position);
G_END_DECLS

#endif /* PNL_DOCK_OVERLAY_H */
