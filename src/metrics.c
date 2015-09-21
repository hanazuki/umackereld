#include <assert.h>

#include "metrics.h"

static json_object *buffer;

void metrics_add(time_t time, json_object *name, json_object *value) {
  assert(name);
  assert(value);

  if (!buffer) {
    buffer = json_object_new_array();
  }

  json_object *obj = json_object_new_object();
  json_object_array_add(buffer, obj);
  json_object_object_add(obj, "time", json_object_new_int64(time));
  json_object_object_add(obj, "name", name);
  json_object_object_add(obj, "value", value);
}

json_object *metrics_flush() {
  json_object *values = buffer;
  buffer = NULL;
  return values;
}
