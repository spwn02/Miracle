#!/usr/bin/env python3
"""Commit/tag-evidenced native history projection. No phase acceptance or schedule edits.

Nine dedicated Project DraftIssue history markers (five area activity spans,
four lightweight version tags), two EXCLUSIVE historical Date fields. Never
updates old Project Start date / Target date or the 172 archived source records.
"""
from __future__ import annotations
from datetime import datetime
import hashlib
import json
import os
from pathlib import Path
import re
import runpy
import subprocess
import sys

PROJECT="PVT_kwHOCp2Gs84Bna-3"
HISTORY="miracle-git-history:v1:"
DATE1="Observed Git first event"
DATE2="Observed Git last event"
PHASE="Git History"
OUTPUT=Path("stage-b-native-history-evidence")
TAG_TARGETS={
    "v0.2.0":("Diagnostics","65759786418325047a3adffbdc1e9331fb55c490"),
    "v0.3.0":("Range","22ad01a4eeae19fa00713a23c778a5dddeb3f3cc"),
    "v0.4.0":("Iter","baa6b733b201a6eece3a3b39713d35c649b82a50"),
    "v0.5.0":("Meta","250b79d79698255ac81378cb0b16fc3b0e919fb4"),
}
AREAS=("Foundation","Diagnostics","Range","Iter","Meta")
COMMANDS={
 "--plan":"/miracle-stage-b history-plan-20261026",
 "--apply":"/miracle-stage-b history-apply-20261026",
 "--verify":"/miracle-stage-b history-verify-20261026",
}
ROOT="https://github.com/spwn02/Miracle"
Q='''query($cursor:String){
  user(login:"spwn02"){projectV2(number:3){
    id
    fields(first:100){nodes{... on ProjectV2Field{id name dataType}}}
    items(first:100,after:$cursor){pageInfo{hasNextPage endCursor}
      nodes{id isArchived content{
        ... on DraftIssue{id title body}
        ... on Issue{id number title}
      }
      fieldValues(first:50){nodes{
        ... on ProjectV2ItemFieldTextValue{field{... on ProjectV2Field{id name}} text}
        ... on ProjectV2ItemFieldDateValue{field{... on ProjectV2Field{id name}} date}
      }}
    }}}
  }
}'''
CREATE_FIELD='''mutation($p:ID!,$n:String!){
  createProjectV2Field(input:{projectId:$p,name:$n,dataType:DATE}){
    projectV2Field{... on ProjectV2Field{id name dataType}}}}'''
CREATE_ITEM='''mutation($p:ID!,$t:String!,$b:String!){
  addProjectV2DraftIssue(input:{projectId:$p,title:$t,body:$b}){
    projectItem{id content{... on DraftIssue{id title body}}}}}'''
SET_DATE='''mutation($p:ID!,$i:ID!,$f:ID!,$d:Date!){
  updateProjectV2ItemFieldValue(input:{projectId:$p,itemId:$i,fieldId:$f,value:{date:$d}}){
    projectV2Item{id}}}'''
SET_PHASE='''mutation($p:ID!,$i:ID!,$f:ID!,$s:String!){
  updateProjectV2ItemFieldValue(input:{projectId:$p,itemId:$i,fieldId:$f,value:{text:$s}}){
    projectV2Item{id}}}'''

class Stop(RuntimeError):
    pass

def need(ok,why):
    if not ok: raise Stop(why)

def gh(query, **variables):
    cmd=["gh","api","graphql","-f","query="+query]
    for key,val in variables.items():cmd+=["-f",key+"="+str(val)]
    result=subprocess.run(cmd,text=True,capture_output=True)
    need(result.returncode==0,"GitHub Project GraphQL operation failed")
    raw=json.loads(result.stdout)
    need(not raw.get("errors"),"GitHub GraphQL error; refusing assumed success")
    return raw.get("data") or {}

def inspect():
    pages=[];cursor=""
    project=None;fields=None
    while True:
        data=gh(Q,**({"cursor":cursor} if cursor else {})).get("user") or {}
        p=data.get("projectV2") or {}
        need(p.get("id")==PROJECT,"unexpected owner / Project ID")
        current=[f for f in (p.get("fields") or {}).get("nodes",[]) if f]
        if fields is None:fields=current
        else:need(fields==current,"Project field enumeration changed mid-read")
        page=p.get("items") or {}
        pages+=page.get("nodes") or []
        info=page.get("pageInfo") or {}
        if not info.get("hasNextPage"):break
        cursor=info.get("endCursor") or ""
        need(cursor and len(pages)<400,"incomplete Project pagination")
    need(len({r.get("id") for r in pages})==len(pages),"duplicate Project items")
    return pages,{f["name"]:f for f in fields}

def slug(name):return re.sub(r"[^a-z0-9]+","-",name.lower()).strip("-")

def make_rows():
    gen=runpy.run_path("scripts/github/stage-b-reconstruct-history.py")
    evidence=gen["build"]()
    need(evidence.get("canonical_archive_count")==172 and
         len(evidence.get("area_activity_spans",[]))==5 and
         evidence.get("git_commit_count",0)>=545,
         "Git history insufficient or incomplete")
    spans={x["area"]:x for x in evidence["area_activity_spans"]}
    refs={x["name"]:x for x in evidence["observed_git_tags"]}
    rows={}
    for area in AREAS:
        e=spans[area]
        shas=e["commit_shas"]
        need(e["evidence_count"]==len(shas)>0 and
             len(set(shas))==len(shas) and
             e["first_observed_commit_date"] and e["last_observed_commit_date"],
             "invalid activity source coverage: "+area)
        first=e["first_observed_commit_date"][:10]
        last=e["last_observed_commit_date"][:10]
        key=HISTORY+"area:"+slug(area)
        body=("\x3c!-- "+key+" -->\n\n# Recorded "+area+" implementation activity\n\n"+
          "**Observed evidence, not a historical phase start/completion or acceptance.** "+
          "Dates are the earliest and latest matching **committerDate** from source, tests and examples "+
          "in reachable protected master. Documentation-only commits are excluded.\n\n"+
          "**First observed event:** "+e["first_observed_commit_date"]+"\n\n"+
          "**Last observed event:** "+e["last_observed_commit_date"]+"\n\n"+
          "**Source/test commits:** "+str(len(shas))+"\n\n"+
          "**Git evidence:**\n\n"+
          "\n".join("- ["+v[:12]+"]("+ROOT+"/commit/"+v+")" for v in shas)+"\n\n"+
          "**Provenance:** automatically generated from tracked Git history by "+
          "scripts/github/stage-b-reconstruct-history.py. Not a canonical product contract.")
        rows[key]={"title":"[Git History/Activity] "+area+" — observed C++ source changes",
                   "body":body,"first":first,"last":last,"kind":"area","evidence":shas}
    for tag,(area,full_sha) in TAG_TARGETS.items():
        x=refs.get(tag)
        need(x is not None and x["ref_kind"]=="commit" and
             x["pointed_commit_sha"]==full_sha,
             "tag identity did not match approved commit: "+tag)
        dt=x["creator_date"]
        need(dt and re.match(r"20\d\d-\d\d-\d\d",dt),"tag commit date unavailable")
        day=dt[:10]
        key=HISTORY+"milestone:"+tag
        body=("\x3c!-- "+key+" -->\n\n# Observed version-tag commit: "+tag+"\n\n"+
              "**Milestone evidence, not automatic release publication or phase acceptance.**\n\n"+
              "**Area:** "+area+"\n\n"+
              "**Git tag:** ["+tag+"]("+ROOT+"/tree/"+tag+")\n\n"+
              "**Pinned referenced commit:** ["+full_sha+"]("+ROOT+"/commit/"+full_sha+")\n\n"+
              "**Observed committer date:** "+dt+"\n\n"+
              "**Tag object:** lightweight Git tag; the displayed tag creator date is the "+
              "referenced commit's date, **not** an independent tag-creation timestamp.\n\n"+
              "**Historical event interval:** same-day commit point; not a duration or scheduling deadline.")
        rows[key]={"title":"[Git History/Milestone] "+tag+" — "+area,
                   "body":body,"first":day,"last":day,"kind":"milestone","evidence":[full_sha]}
    need(len(rows)==9,"not nine bounded historical milestone/activity markers")
    return rows,evidence

def values(item):
    out={}
    for v in (item.get("fieldValues") or {}).get("nodes",[]):
        if v and v.get("field"):
            out[v["field"]["name"]]=v.get("date") if "date" in v else v.get("text")
    return out

def index(items,rows):
    previous=[]
    history={}
    for item in items:
        content=item.get("content") or {}
        body=content.get("body") or ""
        if body.startswith("<!-- "+HISTORY):
            marker=body.split(" -->",1)[0].removeprefix("<!-- ")
            need(marker in rows and marker not in history and body==rows[marker]["body"] and
                 content.get("title")==rows[marker]["title"] and not item["isArchived"],
                 "out-of-band history item or content changed: "+marker)
            history[marker]=item
        else:
            previous.append(item)
    need(len(previous)==38 and
         sum(x["content"].get("number") is not None for x in previous)==18 and
         sum(x["content"].get("number") is None for x in previous)==20,
         "original 20+18 Project active identities unexpectedly changed")
    return history

def journal(stage,rows,items,fields):
    from hashlib import sha256
    out={"stage":stage,"project":PROJECT,"expected_count":9,
         "observed_live_markers":sorted(items),
         "date_field_ids":{name:fields.get(name,{}).get("id") for name in (DATE1,DATE2)},
         "historical_rows":[{"key":key,"title":v["title"],"dates":[v["first"],v["last"]],
                             "body_sha256":sha256(v["body"].encode()).hexdigest(),
                             "item_id":items.get(key,{}).get("id"),
                             "evidence_shas":v["evidence"]} for key,v in sorted(rows.items())]}
    OUTPUT.mkdir(parents=True,exist_ok=True)
    (OUTPUT/"projection.json").write_text(json.dumps(out,indent=2)+"\n")

def main():
    if sys.argv[1:]==["--self-test"]:
        need(len(TAG_TARGETS)==4 and len(AREAS)==5 and len(COMMANDS)==3,"fixed manifest mutated")
        need(all(re.fullmatch("[a-f0-9]{40}",sha) for _,sha in TAG_TARGETS.values()),"not pinned tag commits")
        print("PASS: fixed 4 tags + 5 source activity areas, approved custom historical Date semantics")
        return
    need(len(sys.argv)==2 and sys.argv[1] in COMMANDS,"wrong bounded history command")
    action=sys.argv[1]
    need(os.getenv("GITHUB_ACTOR")=="spwn02" and
         os.getenv("GITHUB_EVENT_NAME")=="issue_comment" and
         os.getenv("STAGE_B_COMMAND")==COMMANDS[action] and os.getenv("GH_TOKEN"),
         "owner/project Issue #69 action context invalid")
    rows,evidence=make_rows()
    items,fields=inspect()
    history=index(items,rows)
    for x in (DATE1,DATE2):
        if x in fields:need(fields[x]["dataType"]=="DATE","unexpected historical date field type")
    need("Phase" in fields and fields["Phase"]["dataType"]=="TEXT","Project Phase field unavailable")
    journal("read-only-plan",rows,history,fields)
    if action=="--plan":
        print("PLAN: matched Git tags and actual source commits; native history",len(history),
              "of 9; date fields",sum(x in fields for x in (DATE1,DATE2)),
              "of 2; no Project write")
        return
    if action=="--apply":
        for name in (DATE1,DATE2):
            if name not in fields:
                x=(gh(CREATE_FIELD,p=PROJECT,n=name).get("createProjectV2Field") or {}).get("projectV2Field") or {}
                need(x.get("name")==name and x.get("dataType")=="DATE",
                     "date field creation result incorrect")
                items,fields=inspect()
                need(fields.get(name,{}).get("id")==x["id"],"date field readback mismatch")
                journal("date-field-created",rows,history,fields)
        for key,r in sorted(rows.items()):
            if key not in history:
                item=(gh(CREATE_ITEM,p=PROJECT,t=r["title"],b=r["body"])
                      .get("addProjectV2DraftIssue") or {}).get("projectItem") or {}
                need(item.get("id") and (item.get("content") or {}).get("body")==r["body"],
                     "native history draft creation mismatch: "+key)
                items,fields=inspect()
                history=index(items,rows)
                need(key in history and item["id"]==history[key]["id"],
                     "native history item readback mismatch: "+key)
                journal("history-card-created",rows,history,fields)
            item=history[key]
            for name,want in ((DATE1,r["first"]),(DATE2,r["last"])):
                now=values(item).get(name)
                if now!=want:
                    need(now is None,"unexpected pre-existing history date: "+key+" "+name)
                    result=(gh(SET_DATE,p=PROJECT,i=item["id"],f=fields[name]["id"],d=want)
                            .get("updateProjectV2ItemFieldValue") or {}).get("projectV2Item") or {}
                    need(result.get("id")==item["id"],"date write result mismatch")
                    items,fields=inspect();history=index(items,rows);item=history[key]
                    need(values(item).get(name)==want,"historical date readback mismatch")
            if values(item).get("Phase")!=PHASE:
                need(values(item).get("Phase") is None,"unexpected preexisting Phase value")
                result=(gh(SET_PHASE,p=PROJECT,i=item["id"],f=fields["Phase"]["id"],s=PHASE)
                        .get("updateProjectV2ItemFieldValue") or {}).get("projectV2Item") or {}
                need(result.get("id")==item["id"],"Phase write mismatch")
                items,fields=inspect();history=index(items,rows);item=history[key]
                need(values(item).get("Phase")==PHASE,"Phase readback mismatch")
            journal("history-projection-writing",rows,history,fields)
    items,fields=inspect();history=index(items,rows)
    need(len(history)==9 and len(items)==47,"nine source-backed marker cards not present")
    for key,r in rows.items():
        data=values(history[key])
        need(data.get(DATE1)==r["first"] and data.get(DATE2)==r["last"] and
             data.get("Phase")==PHASE,"historical marker/dates not verified: "+key)
    journal("verified-nine-git-history-items",rows,history,fields)
    print("PASS: nine native commit-backed historical Project cards, 18 date values, 9 Phase markers; existing 38 identities preserved; no planning/Status edits")
if __name__=="__main__":
    try:main()
    except (Stop,OSError,ValueError,KeyError,subprocess.CalledProcessError,TypeError) as e:
        print("STOP: "+str(e),file=sys.stderr);raise SystemExit(1)
