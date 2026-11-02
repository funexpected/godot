#!/usr/bin/env bash
# SCons flags that leave out of the library-mode iOS engine (ios.library,
# ios.library.simulator) every module the futuramath game never touches, so
# they stop costing app size. Printed on one line for `$(...)` in the CI job
# and mirrored by apps/math/scripts/build-godot.ts in the futuramath repo —
# change both together.
#
# Kept on purpose: gdnative (game/libcpp), regex (without the PCRE2 JIT, as
# the Android template already builds it), png/jpg/pvr/squish, ogg/vorbis and
# minimp3 (speech, music), freetype, mbedtls, the fnx modules (flash, spine,
# animation_state, fnxext, steering_behaviours) and the 3D nodes the ARKit
# minigame needs — with GodotPhysics in place of Bullet, like Android.
modules=(
  visual_script
  bullet
  csg
  gridmap
  svg
  vhacd
  webrtc
  upnp
  enet
  jsonrpc
  webxr
  mobile_vr
  opensimplex
  theora
  webm
  opus
  tinyexr
  hdr
  dds
  tga
  bmp
  etc
  cvtt
  fbx
  lightmapper_cpu
  xatlas_unwrap
  raycast
  denoise
  mono
)
flags=()
for m in "${modules[@]}"; do
  flags+=("module_${m}_enabled=no")
done
flags+=("builtin_pcre2_with_jit=no")
echo "${flags[*]}"
