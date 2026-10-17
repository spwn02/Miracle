#!/usr/bin/env bash
# Miracle infrastructure G2 bootstrap.
# Run with --apply after reading docs/infrastructure/G2-OPERATING-PLAN.md.
# Uses your locally authenticated gh session; never store tokens here.
set -euo pipefail

OWNER="spwn02"
REPO="spwn02/Miracle"
PROJECT_TITLE="Miracle Development"
MILESTONE_TITLE="GitHub Infrastructure — G2"

usage() {
  cat <<'HELP'
Usage: scripts/github/bootstrap-g2.sh --check | --apply

--check  Inspect existing project/labels/milestones without mutating them.
--apply  Create/reuse the GitHub Project; link the repo; add fields, labels and milestone.

Requirements: authenticated GitHub CLI with permission for spwn02/Miracle
and the GitHub Projects 'project' OAuth scope.

This script does NOT configure Project Status options, saved views, auto-add
workflows, issue type settings or deadline values. Finish those in the UI.
HELP
}

if [[ ${1:-} != "--check" && ${1:-} != "--apply" ]]; then
  usage
  exit 2
fi
mode="$1"

if ! command -v gh >/dev/null 2>&1; then
  echo "GitHub CLI (gh) is required: https://cli.github.com/" >&2
  exit 1
fi
gh auth status >/dev/null
echo "Authenticated to GitHub; target: $REPO"

# User-owned Projects are separate from repository-local Projects settings.
project_numbers="$(gh project list --owner "$OWNER" --limit 100 --format json \
  --jq '.projects[] | select(.title == "Miracle Development") | .number')"

if [[ -n "$project_numbers" && "$project_numbers" == *$'\n'* ]]; then
  echo "Multiple projects share the title '$PROJECT_TITLE'; resolve ambiguity manually." >&2
  exit 1
fi
if [[ -n "$project_numbers" ]]; then
  echo "Found existing Project #$project_numbers"
else
  echo "No existing Project titled '$PROJECT_TITLE' was found."
fi

if [[ "$mode" == "--check" ]]; then
  if [[ -n "$project_numbers" ]]; then
    gh project view "$project_numbers" --owner "$OWNER" --format json
    gh project field-list "$project_numbers" --owner "$OWNER" --format json
  fi
  echo "Existing matching repo labels:"
  gh label list --repo "$REPO" --limit 200 --json name --jq '.[].name' | grep -E '^(type:|triage:|help-wanted$|good-first-issue$)' || true
  echo "Existing matching milestones:"
  gh api "repos/$REPO/milestones?state=all&per_page=100" --jq '.[].title' | grep -F "$MILESTONE_TITLE" || true
  exit 0
fi

# No destructive overwrites or field deletions; existing projects are reused.
if [[ -z "$project_numbers" ]]; then
  project_numbers="$(gh project create --owner "$OWNER" --title "$PROJECT_TITLE" \
    --format json --jq '.number')"
  echo "Created Project #$project_numbers"
fi
gh project edit "$project_numbers" --owner "$OWNER" \
  --visibility PUBLIC \
  --description "Public development and verification tracker for the Miracle C++26 foundation library"

# Linking an already-linked repo can report an error. Verify the project in
# GitHub if this operation fails; do not silently treat a failure as success.
if ! gh project link "$project_numbers" --owner "$OWNER" --repo "Miracle"; then
  echo "Project/repository linking was not confirmed; check Project settings." >&2
fi

fields="$(gh project field-list "$project_numbers" --owner "$OWNER" --limit 100 \
  --format json --jq '.fields[].name')"

ensure_field() {
  local name="$1" data_type="$2" options="${3:-}"
  if grep -Fxq "$name" <<< "$fields"; then
    echo "Field exists: $name"
    return
  fi
  echo "Creating field: $name ($data_type)"
  if [[ "$data_type" == "SINGLE_SELECT" ]]; then
    gh project field-create "$project_numbers" --owner "$OWNER" \
      --name "$name" --data-type "$data_type" --single-select-options "$options" >/dev/null
  else
    gh project field-create "$project_numbers" --owner "$OWNER" \
      --name "$name" --data-type "$data_type" >/dev/null
  fi
  fields+=$'\n'"$name"
}

# Built-in 'Status' must be EDITED in the UI, never duplicated.
if ! grep -Fxq "Status" <<< "$fields"; then
  echo "WARNING: Built-in Status field missing; inspect Project settings." >&2
fi

ensure_field "Phase" TEXT
ensure_field "Area" SINGLE_SELECT "Core,Meta,Iter,Diagnostics,Range,Serde,Clap,Infrastructure,Toolchain,Documentation,Other"
ensure_field "Priority" SINGLE_SELECT "P0,P1,P2,P3"
ensure_field "Risk" SINGLE_SELECT "Low,Medium,High"
ensure_field "Target release" TEXT
ensure_field "Start date" DATE
ensure_field "Target date" DATE

existing_labels="$(gh label list --repo "$REPO" --limit 200 --json name --jq '.[].name')"
ensure_label() {
  local name="$1" color="$2" description="$3"
  if grep -Fxq "$name" <<< "$existing_labels"; then
    echo "Label exists: $name"
    return
  fi
  gh label create "$name" --repo "$REPO" --color "$color" --description "$description" >/dev/null
  existing_labels+=$'\n'"$name"
  echo "Created label: $name"
}

ensure_label "type:bug" "d73a4a" "Confirmed or proposed Miracle library bug"
ensure_label "type:toolchain" "8250df" "Compiler, libc++, module or GCC compatibility problem"
ensure_label "type:feature" "a2eeef" "Proposed Miracle capability or API"
ensure_label "type:architecture" "5319e7" "Architecture, invariant or contract change"
ensure_label "type:infrastructure" "1d76db" "CI, repository, builds or operational infrastructure"
ensure_label "type:documentation" "0075ca" "Documentation-content change or defect"
ensure_label "triage:needs-info" "fbca04" "Further diagnostic or design evidence requested"
ensure_label "help-wanted" "008672" "Maintainer invites outside contributions"
ensure_label "good-first-issue" "7057ff" "Small, well-scoped and documented starting issue"

milestones="$(gh api "repos/$REPO/milestones?state=all&per_page=100" --jq '.[].title')"
if grep -Fxq "$MILESTONE_TITLE" <<< "$milestones"; then
  echo "Milestone exists: $MILESTONE_TITLE"
else
  gh api --method POST "repos/$REPO/milestones" \
    -f title="$MILESTONE_TITLE" \
    -f description="Create and validate the public Miracle Development Project, its fields, views, issue routing and verification process. No deadline is implied." >/dev/null
  echo "Created milestone: $MILESTONE_TITLE"
fi

# Seed only genuine, already-opened infrastructure work; don't backfill
# imaginary historical tasks or assign unreviewed priorities/deadlines.
existing_urls="$(gh project item-list "$project_numbers" --owner "$OWNER" \
  --limit 100 --format json --jq '.items[].content.url')"
for number in 1 2 3 4 5 6; do
  url="https://github.com/$REPO/issues/$number"
  # The six URLs are explicit G1.5/G2-G6 tasks recorded in G2-PROGRESS.md.
  # A missing issue should stop bootstrap instead of silently inventing work.
  if ! gh issue view "$number" --repo "$REPO" --json url --jq '.url' >/dev/null; then
    echo "Required issue $number not found; review the G2 progress ledger." >&2
    exit 1
  fi
  if grep -Fxq "$url" <<< "$existing_urls"; then
    echo "Issue already in Project: #$number"
  else
    gh project item-add "$project_numbers" --owner "$OWNER" --url "$url" >/dev/null
    existing_urls+=
gh project view "$project_numbers" --owner "$OWNER" --format json --jq '.url'

cat <<'NEXT'
Complete these steps in the GitHub Project UI:
  1. Edit the EXISTING Status field:
     Backlog, Design, Ready, Implementing, Review, Verification, Accepted, Blocked
  2. Create saved views: All Work (table), Active (board by Status),
     Roadmap (timeline), Verification Queue (table).
  3. Set the single built-in Auto-add workflow to new issues from spwn02/Miracle.
  4. Verify that real outstanding issues #1-#6 appear with correct Phase/Area.
  5. Confirm Project visibility and status transitions, then record G2 evidence.
No fabricated due dates, automatic acceptance, or project PATs are required.
NEXT
\n'"$url"
    echo "Added issue #$number"
  fi
done

# Set only factual planning metadata. Status, Priority and Risk still require
# human triage; Accepted must never be assigned automatically.
for entry in "1 G1 Infrastructure" "2 G2 Infrastructure" \
             "3 G3 Documentation" "4 G4 Infrastructure" \
             "5 G5 Infrastructure" "6 G6 Infrastructure"; do
  read -r number phase area <<< "$entry"
  issue_url="https://github.com/$REPO/issues/$number"
  gh project item-edit "$project_numbers" --owner "$OWNER" \
    --url "$issue_url" --field "Phase" --text "$phase" >/dev/null
  gh project item-edit "$project_numbers" --owner "$OWNER" \
    --url "$issue_url" --field "Area" --value "$area" >/dev/null
done

# Only the actual G2 provisioning task belongs to the finite G2 milestone.
gh issue edit 2 --repo "$REPO" --milestone "$MILESTONE_TITLE" >/dev/null

echo
echo "GitHub Project bootstrap finished (subject to the link warning above)."
gh project view "$project_numbers" --owner "$OWNER" --format json --jq '.url'

cat <<'NEXT'
Complete these steps in the GitHub Project UI:
  1. Edit the EXISTING Status field:
     Backlog, Design, Ready, Implementing, Review, Verification, Accepted, Blocked
  2. Create saved views: All Work (table), Active (board by Status),
     Roadmap (timeline), Verification Queue (table).
  3. Set the single built-in Auto-add workflow to new issues from spwn02/Miracle.
  4. Add the outstanding G1.5 and G2/G3/G4/G5/G6 issues by their URLs.
  5. Confirm Project visibility and status transitions, then record G2 evidence.
No fabricated due dates, automatic acceptance, or project PATs are required.
NEXT
