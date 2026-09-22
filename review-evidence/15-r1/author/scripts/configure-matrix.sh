#!/bin/bash
# Usage: configure-matrix.sh <label> <source-dir> <build-root> <capture-root>
# Configures <source-dir> (Unix Makefiles, as setup.sh does) once per build
# type (empty, Debug, Release, MinSizeRel, RelWithDebInfo) and per
# BUILD_SHARED_LIBS choice (unset, ON), then captures the generated
# compile_commands.json, every link.txt and flags.make, and the cached build
# type into <capture-root>/<label>/<cfg>-<shared>/. Configure only: nothing
# is compiled here.
set -u
label=$1
src=$2
broot=$3
croot=$4
status=0
for cfg in empty Debug Release MinSizeRel RelWithDebInfo; do
	for shared in unset ON; do
		name="${cfg}-shared_${shared}"
		bdir="$broot/$label/$name"
		cdir="$croot/$label/$name"
		rm -rf "$bdir" "$cdir"
		mkdir -p "$bdir" "$cdir"
		args=(-S "$src" -B "$bdir" -G "Unix Makefiles" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON)
		[ "$cfg" != empty ] && args+=(-DCMAKE_BUILD_TYPE="$cfg")
		[ "$shared" = ON ] && args+=(-DBUILD_SHARED_LIBS=ON)
		env -u CMAKE_BUILD_TYPE cmake "${args[@]}" > "$cdir/configure.log" 2>&1
		rc=$?
		echo "$rc" > "$cdir/configure.exit"
		printf '%q ' cmake "${args[@]}" > "$cdir/configure.argv"
		echo "$label $name configure exit $rc"
		[ $rc -ne 0 ] && status=$rc && continue
		cp "$bdir/compile_commands.json" "$cdir/"
		grep -E '^(CMAKE_BUILD_TYPE|BUILD_SHARED_LIBS|CMAKE_CXX_COMPILER|CMAKE_CXX_FLAGS[A-Z_]*|ENABLE_PARSER_TESTS|CMAKE_GENERATOR)[:=]' \
			"$bdir/CMakeCache.txt" > "$cdir/cache-selected.txt"
		(cd "$bdir" && find . -name link.txt -path '*CMakeFiles/*' | sort | while read -r f; do
			d="$cdir/link/${f#./}"
			mkdir -p "$(dirname "$d")"
			cp "$f" "$d"
		done
		find . -name flags.make -path '*CMakeFiles/*' | sort | while read -r f; do
			d="$cdir/flags/${f#./}"
			mkdir -p "$(dirname "$d")"
			cp "$f" "$d"
		done)
	done
done
exit $status
