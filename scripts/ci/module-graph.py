#!/usr/bin/env python3
"""Module graph validator (ALG-MOD-001 / TEST-MOD-DAG, TEST-MOD-OWNER).

Parses every module unit under src/ (including *.cppm.in templates), normalises imports to module or
partition edges, and rejects: duplicate primaries, unresolved imports, missing partitions, legacy
`module Miracle;` implementation units, cycles, forbidden architectural edges, umbrella content other than
re-exports, and interface units missing from CMakeLists.txt. `import std;` is an external leaf.
Regex parsing is a cross-check, not a replacement for the compiler's scan output.
"""
import argparse
import collections
import pathlib
import re
import sys

DECL = re.compile(r"^\s*(export\s+)?module\s+([A-Za-z_][\w.]*)?(?::([A-Za-z_]\w*))?\s*;", re.M)
IMPORT = re.compile(r"^\s*(export\s+)?import\s+(?:(std(?:\.compat)?)|([A-Za-z_][\w.]*)(?::([A-Za-z_]\w*))?|:([A-Za-z_]\w*))\s*;", re.M)

# Architectural dependency matrix: module -> modules it must never import (directly).
FORBIDDEN = {
    "Miracle.Numeric": {"*"},
    "Miracle.Color": {"Miracle.Format", "Miracle.Diagnostics", "Miracle.StaticString", "Miracle.Meta", "Miracle.Iter", "Miracle"},
    "Miracle.Format": {"Miracle.Diagnostics", "Miracle.StaticString", "Miracle.Meta", "Miracle.Iter", "Miracle"},
    "Miracle.Diagnostics": {"Miracle.Meta", "Miracle"},
    "Miracle.StaticString": {"Miracle.Diagnostics", "Miracle"},
}


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def scan(root):
    units = []
    for path in sorted(root.glob("src/**/*")):
        if path.suffix not in {".cppm", ".cpp"} and not path.name.endswith(".cppm.in"):
            continue
        text = strip_comments(path.read_text())
        decl = DECL.search(text)
        if not decl or not decl.group(2):
            continue  # not a named-module unit (e.g. plain source)
        exported, module, part = bool(decl.group(1)), decl.group(2), decl.group(3)
        kind = ("partition" if part else "primary") if exported else "implementation"
        imports = []
        for match in IMPORT.finditer(text):
            reexport, std, other, other_part, local_part = match.groups()
            if std:
                continue
            if local_part:
                imports.append((module, local_part, bool(reexport)))
            else:
                imports.append((other, other_part, bool(reexport)))
        units.append({"path": path.relative_to(root).as_posix(), "module": module, "part": part, "kind": kind,
                      "imports": imports, "text": text})
    return units


def find_cycle(nodes, edges):
    state, stack = {}, []

    def visit(node):
        state[node] = 1
        stack.append(node)
        for nxt in sorted(edges.get(node, ())):
            if state.get(nxt) == 1:
                return stack[stack.index(nxt):] + [nxt]
            if nxt not in state:
                found = visit(nxt)
                if found:
                    return found
        stack.pop()
        state[node] = 2
        return None

    for node in sorted(nodes):
        if node not in state:
            found = visit(node)
            if found:
                return found
    return None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", default=str(pathlib.Path(__file__).resolve().parents[2]))
    parser.add_argument("--dot", action="store_true", help="print the module graph in DOT format")
    args = parser.parse_args()
    root = pathlib.Path(args.root)
    units = scan(root)
    errors = []

    primaries = collections.defaultdict(list)
    partitions = collections.defaultdict(list)
    for unit in units:
        if unit["kind"] == "primary":
            primaries[unit["module"]].append(unit["path"])
        elif unit["kind"] == "partition":
            partitions[unit["module"]].append(unit["part"])
    for module, paths in primaries.items():
        if len(paths) != 1:
            errors.append(f"duplicate primary interface for {module}: {paths}")

    module_edges = collections.defaultdict(set)
    part_edges = collections.defaultdict(set)
    for unit in units:
        owner = unit["module"]
        if unit["kind"] == "implementation":
            if owner not in primaries:
                errors.append(f"{unit['path']}: implementation unit of unknown module {owner}")
            if owner == "Miracle":
                errors.append(f"{unit['path']}: legacy `module Miracle;` implementation unit")
        for target, part, _ in unit["imports"]:
            if part:
                if part not in partitions.get(target, []):
                    errors.append(f"{unit['path']}: unresolved partition import {target}:{part}")
                if unit["part"] is not None or unit["kind"] != "primary":
                    part_edges[f"{owner}:{unit['part'] or ''}"].add(f"{target}:{part}")
                if target != owner:
                    errors.append(f"{unit['path']}: imports foreign partition {target}:{part}")
                continue
            if target not in primaries:
                errors.append(f"{unit['path']}: unresolved import {target}")
                continue
            if target != owner:
                module_edges[owner].add(target)

    for module, forbidden in FORBIDDEN.items():
        for target in module_edges.get(module, ()):
            if "*" in forbidden or target in forbidden:
                errors.append(f"forbidden edge {module} -> {target}")

    # TEST-MOD-OWNER-001: the umbrella has no partitions; only these modules may own interface partitions.
    allowed_partitioned = {"Miracle.Format", "Miracle.Feature", "Miracle.Iter", "Miracle.Meta"}
    for module in partitions:
        if module not in allowed_partitioned:
            errors.append(f"unexpected interface partitions in {module}: {sorted(partitions[module])}")

    # TEST-AUDIT-COVER-001: no legacy suffixes, and every C++ source under src/ is a classified module unit.
    for path in sorted(root.glob("**/*")):
        rel = path.relative_to(root).as_posix()
        if rel.startswith(("build/", ".git/", "site/", ".cache/")) or "/build/" in rel:
            continue
        if path.suffix in {".ixx", ".cxx"}:
            errors.append(f"{rel}: legacy source suffix")
    classified = {u["path"] for u in units}
    for path in sorted(root.glob("src/**/*")):
        rel = path.relative_to(root).as_posix()
        if (path.suffix in {".cppm", ".cpp"} or rel.endswith(".cppm.in")) and rel not in classified:
            errors.append(f"{rel}: C++ source is not a named-module unit")

    umbrella = [u for u in units if u["module"] == "Miracle" and u["kind"] == "primary"]
    for unit in umbrella:
        body = DECL.sub("", unit["text"])
        body = IMPORT.sub("", body).strip()
        if body:
            errors.append(f"{unit['path']}: umbrella contains more than re-exports: {body[:60]!r}")
        for target, part, reexport in unit["imports"]:
            if not reexport:
                errors.append(f"{unit['path']}: umbrella imports {target} without re-exporting it")

    cycle = find_cycle(set(primaries), module_edges)
    if cycle:
        errors.append("module cycle: " + " -> ".join(cycle))
    cycle = find_cycle({n for e in part_edges.values() for n in e} | set(part_edges), part_edges)
    if cycle:
        errors.append("partition cycle: " + " -> ".join(cycle))

    cmake = (root / "CMakeLists.txt").read_text()
    for unit in units:
        if unit["kind"] in {"primary", "partition"}:
            if unit["path"] not in cmake:
                errors.append(f"{unit['path']}: not listed in CMakeLists.txt")
        elif unit["path"] not in cmake:
            errors.append(f"{unit['path']}: implementation unit not listed in CMakeLists.txt")

    if args.dot:
        print("digraph miracle {")
        for src in sorted(module_edges):
            for dst in sorted(module_edges[src]):
                print(f'  "{src}" -> "{dst}";')
        print("}")
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    print(f"module graph OK: {len(primaries)} modules, {sum(len(p) for p in partitions.values())} partitions, "
          f"{sum(len(e) for e in module_edges.values())} module edges, {len(units)} units")
    return 0


if __name__ == "__main__":
    sys.exit(main())
