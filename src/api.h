#ifndef UMACKERELD_API_H
#define UMACKERELD_API_H
#include <curl/curl.h>
#include <json-c/json.h>

#include "hostspec.h"

struct mackerel_params {
  const char *base_uri;
  const char *apikey;
};

struct mackerel_client;

struct mackerel_client *mackerel_client_alloc(struct mackerel_params params);

void mackerel_client_free(struct mackerel_client *client);

typedef void (*mackerel_request_callback)(CURLcode result, json_object *data, void *pdata);

int mackerel_get_services(struct mackerel_client *client, mackerel_request_callback cb, void *pdata);

int mackerel_create_host(struct mackerel_client *client, struct hostspec const *hostspec, mackerel_request_callback cb,
                         void *pdata);

int mackerel_update_host(struct mackerel_client *client, char const *hostid, struct hostspec const *hostspec,
                         mackerel_request_callback cb, void *pdata);

int mackerel_update_metrics(struct mackerel_client *client, char const *hostid, struct json_object *metric_values,
                            mackerel_request_callback cb, void *pdata);

#endif
