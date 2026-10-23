#!/usr/bin/env bash
# Bounded, read-only audit of user-owned Miracle Development GitHub Project #3.
# Uses direct GraphQL, avoiding gh project owner auto-detection ("unknown owner type").
# The Projects token is provided by Actions as GH_TOKEN; do not print it.
set -euo pipefail
for cmd in gh jq; do
  command -v "$cmd" >/dev/null || { echo "Missing $cmd" >&2; exit 1; }
done
readonly PROJECT_ID='PVT_kwHOCp2Gs84Bna-3'
readonly PROJECT_URL='https://github.com/users/spwn02/projects/3'

# Fail with a useful category before attempting a more complicated Project query.
# The returned user login is public; neither the credential nor API headers are printed.
if ! login="$(gh api user --jq .login)"; then
  echo 'Project credential rejected by GitHub REST API. Check the repository Actions secret, expiration and token validity.' >&2
  exit 1
fi
if [[ "$login" != spwn02 ]]; then
  echo "Project credential belongs to unexpected account: $login; refusing." >&2
  exit 1
fi
echo 'PASS: GitHub credential is authenticated as the expected Project owner.'

read -r -d '' query <<'GQL' || true
query {
  user(login:"spwn02") {
    projectV2(number:3) {
      id
      title
      url
      fields(first:100) {
        totalCount
        nodes {
          ... on ProjectV2FieldCommon { id name dataType }
          ... on ProjectV2SingleSelectField { options { id name } }
        }
      }
      views(first:100) { totalCount nodes { name layout filter } }
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

if ! result="$(gh api graphql -f query="$query")"; then
  echo 'GitHub GraphQL request failed; verify the token project scope and user-owned Project visibility.' >&2
  exit 1
fi
if jq -e '((.errors // []) | length) > 0' <<<"$result" >/dev/null; then
  echo 'GitHub GraphQL returned errors (messages only):' >&2
  jq -r '.errors[] | "  " + .message' <<<"$result" >&2
  exit 1
fi
jq -e --arg id "$PROJECT_ID" --arg url "$PROJECT_URL" '
  .data.user.projectV2.id==$id
  and .data.user.projectV2.title=="Miracle Development"
  and .data.user.projectV2.url==$url
  and .data.user.projectV2.fields.totalCount<=100
  and (.data.user.projectV2.views.nodes | type)=="array"
  and .data.user.projectV2.views.totalCount<=100
  and (.data.user.projectV2.workflows.nodes | type)=="array"
  and .data.repository.issue.number==31
  and .data.repository.issue.url=="https://github.com/spwn02/Miracle/issues/31"
  and .data.repository.issue.state=="OPEN"
  and .data.repository.issue.projectItems.totalCount<=100
' <<<"$result" >/dev/null || {
  echo 'Incomplete Project/Issue GraphQL response or unexpected Project identity.' >&2
  exit 1
}
for name in Status Kind Disposition Area; do
  jq -e --arg name "$name" '
    [.data.user.projectV2.fields.nodes[] | select(.name==$name and .dataType=="SINGLE_SELECT")] | length==1
  ' <<<"$result" >/dev/null || { echo "Project field missing or ambiguous: $name" >&2; exit 1; }
done
for spec in 'Kind|Feature' 'Disposition|Deferred' 'Area|Toolchain'; do
  IFS='|' read -r name option <<<"$spec"
  jq -e --arg name "$name" --arg option "$option" '
    [.data.user.projectV2.fields.nodes[] | select(.name==$name) | .options[] | select(.name==$option)] | length==1
  ' <<<"$result" >/dev/null || { echo "Missing/ambiguous field option $name=$option" >&2; exit 1; }
done
item="$(jq -c --arg id "$PROJECT_ID" '[.data.repository.issue.projectItems.nodes[] | select(.project.id==$id)]' <<<"$result")"
jq -e 'length==1 and .[0].isArchived==false and .[0].fieldValues.totalCount<=100'   <<<"$item" >/dev/null || { echo 'Issue #31 has missing, duplicate or archived Project membership.' >&2; exit 1; }
for spec in 'Kind|Feature' 'Disposition|Deferred' 'Area|Toolchain'; do
  IFS='|' read -r name option <<<"$spec"
  jq -e --arg name "$name" --arg option "$option" '
    [.[0].fieldValues.nodes[] | select(.field.name==$name and .name==$option)] | length==1
  ' <<<"$item" >/dev/null || { echo "Issue #31 mismatch: $name should equal $option" >&2; exit 1; }
done

echo 'PASS: Project #3 identity, required fields and option definitions.'
echo 'PASS: Issue #31 active membership and Kind=Feature, Disposition=Deferred, Area=Toolchain.'
echo 'Native Project views (layout and exact saved filter):'
jq -r '.data.user.projectV2.views.nodes[] |
  "  \(.name) [\(.layout)]: \(.filter // "(no filter)")"' <<<"$result"

echo 'Native view structural checks (read-only, warning-only):'
view_warnings=0
for spec in \
  'Overview|TABLE_LAYOUT|is:issue' \
  'Development|BOARD_LAYOUT|is:issue is:open -disposition:Deferred' \
  'Backlog|TABLE_LAYOUT|is:issue is:open status:Backlog,Design,Ready -disposition:Deferred' \
  'Deferred|TABLE_LAYOUT|is:issue is:open disposition:Deferred' \
  'Bugs|TABLE_LAYOUT|is:issue kind:Bug' \
  'Blocked|TABLE_LAYOUT|is:issue is:open status:Blocked' \
  'Contract Reviews|TABLE_LAYOUT|is:issue kind:"Contract Revision"' \
  'Decisions|TABLE_LAYOUT|is:issue kind:Decision' \
  'Roadmap|ROADMAP_LAYOUT|' \
  'Archive|TABLE_LAYOUT|is:draft'; do
  IFS='|' read -r name layout filter <<<"$spec"
  if jq -e --arg name "$name" --arg layout "$layout" --arg filter "$filter" '
    [.data.user.projectV2.views.nodes[] | select(.name==$name and .layout==$layout and
     ($filter=="" or .filter==$filter))] | length == 1
  ' <<<"$result" >/dev/null; then
    echo "  PASS: $name native layout and filter contract"
  else
    echo "  WARN: $name missing/duplicated or differs from expected filter/layout"
    view_warnings=$((view_warnings+1))
  fi
done

# Optional query: GitHub's GraphQL backend can fail on Project view grouping
# resolution for a populated Project. Keep the primary identity/audit usable
# and explicitly label unavailable grouping as UNVERIFIED.
read -r -d '' group_query <<'GQL' || true
query {
  user(login:"spwn02") {
    projectV2(number:3) {
      id
      views(first:100) {
        nodes {
          name
          groupByFields(first:20) { nodes { ... on ProjectV2FieldCommon { name } } }
          verticalGroupByFields(first:20) { nodes { ... on ProjectV2FieldCommon { name } } }
          configuration {
            visibleFields(first:50) { nodes { ... on ProjectV2FieldCommon { name } } }
          }
        }
      }
    }
  }
}
GQL
group_result=''
if group_result="$(gh api graphql -f query="$group_query" 2>/dev/null)" &&
   jq -e --arg id "$PROJECT_ID" '
     ((.errors // [])|length)==0 and .data.user.projectV2.id==$id and
     (.data.user.projectV2.views.nodes|type)=="array"
   ' <<<"$group_result" >/dev/null; then
  echo 'Optional native view grouping/visible-field details:'
  jq -r '.data.user.projectV2.views.nodes[] |
    "  \(.name): group=" + ([.groupByFields.nodes[].name] | join(",")) +
    ", vertical=" + ([.verticalGroupByFields.nodes[].name] | join(",")) +
    ", visible=" + ([.configuration.visibleFields.nodes[].name] | join(","))' <<<"$group_result"
  if jq -e '[.data.user.projectV2.views.nodes[] | select(.name=="Development") |
      .groupByFields.nodes[].name] | index("Status") != null' <<<"$group_result" >/dev/null; then
    echo '  PASS: Development board groups by Status'
  else
    echo '  WARN: Development board Status grouping missing or not exposed'
    view_warnings=$((view_warnings+1))
  fi
else
  echo '  WARN: GitHub did not return optional native grouping details. Development board grouping is UNVERIFIED, not failed.'
  view_warnings=$((view_warnings+1))
fi

if jq -e '[.data.user.projectV2.views.nodes[] | select(.name=="Roadmap" and
    ((.filter // "") == ""))] | length==1' <<<"$result" >/dev/null; then
  echo '  WARN: Roadmap currently has no saved filter; archival and deferred records may appear.'
  view_warnings=$((view_warnings+1))
fi
if [[ "$view_warnings" -gt 0 ]]; then
  echo "WARN: $view_warnings native view presentation findings require follow-up; primary Project audit remains successful."
else
  echo 'PASS: structural native view contract and grouping checks.'
fi
echo 'Native workflow names and enabled status:'
jq -r '.data.user.projectV2.workflows.nodes[] |
  "  \(.name): enabled=\(.enabled)"' <<<"$result"
echo 'This was a read-only audit, not a final G2 acceptance decision.'
