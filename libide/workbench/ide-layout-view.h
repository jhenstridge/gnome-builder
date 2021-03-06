/* ide-layout-view.h
 *
 * Copyright (C) 2015 Christian Hergert <christian@hergert.me>
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef IDE_LAYOUT_VIEW_H
#define IDE_LAYOUT_VIEW_H

#include <gtk/gtk.h>

#include "diagnostics/ide-source-location.h"
#include "history/ide-back-forward-list.h"

G_BEGIN_DECLS

#define IDE_TYPE_LAYOUT_VIEW (ide_layout_view_get_type())

G_DECLARE_DERIVABLE_TYPE (IdeLayoutView, ide_layout_view, IDE, LAYOUT_VIEW, GtkBox)

struct _IdeLayoutViewClass
{
  GtkBinClass parent;

  gboolean       (*get_can_preview)       (IdeLayoutView             *self);
  gboolean       (*get_can_split)         (IdeLayoutView             *self);
  gboolean       (*get_modified)          (IdeLayoutView             *self);
  gchar         *(*get_title)             (IdeLayoutView             *self);
  gchar         *(*get_special_title)     (IdeLayoutView             *self);
  IdeLayoutView *(*create_split)          (IdeLayoutView             *self,
                                           GFile                     *file);
  gboolean       (*get_split_view)        (IdeLayoutView             *self);
  void           (*set_split_view)        (IdeLayoutView             *self,
                                           gboolean                   split_view);
  void           (*set_back_forward_list) (IdeLayoutView             *self,
                                           IdeBackForwardList        *back_forward_list);
  void           (*navigate_to)           (IdeLayoutView             *self,
                                           IdeSourceLocation         *location);
  gboolean       (*agree_to_close)        (IdeLayoutView             *self);

  gpointer _reserved1;
  gpointer _reserved2;
  gpointer _reserved3;
  gpointer _reserved4;
  gpointer _reserved5;
  gpointer _reserved6;
  gpointer _reserved7;
  gpointer _reserved8;
};

gboolean       ide_layout_view_agree_to_close        (IdeLayoutView             *self);
IdeLayoutView *ide_layout_view_create_split          (IdeLayoutView             *self,
                                                      GFile                     *file);
gboolean       ide_layout_view_get_can_preview       (IdeLayoutView             *self);
gboolean       ide_layout_view_get_can_split         (IdeLayoutView             *self);
gchar         *ide_layout_view_get_title             (IdeLayoutView             *self);
gchar         *ide_layout_view_get_special_title     (IdeLayoutView             *self);
gboolean       ide_layout_view_get_modified          (IdeLayoutView             *self);
gboolean       ide_layout_view_get_split_view        (IdeLayoutView             *self);
void           ide_layout_view_set_split_view        (IdeLayoutView             *self,
                                                      gboolean                   split_view);
void           ide_layout_view_set_back_forward_list (IdeLayoutView             *self,
                                                      IdeBackForwardList        *back_forward_list);
void           ide_layout_view_navigate_to           (IdeLayoutView             *self,
                                                      IdeSourceLocation         *location);

G_END_DECLS

#endif /* IDE_LAYOUT_VIEW_H */
