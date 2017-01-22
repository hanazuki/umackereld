#!/bin/bash
set -eu
cd "$(dirname "${BASH_SOURCE[0]}")/.."

TARGET_ARCH=x86_64-openwrt-linux
SDK_IMAGE=hanazuki/openwrt-sdk:15.05.1_x64
SYS_IMAGE=nmaas87/docker-openwrt:15.05.1_x64

docker run --rm -i -v "$PWD:/mnt/src" -w /mnt/src "$SDK_IMAGE" /bin/bash -exs <<EOF
(
  prefix=\$PWD/vendor/${TARGET_ARCH}
  mkdir -p build/${TARGET_ARCH}/cmocka
  mkdir -p \$prefix
  cd build/${TARGET_ARCH}/cmocka
  cmake -DCMAKE_C_COMPILER="${TARGET_ARCH}-gcc" -DCMAKE_INSTALL_PREFIX="\$prefix" ../../../vendor/cmocka
  make install
)

./autogen.sh
./configure --host=${TARGET_ARCH}
make
EOF
