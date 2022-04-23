#!/bin/sh

set -ev

FILE_BASENAME="tcp_file_transfer_server"

if test -z "${MESON_BUILD_ROOT}"; then
  echo "This script is used by the build system. Use 'ninja test'."
  exit 1
fi

cd "$MESON_BUILD_ROOT/test"
${MESON_BUILD_ROOT}/tcp_file_transfer_server &

# Give the server time to bind and listen ;)
sleep 3s
# Test by transferring the server binary
${MESON_BUILD_ROOT}/tcp_file_transfer_client -f "${MESON_BUILD_ROOT}/${FILE_BASENAME}"

rm "${FILE_BASENAME}"

exit 0
