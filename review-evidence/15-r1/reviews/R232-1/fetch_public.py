#!/usr/bin/env python3
"""Read-only public evidence retrieval. Requires gh read access; no GitHub writes."""
import concurrent.futures,hashlib,json,pathlib,subprocess
OUT=pathlib.Path(__file__).resolve().parent; PUB=OUT/'public'; PUB.mkdir(exist_ok=True)
REPO='repos/kebag-logic/tsn-gen/'
ARCHIVE='0be5bf0dc34bd7924b4227c1c2a2602849db4c52'
HEAD='76906b83eb54ac29040fe72fd910d34727188694'
def api(route,name,*options):
 data=subprocess.check_output(['gh','api',*options,REPO+route]);(PUB/name).write_bytes(data);return json.loads(data)
def main():
 api('issues/15','issue15.json')
 api('issues/comments/5772396945','frozen-decision.json')
 api('issues/15/comments','issue15-comments.json','--paginate')
 api('pulls/18','pr18.json')
 api('issues/18/comments','pr18-comments.json','--paginate')
 api('pulls/18/commits','pr-commits.json')
 tree=api('git/trees/'+ARCHIVE+'?recursive=1','archive-tree.json')
 entries=[x for x in tree['tree'] if x['type']=='blob' and x['path'].startswith('review-evidence/15-r1/')]
 def blob(x):
  data=subprocess.check_output(['gh','api','-H','Accept: application/vnd.github.raw+json',REPO+'git/blobs/'+x['sha']])
  assert hashlib.sha1(b'blob '+str(len(data)).encode()+b'\0'+data).hexdigest()==x['sha']
  dst=PUB/'archive'/x['path'];dst.parent.mkdir(parents=True,exist_ok=True);dst.write_bytes(data)
 with concurrent.futures.ThreadPoolExecutor(max_workers=4) as ex:list(ex.map(blob,entries))
 runs=api('actions/runs?head_sha='+HEAD+'&per_page=100','workflow-runs.json')
 for r in runs['workflow_runs']:
  jobs=api('actions/runs/'+str(r['id'])+'/jobs','jobs-'+str(r['id'])+'.json')
  for j in jobs['jobs']:
   data=subprocess.check_output(['gh','api','--allow-escape-sequences',REPO+'actions/jobs/'+str(j['id'])+'/logs'])
   (PUB/('job-'+str(j['id'])+'.log')).write_bytes(data)
 api('git/trees/9c9917e0148376a086dd8587e0a02e4080c79584?recursive=1','original-archive-tree.json')
 api('git/ref/heads/main','live-main.json')
 pr=api('pulls/18','live-pr18.json')
 api('git/commits/'+pr['merge_commit_sha'],'hosted-merge-commit.json')
if __name__=='__main__':main()

