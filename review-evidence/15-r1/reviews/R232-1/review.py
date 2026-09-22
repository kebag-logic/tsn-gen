#!/usr/bin/env python3
"""R232: serial builds, --parallel 8, inherited 12 GiB cgroup; all outputs external."""
import collections, hashlib, io, json, os, pathlib, re, shlex, shutil, subprocess, sys, tarfile, time
ROOT=pathlib.Path('$VALIDATION_STORAGE/reviews/r232-18-r1')
OUT=pathlib.Path(__file__).resolve().parent
WORK=OUT/'work'; REC=OUT/'receipts'; WORK.mkdir(exist_ok=True); REC.mkdir(exist_ok=True)
BASE='deca300c9eb4863fa13383b45ba6cb6fd0828671'; HEAD='76906b83eb54ac29040fe72fd910d34727188694'
ENV=os.environ.copy()
for key in ['ASAN_OPTIONS','UBSAN_OPTIONS']: ENV.pop(key,None)
ENV['CMAKE_BUILD_PARALLEL_LEVEL']='8'
TYPES=['empty','Debug','Release','MinSizeRel','RelWithDebInfo']
def save(path,data):
 path=pathlib.Path(path);path.parent.mkdir(parents=True,exist_ok=True)
 path.write_text(json.dumps(data,indent=2)+'\n')
def run(name,args,cwd=ROOT,env=None,expect=0):
 d=REC/'commands'/name;d.mkdir(parents=True,exist_ok=True)
 effective=ENV.copy();effective.update(env or {})
 start=time.time()
 with (d/'stdout').open('wb') as out,(d/'stderr').open('wb') as err:
  r=subprocess.run([str(a) for a in args],cwd=cwd,env=effective,stdout=out,stderr=err)
 save(d/'command.json',{'argv':[str(a) for a in args],'shell':'rtk proxy '+shlex.join([str(a) for a in args]),'cwd':str(cwd),'environment':{'unset':['ASAN_OPTIONS','UBSAN_OPTIONS'],'CMAKE_BUILD_PARALLEL_LEVEL':'8',**(env or {})},'exit':r.returncode,'seconds':round(time.time()-start,3)})
 print(name,'exit',r.returncode,flush=True)
 if expect is not None and r.returncode!=expect:
  print((d/'stdout').read_text(errors='replace')[-4000:]);print((d/'stderr').read_text(errors='replace')[-4000:]);raise AssertionError(name)
 return r.returncode,(d/'stdout').read_text(errors='replace'),(d/'stderr').read_text(errors='replace')
def manifest(path):
 return {str(x.relative_to(path)):{'sha256':hashlib.sha256(x.read_bytes()).hexdigest(),'mode':oct(x.stat().st_mode&0o777)} for x in sorted(path.rglob('*')) if x.is_file() and '.git' not in x.parts}
def prep():
 for label,commit in [('base',BASE),('head',HEAD)]:
  dest=WORK/label;dest.mkdir(exist_ok=True)
  data=subprocess.check_output(['git','archive',commit],cwd=ROOT)
  (REC/f'{label}-source.tar').write_bytes(data)
  with tarfile.open(fileobj=io.BytesIO(data)) as t:t.extractall(dest,filter='data')
  for dep in ['googletest','rapidyaml']:
   source=ROOT/'external'/dep;target=dest/'external'/dep
   shutil.copytree(source,target,dirs_exist_ok=True,ignore=shutil.ignore_patterns('.git'),symlinks=True)
   original=manifest(source);assert original==manifest(target)
   save(REC/f'{label}-{dep}-source-manifest.json',original)
  save(REC/f'{label}-export-manifest.json',manifest(dest))
 run('dependency-pins',['git','submodule','status','--recursive'])
 run('diff-check',['git','diff','--check',BASE,HEAD])
 save(REC/'resource-bound.json',{'memory_max':pathlib.Path('/sys/fs/cgroup/user.slice/user-1000.slice/user@1000.service/app.slice/milan-18-r1-r232.service/memory.max').read_text().strip(),'max_build_jobs':8})
def config(label,kind,shared='ON'):
 src=WORK/label;bld=WORK/'builds'/f'{label}-{shared}-{kind}'
 args=['cmake','-S',src,'-B',bld,'-G','Unix Makefiles','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON']
 if kind!='empty':args+=['-DCMAKE_BUILD_TYPE='+kind]
 if shared!='unset':args+=['-DBUILD_SHARED_LIBS='+shared]
 run('configure-'+bld.name,args)
 return bld
def normalized(s,src,bld):return s.replace(str(bld),'<BUILD>').replace(str(src),'<SOURCE>')
def entries(src,bld):
 out={}
 for e in json.loads((bld/'compile_commands.json').read_text()):
  key=normalized(e['output'],src,bld);assert key not in out
  out[key]={'file':normalized(e['file'],src,bld),'command':normalized(e['command'],src,bld)}
 return out
def matrix():
 results=[]
 for shared in ['ON','unset']:
  for kind in TYPES:
   b=config('base',kind,shared);h=config('head',kind,shared)
   be=entries(WORK/'base',b);he=entries(WORK/'head',h)
   assert len(be)==125 and len(he)==131
   changed=[];population=collections.Counter();rows=[]
   for key,e in be.items():
    ne=he[key];assert e['file']==ne['file']
    target=key.split('CMakeFiles/')[1].split('.dir/')[0]
    expected='address' if target.endswith(('_asan','_ASAN')) else 'undefined' if target.endswith(('_ubsan','_UBSAN')) else None
    flags=re.findall(r'-fsanitize=\S+',ne['command'])
    assert flags==(['-fsanitize='+expected] if expected else []),(kind,key,flags)
    population[target]+=1
    bt=shlex.split(e['command']);ht=shlex.split(ne['command'])
    if kind=='empty' and expected:
     assert '-fsanitize='+expected not in bt
     assert [x for x in ht if x!='-fsanitize='+expected]==bt,(kind,key)
     changed.append(key)
    else: assert ne==e,(kind,key,e,ne)
    rows.append({'object':key,'source':ne['file'],'target':target,'sanitizer':expected,'before':e['command'],'after':ne['command']})
   additions=sorted(set(he)-set(be));assert len(additions)==6 and all(x.startswith('tests/sanitizer/') for x in additions)
   comparisons={}
   for glob in ['link.txt','flags.make']:
    n=0
    for old in sorted(b.rglob(glob)):
     rel=old.relative_to(b);new=h/rel
     before=normalized(old.read_text(),WORK/'base',b);after=normalized(new.read_text(),WORK/'head',h)
     if glob=='flags.make' and kind=='empty':after=after.replace(' -fsanitize=address','').replace(' -fsanitize=undefined','')
     assert before==after,(kind,str(rel));n+=1
    comparisons[glob]=n
   result={'shared':shared,'type':kind,'base_compile_count':len(be),'head_compile_count':len(he),'changed_existing_compile_count':len(changed),'unchanged_existing_compile_count':len(be)-len(changed),'existing_targets':dict(population),'additions':additions,'generated_files_compared':comparisons}
   save(REC/'matrix'/f'{shared}-{kind}.json',{'summary':result,'entries':rows,'new_entries':{k:he[k] for k in additions}})
   results.append(result)
 save(REC/'matrix-summary.json',results)
def inspect_objects(b,label,only_probe=False):
 rows=[]
 for e in json.loads((b/'compile_commands.json').read_text()):
  if only_probe and '/tests/sanitizer/' not in e['file']:continue
  obj=b/e['output']
  assert obj.is_file(), obj
  symbols=subprocess.check_output(['nm','-u',str(obj)],text=True)
  rows.append({'object':str(obj.relative_to(b)),'compile_command':e['command'],'asan_references':sorted(set(re.findall(r'\b__asan_\w+',symbols))),'ubsan_references':sorted(set(re.findall(r'\b__ubsan_\w+',symbols))),'raw_nm':symbols})
 save(REC/'objects'/f'{label}.json',rows)
def probe_builds():
 for kind in TYPES:
  b=WORK/'builds'/f'head-ON-{kind}'
  run('probe-build-'+kind,['cmake','--build',b,'--parallel','8','--target','sanitizer_probe-test','sanitizer_probe-test_ASAN','sanitizer_probe-test_UBSAN'])
  run('probe-ctest-'+kind,['ctest','--test-dir',b,'-R','^sanitizer_probe-test','-V','-j','1'])
  run('probe-test-list-'+kind,['ctest','--test-dir',b,'--show-only=json-v1'])
  inspect_objects(b,'probe-'+kind,only_probe=True)
def mutations():
 headhelper=(WORK/'head/cmake/CMakeGenTestingLibraries.cmake').read_text()
 basehelper=(WORK/'base/cmake/CMakeGenTestingLibraries.cmake').read_text()
 variants={'reverted':basehelper,'private':headhelper.replace('target_compile_options('+chr(36)+'{BASENAME}_asan PUBLIC','target_compile_options('+chr(36)+'{BASENAME}_asan PRIVATE').replace('target_compile_options('+chr(36)+'{BASENAME}_ubsan PUBLIC','target_compile_options('+chr(36)+'{BASENAME}_ubsan PRIVATE'),'interface':headhelper.replace('target_compile_options('+chr(36)+'{BASENAME}_asan PUBLIC','target_compile_options('+chr(36)+'{BASENAME}_asan INTERFACE').replace('target_compile_options('+chr(36)+'{BASENAME}_ubsan PUBLIC','target_compile_options('+chr(36)+'{BASENAME}_ubsan INTERFACE')}
 for label,helper in variants.items():
  source=WORK/'mutations'/label;source.mkdir(parents=True,exist_ok=True)
  (source/'cmake').mkdir(exist_ok=True)
  (source/'cmake/CMakeGenTestingLibraries.cmake').write_text(helper)
  shutil.copy2(WORK/'head/cmake/CMakeLinkToTestlibs.cmake',source/'cmake')
  shutil.copytree(WORK/'head/tests/sanitizer',source/'tests/sanitizer',dirs_exist_ok=True)
  (source/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.21...3.31)\nproject(r232_probe LANGUAGES CXX)\nset(CMAKE_CXX_STANDARD 17)\nset(CMAKE_CXX_STANDARD_REQUIRED ON)\nset(CMAKE_POSITION_INDEPENDENT_CODE ON)\nset(COMP_COVERAGE_FLAGS "")\nset(COMP_ASAN_FLAGS "-fsanitize=address")\nset(COMP_UBSAN_FLAGS "-fsanitize=undefined")\nlist(APPEND CMAKE_MODULE_PATH "'+'$'+'{CMAKE_CURRENT_SOURCE_DIR}/cmake")\nenable_testing()\nadd_subdirectory(tests/sanitizer)\n')
  b=source/'build'
  run('mutation-configure-'+label,['cmake','-S',source,'-B',b,'-G','Unix Makefiles','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'])
  run('mutation-build-'+label,['cmake','--build',b,'--parallel','8'])
  run('mutation-test-'+label,['ctest','--test-dir',b,'-V','-j','1'],expect=8)
  inspect_objects(b,'mutation-'+label)
  actual=[x.split(':',1)[1] for x in (b/'Testing/Temporary/LastTestsFailed.log').read_text().splitlines()]
  expected=[f'sanitizer_probe-test_{san}.{side}-{fault}' for san,fault in [('ASAN','heap-overflow'),('UBSAN','signed-overflow')] for side in ['library','consumer'] if label=='reverted' or (label=='private' and side=='consumer') or (label=='interface' and side=='library')]
  assert sorted(actual)==sorted(expected),(label,actual,expected)
  save(REC/f'mutation-{label}-summary.json',{'expected_failing_tests':expected,'actual_failing_tests':actual})
 b=WORK/'mutations/reverted/build-Debug'
 run('mutation-configure-reverted-Debug',['cmake','-S',WORK/'mutations/reverted','-B',b,'-DCMAKE_BUILD_TYPE=Debug'])
 run('mutation-build-reverted-Debug',['cmake','--build',b,'--parallel','8'])
 run('mutation-test-reverted-Debug',['ctest','--test-dir',b,'-V','-j','1'])
if __name__=='__main__':
 {'prep':prep,'matrix':matrix,'probes':probe_builds,'mutations':mutations}[sys.argv[1]]()

