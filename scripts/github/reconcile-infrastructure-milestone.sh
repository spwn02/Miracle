#!/usr/bin/env bash
# Reconcile a single GitHub Infrastructure (G0–G6) milestone.
# Uses the maintainer's existing gh credentials; no secrets or PATs stored.
set -euo pipefail

REPO="spwn02/Miracle"
TARGET="GitHub Infrastructure (G0–G6)"
LEGACY="GitHub Infrastructure — G2"
DESCRIPTION="GitHub infrastructure and governance G0–G6: audit, contribution workflow, project management, documentation publishing, security/CI, reproducible environments, and community acceptance. Completion requires verified evidence for every workstream. No deadline is implied."
ISSUES=(1 2 3 4 5 6 8) # #7 is later, unscheduled contract migration.

if [[ ${1:-} != "--check" && ${1:-} != "--apply" ]]; then
  echo "Usage: bash scripts/github/reconcile-infrastructure-milestone.sh --check|--apply" >&2
  exit 2
fi
mode="$1"
for binary in gh jq; do
  command -v "$binary" >/dev/null || { echo "Missing $binary" >&2; exit 1; }
done
gh auth status >/dev/null

# Reuse the existing REST milestone. Do not create one per workstream.
all="$(gh api "repos/$REPO/milestones?state=all&per_page=100")"
number="$(jq -r --arg title "$TARGET" '.[] | select(.title == $title) | .number' <<< "$all")"
legacy_number="$(jq -r --arg title "$LEGACY" '.[] | select(.title == $title) | .number' <<< "$all")"
if [[ "$number" == *$'\n'* || "$legacy_number" == *$'\n'* ]]; then
  echo "Ambiguous milestone records. Resolve manually." >&2
  exit 1
fi
if [[ -n "$number" && -n "$legacy_number" ]]; then
  echo "Both target and legacy milestones exist. Resolve to avoid duplicates." >&2
  exit 1
fi

if [[ "$mode" == "--check" ]]; then
  echo "Target milestone: $TARGET"
  echo "Target number: ${number:-absent}; legacy number: ${legacy_number:-absent}"
  for issue in "${ISSUES[@]}"; do
    gh issue view "$issue" --repo "$REPO" --json number,milestone \
      --jq '{number: .number, milestone: .milestone.title}'
  done
  echo "Excluded: #7 (deferred contracts, unscheduled)."
  exit 0
fi

if [[ -z "$number" && -n "$legacy_number" ]]; then
  number="$legacy_number"
  gh api --method PATCH "repos/$REPO/milestones/$number" \
    -f title="$TARGET" -f description="$DESCRIPTION" >/dev/null
  echo "Renamed existing milestone #$number: $LEGACY -> $TARGET"
elif [[ -z "$number" ]]; then
  number="$(gh api --method POST "repos/$REPO/milestones" \
    -f title="$TARGET" -f description="$DESCRIPTION" --jq '.number')"
  echo "Created milestone #$number: $TARGET"
else
  gh api --method PATCH "repos/$REPO/milestones/$number" \
    -f description="$DESCRIPTION" >/dev/null
  echo "Reusing milestone #$number: $TARGET"
fi

for issue in "${ISSUES[@]}"; do
  actual="$(gh issue view "$issue" --repo "$REPO" --json milestone --jq '.milestone.title // ""')"
  if [[ "$actual" != "$TARGET" ]]; then
    gh issue edit "$issue" --repo "$REPO" --milestone "$TARGET" >/dev/null
    echo "Assigned issue #$issue"
  else
    echo "Already assigned issue #$issue"
  fi
done

contract_milestone="$(gh issue view 7 --repo "$REPO" --json milestone --jq '.milestone.title // ""')"
if [[ "$contract_milestone" == "$TARGET" ]]; then
  echo "WARNING: Deferred issue #7 was already added to this milestone." >&2
fi

echo "Infrastructure milestone: https://github.com/$REPO/milestone/$number"
