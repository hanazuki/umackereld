#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "hostid.h"
#include "log.h"

static char const *const vardir = "/etc/mackerel";
static char const *const hostid_file = "/etc/mackerel/id";

char *hostid = NULL;

int hostid_set(char const *id) {
  if (hostid && strcmp(id, hostid) == 0) {
    return 0;
  }

  free(hostid);
  hostid = strdup(id);
  ULOG_INFO("HostID assigned: %s\n", hostid);

  (void)mkdir(vardir, 0755);

  int fd = open(hostid_file, O_CREAT | O_RDWR, 0644);
  if (fd == -1) {
    ULOG_ERR("Unable to open %s. HostID is not saved.\n", hostid_file);
    return -1;
  }

  if (flock(fd, LOCK_EX) != 0) {
    ULOG_ERR("Unable to lock %s. HostID is not saved.\n", hostid_file);
    close(fd);
    return -1;
  }

  ftruncate(fd, 0);
  ssize_t len = strlen(hostid);
  if (write(fd, hostid, strlen(hostid)) != len) {
    ULOG_ERR("Unable to write into %s. HostID is not saved.\n", hostid_file);
    ftruncate(fd, 0);
    close(fd);
    return -1;
  }

  ULOG_INFO("HostID is saved to %s.\n", hostid_file);

  return 0;
}

int hostid_get() {
  int fd = open(hostid_file, O_RDONLY);
  if (fd == -1) {
    ULOG_ERR("Unable to open %s. HostID is not loaded.\n", hostid_file);
    return -1;
  }

  if (flock(fd, LOCK_SH) != 0) {
    ULOG_ERR("Unable to lock %s. HostID is not loaded.\n", hostid_file);
    close(fd);
    return -1;
  }

  char buffer[100];
  ssize_t cnt = read(fd, buffer, sizeof buffer);
  if (cnt == -1) {
    ULOG_ERR("Unable to read %s. HostID is not loaded.\n", hostid_file);
    close(fd);
    return -1;
  }

  close(fd);

  for (size_t i = 0; i < (size_t)cnt; ++i) {
    if (buffer[i] == '\n') {
      cnt = i;
      break;
    }
  }

  if (cnt == 0) {
    ULOG_ERR("%s is empty. HostID is not loaded.\n", hostid_file);
    return -1;
  }

  free(hostid);
  hostid = strndup(buffer, (size_t)cnt);
  ULOG_INFO("HostID loaded: %s\n", hostid);

  return 0;
}
