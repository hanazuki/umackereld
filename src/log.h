#ifndef UMACKERELD_LOG_H
#define UMACKERELD_LOG_H
#include <libubox/ulog.h>

#define DEBUG(fmt, ...)                                                        \
  ulog(LOG_DEBUG, "%s:%d(%s) " fmt "\n", __FILE__, __LINE__,                   \
       __PRETTY_FUNCTION__, ##__VA_ARGS__)

#define DEBUG_ENTER DEBUG("enter")
#define DEBUG_EXIT DEBUG("exit")

#endif
