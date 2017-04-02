/* ide-xml-service.h
 *
 * Copyright (C) 2017 Sébastien Lafargue <slafargue@gnome.org>
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

#ifndef IDE_XML_SERVICE_H
#define IDE_XML_SERVICE_H

#include <gtksourceview/gtksource.h>
#include <egg-task-cache.h>

#include "ide-xml-position.h"
#include "ide-xml-symbol-node.h"
#include <ide.h>

G_BEGIN_DECLS

#define IDE_TYPE_XML_SERVICE (ide_xml_service_get_type())

G_DECLARE_FINAL_TYPE (IdeXmlService, ide_xml_service, IDE, XML_SERVICE, IdeObject)

IdeDiagnostics     *ide_xml_service_get_cached_diagnostics             (IdeXmlService        *self,
                                                                        GFile                *gfile);
IdeXmlSymbolNode   *ide_xml_service_get_cached_root_node               (IdeXmlService        *self,
                                                                        GFile                *gfile);
IdeDiagnostics     *ide_xml_service_get_diagnostics_finish             (IdeXmlService        *self,
                                                                        GAsyncResult         *result,
                                                                        GError              **error);
void                ide_xml_service_get_diagnostics_async              (IdeXmlService        *self,
                                                                        IdeFile              *ifile,
                                                                        IdeBuffer            *buffer,
                                                                        GCancellable         *cancellable,
                                                                        GAsyncReadyCallback   callback,
                                                                        gpointer              user_data);
void                ide_xml_service_get_position_from_cursor_async     (IdeXmlService        *self,
                                                                        IdeFile              *ifile,
                                                                        IdeBuffer            *buffer,
                                                                        gint                  line,
                                                                        gint                  line_offset,
                                                                        GCancellable         *cancellable,
                                                                        GAsyncReadyCallback   callback,
                                                                        gpointer              user_data);
IdeXmlPosition     *ide_xml_service_get_position_from_cursor_finish    (IdeXmlService        *self,
                                                                        GAsyncResult         *result,
                                                                        GError              **error);
void                ide_xml_service_get_root_node_async                (IdeXmlService        *self,
                                                                        IdeFile              *ifile,
                                                                        IdeBuffer            *buffer,
                                                                        GCancellable         *cancellable,
                                                                        GAsyncReadyCallback   callback,
                                                                        gpointer              user_data);
IdeXmlSymbolNode   *ide_xml_service_get_root_node_finish               (IdeXmlService        *self,
                                                                        GAsyncResult         *result,
                                                                        GError              **error);
EggTaskCache       *ide_xml_service_get_schemas_cache                  (IdeXmlService        *self);

G_END_DECLS

#endif /* IDE_XML_SERVICE_H */
