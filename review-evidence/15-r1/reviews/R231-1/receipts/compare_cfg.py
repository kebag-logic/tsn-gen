#!/usr/bin/env python3
"""compare_cfg.py <scratch> <out.json>: compare generated compile commands,
link.txt and flags.make between base and candidate configure trees for every
build type / BUILD_SHARED_LIBS setting, with source/build roots neutralized."""
import sys, os, json, glob, shlex, re, difflib
S, OUT = sys.argv[1], sys.argv[2]
FLAG = {'asan': '-fsanitize=address', 'ubsan': '-fsanitize=undefined'}
def kind(tgt):
    if tgt.endswith('_asan') or tgt.endswith('_ASAN'): return 'asan'
    if tgt.endswith('_ubsan') or tgt.endswith('_UBSAN'): return 'ubsan'
    return 'plain'
def target_of(path):
    m = re.search(r'CMakeFiles/([^/]+)\.dir/', path)
    return m.group(1) if m else None
def norm(s, side, cfg):
    s = s.replace(f'{S}/build/cfg/{side}/{cfg}', '<BUILD>')
    return s.replace(f'{S}/src/{side}', '<SRC>')
def load(side, cfg):
    B = f'{S}/build/cfg/{side}/{cfg}'
    cc = {}
    for e in json.load(open(f'{B}/compile_commands.json')):
        key = norm(e['output'] if 'output' in e else e['file'], side, cfg)
        if not key.startswith('<BUILD>') and 'output' in e:
            key = norm(os.path.join(e['directory'], e['output']), side, cfg)
        cc[key] = shlex.split(norm(e['command'], side, cfg))
    files = {}
    for kindf in ('link.txt', 'flags.make'):
        for f in glob.glob(f'{B}/**/CMakeFiles/*.dir/{kindf}', recursive=True):
            files[norm(f, side, cfg)] = norm(open(f).read(), side, cfg)
    return cc, files
res = {}
for cfg in sorted(os.listdir(f'{S}/build/cfg/base')):
    bcc, bfiles = load('base', cfg); ccc, cfiles = load('cand', cfg)
    r = {'base_compile': len(bcc), 'cand_compile': len(ccc),
         'only_base_compile': sorted(set(bcc) - set(ccc)),
         'only_cand_compile_targets': sorted({target_of(k) for k in set(ccc) - set(bcc)}),
         'only_cand_compile_count': len(set(ccc) - set(bcc)),
         'identical_compile': 0, 'changed_compile': [], 'bad_changes': [],
         'flagged_base': {}, 'flagged_cand': {}, 'unexpected_flagged_cand': [],
         'links_base': sum(k.endswith('link.txt') for k in bfiles),
         'links_cand': sum(k.endswith('link.txt') for k in cfiles),
         'flags_base': sum(k.endswith('flags.make') for k in bfiles),
         'flags_cand': sum(k.endswith('flags.make') for k in cfiles),
         'only_base_files': sorted(set(bfiles) - set(cfiles)),
         'only_cand_files_targets': sorted({target_of(k) for k in set(cfiles) - set(bfiles)}),
         'identical_link': 0, 'changed_link': [], 'identical_flags': 0, 'changed_flags': [],
         'sanitize_in_link_cand': {}, 'sanitize_in_link_base': {}}
    for side, cc, key in (('base', bcc, 'flagged_base'), ('cand', ccc, 'flagged_cand')):
        for k, toks in cc.items():
            fl = [t for t in toks if t.startswith('-fsanitize')]
            if fl:
                t = target_of(k); r[key].setdefault(t, {'count': 0, 'flags': set()})
                r[key][t]['count'] += 1; r[key][t]['flags'].update(fl)
                if side == 'cand' and (kind(t) == 'plain' or fl != [FLAG[kind(t)]] * len(fl)):
                    r['unexpected_flagged_cand'].append((k, fl))
    for k in sorted(set(bcc) & set(ccc)):
        b, c = bcc[k], ccc[k]
        if b == c:
            r['identical_compile'] += 1; continue
        t = target_of(k); exp = FLAG.get(kind(t))
        sm = difflib.SequenceMatcher(a=b, b=c, autojunk=False)
        ops = [(op, b[i1:i2], c[j1:j2]) for op, i1, i2, j1, j2 in sm.get_opcodes() if op != 'equal']
        ok = (exp is not None and len(ops) == 1 and ops[0][0] == 'insert' and ops[0][2] == [exp]
              and exp not in b)
        r['changed_compile'].append({'key': k, 'target': t, 'ops': ops, 'exactly_one_expected_flag_added': ok})
        if not ok: r['bad_changes'].append(k)
    for k in sorted(set(bfiles) & set(cfiles)):
        which = 'link' if k.endswith('link.txt') else 'flags'
        if bfiles[k] == cfiles[k]: r[f'identical_{which}'] += 1
        else:
            r[f'changed_{which}'].append({'key': k, 'diff': list(difflib.unified_diff(bfiles[k].splitlines(), cfiles[k].splitlines(), lineterm='', n=0))})
    for side, files, key in (('base', bfiles, 'sanitize_in_link_base'), ('cand', cfiles, 'sanitize_in_link_cand')):
        for k, v in files.items():
            if k.endswith('link.txt'):
                fl = sorted(set(re.findall(r'-fsanitize=\w+', v)))
                if fl: r[key][target_of(k)] = fl
    for key in ('flagged_base', 'flagged_cand'):
        r[key] = {t: {'count': v['count'], 'flags': sorted(v['flags'])} for t, v in sorted(r[key].items())}
    r['flagged_base_total'] = sum(v['count'] for v in r['flagged_base'].values())
    r['flagged_cand_total'] = sum(v['count'] for v in r['flagged_cand'].values())
    res[cfg] = r
json.dump(res, open(OUT, 'w'), indent=1, default=list)
for cfg, r in res.items():
    ch = r['changed_compile']
    print(f"== {cfg}: compile base {r['base_compile']} cand {r['cand_compile']}; identical {r['identical_compile']}; "
          f"changed {len(ch)} (all exactly +1 matching -fsanitize: {all(c['exactly_one_expected_flag_added'] for c in ch)}); "
          f"only-base {len(r['only_base_compile'])}; only-cand {r['only_cand_compile_count']} {r['only_cand_compile_targets']}")
    print(f"   link.txt base {r['links_base']} cand {r['links_cand']} identical {r['identical_link']} changed {len(r['changed_link'])}; "
          f"flags.make base {r['flags_base']} cand {r['flags_cand']} identical {r['identical_flags']} changed {len(r['changed_flags'])}; "
          f"only-base files {len(r['only_base_files'])}; only-cand file targets {r['only_cand_files_targets']}")
    print(f"   flagged compile entries: base {r['flagged_base_total']} cand {r['flagged_cand_total']}; unexpected cand {len(r['unexpected_flagged_cand'])}")
    chg = {}
    for c in ch: chg[c['target']] = chg.get(c['target'], 0) + 1
    print(f"   changed by target: {chg}")
    print(f"   cand flagged by target: { {t: v['count'] for t, v in r['flagged_cand'].items()} }")
    print(f"   link.txt with -fsanitize base==cand: {r['sanitize_in_link_base'] == {k: v for k, v in r['sanitize_in_link_cand'].items() if k in r['sanitize_in_link_base']}}; cand-only: { {k: v for k, v in r['sanitize_in_link_cand'].items() if k not in r['sanitize_in_link_base']} }")
