#!/usr/bin/env python3
"""Owner-authorized 152-to-20 consolidation. Reversible archive, never delete."""
from __future__ import annotations
import hashlib, json, os, re, runpy, subprocess, sys
from pathlib import Path

S = runpy.run_path("scripts/github/stage-b-draft-sync.py")
gql = S["run_gh"]
sha = S["digest"]
Error = S["SyncError"]
PROJ = "PVT_kwHOCp2Gs84Bna-3"
REV = "cd2c02b2691f32f6a624c32bfdef2f162bdf1aad"
MAN = Path("docs/infrastructure/STAGE-B-PROPOSED-ARCHIVE-CONSOLIDATION-TARGETS.json")
BAK = Path("stage-b-draft-backup/draft-body-backup-and-plan.json")
OUT = Path("stage-b-consolidation-evidence")
COMMANDS = {"--plan": "/miracle-stage-b archive-plan-20261026",
            "--apply": "/miracle-stage-b archive-apply-20261026",
            "--restore": "/miracle-stage-b archive-restore-20261026"}
ITEM = "query($id:ID!){node(id:$id){... on ProjectV2Item{id isArchived content{... on DraftIssue{id title body}}}}}"
UPD = "mutation($id:ID!,$body:String!){updateProjectV2DraftIssue(input:{draftIssueId:$id,body:$body}){draftIssue{id title body}}}"
ARC = "mutation($p:ID!,$i:ID!){archiveProjectV2Item(input:{projectId:$p,itemId:$i}){item{id isArchived}}}"
UNARC = "mutation($p:ID!,$i:ID!){unarchiveProjectV2Item(input:{projectId:$p,itemId:$i}){item{id isArchived}}}"

def require(ok, reason):
    if not ok:
        raise Error(reason)

def read():
    m=json.loads(MAN.read_text())
    b=json.loads(BAK.read_text())
    require(m.get("repository")=="spwn02/Miracle" and m.get("original_record_count")==172 and
            m.get("proposed_retain")==20 and m.get("proposed_consolidate")==152 and
            m.get("exact_destructive_authorization") is False, "untrusted approved proposal")
    require(b.get("project_id")==PROJ and b.get("revision")==REV and b.get("count")==172, "invalid original body backup")
    rows={x["stable_id"]:x for x in b["records"]}
    plan={x["stable_id"]:x for x in m["records"]}
    require(len(rows)==len(plan)==172 and set(rows)==set(plan), "catalog identity drift")
    require(len({x["item_id"] for x in rows.values()})==172 and
            len({x["draft_issue_id"] for x in rows.values()})==172, "duplicate native IDs")
    kept={k for k,v in plan.items() if v["disposition"]=="retain"}
    removed={k for k,v in plan.items() if v["disposition"]=="propose_consolidate_after_owner_approval"}
    require(len(kept)==20 and len(removed)==152 and not(kept & removed), "wrong archive cardinality")
    counts={}
    for k,p in plan.items():
        row=rows[k]
        target=p["retained_stable_id"]
        require(target in kept and p["project_item_node_id"]==row["item_id"] and
                p["retained_project_item_node_id"]==rows[target]["item_id"] and
                p["source"]["path"]==row["source_path"] and
                p["source"]["heading_start_line"]==row["source_lines"][0] and
                sha(row["after"])==row["after_sha256"], "mapping/backup mismatch: "+k)
        owner=rows[target]
        bound=(1,94) if k.startswith("architecture:") else owner["source_lines"]
        require(row["source_path"]==owner["source_path"] and
                bound[0]<=row["source_lines"][0]<=row["source_lines"][1]<=bound[1],
                "missing source span in target: "+k)
        group=k.split(":")[0]
        counts.setdefault(group,[0,0])[0 if k in kept else 1]+=1
    require(counts=={"foundation":[8,89],"phase-5-4":[1,28],"phase-5-6":[9,12],
                     "development-protocol":[1,15],"architecture":[1,8]}, "source coverage group drift")
    return rows,plan,kept,removed

def native(row):
    n=gql(ITEM,id=row["item_id"]).get("node") or {}
    c=n.get("content") or {}
    require(n.get("id")==row["item_id"] and isinstance(n.get("isArchived"),bool) and
            c.get("id")==row["draft_issue_id"] and c.get("title")==row["title"] and
            isinstance(c.get("body"),str), "node identity/schema error: "+row["stable_id"])
    return n

def aliases(rows,plan,kept):
    wanted={}
    for k in sorted(kept):
        body=rows[k]["after"]
        if k=="architecture:chapter:0003":
            path=Path("docs/architecture.md")
            blob=subprocess.check_output(["git","hash-object",str(path)],text=True).strip()
            require(blob==rows[k]["source_blob"], "architecture source changed")
            txt=path.read_text(encoding="utf-8")
            require(len(txt.splitlines())==94, "architecture source line count drift")
            body=S["desired_body"]({"id":k,"kind":"Chapter","group":"Architecture"},
                  {"body":txt,"path":"docs/architecture.md","line_start":1,"line_end":94,
                   "file_git_blob_sha1":blob,"source_sha256":sha(txt)},REV)
        linked=sorted(x for x,v in plan.items() if v["retained_stable_id"]==k and x!=k)
        if linked:
            body+="\n\n---\n\n### Consolidated historical identities\n\n"
            body+="Historical record identifiers (source-provenance aliases; underlying native cards archived reversibly, not deleted):\n\n"
            for child in linked:
                r=rows[child]; a,b=r["source_lines"]
                body+="- "+child+" | "+r["item_id"]+" | "+r["source_path"]+":"+str(a)+"-"+str(b)+" | sha256:"+r["source_sha256"]+"\n"
        require(len(body.encode())<=60000 and
                body.startswith("<!-- miracle-project-catalog:"+k+" -->"),
                "retained body invalid/oversize: "+k)
        wanted[k]=body
    return wanted

def record(stage,rows,plan,wanted,changed,archived):
    OUT.mkdir(parents=True,exist_ok=True)
    (OUT/"result.json").write_text(json.dumps({
        "stage":stage,"project":PROJ,"original_revision":REV,
        "approved_retained":20,"approved_archived":152,
        "changed_retained":changed,"changed_archive":archived,
        "desired_retained_sha256":{k:sha(s) for k,s in wanted.items()},
        "original_backup_artifact":12608038204,
        "mapping":[{"stable_id":k,"project_item_id":rows[k]["item_id"],
                    "draft_issue_id":rows[k]["draft_issue_id"],"keep":k in wanted,
                    "target":plan[k]["retained_stable_id"]}
                   for k in sorted(rows)]},indent=2)+"\n")
    print("STATE",stage,"updated",len(changed),"archived/unarchived",len(archived),flush=True)

def write_body(row,old,new):
    n=native(row)
    require(not n["isArchived"] and sha(n["content"]["body"])==sha(old),
            "stale body before write: "+row["stable_id"])
    result=(gql(UPD,id=row["draft_issue_id"],body=new).get("updateProjectV2DraftIssue") or {}).get("draftIssue") or {}
    require(result.get("id")==row["draft_issue_id"] and sha(result.get("body") or "")==sha(new),
            "body update response mismatch: "+row["stable_id"])
    require(sha(native(row)["content"]["body"])==sha(new),
            "body update readback mismatch: "+row["stable_id"])

def set_archive(row,archive):
    n=native(row)
    require(n["isArchived"] is (not archive) and sha(n["content"]["body"])==row["after_sha256"],
            "stale body/archive state: "+row["stable_id"])
    operation=ARC if archive else UNARC
    field="archiveProjectV2Item" if archive else "unarchiveProjectV2Item"
    result=(gql(operation,p=PROJ,i=row["item_id"]).get(field) or {}).get("item") or {}
    require(result.get("id")==row["item_id"] and result.get("isArchived") is archive,
            "archive mutation result mismatch: "+row["stable_id"])
    n=native(row)
    require(n["isArchived"] is archive and sha(n["content"]["body"])==row["after_sha256"],
            "archive readback mismatch: "+row["stable_id"])

def preflight(rows):
    current,meta=S["preflight"](pinned_revision=REV)
    require(len(current)==172 and meta["revision"]==REV, "full Project source snapshot incomplete")
    current={r["stable_id"]:r for r in current}
    require(set(current)==set(rows), "native ID coverage changed")
    for k,r in rows.items():
        n=current[k]
        require(n["item_id"]==r["item_id"] and n["draft_issue_id"]==r["draft_issue_id"] and
                n["title"]==r["title"] and n["source_blob"]==r["source_blob"] and
                n["source_sha256"]==r["source_sha256"] and
                n["before_sha256"]==r["after_sha256"],"native source/body drift: "+k)
    require(native(rows["architecture:chapter:0003"])["isArchived"] is False,
            "GraphQL archive schema or architecture state unexpected")

def execute(mode):
    rows,plan,keep,remove=read()
    wanted=aliases(rows,plan,keep)
    updated=[];archived=[]
    record("preflight-started",rows,plan,wanted,updated,archived)
    if mode in ("--plan","--apply"):
        preflight(rows)
        record("full-preflight-passed",rows,plan,wanted,updated,archived)
        if mode=="--plan":
            print("PASS: exact 172-body backup, 152-to-20 mapping and GraphQL archival schema; no Project write")
            return
        for k in sorted(wanted):
            if wanted[k]!=rows[k]["after"]:
                write_body(rows[k],rows[k]["after"],wanted[k])
                updated.append(k)
                record("retained-updates-in-progress",rows,plan,wanted,updated,archived)
        record("all-20-retained-ready",rows,plan,wanted,updated,archived)
        for k in sorted(remove):
            set_archive(rows[k],True)
            archived.append(k)
            record("archiving-in-progress",rows,plan,wanted,updated,archived)
        require(len(archived)==152, "incomplete archive writes")
        for k,row in sorted(rows.items()):
            n=native(row)
            require(n["isArchived"] is (k in remove) and
                    sha(n["content"]["body"])==(row["after_sha256"] if k in remove else sha(wanted[k])),
                    "final 172-node audit discrepancy: "+k)
        record("verified-20-live-152-archived",rows,plan,wanted,updated,archived)
        print("PASS: exactly 20 source-complete live archives, 152 reversible archives, all 172 IDs preserved")
        return
    # Restore is idempotent and refuses out-of-band edits, including partial runs.
    for k,row in sorted(rows.items()):
        n=native(row)
        permitted={row["after_sha256"]}
        if k in keep: permitted.add(sha(wanted[k]))
        require(sha(n["content"]["body"]) in permitted and
                (not n["isArchived"] if k in keep else True),
                "out-of-band change blocks restoration: "+k)
    for k in sorted(remove):
        if native(rows[k])["isArchived"]:
            set_archive(rows[k],False)
            archived.append(k)
            record("restoring-archive",rows,plan,wanted,updated,archived)
    for k in sorted(keep):
        if sha(wanted[k])!=rows[k]["after_sha256"] and
            sha(native(rows[k])["content"]["body"])==sha(wanted[k]):
            write_body(rows[k],wanted[k],rows[k]["after"])
            updated.append(k)
            record("restoring-bodies",rows,plan,wanted,updated,archived)
    for k,row in sorted(rows.items()):
        n=native(row)
        require(not n["isArchived"] and sha(n["content"]["body"])==row["after_sha256"],
                "incomplete restoration: "+k)
    record("verified-restored-all-172",rows,plan,wanted,updated,archived)
    print("PASS: all 172 original active Project identities restored")

def main():
    if sys.argv[1:]==["--self-test"]:
        require(20+152==172 and len(COMMANDS)==3 and "deleteProject" not in ARC+UNARC,
                "fixed safe archive contract invalid")
        print("PASS: bounded approval, 152 -> 20 and no destructive mutation")
        return
    require(len(sys.argv)==2 and sys.argv[1] in COMMANDS, "invalid operation")
    mode=sys.argv[1]
    require(os.getenv("GITHUB_ACTOR")=="spwn02" and
            os.getenv("GITHUB_EVENT_NAME")=="issue_comment" and
            os.getenv("STAGE_B_COMMAND")==COMMANDS[mode] and os.getenv("GH_TOKEN"),
            "not a recognized owner-authenticated Stage B command")
    execute(mode)

if __name__=="__main__":
    try: main()
    except (Error,ValueError,KeyError,OSError,subprocess.CalledProcessError) as exc:
        print("STOP: "+str(exc),file=sys.stderr)
        sys.exit(1)
