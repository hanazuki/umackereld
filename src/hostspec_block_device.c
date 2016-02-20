#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <libubox/ulog.h>

#include "fileutil.h"
#include "hostspec.h"

json_object *hostspec_collect_block_device() {
  json_object *obj = json_object_new_object();
  assert(obj);

  DIR *dir = opendir("/sys/block");
  if (!dir) {
    ULOG_ERR("Unable to open /sys/block: %s\n", strerror(errno));
    goto error;
  }

  struct dirent *ent;
  while ((ent = readdir(dir))) {
    char const *name = ent->d_name;

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
      continue;
    }

    json_object *obj_dev = json_object_new_object();
    assert(obj_dev);
    json_object_object_add(obj, name, obj_dev);

    char buf[1024], *p;
    if ((p = fgets_close(buf, sizeof buf, fopenf("r", "/sys/block/%s/size", name)))) {
      int64_t size;
      if (json_parse_int64(p, &size) == 0) {
        json_object_object_add(obj_dev, "size", json_object_new_int64(size));
      }
    }

    if ((p = fgets_close(buf, sizeof buf, fopenf("r", "/sys/block/%s/removable", name)))) {
      int64_t removable;
      if (json_parse_int64(p, &removable) == 0) {
        json_object_object_add(obj_dev, "removable", json_object_new_int64(removable));
      }
    }
  }

  closedir(dir);
  return obj;

error:
  if (dir) {
    closedir(dir);
  }

  json_object_put(obj);
  return NULL;
}
