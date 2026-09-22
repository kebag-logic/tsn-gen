#!/usr/bin/env python3
"""export_tree.py <clone> <commit> <dest>: export the superproject tree at
<commit> plus the initialized googletest/rapidyaml submodules (recursively, at
the commits the superproject/submodule trees pin) into <dest> with git
archive, then verify every exported regular file byte-for-byte (git blob
SHA-1) and mode against the pinned trees. No symlinks to the clone."""
import sys, os, subprocess, hashlib, io, tarfile
clone, commit, dest = sys.argv[1:4]
def git(repo, *a):
    return subprocess.run(['git', '-C', repo, *a], check=True, capture_output=True).stdout
def export(repo, rev, prefix):
    data = git(repo, 'archive', '--format=tar', f'--prefix={prefix}', rev)
    tarfile.open(fileobj=io.BytesIO(data)).extractall(dest, filter='tar')
def verify(repo, rev, prefix):
    n = bad = 0; gitlinks = []
    out = git(repo, 'ls-tree', '-r', '-z', rev).split(b'\0')
    for line in out:
        if not line: continue
        meta, path = line.split(b'\t', 1)
        mode, typ, sha = meta.split()
        path = path.decode()
        if typ == b'commit':
            gitlinks.append((path, sha.decode())); continue
        f = os.path.join(dest, prefix, path)
        if mode == b'120000':
            ok = os.path.islink(f) and os.readlink(f).encode() == git(repo, 'cat-file', 'blob', sha.decode())
        else:
            b = open(f, 'rb').read()
            ok = hashlib.sha1(b'blob %d\0' % len(b) + b).hexdigest() == sha.decode()
            ok = ok and (bool(os.stat(f).st_mode & 0o100) == (mode == b'100755'))
        n += 1; bad += (not ok)
        if not ok: print('MISMATCH', prefix + path)
    return n, bad, gitlinks
os.makedirs(dest, exist_ok=False)
export(clone, commit, '')
n, bad, links = verify(clone, commit, '')
print(f'superproject {commit}: {n} entries verified, mismatches {bad}')
want = {'external/googletest', 'external/rapidyaml'}
todo = [(clone, p, s) for p, s in links if p in want]
while todo:
    parent, path, sha = todo.pop(0)
    repo = os.path.join(clone, path) if parent == clone else parent
    sub = os.path.join(clone, path)
    head = git(sub, 'rev-parse', 'HEAD').decode().strip()
    assert head == sha, (path, head, sha)
    export(sub, sha, path + '/')
    n, bad, sublinks = verify(sub, sha, path + '/')
    print(f'submodule {path} @ {sha}: {n} entries verified, mismatches {bad}')
    todo += [(sub, path + '/' + p, s) for p, s in sublinks]
