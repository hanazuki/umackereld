#ifndef UMACKERELD_CONF_H
#define UMACKERELD_CONF_H

struct global_options {
  const char *apikey;
  const char *hostname;
};

struct metric_options {
  const char *command;
};

extern struct global_options global_options;

int load_config();
int foreach_metric_config(void (*)(struct metric_options options));

#endif
