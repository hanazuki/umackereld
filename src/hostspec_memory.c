#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "hostspec.h"
#include "stringutil.h"
#include "log.h"

#define FIELD(Fjson, Fmeminfo)                                                 \
  if (begin_with(p, Fmeminfo ":")) {                                           \
    json_object_object_add(obj, Fjson,                                         \
                           json_object_new_string(after_colon(p)));            \
    continue;                                                                  \
  }

json_object *hostspec_collect_memory() {
  json_object *obj = json_object_new_object();
  assert(obj);

  FILE *fp = fopen("/proc/meminfo", "r");
  if (!fp) {
    ULOG_ERR("Unable to open /proc/meminfo\n");
    goto error;
  }

  char buf[1024], *p;
  while ((p = fgets(buf, sizeof buf, fp))) {
    chomp(p);
    FIELD("total", "MemTotal");
    FIELD("free", "MemFree");
    FIELD("buffers", "Buffers");
    FIELD("cached", "Cached");
    FIELD("active", "Active");
    FIELD("inactive", "Inactive");
    FIELD("high_total", "HighTotal");
    FIELD("high_free", "HighFree");
    FIELD("low_total", "LowTotal");
    FIELD("low_free", "LowFree");
    FIELD("dirty", "Dirty");
    FIELD("writeback", "Writeback");
    FIELD("anon_pages", "AnonPages");
    FIELD("mapped", "Mapped");
    FIELD("slab", "Slab");
    FIELD("slab_reclaimable", "SReclaimable");
    FIELD("slab_unreclaim", "SUnreclaim");
    FIELD("page_tables", "PageTables");
    FIELD("nfs_unstable", "NFS_Unstable");
    FIELD("bounce", "Bounce");
    FIELD("commit_limit", "CommitLimit");
    FIELD("committed_as", "Committed_AS");
    FIELD("vmalloc_total", "VmallocTotal");
    FIELD("vmalloc_used", "VmallocUsed");
    FIELD("vmalloc_chunk", "VmallocChunk");
    FIELD("swap_cached", "SwapCached");
    FIELD("swap_total", "SwapTotal");
    FIELD("swap_free", "SwapFree");
  }

  fclose(fp);
  return obj;

error:
  if (fp) {
    fclose(fp);
  }

  json_object_put(obj);
  return NULL;
}
