#!/usr/bin/env python3
"""R231 receipt runner: rr.py <label> <cwd> -- <argv...>
Runs argv in cwd (optionally with KEY=VAL env via --env before --), writes the
combined stdout/stderr to logs/<label>.log and appends one JSON line with the
argv, cwd, env overrides, exit status, timestamps and log SHA-256 to
commands.jsonl. Exits with the child's status."""
import sys, os, json, subprocess, hashlib, datetime, time
here = os.path.dirname(os.path.abspath(__file__))
args = sys.argv[1:]
label, cwd = args[0], args[1]
rest = args[2:]
env_over = {}
while rest and rest[0] != '--':
    if rest[0] == '--env':
        k, v = rest[1].split('=', 1); env_over[k] = v; rest = rest[2:]
    else:
        raise SystemExit('bad args')
argv = rest[1:]
env = dict(os.environ); env.update(env_over)
log = os.path.join(here, 'logs', label + '.log')
start = datetime.datetime.now().astimezone().isoformat()
t0 = time.monotonic()
with open(log, 'wb') as f:
    p = subprocess.run(argv, cwd=cwd, env=env, stdout=f, stderr=subprocess.STDOUT)
dt = round(time.monotonic() - t0, 3)
data = open(log, 'rb').read()
rec = {'label': label, 'cwd': cwd, 'argv': argv, 'env': env_over,
       'exit': p.returncode, 'start': start, 'seconds': dt,
       'log': 'logs/' + label + '.log', 'log_sha256': hashlib.sha256(data).hexdigest()}
with open(os.path.join(here, 'commands.jsonl'), 'a') as f:
    f.write(json.dumps(rec) + '\n')
print(f"[{label}] exit={p.returncode} seconds={dt} log={rec['log']}")
sys.exit(p.returncode)
