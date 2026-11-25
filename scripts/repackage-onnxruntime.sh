#!/usr/bin/env bash
# Rebuild the per-platform ONNX Runtime archives that cmake/onnxruntime.cmake downloads from
# this repo's GitHub release, starting from Microsoft's official NuGet package.
#
#   scripts/repackage-onnxruntime.sh <out-dir>
#   gh release upload onnxruntime-<version> <out-dir>/*.tgz      # (maintainer only)
#
# Why: github.com/microsoft/onnxruntime returned 404 from 2026-11-25, so the official
# per-platform archives were unreachable. NuGet still serves the same official binaries, but as
# one 184 MB package for all platforms. We keep a verified, trimmed copy instead.
# Upstream is MIT-licensed; LICENSE and ThirdPartyNotices.txt are kept in every archive.
set -euo pipefail

version="1.32.0"
# SHA512 (base64) of the .nupkg as published in the NuGet catalog ("packageHash").
nupkg_sha512="fj2P9s5o2s5MvwwdDiCF4frwXUJed1gLh1wb0igCUnonbrvfoHMmAFtaj9atgYbBflcQtE3FQ6zGv/s75FQEsQ=="
url="https://api.nuget.org/v3-flatcontainer/microsoft.ml.onnxruntime/${version}/microsoft.ml.onnxruntime.${version}.nupkg"

out="$(mkdir -p "${1:?usage: $0 <out-dir>}" && cd "$1" && pwd)"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT

echo "Downloading Microsoft.ML.OnnxRuntime ${version} from NuGet..."
curl -sSfL -o "$work/ort.nupkg" "$url"
actual="$(openssl dgst -sha512 -binary "$work/ort.nupkg" | base64 | tr -d '\n')"
if [[ "$actual" != "$nupkg_sha512" ]]; then
  echo "SHA512 mismatch for nupkg: $actual" >&2
  exit 1
fi
mkdir "$work/pkg" && (cd "$work/pkg" && unzip -q ../ort.nupkg)

# Layout matches the official GitHub archives: include/, lib/ with versioned name + symlinks
# matching the install name (@rpath/libonnxruntime.1.dylib) or SONAME (libonnxruntime.so.1).
package() {  # <platform> <nuget-runtime> <lib-ext>
  local plat="$1" rid="$2" ext="$3"
  local name="onnxruntime-${plat}-${version}"
  local dst="$work/$name"
  mkdir -p "$dst/include" "$dst/lib"
  cp "$work/pkg/build/native/include/"* "$dst/include/"
  cp "$work/pkg/LICENSE" "$work/pkg/ThirdPartyNotices.txt" "$dst/"
  printf 'Microsoft.ML.OnnxRuntime %s (NuGet runtimes/%s), repackaged by %s\n' \
    "$version" "$rid" "scripts/repackage-onnxruntime.sh" >"$dst/PROVENANCE.txt"
  local src="$work/pkg/runtimes/$rid/native"
  if [[ "$ext" == dylib ]]; then
    cp "$src/libonnxruntime.dylib" "$dst/lib/libonnxruntime.${version}.dylib"
    ln -s "libonnxruntime.${version}.dylib" "$dst/lib/libonnxruntime.1.dylib"
    ln -s "libonnxruntime.1.dylib" "$dst/lib/libonnxruntime.dylib"
  else
    cp "$src/libonnxruntime.so" "$dst/lib/libonnxruntime.so.${version}"
    ln -s "libonnxruntime.so.${version}" "$dst/lib/libonnxruntime.so.1"
    ln -s "libonnxruntime.so.1" "$dst/lib/libonnxruntime.so"
    cp "$src/libonnxruntime_providers_shared.so" "$dst/lib/"
  fi
  # Neutral owner and no macOS metadata (xattrs, ._ files) in the published archives.
  COPYFILE_DISABLE=1 tar -C "$work" --uid 0 --gid 0 --uname root --gname root \
    --no-xattrs --no-mac-metadata -czf "$out/$name.tgz" "$name"
  echo "$(shasum -a 256 "$out/$name.tgz" | cut -d' ' -f1)  $name.tgz"
}

package osx-arm64 osx-arm64 dylib
package linux-x64 linux-x64 so
package linux-aarch64 linux-arm64 so
