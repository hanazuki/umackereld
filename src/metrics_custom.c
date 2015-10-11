#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <libubox/uloop.h>
#include <json-c/json.h>

#include "stringutil.h"
#include "metrics.h"
#include "log.h"

struct metrics_process {
  struct uloop_process up;
  FILE *fp;
  collector_callback cb;
};

static void process_line(char *line, collector_callback yield) {
  // {name}\t{value}\t{timestamp}\n
  chomp(line);
  char *name = strtok(line, "\t");
  char *value = strtok(NULL, "\t");
  char *timestamp = strtok(NULL, "\t");
  if (!name || !value || !timestamp || strtok(NULL, "\t") != NULL) {
    return;
  }

  char *metric_name;
  if (asprintf(&metric_name, "custom.%s", name) == -1) {
    ULOG_ERR("asprintf failed.\n");
    return;
  }
  json_object *name_object = json_object_new_string(metric_name);
  free(metric_name);

  json_object *value_object;

  double dv;
  int64_t iv;
  if(strchr(value, '.') && json_parse_double(value, &dv) == 0) {
    value_object = json_object_new_double(dv);
  } else if(json_parse_int64(value, &iv) == 0) {
    value_object = json_object_new_int64(iv);
  } else {
    ULOG_ERR("Unparsable value [%s] from custom metrics command.\n", value);
    json_object_put(name_object);
    return;
  }

  yield((time_t)atol(timestamp), name_object, value_object);
}

static void metrics_process_callback(struct uloop_process *c, int ret) {
  struct metrics_process *p = container_of(c, struct metrics_process, up);
  ULOG_INFO("Process %d exited with status 0x%X.\n", p->up.pid, ret);

  if(WIFEXITED(ret) && WEXITSTATUS(ret) == 0) {
    if(fseek(p->fp, 0, SEEK_SET) == 0) {
      char buf[1000];
      while(fgets(buf, sizeof buf, p->fp)) {
        process_line(buf, p->cb);
      }
    }
  }
  fclose(p->fp);
  free(p);
}

void metrics_collect_custom(collector_callback yield, char const *command) {
  FILE *tfp = tmpfile();
  if(!tfp) {
    ULOG_ERR("tmpfile failed.\n");
    return;
  }

  pid_t pid = fork();
  switch (pid) {
    case -1:  // error
      ULOG_ERR("fork failed.\n");
      fclose(tfp);
      return;
    case 0: {  // child
      dup2(fileno(tfp), STDOUT_FILENO);
      fclose(tfp);

      execv("/bin/sh", (char *[]){"sh", "-c", strdup(command), NULL});

      ULOG_ERR("execv failed.\n");
      _exit(-1);
    }
    default: {  // parent
      ULOG_INFO("Spawned process %d for command: %s.\n", pid, command);

      struct metrics_process *p = malloc(sizeof *p);
      if (!p) {
        ULOG_ERR("malloc failed.\n");
        fclose(tfp);
        return;
      }

      *p = (struct metrics_process){
          .up =
              {
                  .cb = metrics_process_callback, .pid = pid,
              },
          .fp = tfp,
          .cb = yield
      };
      uloop_process_add(&p->up);
      return;
    }
  }
}
