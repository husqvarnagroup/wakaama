#!/usr/bin/env bash

set -u
set -e

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

cd "$SCRIPT_DIR"

IMAGE_NAME=wakaama-s390x

# needed only once
docker run --rm --privileged multiarch/qemu-user-static --reset -p yes

# nedded only once
docker build -t $IMAGE_NAME .


CONTAINER_NAME=wakaama-build
docker run --rm -d -it -v $(realpath $(pwd)/../..):/work --name $CONTAINER_NAME $IMAGE_NAME /bin/bash

trap "docker kill $CONTAINER_NAME &> /dev/null || true" EXIT

#docker exec -it -w /work $CONTAINER_NAME bash -c "addr2line -e /work/build-s390x/tests/lwm2munittests_server +0x1040a"
docker exec -it -w /work $CONTAINER_NAME bash -c "git config --global --add safe.directory /work && git config --global --add safe.directory /work/examples/shared/tinydtls"
docker exec -it -w /work $CONTAINER_NAME bash -c "cmake -G Ninja -S . -B build-s390x"
docker exec -it -w /work $CONTAINER_NAME bash -c "cmake --build build-s390x --target all"
docker exec -it -w /work $CONTAINER_NAME bash -c "CTEST_OUTPUT_ON_FAILURE=1 cmake --build build-s390x --target test"

docker stop $CONTAINER_NAME

