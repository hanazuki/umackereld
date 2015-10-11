#include "fileutil.h"
#include "hostspec.h"

json_object *hostspec_collect_hostname() {
  char buf[1024], *p;
  if ((p = fgets_close(buf, sizeof buf, fopen("/proc/sys/kernel/hostname", "r")))) {
    return json_object_new_string(p);
  }
  return NULL;
}
