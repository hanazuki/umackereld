#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <libubox/ulog.h>

#include "hostspec.h"
#include "stringutil.h"

json_object *hostspec_collect_cpu() {
  json_object *obj = json_object_new_array();
  assert(obj);

  FILE *fp = fopen("/proc/cpuinfo", "r");
  if (!fp) {
    ULOG_ERR("Unable to open /proc/cpuinfo\n");
    goto error;
  }

  char buf[1024], *p;
  json_object *obj_proc = NULL;
  while ((p = fgets(buf, sizeof buf, fp))) {
    chomp(p);

    if (begin_with(p, "processor\t")) {
      obj_proc = json_object_new_object();
      json_object_array_add(obj, obj_proc);
    }

    // Only send the model name for now: this should work for x86/amd64/arm/mips
    if (obj_proc) {
      if (begin_with(p, "cpu model\t") || begin_with(p, "model name\t")) {
        json_object_object_add(obj_proc, "model_name", json_object_new_string(after_colon(p)));
      }
    }
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
