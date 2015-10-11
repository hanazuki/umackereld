#ifndef UMACKERELD_METRICS_H
#define UMACKERELD_METRICS_H
#include <json-c/json.h>

// name is expected to be a string object
// value is expected to be a double or int object
typedef void (*collector_callback)(time_t time, json_object *name, json_object *value);

void metrics_add(time_t time, json_object *name, json_object *value);
json_object *metrics_flush();

void metrics_collect_loadavg5(collector_callback yield);

#endif
