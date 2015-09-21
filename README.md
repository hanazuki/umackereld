# umackereld

`umackereld`, which stands for "micro mackerel daemon," is a small [Mackerel](https://mackerel.io) agent replacement written for [OpenWrt](https://openwrt.org/)-based embedded Linux systems.

## Dependency

`umackereld` depends on the following libraries:

- `libubox`
- `libubus`
- `libuci`
- `libjson-c`
- `libcurl` compiled with a TLS support (only tested with `mbedtls`)

and the following platform components:

- `netifd`
- `ubusd`

In order to run `umackereld` on a standard Chaos Calmer system, you need to install `libcurl` and `ca-certificates` packages with opkg.

## Build

For cross-compiling `umackereld` for your OpenWrt system, you have to prepare OpenWrt Buildroot (or OpenWrt SDK will do, meybe).
Make sure all the libraries listed in the Dependency section is built for your target system.

TODO: Write a friendly howto here...

Something as follows should work (replace "mips" with your favorite architecture).

```
export PATH=/path/to/buildroot/staging_dir/host/bin:/path/to/buildroot/staging_dir/toolchain-mips_foo_gcc-bar_uClibc-baz/bin:$PATH
autoreconf --install
./configure --host=mips-openwrt-linux STAGING_DIR=/path/to/buildroot/staging_dir/target-mips_foo_uClibc-baz
make
```

## Configuration

`umackereld` uses UCI configuration system and by default reads the file at `/etc/config/mackerel`.
Only `mackerel.apikey` option is mandatory.

```
config mackerel
  option apikey 'YOUR_API_KEY'
  option hostname 'override.hostname.localdomain'
```

## License

```
Copyright 2015 Kasumi Hanazuki <kasumi@rollingapple.net>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```
