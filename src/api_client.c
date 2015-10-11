#include <assert.h>
#include <json-c/json.h>
#include <libubox/uloop.h>

#include "api_internal.h"
#include "log.h"

struct mackerel_client {
  struct mackerel_params params;
  CURLM *curlmulti;
};

struct socket_info {
  CURLM *curlmulti;
  struct uloop_fd ufd;
};

struct timer_info {
  CURLM *curlmulti;
  struct uloop_timeout uto;
};

static void check_multi_info(CURLM *curlmulti);

static void socket_handler(struct uloop_fd *u, unsigned int events) {
  // DEBUG("enter");
  struct socket_info *sockinfo = container_of(u, struct socket_info, ufd);
  unsigned const flags = (events & ULOOP_READ ? CURL_POLL_IN : 0) | (events & ULOOP_WRITE ? CURL_POLL_OUT : 0);
  int running;
  curl_multi_socket_action(sockinfo->curlmulti, sockinfo->ufd.fd, flags, &running);
  check_multi_info(sockinfo->curlmulti);
  // DEBUG("exit");
}

static int socket_callback(CURL *easy, curl_socket_t s, int what, void *userp, void *socketp) {
  // DEBUG_ENTER;

  (void)easy;

  struct mackerel_client *client = userp;
  assert(client != NULL);
  struct socket_info *sockinfo = socketp;

  if (what & CURL_POLL_REMOVE) {
    if (!sockinfo) {  // already removed
      // DEBUG_EXIT;
      return 0;
    }

    uloop_fd_delete(&sockinfo->ufd);
    // DEBUG("free %p", sockinfo);
    curl_multi_assign(client->curlmulti, s, NULL);
    free(sockinfo);
    check_multi_info(client->curlmulti);  // XXX: nazeka hitsuyou
  } else {
    if (!sockinfo) {
      sockinfo = malloc(sizeof *sockinfo);
      if (!sockinfo) {
        // DEBUG_EXIT;
        return -1;
      }

      *sockinfo = (struct socket_info){.curlmulti = client->curlmulti, .ufd = {.cb = socket_handler, .fd = s}};

      curl_multi_assign(client->curlmulti, s, sockinfo);
    } else {
      uloop_fd_delete(&sockinfo->ufd);
    }

    unsigned const flags = (what & CURL_POLL_IN ? ULOOP_READ : 0) | (what & CURL_POLL_OUT ? ULOOP_WRITE : 0);
    uloop_fd_add(&sockinfo->ufd, flags);
  }

  // DEBUG_EXIT;
  return 0;
}

static void timeout_handler(struct uloop_timeout *t) {
  struct timer_info *timerinfo = container_of(t, struct timer_info, uto);
  int running;
  curl_multi_socket_action(timerinfo->curlmulti, CURL_SOCKET_TIMEOUT, 0, &running);
  check_multi_info(timerinfo->curlmulti);
  free(timerinfo);
}

static int timer_callback(CURLM *multi, long timeout_ms, void *userp) {
  struct mackerel_clinet *client = userp;
  assert(client != NULL);

  if (timeout_ms == 0) {
    int running;
    curl_multi_socket_action(multi, CURL_SOCKET_TIMEOUT, 0, &running);
    check_multi_info(multi);
  } else {
    struct timer_info *timerinfo = malloc(sizeof *timerinfo);
    if (!timerinfo) {
      ULOG_ERR("malloc failed.\n");
      return -1;
    }

    *timerinfo = (struct timer_info){
        .curlmulti = multi,
        .uto =
            {
                .cb = timeout_handler,
            },
    };

    if (timeout_ms > INT_MAX) {
      ULOG_WARN("Requested timeout is too long.\n");
      uloop_timeout_set(&timerinfo->uto, INT_MAX);
    } else {
      uloop_timeout_set(&timerinfo->uto, (int)timeout_ms);
    }
  }
  return 0;
}

static void check_multi_info(CURLM *curlmulti) {
  // DEBUG_ENTER;
  int msgq;
  CURLMsg *msg;
  while ((msg = curl_multi_info_read(curlmulti, &msgq))) {
    if (msg->msg == CURLMSG_DONE) {
      struct json_request *req = json_request_from_handle(msg->easy_handle);
      assert(req != NULL);

      json_request_invoke_callback(req, msg->data.result);
      curl_multi_remove_handle(curlmulti, msg->easy_handle);
      json_request_free(req);
    }
  }
  // DEBUG_EXIT;
}

void mackerel_client_free(struct mackerel_client *client) {
  // DEBUG_ENTER;
  if (client) {
    if (client->curlmulti) {
      curl_multi_cleanup(client->curlmulti);
    }

    free(client);
  }
  // DEBUG_EXIT;
}

struct mackerel_client *mackerel_client_alloc(struct mackerel_params params) {
  // DEBUG_ENTER;
  struct mackerel_client *client = malloc(sizeof *client);
  if (!client) {
    ULOG_ERR("malloc failed.\n");
    goto error;
  }
  client->params = params;
  client->curlmulti = NULL;

  client->curlmulti = curl_multi_init();
  if (!client->curlmulti) {
    ULOG_ERR("curl_multi_init failed.\n");
    goto error;
  }

  curl_multi_setopt(client->curlmulti, CURLMOPT_SOCKETFUNCTION, socket_callback);
  curl_multi_setopt(client->curlmulti, CURLMOPT_SOCKETDATA, client);
  curl_multi_setopt(client->curlmulti, CURLMOPT_TIMERFUNCTION, timer_callback);
  curl_multi_setopt(client->curlmulti, CURLMOPT_TIMERDATA, client);

  // DEBUG_EXIT;
  return client;

error:
  mackerel_client_free(client);
  // DEBUG_EXIT;
  return NULL;
}

// takes ownership of payload
int mackerel_client_invoke(struct mackerel_client *client, char const *method, char const *path,
                           struct json_object *payload, mackerel_request_callback cb, void *pdata) {
  assert(path[0] == '/');

  struct json_request *req = json_request_alloc(payload, cb, pdata);
  if (!req) {
    ULOG_ERR("json_request_alloc failed.\n");
    goto error;
  }

  ULOG_INFO("Request[%p]: prepare to %s %s\n", req, method, path);

  {
    char *header;
    if (asprintf(&header, "X-Api-Key: %s", client->params.apikey) == -1) {
      ULOG_ERR("asprintf failed.\n");
      goto error;
    }
    json_request_headers_append(req, header);
    free(header);
  }

  {
    char *url;
    if (asprintf(&url, "%s%s", client->params.base_uri, path) == -1) {
      ULOG_ERR("asprintf failed.\n");
      goto error;
    }
    json_request_setopts(req, method, url);
    free(url);
  }

  if (curl_multi_add_handle(client->curlmulti, json_request_handle(req)) != CURLM_OK) {
    ULOG_ERR("curl_multi_add_handle failed.\n");
    goto error;
  }
  ULOG_INFO("Request[%p]: initiated\n", req);

  return 0;

error:
  json_request_free(req);
  return -1;
}
