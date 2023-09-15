#!/bin/bash
set -eu

export PROJECT_DIR=$(pwd)

export BUILD_TYPE="${BUILD_TYPE:-Release}"
export ENABLE_SANITIZER="${ENABLE_SANITIZER:-""}"
export BUILD_TESTS="${BUILD_TESTS:-""}"

echo "Build type: ${BUILD_TYPE}"
echo

export BUILD_DIR="${PROJECT_DIR}/build/"
export SOURCE_DIR="${PROJECT_DIR}"

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

mkdir -p "${BUILD_TYPE}"
cd "${BUILD_TYPE}"

cmake "${SOURCE_DIR}"                           \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"          \
    -DENABLE_SANITIZER="${ENABLE_SANITIZER}"    \
    -DBUILD_TESTS="${BUILD_TESTS}"

cmake --build . -- -j $(nproc)

cd "${PROJECT_DIR}"
mv "${BUILD_DIR}/${BUILD_TYPE}/compile_commands.json" .
