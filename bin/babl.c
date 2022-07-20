/* babl - dynamically extendable universal pixel conversion tool.
 * Copyright (C) 2022 Jehan
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, see
 * <https://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <babl/babl.h>


static const Babl * babl_cli_get_space (const char *path);


int
main (int    argc,
      char **argv)
{
  const Babl *from_format;
  const Babl *from_space       = NULL;
  const Babl *to_format;
  const Babl *to_space         = NULL;
  const Babl *fish;
  const char *from             = "R'G'B' float";
  const char *to               = "R'G'B' float";
  char       *source;
  char       *dest;
  int         set_from         = 0;
  int         set_to           = 0;
  int         set_from_profile = 0;
  int         set_to_profile   = 0;
  int         n_components;
  int         data_index;
  int         c;
  int         i;

  babl_init ();

  /* Looping through arguments to get source and destination formats. */
  for (i = 1; i < argc; i++)
    {
      if (set_from)
        {
          from = argv[i];
          set_from = 0;
          if (! babl_format_exists (from))
            {
              fprintf (stderr, "babl: unknown format: %s\n", from);
              return 1;
            }
        }
      else if (set_to)
        {
          to = argv[i];
          set_to = 0;
          if (! babl_format_exists (to))
            {
              fprintf (stderr, "babl: unknown format: %s\n", to);
              return 1;
            }
        }
      else if (set_from_profile)
        {
          set_from_profile = 0;
          from_space = babl_cli_get_space (argv[i]);

          if (! from_space)
            return 6;
        }
      else if (set_to_profile)
        {
          set_to_profile = 0;
          to_space = babl_cli_get_space (argv[i]);

          if (! to_space)
            return 6;
        }
      else if (strcmp (argv[i], "--from") == 0 ||
               strcmp (argv[i], "-f") == 0)
        {
          set_from = 1;
        }
      else if (strcmp (argv[i], "--to") == 0 ||
               strcmp (argv[i], "-t") == 0)
        {
          set_to = 1;
        }
      else if (strcmp (argv[i], "--input-profile") == 0 ||
               strcmp (argv[i], "-i") == 0)
        {
          set_from_profile = 1;
        }
      else if (strcmp (argv[i], "--output-profile") == 0 ||
               strcmp (argv[i], "-o") == 0)
        {
          set_to_profile = 1;
        }
    }

  from_format  = babl_format_with_space (from, from_space);
  n_components = babl_format_get_n_components (from_format);
  source       = malloc (babl_format_get_bytes_per_pixel (from_format));
  data_index   = 0;

  to_format    = babl_format_with_space (to, to_space);
  dest         = malloc (babl_format_get_bytes_per_pixel (from_format));

  /* Re-looping through arguments, to be more flexible with argument orders.
   * In this second loop, we get the source components' values.
   */
  set_from = set_to = set_to_profile = set_from_profile = 0;
  for (i = 1, c = 0; i < argc; i++)
    {
      if (set_from)
        {
          set_from = 0;
          /* Pass. */
        }
      else if (set_to)
        {
          set_to = 0;
          /* Pass. */
        }
      else if (set_from_profile)
        {
          set_from_profile = 0;
          /* Pass. */
        }
      else if (set_to_profile)
        {
          set_to_profile = 0;
          /* Pass. */
        }
      else if (strcmp (argv[i], "--from") == 0 ||
               strcmp (argv[i], "-f") == 0)
        {
          set_from = 1;
        }
      else if (strcmp (argv[i], "--to") == 0 ||
               strcmp (argv[i], "-t") == 0)
        {
          set_to = 1;
        }
      else if (strcmp (argv[i], "--input-profile") == 0 ||
               strcmp (argv[i], "-i") == 0)
        {
          set_from_profile = 1;
        }
      else if (strcmp (argv[i], "--output-profile") == 0 ||
               strcmp (argv[i], "-o") == 0)
        {
          set_to_profile = 1;
        }
      else
        {
          const Babl *arg_type;
          char       *endptr = NULL;

          if (c >= n_components)
            {
              fprintf (stderr, "babl: unexpected argument: %s\n", argv[i]);
              return 2;
            }

          arg_type = babl_format_get_type (from_format, c);

          if (strcmp (babl_get_name (arg_type), "float") == 0)
            {
              float  value = strtof (argv[i], &endptr);
              float *fsrc = (float *) (source + data_index);

              if (value == 0.0f && endptr == argv[i])
                {
                  fprintf (stderr, "babl: expected type of component %d is '%s', invalid value: %s\n",
                           c, babl_get_name (arg_type), argv[i]);
                  return 3;
                }

              *fsrc = value;
              data_index += 4;
            }
          else if (strncmp (babl_get_name (arg_type), "u", 1) == 0)
            {
              long int value = strtol (argv[i], &endptr, 10);

              if (value == 0 && endptr == argv[i])
                {
                  fprintf (stderr, "babl: expected type of component %d is '%s', invalid value: %s\n",
                           c, babl_get_name (arg_type), argv[i]);
                  return 3;
                }

              if (strcmp (babl_get_name (arg_type), "u8") == 0)
                {
                  uint8_t *usrc = (uint8_t *) (source + data_index);

                  *usrc = value;
                  data_index += 1;
                }
              else if (strcmp (babl_get_name (arg_type), "u16") == 0)
                {
                  uint16_t *usrc = (uint16_t *) (source + data_index);

                  *usrc = value;
                  data_index += 2;
                }
              else if (strcmp (babl_get_name (arg_type), "u32") == 0)
                {
                  uint32_t *usrc = (uint32_t *) (source + data_index);

                  *usrc = value;
                  data_index += 4;
                }
              else
                {
                  fprintf (stderr, "babl: unsupported unsigned type '%s' of component %d: %s\n",
                           babl_get_name (arg_type), c, argv[i]);
                  return 4;
                }
            }
          else
            {
              fprintf (stderr, "babl: unsupported type '%s' of component %d: %s\n",
                       babl_get_name (arg_type), c, argv[i]);
              return 4;
            }

          c++;
        }
    }

  if (c != n_components)
    {
      fprintf (stderr, "babl: %d components expected, %d components were passed\n",
               n_components, c);
      return 2;
    }

  /* Actual processing. */
  fish = babl_fish (from_format, to_format);
  babl_process (fish, source, dest, 1);

  /* Now displaying the result. */
  n_components = babl_format_get_n_components (to_format);
  data_index   = 0;

  printf ("Conversion as \"%s\":\n", babl_get_name (to_format));
  for (c = 0; c < n_components; c++)
    {
      const Babl *arg_type = NULL;

      arg_type = babl_format_get_type (to_format, c);

      if (strcmp (babl_get_name (arg_type), "float") == 0)
        {
          float value = *((float *) (dest + data_index));

          data_index += 4;

          printf ("- %f\n", value);
        }
      else if (strcmp (babl_get_name (arg_type), "u8") == 0)
        {
          uint8_t value = *((uint8_t *) (dest + data_index));

          data_index += 1;

          printf ("- %d\n", value);
        }
      else if (strcmp (babl_get_name (arg_type), "u16") == 0)
        {
          uint16_t value = *((uint16_t *) (dest + data_index));

          data_index += 2;

          printf ("- %d\n", value);
        }
      else if (strcmp (babl_get_name (arg_type), "u32") == 0)
        {
          uint32_t value = *((uint32_t *) (dest + data_index));

          data_index += 4;

          printf ("- %d\n", value);
        }
      else
        {
          fprintf (stderr, "babl: unsupported type '%s' of returned component %d: %s\n",
                   babl_get_name (arg_type), c, argv[i]);
          return 5;
        }
    }

  babl_exit ();

  free (source);
  free (dest);

  return 0;
}

static const Babl *
babl_cli_get_space (const char *path)
{
  const Babl *space;
  FILE       *f;
  char       *icc_data;
  long        icc_length;
  const char *error = NULL;

  f = fopen (path, "r");

  if (f == NULL)
    {
      fprintf (stderr, "babl: failed to open '%s': %s\n",
               path, strerror (errno));
      return NULL;
    }

  fseek (f, 0, SEEK_END);
  icc_length = ftell (f);
  fseek (f, 0, SEEK_SET);

  icc_data = malloc (icc_length);
  fread (icc_data, icc_length, 1, f);

  fclose (f);

  space = babl_space_from_icc (icc_data, icc_length,
                               BABL_ICC_INTENT_RELATIVE_COLORIMETRIC,
                               &error);

  if (space == NULL)
    {
      fprintf (stderr, "babl: failed to load space from '%s': %s\n",
               path, error);
      return NULL;
    }

  return space;
}
