#include <linux/kernel.h>
#include <sys/sysinfo.h>
#include <time.h>

#include "metrics.h"
#include "log.h"

void metrics_collect_loadavg5(collector_callback yield) {
  time_t now = time(NULL);

  struct sysinfo info;
  if (sysinfo(&info) == 0) {
    double load = info.loads[1] / (double)(1 << SI_LOAD_SHIFT);
    yield(now, json_object_new_string("loadavg5"),
          json_object_new_double(load));
  } else {
    ULOG_ERR("sysinfo failed.\n");
  }
}
