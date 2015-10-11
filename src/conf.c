#include <string.h>
#include <uci.h>

#include "conf.h"
#include "log.h"

static char const *const package_name = "mackerel";

struct global_options global_options;
struct uci_context *uci_context;
struct uci_package *uci_package;

static struct uci_section *find_uci_section_by_type(struct uci_package *package, char const *type) {
  struct uci_element *p;
  uci_foreach_element(&package->sections, p) {
    struct uci_section *const ps = uci_to_section(p);
    if (strcmp(ps->type, type) == 0) {
      return ps;
    }
  }

  return NULL;
}

int load_globals(struct uci_context *ctx, struct uci_package *package) {
  struct uci_section *section_global = find_uci_section_by_type(package, "mackerel");
  if (!section_global) {
    ULOG_ERR("No mackerel section found in config\n");
    return -1;
  }

  global_options.apikey = uci_lookup_option_string(ctx, section_global, "apikey");
  if (!global_options.apikey) {
    ULOG_ERR("Option mackerel.apikey is required.\n");
    return -1;
  }

  global_options.hostname = uci_lookup_option_string(ctx, section_global, "hostname");

  return 0;
}

int load_config() {
  uci_context = uci_alloc_context();
  if (!uci_context) {
    ULOG_ERR("uci_alloc_context failed\n");
    goto error;
  }

  if (uci_load(uci_context, package_name, &uci_package) != 0) {
    ULOG_ERR("uci_load failed\n");
    goto error;
  }

  if (load_globals(uci_context, uci_package) != 0) {
    goto error;
  }

  return 0;

error:
  if (uci_package) {
    uci_unload(uci_context, uci_package);
  }

  if (uci_context) {
    uci_free_context(uci_context);
  }

  return -1;
}

int foreach_metric_config(void (*cb)(struct metric_options options)) {
  if(!uci_package) {
    ULOG_ERR("Config package is not loaded.\n");
    return -1;
  }

  struct uci_element *p;
  uci_foreach_element(&uci_package->sections, p) {
    struct uci_section *section = uci_to_section(p);
    if(strcmp(section->type, "metric") == 0) {
      char const *command = uci_lookup_option_string(uci_context, section, "command");
      cb((struct metric_options){.command=command});
    }
  }

  return 0;
}
