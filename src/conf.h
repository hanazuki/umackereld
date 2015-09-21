#ifndef UMACKERELD_CONF_H
#define UMACKERELD_CONF_H

struct global_options {
  const char *apikey;
  const char *hostname;
};

extern struct global_options global_options;

int load_config();

#endif
