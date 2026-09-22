#!/usr/bin/env python3
"""Fetch selected public evidence blobs (read-only) from kebag-logic/tsn-gen
tree 0be5bf0dc34bd7924b4227c1c2a2602849db4c52 and verify each git blob SHA."""
import json, subprocess, sys, base64, hashlib, os
tree = json.load(open('evidence-tree-0be5bf0.json'))
want = sys.argv[1:]
index = {e['path']: e for e in tree['tree'] if e['type'] == 'blob'}
for p in want:
    e = index[p]
    raw = subprocess.run(['gh', 'api', f"repos/kebag-logic/tsn-gen/git/blobs/{e['sha']}"],
                         capture_output=True, check=True).stdout
    data = base64.b64decode(json.loads(raw)['content'])
    h = hashlib.sha1(b'blob %d\0' % len(data) + data).hexdigest()
    assert h == e['sha'], (p, h, e['sha'])
    out = os.path.join('public-evidence', p)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    open(out, 'wb').write(data)
    print('OK', e['sha'], len(data), p)
