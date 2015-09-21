#ifndef UMACKERELD_API_INTERNAL_H
#define UMACKERELD_API_INTERNAL_H
#include <curl/curl.h>
#include <json-c/json.h>

#include "api.h"

int mackerel_client_invoke(struct mackerel_client *client, char const *method,
                           char const *path, struct json_object *payload,
                           mackerel_request_callback cb, void *pdata);

struct json_request;
void json_request_free(struct json_request *req);
struct json_request *json_request_alloc(json_object *payload, mackerel_request_callback cb, void *pdata);
void json_request_headers_append(struct json_request *req, char const *header);
CURL *json_request_handle(struct json_request *req);
void json_request_setopts(struct json_request *req, char const *method, char const *url);
void json_request_invoke_callback(struct json_request *req, CURLcode code);
struct json_request *json_request_from_handle(CURL *curl);

#endif
