# Accessibility

Miracle aims to make its documentation, contribution process, and repository-facing interfaces usable by as many developers as practical, including developers using keyboard navigation, screen readers, and other assistive technology.

## Scope

Miracle is a C++26 library, not a graphical end-user application. This statement concerns maintained documentation, code examples, GitHub templates and contributor workflows. It does not imply that third-party GitHub interfaces, terminal emulators or development tools are controlled by Miracle.

## Authoring standards

When creating or updating project-maintained material, contributors should:

- Use semantic Markdown headings in a logical hierarchy, descriptive link text and clearly labeled form fields.
- Prefer native Markdown structure (lists, tables where truly tabular, code fences with language labels) over text-only visual alignment.
- Provide text alternatives for meaningful images and avoid relying exclusively on color, symbols, position or animations to explain information.
- Keep essential information available in text; do not require access to a screenshot, image, mouse gesture or video to follow instructions.
- Write command-line instructions that can be copied and executed with keyboard-only tooling, explaining platform/toolchain assumptions.
- Explain acronyms and specialized C++26/compiler terms where their meaning is not obvious from context.
- Avoid unnecessary mandatory form fields and allow honest responses such as `Unknown` when a reporter lacks environment details.

These are maintenance goals rather than a claim of independently audited WCAG conformance. No accessibility conformance certification or formal audit has been completed.

## Reporting accessibility barriers

If an accessibility issue affects a repository document, GitHub issue template, or contributing workflow, submit an [Infrastructure Request](https://github.com/spwn02/Miracle/issues/new?template=infrastructure_issue.yml) and select **Documentation / publishing** or **Repository configuration**. Describe the affected page or interaction, expected accessible behavior, and any assistive-technology or browser details you are comfortable providing.

Do not include personal medical details or other private information. If a public issue is not appropriate, consult the [Code of Conduct](CODE_OF_CONDUCT.md) for available contact routes before disclosing sensitive information.

We will assess reports according to practical impact and project resources; there is no guaranteed response or remediation timeline. GitHub-hosted UI defects may require reporting to GitHub itself.

## Future documentation site

Any future GitHub Pages documentation site should retain keyboard accessibility, visible focus indications, readable contrast, text resizing, semantic headings and meaningful alternate text. Specific accessibility criteria and validation belong to the future documentation publishing workstream (G3).
