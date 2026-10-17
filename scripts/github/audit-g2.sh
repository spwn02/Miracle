#!/usr/bin/env bash
# Read-only audit for GitHub Project #3: spwn02/Miracle.
# Uses the maintainer's authenticated gh session; makes no changes.
set -euo pipefail

if ! command -v gh >/dev/null 2>&1; then
  echo "GitHub CLI (gh) is required: https://cli.github.com/" >&2
  exit 1
fi
gh auth status >/dev/null

query="$(cat <<'GRAPHQL'
query {
  user(login: "spwn02") {
    projectV2(number: 3) {
      title
      url
      public
      repositories(first: 20) {
        nodes { nameWithOwner }
      }
      fields(first: 40) {
        nodes {
          __typename
          ... on ProjectV2Field {
            name
            dataType
          }
          ... on ProjectV2SingleSelectField {
            name
            options { name description }
          }
        }
      }
      views(first: 20) {
        nodes {
          name
          number
          layout
          filter
          groupByFields(first: 10) {
            nodes {
              ... on ProjectV2Field { name }
              ... on ProjectV2SingleSelectField { name }
            }
          }
        }
      }
      workflows(first: 30) {
        nodes { name enabled }
      }
    }
  }
}
GRAPHQL
)"

echo "===== G2 PROJECT CONFIGURATION ====="
gh api graphql -f query="$query" --jq '
  .data.user.projectV2 as $p |
  if $p == null then error("Project #3 is not accessible")
  else {
    title: $p.title,
    url: $p.url,
    public: $p.public,
    linked_repositories: [.repositories.nodes[].nameWithOwner],
    fields: [.fields.nodes[] | select(.name != null) |
      {name, type: .__typename, dataType, options: [(.options // [])[] | .name]}],
    views: [.views.nodes[] |
      {name, number, layout, filter, groupBy: [(.groupByFields.nodes // [])[] | .name]}],
    workflows: [.workflows.nodes[] | {name, enabled}]
  } end'

echo "===== G2 ISSUE ITEMS ====="
gh project item-list 3 --owner spwn02 --limit 100 --format json --jq '
  [.items[] |
    select((.content.url // "") | startswith("https://github.com/spwn02/Miracle/issues/")) |
    {issue: .content.url, status: .status, phase: .phase, area: .area}]'

echo "===== ISSUE #7 AUTOMATION EVENTS ====="
gh api "repos/spwn02/Miracle/issues/7/events?per_page=50" --jq '
  [.[] | select(.event == "added_to_project_v2" or .event == "project_v2_item_status_changed") |
    {event, actor: .actor.login, created_at}]'

echo
echo "Read-only audit completed. Review auto-add filters/target repository in Project Workflows UI."
