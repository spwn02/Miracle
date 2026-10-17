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
--apply  Create/reuse the GitHub Project and import real infrastructure issues #1-#6.

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

# Seed only genuine, already-opened infrastructure work; do not backfill
# historical issues or assign unreviewed priorities or target dates.
project_id="$(gh project view "$project_numbers" --owner "$OWNER" --format json --jq '.id')"
phase_field_id="$(gh project field-list "$project_numbers" --owner "$OWNER" --limit 100 \
  --format json --jq '.fields[] | select(.name == "Phase") | .id')"
area_field_id="$(gh project field-list "$project_numbers" --owner "$OWNER" --limit 100 \
  --format json --jq '.fields[] | select(.name == "Area") | .id')"
infra_option_id="$(gh project field-list "$project_numbers" --owner "$OWNER" --limit 100 \
  --format json --jq '.fields[] | select(.name == "Area") | .options[] | select(.name == "Infrastructure") | .id')"
docs_option_id="$(gh project field-list "$project_numbers" --owner "$OWNER" --limit 100 \
  --format json --jq '.fields[] | select(.name == "Area") | .options[] | select(.name == "Documentation") | .id')"

for item in "$project_id" "$phase_field_id" "$area_field_id" "$infra_option_id" "$docs_option_id"; do
  if [[ -z "$item" || "$item" == "null" ]]; then
    echo "Unable to resolve required Project or field IDs; inspect project #$project_numbers." >&2
    exit 1
  fi
done

for entry in "1 G1 Infrastructure" "2 G2 Infrastructure" \
             "3 G3 Documentation" "4 G4 Infrastructure" \
             "5 G5 Infrastructure" "6 G6 Infrastructure"; do
  read -r number phase area <<< "$entry"
  issue_url="https://github.com/$REPO/issues/$number"

  # Do not quietly assume the hard-coded issue list is still current.
  actual_url="$(gh issue view "$number" --repo "$REPO" --json url --jq '.url')"
  if [[ "$actual_url" != "$issue_url" ]]; then
    echo "Unexpected/missing issue #$number: $actual_url" >&2
    exit 1
  fi

  item_id="$(gh project item-list "$project_numbers" --owner "$OWNER" \
    --limit 1000 --format json \
    --jq ".items[] | select(.content.url == \"$issue_url\") | .id")"
  if [[ -z "$item_id" ]]; then
    item_id="$(gh project item-add "$project_numbers" --owner "$OWNER" \
      --url "$issue_url" --format json --jq '.id')"
    echo "Added issue #$number"
  else
    echo "Issue already in Project: #$number"
  fi
  if [[ -z "$item_id" || "$item_id" == "null" ]]; then
    echo "GitHub did not return an item ID for issue #$number" >&2
    exit 1
  fi

  gh project item-edit --id "$item_id" --project-id "$project_id" \
    --field-id "$phase_field_id" --text "$phase" >/dev/null

  if [[ "$area" == "Documentation" ]]; then
    option_id="$docs_option_id"
  else
    option_id="$infra_option_id"
  fi
  gh project item-edit --id "$item_id" --project-id "$project_id" \
    --field-id "$area_field_id" --single-select-option-id "$option_id" >/dev/null
  echo "Set #$number: Phase=$phase, Area=$area"
done

# Only the actual G2 provisioning task belongs in the finite G2 milestone.
gh issue edit 2 --repo "$REPO" --milestone "$MILESTONE_TITLE" >/dev/null

echo
echo "GitHub Project bootstrap finished."
gh project view "$project_numbers" --owner "$OWNER" --format json --jq '.url'

cat <<'NEXT'
Complete these steps in the GitHub Project UI:
  1. Edit the EXISTING Status field:
     Backlog, Design, Ready, Implementing, Review, Verification, Accepted, Blocked
  2. Create saved views: All Work (table), Active (board by Status),
     Roadmap (timeline), Verification Queue (table).
  3. Enable Auto-add for new issues in spwn02/Miracle.
  4. Verify actual issues #1-#6 have the expected Phase and Area values.
  5. Confirm visibility and status transitions, then record G2 acceptance evidence.
No invented deadlines, automatic acceptance, or personal tokens are required.
NEXT
