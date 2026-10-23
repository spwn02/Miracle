#!/usr/bin/env bash
# Read-only, bounded audit of native Miracle Development Project #3.
# No Project mutations. Requires GH_TOKEN with read:project (or project).
set -euo pipefail
for cmd in gh jq; do
  command -v "$cmd" >/dev/null || { echo "Missing $cmd" >&2; exit 1; }
done
readonly PROJECT_ID='PVT_kwHOCp2Gs84Bna-3'
readonly PROJECT_URL='https://github.com/users/spwn02/projects/3'

project="$(gh project view 3 --owner spwn02 --format json)"
jq -e --arg id "$PROJECT_ID" --arg url "$PROJECT_URL"   '.id == $id and .url == $url and .title == "Miracle Development"'   <<<"$project" >/dev/null || { echo "Project identity mismatch; refusing to report success." >&2; exit 1; }

fields="$(gh project field-list 3 --owner spwn02 --limit 100 --format json)"
for name in Status Kind Disposition Area; do
  jq -e --arg name "$name"     '[.fields[] | select(.name==$name and .type=="ProjectV2SingleSelectField")] | length==1'     <<<"$fields" >/dev/null || { echo "Project field missing or ambiguous: $name" >&2; exit 1; }
done
for spec in 'Kind|Feature' 'Disposition|Deferred' 'Area|Toolchain'; do
  IFS='|' read -r name option <<<"$spec"
  jq -e --arg name "$name" --arg option "$option"     '[.fields[] | select(.name==$name) | .options[] | select(.name==$option)] | length==1'     <<<"$fields" >/dev/null || { echo "Missing/ambiguous $name option: $option" >&2; exit 1; }
done

read -r -d '' query <<'GQL' || true
query {
  user(login:"spwn02") {
    projectV2(number:3) {
      id
      views(first:100) { nodes { name layout filter } }
      workflows(first:100) { nodes { name enabled } }
    }
  }
  repository(owner:"spwn02", name:"Miracle") {
    issue(number:31) {
      number
      url
      state
      projectItems(first:100, includeArchived:true) {
        totalCount
        nodes {
          id
          isArchived
          project { id }
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
result="$(gh api graphql -f query="$query")"
jq -e --arg id "$PROJECT_ID" '
  ((.errors // []) | length)==0
  and .data.user.projectV2.id==$id
  and (.data.user.projectV2.views.nodes | type)=="array"
  and (.data.user.projectV2.workflows.nodes | type)=="array"
  and .data.repository.issue.number==31
  and .data.repository.issue.url=="https://github.com/spwn02/Miracle/issues/31"
  and .data.repository.issue.state=="OPEN"
  and .data.repository.issue.projectItems.totalCount<=100
' <<<"$result" >/dev/null || { echo "Incomplete Project/Issue GraphQL audit response." >&2; exit 1; }

item="$(jq -c --arg id "$PROJECT_ID" '[.data.repository.issue.projectItems.nodes[] | select(.project.id==$id)]' <<<"$result")"
jq -e 'length==1 and .[0].isArchived==false and .[0].fieldValues.totalCount<=100'   <<<"$item" >/dev/null || { echo "Issue #31 not uniquely present and active in Project #3." >&2; exit 1; }
for spec in 'Kind|Feature' 'Disposition|Deferred' 'Area|Toolchain'; do
  IFS='|' read -r name option <<<"$spec"
  jq -e --arg name "$name" --arg option "$option"     '[.[0].fieldValues.nodes[] | select(.field.name==$name and .name==$option)] | length==1'     <<<"$item" >/dev/null || { echo "Issue #31 mismatch: expected $name=$option" >&2; exit 1; }
done

echo "PASS: Project #3 identity and required field definitions."
echo "PASS: Issue #31 belongs to Project #3 and Kind=Feature, Disposition=Deferred, Area=Toolchain."
echo "Native saved views (names/layout/filters; inspect UI grouping separately):"
jq -r '.data.user.projectV2.views.nodes[] |
  "  \(.name) [\(.layout)]: \(.filter // "(no filter)")"' <<<"$result"
echo "Enabled workflow names (configuration and G2 provenance still need separate review):"
jq -r '.data.user.projectV2.workflows.nodes[] |
  "  \(.name): enabled=\(.enabled)"' <<<"$result"
echo "No Project fields, Issues, workflows or contracts were modified."
