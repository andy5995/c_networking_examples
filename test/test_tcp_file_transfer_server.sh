#!/bin/sh

set -ev

if [ -e COMMON ]; then
  . ./COMMON
else
  . "${MESON_SOURCE_ROOT}/test/COMMON"
fi

cd ${MESON_BUILD_ROOT}/test
if [ -e ${XFER_FILE_BASENAME} ]; then
  rm ${XFER_FILE_BASENAME}
fi

${VALGRIND} ${MESON_BUILD_ROOT}/tcp_file_transfer_server

rm ${XFER_FILE_BASENAME}

exit 0
