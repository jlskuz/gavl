#include <gavfprivate.h>
#include <string.h>

void
gavf_options_set_sync_distance(gavf_options_t * opt, gavl_time_t sync_distance)
  {
  opt->sync_distance = sync_distance;
  }

void
gavf_options_set_flags(gavf_options_t * opt, int flags)
  {
  opt->flags = flags;
  }

int gavf_options_get_flags(gavf_options_t * opt)
  {
  return opt->flags;
  }

void gavf_options_copy(gavf_options_t * dst, const gavf_options_t * src)
  {
  memcpy(dst, src, sizeof(*src));
  }

