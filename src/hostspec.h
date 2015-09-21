#ifndef UMACKERELD_HOSTSPEC_H
#define UMACKERELD_HOSTSPEC_H

#include <json-c/json.h>

typedef struct hostspec_meta {
  const char *agent_revision;
  const char *agent_version;
  json_object *block_device;
  json_object *cpu;
  json_object *filesystem;
  json_object *kernel;
  json_object *memory;
} hostspec_meta;

typedef struct hostspec {
  json_object *name;
  json_object *interfaces;
  struct hostspec_meta meta;
} hostspec;

json_object *hostspec_collect_hostname();
json_object *hostspec_collect_cpu();
json_object *hostspec_collect_block_device();
json_object *hostspec_collect_kernel();
json_object *hostspec_collect_memory();
json_object *hostspec_collect_filesystem();

#endif
