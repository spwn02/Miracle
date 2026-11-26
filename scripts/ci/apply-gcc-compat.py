#!/usr/bin/env python3
"""Rewrites Meta's reflection-function wrappers for GCC. Idempotent; run on the gcc branch after syncing master.

GCC 16 rejects `constexpr` objects that hold the address of a standard immediate (consteval) function, so
`MetaFunction{std::meta::f}` and `Predicate{std::meta::f}` become forwarding lambdas. libstdc++ 16 also lacks
`std::meta::is_applicable_type`, `is_nothrow_applicable_type` and `apply_result`; their wrappers are dropped.
"""

import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[2]
MISSING = {
    "src/public/meta/TypeQueries.cppm": ("isApplicableType", "isNothrowApplicableType"),
    "src/public/meta/TypeTransforms.cppm": ("applyResult",),
}
META_FUNCTION = re.compile(r"(inline constexpr MetaFunction \w+)\{\s*std::meta::([a-z_0-9]+)\s*\};")
CAST_META_FUNCTION = re.compile(
    r"(inline constexpr MetaFunction \w+)\{\s*static_cast<[^;]*?\(\*\)\([^)]*\)>\(std::meta::([a-z_0-9]+)\)\s*\};"
)
PREDICATE = re.compile(r"(inline constexpr Predicate \w+)\{\s*std::meta::([a-z_0-9]+)\s*\};")


def drop(text: str, name: str) -> str:
    lines, out, index = text.split("\n"), [], 0
    while index < len(lines):
        if lines[index].startswith(f"inline constexpr MetaFunction {name}{{"):
            while out and out[-1].startswith("///"):
                out.pop()
            index += 1
            if index < len(lines) and lines[index] == "" and out and out[-1] == "":
                index += 1
            continue
        out.append(lines[index])
        index += 1
    return "\n".join(out)


def rewrite(text: str) -> str:
    text = CAST_META_FUNCTION.sub(r"\1{std::meta::\2};", text)
    text = META_FUNCTION.sub(
        r"\1{[](auto &&...arguments) consteval -> decltype(auto) { "
        r"return std::meta::\2(std::forward<decltype(arguments)>(arguments)...); }};",
        text,
    )
    return PREDICATE.sub(r"\1{[](Info info) consteval -> bool { return std::meta::\2(info); }};", text)


def self_test() -> None:
    sample = "inline constexpr Predicate isX{std::meta::is_x};\ninline constexpr MetaFunction f{std::meta::g};\n"
    result = rewrite(sample)
    assert "[](Info info) consteval -> bool { return std::meta::is_x(info); }" in result
    assert "std::forward<decltype(arguments)>(arguments)" in result
    assert rewrite(result) == result
    cast = "inline constexpr MetaFunction h{\n    static_cast<std::string_view (*)(std::meta::info)>(std::meta::identifier_of)};\n"
    assert "static_cast" not in rewrite(cast) and "std::meta::identifier_of(std::forward" in rewrite(cast)
    assert "isY" not in drop("/// doc\ninline constexpr MetaFunction isY{std::meta::y};\n\nint z;", "isY")


def main() -> int:
    self_test()
    if "--self-test" in sys.argv:
        print("apply-gcc-compat self-test passed")
        return 0
    for path in sorted((ROOT / "src/public/meta").glob("*.cppm")):
        original = path.read_text(encoding="utf-8")
        text = original
        for name in MISSING.get(path.relative_to(ROOT).as_posix(), ()):
            text = drop(text, name)
        text = rewrite(text)
        if text != original:
            path.write_text(text, encoding="utf-8")
            print(f"rewrote {path.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
