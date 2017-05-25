/* gbp-snap-utils.c
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

#include "gbp-snap-utils.h"

GFile *
gbp_snap_find_snapcraft_yaml (GFile        *directory,
                              GCancellable *cancellable)
{
  static const char *yaml_locations[] = {
    "snap" G_DIR_SEPARATOR_S "snapcraft.yaml",
    "snapcraft.yaml",
    ".snapcraft.yaml",
  };
  int i;

  g_return_val_if_fail (G_IS_FILE (directory), NULL);
  g_return_val_if_fail (!cancellable || G_IS_CANCELLABLE (cancellable), NULL);

  for (i = 0; i < G_N_ELEMENTS (yaml_locations); i++)
    {
      g_autoptr(GFile) file = g_file_resolve_relative_path (directory,
                                                            yaml_locations[i]);
      if (!g_file_query_exists (file, cancellable))
        continue;

      return g_steal_pointer (&file);
    }
  return NULL;
}

/* Find the value under a YAML mapping node that matches the given key */
yaml_node_t *
gbp_snap_yaml_mapping_lookup (yaml_document_t *document,
                              yaml_node_t     *node,
                              const char      *key_string)
{
  yaml_node_pair_t *p;
  g_assert (node->type == YAML_MAPPING_NODE);

  for (p = node->data.mapping.pairs.start; p != node->data.mapping.pairs.top; p++)
    {
      yaml_node_t *key = yaml_document_get_node (document, p->key);
      if (key->type != YAML_SCALAR_NODE)
        continue;

      if (!strncmp((const char*)key->data.scalar.value, key_string,
                   key->data.scalar.length))
        {
          return yaml_document_get_node (document, p->value);
        }
    }
  return NULL;
}

yaml_node_t *gbp_snap_find_main_part (yaml_document_t *document)
{
  yaml_node_t *root, *parts;
  yaml_node_pair_t *p;

  root = yaml_document_get_root_node (document);
  if (!root)
    return NULL;
  g_assert (root->type == YAML_MAPPING_NODE);

  /* Find the "parts" section */
  parts = gbp_snap_yaml_mapping_lookup (document, root, "parts");
  if (!parts || parts->type != YAML_MAPPING_NODE)
    return NULL;

  /* Find a part that uses "." as its source. */
  for (p = parts->data.mapping.pairs.start; p != parts->data.mapping.pairs.top; p++)
    {
      yaml_node_t *part = yaml_document_get_node (document, p->value);
      yaml_node_t *source = gbp_snap_yaml_mapping_lookup (document, part, "source");

      if (source->type != YAML_SCALAR_NODE)
        continue;
      if (!strncmp((const char*)source->data.scalar.value, ".",
                   source->data.scalar.length))
        {
          return part;
        }
    }

  /* Otherwise, pick the first part (assuming there is one) */
  p = parts->data.mapping.pairs.start;
  if (p == parts->data.mapping.pairs.top)
    return NULL;

  return yaml_document_get_node (document, p->value);
}
