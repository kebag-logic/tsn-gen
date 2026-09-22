#!/usr/bin/env python3
"""scan_lasttest.py <LastTest.log>...: per test block, report pass/fail and
whether its retained output contains a sanitizer report marker."""
import re, sys
for path in sys.argv[1:]:
    t = open(path, errors='replace').read()
    parts = re.split(r'\n(?=\d+/\d+ Testing: )', t)
    blocks = [p for p in parts if re.match(r'\d+/\d+ Testing: ', p)]
    res = []
    for b in blocks:
        name = re.match(r'\d+/\d+ Testing: (.*)', b).group(1).strip()
        cmd = re.search(r'\nCommand: (.*)', b).group(1)
        exe = re.search(r'"([^"]+)"', cmd).group(1).rsplit('/', 1)[-1]
        status = 'Passed' if 'Test Passed.' in b else ('Failed' if 'Test Failed.' in b else '?')
        marks = sorted(p for p in ('runtime error:', 'AddressSanitizer', 'UndefinedBehaviorSanitizer', 'LeakSanitizer', 'Sanitizer') if p in b)
        res.append((name, exe, status, marks))
    print(f'== {path}: {len(blocks)} test blocks; passed {sum(r[2]=="Passed" for r in res)}; failed {sum(r[2]=="Failed" for r in res)}')
    print('   blocks with sanitizer markers:')
    for r in res:
        if r[3]: print('    ', r)
    print('   passing blocks retain output (non-empty between Output and <end of output>):',
          sum(1 for b in blocks if 'Test Passed.' in b and re.search(r'Output:\n-+\n(.+?)<end of output>', b, re.S)))
