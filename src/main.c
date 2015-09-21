#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <curl/curl.h>
#include <libubox/uloop.h>
#include <libubox/ulog.h>

#include "../config.h"
#include "api.h"
#include "log.h"
#include "conf.h"
#include "hostid.h"
#include "hostspec.h"
#include "metrics.h"

static char const *const api_beseuri = "https://mackerel.io";

#define ONE_SECOND_IN_MILLISEC 1000
#define ONE_MINUTE_IN_MILLISEC (ONE_SECOND_IN_MILLISEC * 60)
#define ONE_HOUR_IN_MILLISEC (ONE_MINUTE_IN_MILLISEC * 60)
#define SOON ONE_SECOND_IN_MILLISEC

static int hostspec_update_interval = ONE_HOUR_IN_MILLISEC;
static int metrics_update_interval = ONE_MINUTE_IN_MILLISEC;

static struct mackerel_client *mackerel_client;

void ppjson(CURLcode res, json_object *obj, void *p) {
  (void)p;
  ULOG_INFO("[%d] %s\n", res, obj ? json_object_to_json_string(obj) : "(null)");
}


static void hostspec_timeout_handler(struct uloop_timeout *t);
static void metrics_timeout_handler(struct uloop_timeout *t);

struct uloop_timeout hostspec_timeout = {.cb = hostspec_timeout_handler},
                     metrics_timeout = {.cb = metrics_timeout_handler};

void schedule_hostspec_update(int msec) {
  ULOG_INFO("Schedule hostspec update in %d msec.\n", msec);
  uloop_timeout_set(&hostspec_timeout, msec);
}

void schedule_metrics_update(int msec) {
  ULOG_INFO("Schedule metrics update in %d msec.\n", msec);
  uloop_timeout_set(&metrics_timeout, msec);
}

void create_host_callback(CURLcode res, json_object *obj, void *p) {
  (void)p;

  if (res == 0) {
    if (json_object_is_type(obj, json_type_object)) {
      json_object *id;
      if (json_object_object_get_ex(obj, "id", &id) &&
          json_object_is_type(id, json_type_string)) {
        if (hostid_set(json_object_get_string(id)) == 0) {
          schedule_metrics_update(SOON);
        }
        goto ok;
      }
    }
    DEBUG("Malformed JSON");
  }

ok:
  schedule_hostspec_update(hostspec_update_interval);
}

json_object *collect_hostname() {
  // prefer hostname specified in config
  if (global_options.hostname) {
    return json_object_new_string(global_options.hostname);
  }
  return hostspec_collect_hostname();
}

struct hostspec collect_hostspec() {
  return (struct hostspec){
      .name = collect_hostname(),
      .meta =
          {
              .agent_version = PACKAGE_STRING,
              .block_device = hostspec_collect_block_device(),
              .cpu = hostspec_collect_cpu(),
              .memory = hostspec_collect_memory(),
              .kernel = hostspec_collect_kernel(),
              .filesystem = hostspec_collect_filesystem(),
          },
  };
}

static void hostspec_timeout_handler(struct uloop_timeout *t) {
  DEBUG_ENTER;
  (void)t; // == &hostspec_timeout

  assert(mackerel_client);

  struct hostspec hostspec = collect_hostspec();
  if (hostid || hostid_get() == 0) {
    assert(hostid);
    mackerel_update_host(mackerel_client, hostid, &hostspec,
                         create_host_callback, NULL);
  } else {
    mackerel_create_host(mackerel_client, &hostspec, create_host_callback,
                         NULL);
  }
  DEBUG_EXIT;
}

static void metrics_timeout_handler(struct uloop_timeout *t) {
  DEBUG_ENTER;
  (void)t; // == &metrics_timeout

  metrics_collect_loadavg5(metrics_add);
  mackerel_update_metrics(mackerel_client, hostid, metrics_flush(), NULL, NULL);

  schedule_metrics_update(metrics_update_interval);
  DEBUG_EXIT;
}

int main() {
  ulog_open(ULOG_STDIO, LOG_USER, PACKAGE_NAME);

  if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
    ULOG_ERR("curl_global_init failed\n");
    return 1;
  }

  if (uloop_init() != 0) {
    ULOG_ERR("uloop_init failed\n");
    return 1;
  }

  if (load_config() != 0) {
    return 1;
  }

  mackerel_client = mackerel_client_alloc((struct mackerel_params){
      .base_uri = api_beseuri, .apikey = global_options.apikey,
  });

  schedule_hostspec_update(SOON);

  uloop_run();
  uloop_done();
  curl_global_cleanup();

  return 0;
}
