#!/usr/bin/env bash
# Miracle Development Project #3 — audited issue-status reconciliation.
# Native GitHub Projects only. Never touches draft/archive items or Issue state.
# Modes: --plan (read-only), --apply (guarded writes), --verify (read-only).
set -euo pipefail

MODE="${1:---plan}"
case "$MODE" in --plan|--apply|--verify) ;; *) echo 'Usage: miracle-project-reconcile-safe.sh [--plan|--apply|--verify]' >&2; exit 2;; esac
for cmd in gh jq; do command -v "$cmd" >/dev/null || { echo "Missing $cmd" >&2; exit 1; }; done

OWNER=spwn02
REPO=spwn02/Miracle
PROJECT_NUMBER=3
PROJECT_ID='PVT_kwHOCp2Gs84Bna-3'

ACTUAL_ID="$(gh project view "$PROJECT_NUMBER" --owner "$OWNER" --format json --jq .id)"
[[ "$ACTUAL_ID" == "$PROJECT_ID" ]] || { echo "Unexpected project identity: $ACTUAL_ID" >&2; exit 1; }
FIELDS="$(gh project field-list "$PROJECT_NUMBER" --owner "$OWNER" --limit 100 --format json)"
STATUS_ID="$(jq -r '[.fields[] | select(.name=="Status" and .type=="ProjectV2SingleSelectField") | .id] | if length==1 then .[0] else empty end' <<<"$FIELDS")"
[[ -n "$STATUS_ID" ]] || { echo 'Missing/ambiguous Status field.' >&2; exit 1; }
option_id() {
  jq -r --arg name "$1" '[.fields[] | select(.name=="Status" and .type=="ProjectV2SingleSelectField") | .options[] | select(.name==$name) | .id] | if length==1 then .[0] else empty end' <<<"$FIELDS"
}
for status in Backlog Accepted Verification Implementing; do
  [[ -n "$(option_id "$status")" ]] || { echo "Missing/ambiguous Status option $status" >&2; exit 1; }
done

read -r -d '' QUERY <<'GQL' || true
query($owner:String!, $repo:String!, $number:Int!) {
  repository(owner:$owner, name:$repo) {
    issue(number:$number) {
      number
      url
      state
      projectItems(first:100, includeArchived:true) {
        totalCount
        nodes {
          id
          isArchived
          project { id }
          content { ... on Issue { number url repository { nameWithOwner } } }
          fieldValues(first:100) {
            totalCount
            nodes {
              ... on ProjectV2ItemFieldSingleSelectValue {
                name
                field { ... on ProjectV2SingleSelectField { name } }
              }
            }
          }
        }
      }
    }
  }
}
GQL

# Verified from the individual repository Issues, not inferred from a draft archive.
# #8 = independently accepted/closed implementation subtask, not G2 or G6 acceptance.
TARGETS=(
  '1|CLOSED|Accepted'
  '3|CLOSED|Accepted'
  '5|CLOSED|Accepted'
  '8|CLOSED|Accepted'
  '2|OPEN|Verification'
  '4|OPEN|Verification'
  '29|OPEN|Implementing'
)

fetch_item() {
  local issue_number="$1" issue_state="$2" expected_url="https://github.com/$REPO/issues/$1"
  local response matching
  response="$(gh api graphql -f query="$QUERY" -f owner="$OWNER" -f repo=Miracle -F number="$issue_number")" || return 1
  jq -e --argjson n "$issue_number" --arg url "$expected_url" --arg s "$issue_state" \
    '((.errors // []) | length)==0 and .data.repository.issue.number==$n and .data.repository.issue.url==$url and
    .data.repository.issue.state==$s and .data.repository.issue.projectItems.totalCount <= 100' \
    <<<"$response" >/dev/null || { echo "GraphQL issue state, identity, or pagination changed for #$issue_number; refusing." >&2; return 1; }
  matching="$(jq -c --arg pid "$PROJECT_ID" '[.data.repository.issue.projectItems.nodes[] | select(.project.id==$pid)]' <<<"$response")"
  [[ "$(jq length <<<"$matching")" -eq 1 ]] || { echo "Issue #$issue_number is absent or ambiguous in Project #3; refusing." >&2; return 1; }
  jq -e --arg n "$issue_number" --arg url "$expected_url" --arg repo "$REPO" \
    '.[0].isArchived==false and .[0].content.number==($n|tonumber) and .[0].content.url==$url and
    .[0].content.repository.nameWithOwner==$repo and .[0].fieldValues.totalCount <= 100' \
    <<<"$matching" >/dev/null || { echo "Issue #$issue_number item is archived/incomplete/incorrect; refusing." >&2; return 1; }
  printf '%s\n' "$matching" | jq -c '.[0]'
}

status_of() {
  jq -r '[.fieldValues.nodes[] | select(.field.name=="Status") | .name] | if length == 1 then .[0] else if length==0 then "" else "<ambiguous>" end end' <<<"$1"
}

# Validate every target first (no partial apply because a *later* preflight failed).
# Deliberately refuse any status other than Backlog or the individually audited target.
declare -a CHANGES=()
for target in "${TARGETS[@]}"; do
  IFS='|' read -r n state desired <<<"$target"
  item="$(fetch_item "$n" "$state")" || exit 1
  item_id="$(jq -r .id <<<"$item")"
  current="$(status_of "$item")"
  [[ -n "$current" && "$current" != '<ambiguous>' ]] || { echo "Issue #$n: no unique Status. Refusing." >&2; exit 1; }
  if [[ "$current" == "$desired" ]]; then
    echo "SKIP #$n: already $desired"
  elif [[ "$current" == Backlog ]]; then
    echo "CHANGE #$n: Backlog -> $desired"
    CHANGES+=("$n|$state|$desired|$item_id")
  else
    echo "Issue #$n has unexpected status '$current'; refusing to overwrite owner work." >&2; exit 1
  fi
done

if [[ "$MODE" == --plan ]]; then
  echo "PLAN: ${#CHANGES[@]} guarded Project field changes; zero writes."
  exit 0
fi
if [[ "$MODE" == --verify ]]; then
  [[ ${#CHANGES[@]} -eq 0 ]] || { echo "VERIFY FAILED: ${#CHANGES[@]} targets have not reached audited statuses." >&2; exit 1; }
  echo 'PASS: all seven status values match audited Issue states. No archive/issue mutations.'
  exit 0
fi

# Apply one field mutation at a time, re-check each live item immediately before
# writing (optimistic guard), and verify after write. No automatic rollback.
for change in "${CHANGES[@]}"; do
  IFS='|' read -r n state desired item_id <<<"$change"
  item="$(fetch_item "$n" "$state")" || exit 1
  [[ "$(jq -r .id <<<"$item")" == "$item_id" ]] || { echo "Issue #$n membership changed; refusing." >&2; exit 1; }
  current="$(status_of "$item")"
  if [[ "$current" == "$desired" ]]; then echo "SKIP #$n: updated meanwhile"; continue; fi
  [[ "$current" == Backlog ]] || { echo "Issue #$n status changed to $current; stopping without overwrite." >&2; exit 1; }
  gh project item-edit --id "$item_id" --project-id "$PROJECT_ID" --field-id "$STATUS_ID" \
    --single-select-option-id "$(option_id "$desired")" >/dev/null
  success=0
  for attempt in 1 2 3 4 5; do
    if new_item="$(fetch_item "$n" "$state")" && [[ "$(status_of "$new_item")" == "$desired" ]]; then
      success=1; break
    fi
    [[ "$attempt" -lt 5 ]] && sleep 1
  done
  [[ "$success" -eq 1 ]] || { echo "Post-write verification did not confirm #$n=$desired. Stop, inspect Project, then rerun --plan." >&2; exit 1; }
  echo "PASS #$n: $desired"
done
echo 'APPLY COMPLETE. Run --verify for final read-only audit.'
echo 'No repository Issues, contracts, dates, workflows or archive draft items modified.'