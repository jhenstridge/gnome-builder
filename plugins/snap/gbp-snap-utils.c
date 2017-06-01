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

#define G_LOG_DOMAIN "gbp-snap-utils"

#include "gbp-snap-utils.h"

#include <ide.h>
#include <yaml.h>

G_DEFINE_AUTO_CLEANUP_CLEAR_FUNC(yaml_parser_t, yaml_parser_delete);
G_DEFINE_AUTO_CLEANUP_CLEAR_FUNC(yaml_document_t, yaml_document_delete);

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
static yaml_node_t *
mapping_lookup (yaml_document_t *document,
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

const char *
get_snap_arch (void)
{
  static const char *arch = NULL;

  if (!arch)
    {
      g_autofree char *m = ide_get_system_arch ();

      /* This is based on the code in snapcraft/_options.py */
      if (!strcmp (m, "armv7l"))
        arch = "armhf";
      else if (!strcmp (m, "aarch64"))
        arch = "arm64";
      else if (strlen (m) == 4 && m[0] == 'i' && m[2] == '8' && m[3] == '6')
        arch = "i386";
      else if (!strcmp (m, "ppc64le"))
        arch = "ppc64el";
      else if (!strcmp (m, "ppc"))
        arch = "powerpc";
      else if (!strcmp (m, "x86_64"))
        arch = "amd64";
      else if (!strcmp (m, "s390x"))
        arch = "s390x";

      if (!arch)
        g_critical ("Unknown architecture: %s", m);
    }
  return arch;
}

char *
gbp_snap_calculate_package_name (GFile         *snapcraft_yaml,
                                 GCancellable  *cancellable)
{
  g_autoptr(GError) error = NULL;
  g_autofree char *yaml_contents = NULL;
  gsize yaml_length = 0;
  g_auto(yaml_parser_t) parser = { 0, };
  g_auto(yaml_document_t) doc = { 0, };
  yaml_node_t *root, *node;
  g_autofree char *name = NULL, *version = NULL, *arch = NULL;

  IDE_ENTRY;

  g_assert (G_IS_FILE (snapcraft_yaml));
  g_assert (!cancellable || G_IS_CANCELLABLE (cancellable));

  if (!g_file_load_contents (snapcraft_yaml, cancellable,
                             &yaml_contents, &yaml_length,
                             NULL, &error))
    {
      g_warning ("Could not read snapcraft.yaml: %s", error->message);
      IDE_RETURN (NULL);
    }

  if (!yaml_parser_initialize (&parser))
    IDE_RETURN (NULL);

  yaml_parser_set_input_string (&parser, (const unsigned char *)yaml_contents,
                                (gssize)yaml_length);

  if (!yaml_parser_load (&parser, &doc))
    IDE_RETURN (NULL);

  root = yaml_document_get_root_node (&doc);

  node = mapping_lookup (&doc, root, "name");
  if (!node || node->type != YAML_SCALAR_NODE)
    IDE_RETURN (NULL);
  name = g_strndup((const char*)node->data.scalar.value,
                   node->data.scalar.length);

  node = mapping_lookup (&doc, root, "version");
  if (!node || node->type != YAML_SCALAR_NODE)
    IDE_RETURN (NULL);
  version = g_strndup((const char*)node->data.scalar.value,
                      node->data.scalar.length);

  /* A snapcraft project can force the architecture choice.  This is
   * generally only used to produce noarch snaps. */
  node = mapping_lookup (&doc, root, "architectures");
  if (node && node->type == YAML_SEQUENCE_NODE &&
    node->data.sequence.items.start != node->data.sequence.items.top)
    {
      yaml_node_t *first = yaml_document_get_node (&doc, *node->data.sequence.items.start);
      if (first && first->type == YAML_SCALAR_NODE)
        arch = g_strndup((const char*)first->data.scalar.value,
                         first->data.scalar.length);
    }
  if (!arch)
    arch = g_strdup (get_snap_arch ());

  return g_strdup_printf("%s_%s_%s.snap", name, version, arch);
}
