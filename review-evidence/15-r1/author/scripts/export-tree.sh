#!/bin/bash
# Usage: export-tree.sh <checkout> <commit> <dest>
# Writes a clean copy of <commit> plus the content of every initialized
# submodule (recursive, at the gitlinks recorded in the checkout) to <dest>,
# without any .git metadata, so disposable builds never touch the checkout.
set -eu
co=$1
commit=$2
dest=$3
rm -rf "$dest"
mkdir -p "$dest"
git -C "$co" archive --format=tar "$commit" | tar -x -C "$dest"
git -C "$co" submodule foreach --quiet --recursive 'echo "$displaypath $sha1"' |
while read -r path sha; do
	mkdir -p "$dest/$path"
	git -C "$co/$path" archive --format=tar "$sha" | tar -x -C "$dest/$path"
	echo "exported $path at $sha"
done
echo "exported $commit to $dest"
