#include <assert.h>
#include <string.h>

#include "api_internal.h"
#include "log.h"

struct json_request {
  struct json_tokener *tokener;
  CURL *curl;

  struct curl_slist *custom_headers;

  json_object *payload_object;
  char const *payload_buffer;
  size_t payload_length;

  json_object *result_object;

  mackerel_request_callback cb;
  void *pdata;
};

void json_request_free(struct json_request *req) {
  if (req) {
    if (req->payload_object) {
      json_object_put(req->payload_object);
    }

    if (req->custom_headers) {
      curl_slist_free_all(req->custom_headers);
    }

    if (req->curl) {
      curl_easy_cleanup(req->curl);
    }

    if (req->tokener) {
      json_tokener_free(req->tokener);
    }

    free(req);
  }
}

// takes ownership of payload
struct json_request *json_request_alloc(json_object *payload, mackerel_request_callback cb, void *pdata) {
  struct json_request *req = malloc(sizeof *req);
  if (!req) {
    ULOG_ERR("malloc failed.\n");
    goto error;
  }
  *req = (struct json_request){.cb = cb, .pdata = pdata};

  req->tokener = json_tokener_new();
  if (!req->tokener) {
    ULOG_ERR("json_tokener_new failed.\n");
    goto error;
  }

  req->curl = curl_easy_init();
  if (!req->curl) {
    ULOG_ERR("curl_easy_init failed.\n");
    goto error;
  }

  if (payload) {
    req->payload_object = payload;
    req->payload_buffer = json_object_to_json_string(req->payload_object);
    req->payload_length = strlen(req->payload_buffer);
  }

  return req;
error:
  json_request_free(req);
  return NULL;
}

size_t json_request_read_callback(char *buffer, size_t size, size_t nitems, void *instream) {
  struct json_request *req = instream;

  size_t const available = req->payload_length;
  size_t const requested = size * nitems;
  size_t const copied = available < requested ? available : requested;

  memcpy(buffer, req->payload_buffer, copied);
  req->payload_buffer += copied;
  req->payload_length -= copied;

  return copied;
}

size_t json_request_write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  struct json_request *req = userdata;
  size_t const available = size * nmemb;

  if (!req->result_object) {
    json_object *obj = json_tokener_parse_ex(req->tokener, ptr, available);

    if (obj) {
      req->result_object = obj;
    } else if (json_tokener_get_error(req->tokener) != json_tokener_continue) {
      ULOG_ERR("JSON parse error\n");
      return 0;  // signal error to curl
    }
  }

  return available;
}

void json_request_headers_append(struct json_request *req, char const *header) {
  req->custom_headers = curl_slist_append(req->custom_headers, header);
}

struct curl_slist *json_request_headers_get(struct json_request *req) {
  return req->custom_headers;
}

CURL *json_request_handle(struct json_request *req) { return req->curl; }

void json_request_setopts(struct json_request *req, char const *method, char const *url) {
  CURL *curl = req->curl;
  assert(curl);
  assert(method);

  curl_easy_setopt(curl, CURLOPT_PRIVATE, req);

  if (req->payload_buffer && req->payload_length > 0) {
    ULOG_INFO("Request[%p]: payload %.*s\n", req, req->payload_length, req->payload_buffer);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE, req->payload_length);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, json_request_read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, req);
    json_request_headers_append(req, "Content-Type: application/json");
  }

  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, req->custom_headers);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, json_request_write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, req);
}

void json_request_invoke_callback(struct json_request *req, CURLcode code) {
  if (req->cb) {
    req->cb(code, req->result_object, req->pdata);
  }
}

struct json_request *json_request_from_handle(CURL *curl) {
  struct json_request *req = NULL;
  curl_easy_getinfo(curl, CURLINFO_PRIVATE, (char **)&req);
  return req;
}
