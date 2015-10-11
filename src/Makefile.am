export STAGING_DIR=@STAGING_DIR@

bin_PROGRAMS=umackereld
umackereld_SOURCES=main.c conf.c hostid.c fileutil.c stringutil.c \
	api.c api_client.c api_request.c \
	hostspec_hostname.c hostspec_block_device.c hostspec_kernel.c hostspec_cpu.c hostspec_memory.c hostspec_filesystem.c \
	metrics.c metrics_loadavg.c metrics_custom.c
umackereld_CFLAGS=-Wall -Wextra -Werror -D_GNU_SOURCE -std=gnu11
