#!/bin/sh

## build SPIRV-Cross
#
# 1. git clone https://github.com/KhronosGroup/SPIRV-Cross
# 2. ./checkout_glslang_spirv_tools.sh && ./build_glslang_spirv_tools.sh
# 3. cmake .

glslc -o fade_edge_fs.spv fade_edge.frag -I.
spirv-cross --version 350 --no-es fade_edge_fs.spv --output fade_edge_fs_350.glsl

## prevent select file "*.glsl"
shopt -s nullglob

for f in *.{glsl,metal}; do
	dos2unix ${f} ${f};
done;

for f in *.{glsl,metal}; do
	filename="${f%.*}"
	text2c ${f} ${f}.h $filename
done;

shopt -u nullglob
