#include <assert.h>
#include <sys/utsname.h>

#include "fileutil.h"
#include "hostspec.h"

json_object *hostspec_collect_kernel() {
  json_object *obj = json_object_new_object();
  assert(obj);

  struct utsname nm;
  if (uname(&nm) == -1) {
    goto error;
  }

  json_object_object_add(obj, "name", json_object_new_string(nm.sysname));
  json_object_object_add(obj, "release", json_object_new_string(nm.release));
  json_object_object_add(obj, "version", json_object_new_string(nm.version));
  json_object_object_add(obj, "machine", json_object_new_string(nm.machine));

  // Are you serious porting this to other platform??
  json_object_object_add(obj, "os", json_object_new_string("GNU/Linux"));

  return obj;

error:
  json_object_put(obj);
  return NULL;
}
