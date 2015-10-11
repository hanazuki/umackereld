#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <curl/curl.h>
#include <json-c/json.h>

#include "api_internal.h"
#include "log.h"

int mackerel_get_services(struct mackerel_client *client, mackerel_request_callback cb, void *pdata) {
  int result = mackerel_client_invoke(client, "GET", "/api/v0/services", NULL, cb, pdata);
  return result;
}

static json_object *build_hostspec_payload(struct hostspec const *hostspec) {
  json_object *payload = json_object_new_object();
  if (hostspec->name) {
    json_object_object_add(payload, "name", hostspec->name);
  }
  if (hostspec->interfaces) {
    json_object_object_add(payload, "interfaces", hostspec->interfaces);
  }
  json_object *payload_meta = json_object_new_object();
  json_object_object_add(payload, "meta", payload_meta);
  if (hostspec->meta.agent_revision) {
    json_object_object_add(payload_meta, "agent-revision", json_object_new_string(hostspec->meta.agent_revision));
  }
  if (hostspec->meta.agent_version) {
    json_object_object_add(payload_meta, "agent-version", json_object_new_string(hostspec->meta.agent_version));
  }
  if (hostspec->meta.block_device) {
    json_object_object_add(payload_meta, "block_device", hostspec->meta.block_device);
  }
  if (hostspec->meta.cpu) {
    json_object_object_add(payload_meta, "cpu", hostspec->meta.cpu);
  }
  if (hostspec->meta.filesystem) {
    json_object_object_add(payload_meta, "filesystem", hostspec->meta.filesystem);
  }
  if (hostspec->meta.kernel) {
    json_object_object_add(payload_meta, "kernel", hostspec->meta.kernel);
  }
  if (hostspec->meta.memory) {
    json_object_object_add(payload_meta, "memory", hostspec->meta.memory);
  }

  return payload;
}

int mackerel_create_host(struct mackerel_client *client, struct hostspec const *hostspec, mackerel_request_callback cb,
                         void *pdata) {
  assert(client);
  assert(hostspec);

  json_object *payload = build_hostspec_payload(hostspec);
  return mackerel_client_invoke(client, "POST", "/api/v0/hosts", payload, cb, pdata);
}

int mackerel_update_host(struct mackerel_client *client, char const *hostid, struct hostspec const *hostspec,
                         mackerel_request_callback cb, void *pdata) {
  assert(client);
  assert(hostid);
  assert(hostspec);

  char *path;
  if (asprintf(&path, "/api/v0/hosts/%s", hostid) == -1) {
    return -1;
  }

  json_object *payload = build_hostspec_payload(hostspec);
  int result = mackerel_client_invoke(client, "PUT", path, payload, cb, pdata);
  free(path);
  return result;
}

// takes ownership of metric_values
int mackerel_update_metrics(struct mackerel_client *client, char const *hostid, struct json_object *metric_values,
                            mackerel_request_callback cb, void *pdata) {
  assert(client);
  assert(hostid);
  assert(metric_values);
  assert(json_object_is_type(metric_values, json_type_array));

  json_object *hostid_obj = json_object_new_string(hostid);
  for (int i = 0, len = json_object_array_length(metric_values); i < len; ++i) {
    json_object *ith = json_object_array_get_idx(metric_values, i);
    assert(json_object_is_type(ith, json_type_object));
    json_object_object_add(ith, "hostId", json_object_get(hostid_obj));
  }
  json_object_put(hostid_obj);

  return mackerel_client_invoke(client, "POST", "/api/v0/tsdb", metric_values, cb, pdata);
}
