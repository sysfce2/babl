/* babl - dynamically extendable universal pixel conversion library.
 * Copyright (C) 2005, Øyvind Kolås.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "babl.h"
#include "babl-internal.h"    /* for babl_log */

#define BABL_LOG

#ifdef BABL_LOG

static void sampling_introspect     (Babl *babl);
static void model_introspect        (Babl *babl);
static void type_introspect         (Babl *babl);
static void format_introspect       (Babl *babl);

static int  each_introspect         (Babl *babl,
                                     void *user_data);
#endif

void
babl_introspect (Babl *babl)
{
#ifdef BABL_LOG
  Babl *extender_backup = babl_extender ();
  
  babl_set_extender (babl_extension_quiet_log());

  if (babl)
    {
      each_introspect (babl, NULL);
      return;
    }
  babl_log ("Introspection report");  
  babl_log ("====================================================");  

  babl_log ("");
  babl_log ("Data Types:");
  babl_type_each         (each_introspect, NULL);
  babl_log ("");
  babl_log ("Sampling (chroma subsampling) factors:");
  babl_sampling_each     (each_introspect, NULL);
  babl_log ("");
  babl_log ("Components:");
  babl_component_each    (each_introspect, NULL);
  babl_log ("");
  babl_log ("Models (of components):");
  babl_model_each        (each_introspect, NULL);
  babl_log ("");
  babl_log ("Pixel formats:");
  babl_format_each (each_introspect, NULL);
  babl_log ("");
  babl_log ("conversions:");
  babl_conversion_each   (each_introspect, NULL);
  babl_log ("");
  babl_log ("extensions:");
  babl_extension_each    (each_introspect, NULL);
  babl_log ("");
  babl_log ("fishes");
  babl_fish_each         (each_introspect, NULL);
  babl_log ("");

  babl_set_extender (extender_backup);
#endif
}

#ifdef BABL_LOG
static int list_length (void **list)
{
  void **ptr;
  int    len=0;
  
  ptr = list;
  while (NULL!=*ptr)
    {
      ptr++;
      len++;
    }
  return len;
}


static void
item_conversions_introspect (Babl *babl)
{
  void **ptr;

  if (babl->type.from)
    babl_log ("\t\tconversions from %s: %i",
       babl->instance.name, list_length ((void **)(babl->type.from)));

  ptr = (void **)babl->type.from;

  while (ptr && NULL!=*ptr)
    {
      babl_log ("\t\t\t'%s'", ((Babl *)(*ptr))->instance.name);
      ptr++;
    }
  
  if (babl->type.to)
    babl_log ("\t\tconversions to %s: %i",
       babl->instance.name, list_length ((void **)(babl->type.to)));


  ptr = (void **)babl->type.to;

  while (ptr && NULL!=*ptr)
    {
      babl_log ("\t\t\t'%s'", ((Babl *)(*ptr))->instance.name);
      ptr++;
    }
}

static void
model_introspect (Babl *babl)
{
  int i;
  babl_log ("\t\tcomponents=%i", babl->model.components);

  for (i=0; i< babl->model.components; i++)
    {
      babl_log ("\t\tindex[%i] = '%s'", i,
         BABL(babl->model.component[i])->instance.name  );
    }
}

static void
type_introspect (Babl *babl)
{
  babl_log ("\t\tbits=%i", babl->type.bits);
}


static void
sampling_introspect (Babl *babl)
{
  babl_log ("\t\thorizontal = %i",
            babl->sampling.horizontal);
  babl_log ("\t\tvertical   = %i",
            babl->sampling.vertical);
}


static void
format_introspect (Babl *babl)
{
  int i;
  babl_log ("\t\tplanar=%i", babl->format.planar);
  babl_log ("\t\tcomponents=%i",  babl->format.components);

  for (i=0; i< babl->format.components; i++)
    {
      babl_log ("\t\tband[%i] type='%s' component='%s' sampling='%s'",
                i,   babl->format.type[i]->instance.name,
                     babl->format.component[i]->instance.name,
                     babl->format.sampling[i]->instance.name);
    }
}

static int
each_introspect (Babl *babl,
                 void *user_data)
{
  babl_log ("\t'%s'\t%i\t%s",
   babl->instance.name,
   babl->instance.id,
   babl_class_name (babl->class_type));
  switch (babl->class_type)
    {
      case BABL_TYPE:
        type_introspect (babl);
        item_conversions_introspect (babl);
        break;
      case BABL_COMPONENT:
        item_conversions_introspect (babl);
        break;
      case BABL_MODEL:
        model_introspect (babl);
        item_conversions_introspect (babl);
        break;
      case BABL_FORMAT:
        format_introspect (babl);
        item_conversions_introspect (babl);
        break;
      case BABL_SAMPLING:
        sampling_introspect (babl);
        item_conversions_introspect (babl);
        break;
      case BABL_FISH:
        babl_log ("\t\tneed more data here%s", "");
        break;
      default:
        break;
    }
  return 0;
}
#endif
