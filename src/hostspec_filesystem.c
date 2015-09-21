#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <mntent.h>
#include <sys/statfs.h>

#include "hostspec.h"
#include "stringutil.h"
#include "log.h"

json_object *hostspec_collect_filesystem() {
  json_object *obj = json_object_new_array();
  assert(obj);

  FILE *fp = setmntent("/proc/mounts", "r");
  if (!fp) {
    ULOG_ERR("Unable to open /proc/mounts\n");
    goto error;
  }

  struct mntent *ent;
  while ((ent = getmntent(fp))) {
    struct statfs s;
    if (strcmp(ent->mnt_fsname, "rootfs") == 0 || // skip rootfs
        statfs(ent->mnt_dir, &s) == -1 ||         // failed to stat
        s.f_blocks == 0 ||                        // virtual file system
        s.f_blocks < s.f_bfree) {                 // something strange happens
      continue;
    }

    fsblkcnt_t total = s.f_blocks;
    fsblkcnt_t used = total - s.f_bfree;
    fsblkcnt_t available = s.f_bavail;
    fsblkcnt_t usable_total = used + available;

    if (usable_total <=
        0) { // every block is reserved for root? -- avoid div_by_zero
      continue;
    }

    char percent_used[50];
    if (sprintf(percent_used, "%d%%", (int)(used * 100.0 / usable_total)) ==
        EOF) {
      continue;
    }

    json_object *obj_fs = json_object_new_object();
    json_object_array_add(obj, obj_fs);
    json_object_object_add(obj_fs, "kb_size",
                           json_object_new_int64(total * s.f_bsize / 1024));
    json_object_object_add(obj_fs, "kb_used",
                           json_object_new_int64(used * s.f_bsize / 1024));
    json_object_object_add(obj_fs, "kb_available",
                           json_object_new_int64(available * s.f_bsize / 1024));
    json_object_object_add(obj_fs, "percent_used",
                           json_object_new_string(percent_used));
    json_object_object_add(obj_fs, "mount",
                           json_object_new_string(ent->mnt_dir));
  }

  endmntent(fp);
  return obj;

error:
  if (fp) {
    endmntent(fp);
  }

  json_object_put(obj);
  return NULL;
}
