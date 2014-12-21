#!/bin/bash
src_dir=$(dirname $(readlink -f "$0"))
build_dir=${src_dir}/build

if [ -f "/usr/bin/ninja" ] ; then
  BUILD_COMMAND="ninja"
else
  BUILD_COMMAND="make"
fi

${src_dir}/build.sh
cd ${build_dir}
sudo ${BUILD_COMMAND} install
