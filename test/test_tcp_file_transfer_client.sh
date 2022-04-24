#!/bin/sh

set -ev

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

# Give the server time to bind and listen ;)
sleep 3s
# Test by transferring the server binary
${VALGRIND} ${MESON_BUILD_ROOT}/tcp_file_transfer_client -f ${XFER_FILE_URI}


exit 0
