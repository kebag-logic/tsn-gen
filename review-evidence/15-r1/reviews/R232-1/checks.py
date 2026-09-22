#!/usr/bin/env python3
from review import *
def registrations():
 lists={}
 for label in ['base','head']:
  b=WORK/'builds'/f'{label}-ON-empty'
  run('registration-build-'+label,['cmake','--build',b,'--parallel','8'])
  _,out,_=run('registration-list-'+label,['ctest','--test-dir',b,'--show-only=json-v1'])
  data=json.loads(out);tests=data['tests']
  # Compare the actual name, command and properties; backtrace indexes are metadata.
  cleaned=[{k:v for k,v in t.items() if k!='backtrace'} for t in tests]
  lists[label]=json.loads(normalized(json.dumps(cleaned),WORK/label,b))
  inspect_objects(b,'full-'+label+'-empty')
  runtime=[]
  for e in sorted(b.rglob('*')):
   if e.is_file() and (e.name.endswith(('_ASAN','_UBSAN')) or re.match(r'lib.*_(asan|ubsan)[.]so$',e.name)):
    p=subprocess.run(['readelf','-d',str(e)],text=True,capture_output=True)
    assert p.returncode==0
    runtime.append({'file':str(e.relative_to(b)),'needed':re.findall(r'\(NEEDED\).*?\[(.*?)\]',p.stdout),'raw_readelf':p.stdout})
  save(REC/'runtime'/f'{label}-empty.json',runtime)
 assert len(lists['base'])==285 and len(lists['head'])==292
 assert lists['head'][:285]==lists['base']
 new=lists['head'][285:]
 assert sum(any(p.get('name')=='ENVIRONMENT' for p in t['properties']) for t in new)==2
 save(REC/'registration-compare.json',{'base_count':285,'head_count':292,'all_original_names_commands_properties_identical_in_order':True,'added':new})
def exits():
 b=WORK/'builds/head-ON-empty'; exe=b/'tests/sanitizer/sanitizer_probe-test_UBSAN';checker=WORK/'head/tests/sanitizer/check_sanitizer_child.cmake'
 for side in ['library','consumer']:
  mode=side+'-signed-overflow'
  for setting,env,code in [('recover',{},0),('fatal',{'UBSAN_OPTIONS':'halt_on_error=1'},1)]:
   _,out,err=run(f'direct-ubsan-{side}-{setting}',[exe,mode],env=env,expect=code)
   assert 'runtime error: signed integer overflow' in err and f'{mode} started' in out
   assert (f'{mode} completed' in out)==(code==0)
  _,out,err=run('checker-rejects-recovery-'+side,['cmake','-DCHILD='+str(exe),'-DMODE='+mode,'-DEXPECT_EXIT=1','-DEXPECT_REPORT=runtime error: signed integer overflow','-P',checker],expect=1)
  assert 'child exit status: 0' in out and 'continued past' in err
 for mode in ['library-heap-overflow','consumer-heap-overflow']:
  _,out,err=run('direct-asan-'+mode,[b/'tests/sanitizer/sanitizer_probe-test_ASAN',mode],expect=1)
  assert 'ERROR: AddressSanitizer: heap-buffer-overflow' in err and f'{mode} started' in out and f'{mode} completed' not in out
 run('direct-runner-preserves-ctest-failure',['bash',ROOT/'tests/run-tests.sh',WORK/'mutations/reverted/build'],expect=8)
 source=WORK/'mutations/no-runtime'
 shutil.copytree(WORK/'mutations/reverted',source,ignore=shutil.ignore_patterns('build','build-Debug'),dirs_exist_ok=True)
 shutil.copy2(WORK/'head/cmake/CMakeGenTestingLibraries.cmake',source/'cmake')
 link=source/'cmake/CMakeLinkToTestlibs.cmake'
 text=link.read_text().replace('\t\t'+'$'+'{COMP_ASAN_FLAGS}\n','').replace('\t\t'+'$'+'{COMP_UBSAN_FLAGS}\n','')
 assert '-fsanitize' not in text and 'COMP_ASAN_FLAGS' not in text and 'COMP_UBSAN_FLAGS' not in text
 link.write_text(text);build=source/'build'
 run('no-runtime-configure',['cmake','-S',source,'-B',build,'-DCMAKE_EXPORT_COMPILE_COMMANDS=ON'])
 for san in ['ASAN','UBSAN']:
  _,out,err=run('no-runtime-link-'+san,['cmake','--build',build,'--parallel','8','--target','sanitizer_probe-test_'+san],expect=2)
  assert 'undefined reference' in err and '__'+san.lower()+'_' in err
def consumers():
 comparisons=[]
 for mode in ['tests-off','embedded']:
  for kind in ['empty','Debug']:
   pair={}
   for label in ['base','head']:
    src=WORK/label;build=WORK/'consumers'/f'{label}-{mode}-{kind}'
    args=['cmake','-B',build,'-G','Unix Makefiles','-DCMAKE_EXPORT_COMPILE_COMMANDS=ON','-DBUILD_SHARED_LIBS=ON']
    if kind!='empty':args+=['-DCMAKE_BUILD_TYPE='+kind]
    if mode=='tests-off':
     args+=['-S',src,'-DENABLE_PARSER_TESTS=OFF'];normal_src=src
    else:
     parent=WORK/'consumers'/f'{label}-source';parent.mkdir(parents=True,exist_ok=True)
     (parent/'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.21...3.31)\nproject(r232_consumer LANGUAGES CXX)\nset(CMAKE_CXX_STANDARD 17)\nadd_subdirectory("'+str(src)+'" tsn-gen)\nadd_executable(app main.cpp)\ntarget_link_libraries(app PRIVATE tsn::traffic_gen tsn::protocol_logic)\ntarget_compile_options(app PRIVATE -DR232_PARENT_OPTION)\ntarget_compile_definitions(app PRIVATE CORPUS="'+str(src/'protocols')+'")\n')
     (parent/'main.cpp').write_text('#include <tsn/session.h>\n#ifndef R232_PARENT_OPTION\n#error parent option lost\n#endif\nint main() { tsn::Session session(CORPUS); return session.parse() ? 0 : 1; }\n')
     args+=['-S',parent];normal_src=parent
    run('consumer-configure-'+build.name,args)
    cache=(build/'CMakeCache.txt').read_text();assert 'ENABLE_PARSER_TESTS:BOOL=OFF' in cache
    commands=json.loads((build/'compile_commands.json').read_text())
    assert all('-fsanitize' not in e['command'] for e in commands)
    assert not any('sanitizer' in e['file'] or 'googletest' in e['file'] for e in commands)
    assert not any('CMAKE_BUILD_TYPE:STRING='+t in cache for t in TYPES[1:]) if kind=='empty' else True
    for link in build.rglob('link.txt'):assert '-fsanitize' not in link.read_text()
    # Normalize only source/build paths, leaving options and order intact.
    def norm(s):
     return normalized(s,src,build).replace(str(normal_src),'<PARENT>')
    pair[label]={'compile':[{k:norm(v) for k,v in e.items()} for e in commands],
     'links':{str(f.relative_to(build)):norm(f.read_text()) for f in build.rglob('link.txt')},
     'flags':{str(f.relative_to(build)):norm(f.read_text()) for f in build.rglob('flags.make')}}
    run('consumer-build-'+build.name,['cmake','--build',build,'--parallel','8'])
    run('consumer-run-'+build.name,[build/'app'] if mode=='embedded' else [build/'traffic-gen/packet_gen','--help'])
   assert pair['base']==pair['head'],(mode,kind)
   comparisons.append({'mode':mode,'type':kind,'existing_compile_count':len(pair['head']['compile']),'links':len(pair['head']['links']),'flags':len(pair['head']['flags']),'base_head_commands_identical':True,'sanitizers_absent':True,'build_and_run':0})
 save(REC/'consumer-summary.json',comparisons)
if __name__=='__main__':
 {'registrations':registrations,'exits':exits,'consumers':consumers}[sys.argv[1]]()

