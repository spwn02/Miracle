#!/usr/bin/env node
"use strict";

// Browser-accessibility regression probe, not an actual screen-reader session.
// Tests rendered mdBook against WCAG axe rules and Chrome's accessibility tree.
const fs = require("node:fs");
const http = require("node:http");
const path = require("node:path");
const { chromium } = require("playwright-core");
const AxeBuilder = require("@axe-core/playwright").default;

const summary = fs.readFileSync(path.resolve("docs/SUMMARY.md"), "utf8");
const pages = [...new Set([...summary.matchAll(/\]\(([^)]+)\.md\)/g)].map(m => "/" + m[1] + ".html"))];
const samples = ["/index.html", "/infrastructure/G3-PROGRESS.html", "/meta.html", "/diagnostics.html"];
if (pages.length < 20) throw new Error("Expected all mdBook chapters from SUMMARY.md");
const tags = ["wcag2a", "wcag2aa", "wcag21a", "wcag21aa", "wcag22aa"];
const mime = {
  ".html": "text/html; charset=utf-8",
  ".css": "text/css; charset=utf-8",
  ".js": "text/javascript; charset=utf-8",
  ".json": "application/json",
  ".svg": "image/svg+xml",
  ".woff2": "font/woff2",
  ".woff": "font/woff",
  ".png": "image/png",
  ".jpg": "image/jpeg",
};
const root = path.resolve(process.argv[2] || "site");

function serve() {
  return http.createServer((req, res) => {
    try {
      const pathname = decodeURIComponent(new URL(req.url, "http://localhost").pathname);
      const full = path.resolve(root, "." + pathname);
      if (full !== root && !full.startsWith(root + path.sep)) {
        res.writeHead(403).end();
        return;
      }
      const filename = fs.statSync(full).isDirectory() ? path.join(full, "index.html") : full;
      res.writeHead(200, { "Content-Type": mime[path.extname(filename)] || "application/octet-stream" });
      fs.createReadStream(filename).on("error", () => res.destroy()).pipe(res);
    } catch {
      res.writeHead(404).end();
    }
  });
}

async function audit(page, url, theme = "rust") {
  await page.goto(url, { waitUntil: "networkidle", timeout: 45000 });
  // Use the actual mdBook menu: a CSS-class-only switch leaves syntax
  // highlighting and persisted theme assets in the previous color scheme.
  if (theme === "navy") {
    await page.locator("#mdbook-theme-toggle").click();
    await page.locator("#mdbook-theme-navy").click();
    if (!(await page.locator("html").getAttribute("class") || "").includes("navy")) {
      throw new Error("Theme menu failed to switch to Navy");
    }
  }
  // The toggle's controlled menu exists, even when hidden until activation:
  // axe marks that closed-popup relationship INCOMPLETE, so assert it directly.
  const control = await page.locator("#mdbook-theme-toggle").getAttribute("aria-controls");
  if (control !== "mdbook-theme-list" || await page.locator("#mdbook-theme-list").count() !== 1) {
    throw new Error("Theme popup aria-controls points to a missing element");
  }
  const title = await page.title();
  if (!title.includes("Miracle")) throw new Error("Expected Miracle page title: " + title);

  // This is an actual browser accessibility-tree query, not an HTML grep.
  const cdp = await page.context().newCDPSession(page);
  await cdp.send("Accessibility.enable");
  const result = await cdp.send("Accessibility.getFullAXTree");
  const nodes = result.nodes.filter(n => !n.ignored);
  const roles = nodes.map(n => n.role?.value).filter(Boolean);
  const names = nodes.map(n => n.name?.value).filter(Boolean);

  console.log("AX roles:", JSON.stringify(
    Object.fromEntries(["main", "navigation", "heading", "link", "button", "searchbox", "textbox"].map(
      role => [role, roles.filter(x => x === role).length]
    ))
  ));
  console.log("AX named examples:", JSON.stringify(names.slice(0, 12)));
  if (!roles.includes("heading") || !roles.includes("link")) {
    throw new Error("Generated accessibility tree lacks expected headings/links");
  }
  const results = await new AxeBuilder({ page }).withTags(tags).analyze();
  for (const issue of results.violations) {
    console.error(
      "A11Y VIOLATION:",
      issue.id,
      issue.impact,
      issue.description,
      issue.nodes.slice(0, 10).map(node => ({
        target: node.target.join(" "),
        html: node.html.slice(0, 800),
        message: node.failureSummary?.slice(0, 1200),
      }))
    );
  }
  const unresolved = results.incomplete.map(item => ({
    id: item.id, impact: item.impact,
    targets: item.nodes.slice(0, 4).map(n => ({
      selector: n.target.join(" "),
      reason: n.any.concat(n.all, n.none).map(x => x.message).filter(Boolean).slice(0, 2),
    })),
  }));
  console.log("AXE summary:", JSON.stringify({
    url, theme, violations: results.violations.length,
    passes: results.passes.length, incomplete: unresolved,
  }));
  // An incomplete axe result is not a PASS. Check the actual DOM invariant
  // for the two known mdBook overlay/hidden-popup uncertainties. Fail closed
  // on any newly surfaced kind of inconclusive result.
  for (const issue of results.incomplete) {
    if (issue.id === "aria-valid-attr-value") {
      if (issue.nodes.some(n => n.target.join(" ") !== "#mdbook-theme-toggle")) {
        throw new Error("Unexpected unresolved ARIA attribute target");
      }
      continue; // The referenced theme-menu ID was independently asserted above.
    }
    if (issue.id !== "color-contrast") {
      throw new Error("Unreviewed axe incomplete rule: " + issue.id);
    }
    for (const node of issue.nodes) {
      const selector = node.target.join(" ");
      const contrast = await page.evaluate((sel) => {
        const element = document.querySelector(sel);
        const sidebar = element?.closest(".sidebar");
        if (!element || !sidebar) return null;
        const fg = getComputedStyle(element).color;
        const bg = getComputedStyle(sidebar).backgroundColor;
        function rgb(text) {
          const matches = text.match(/[0-9]+(?:\.[0-9]+)?/g);
          if (!matches || matches.length < 3) return null;
          return matches.slice(0, 3).map(Number);
        }
        const x = rgb(fg), y = rgb(bg);
        if (!x || !y) return null;
        function luminance(values) {
          const v = values.map(c => {
            c /= 255;
            return c <= 0.04045 ? c / 12.92 : Math.pow((c + 0.055) / 1.055, 2.4);
          });
          return 0.2126 * v[0] + 0.7152 * v[1] + 0.0722 * v[2];
        }
        const a = luminance(x), b = luminance(y);
        return (Math.max(a,b) + 0.05) / (Math.min(a,b) + 0.05);
      }, selector);
      if (contrast === null || contrast < 4.5) {
        throw new Error("Unresolved partially obscured sidebar contrast: " + selector +
          " ratio=" + contrast + " theme=" + theme + " " + url);
      }
    }
  }
  if (results.incomplete.length) {
    console.log("PASS: supplementary DOM/palette checks for " +
      results.incomplete.length + " axe-incomplete sidebar/popup rule(s)");
  }
  return results.violations.length;
}

async function main() {
  if (!fs.existsSync(path.join(root, "index.html"))) {
    throw new Error("Run mdbook build before the accessibility audit");
  }
  const server = serve();
  await new Promise(resolve => server.listen(0, "127.0.0.1", resolve));
  const address = "http://127.0.0.1:" + server.address().port;
  const chrome = process.env.CHROME_BIN || "/usr/bin/google-chrome";
  const browser = await chromium.launch({
    executablePath: chrome, headless: true,
    args: ["--no-sandbox", "--disable-dev-shm-usage", "--force-renderer-accessibility"],
  });
  try {
    const context = await browser.newContext({ viewport: { width: 1300, height: 900 } });
    const page = await context.newPage();
    let violations = 0;
    // All canonical chapters at desktop size, not merely two examples.
    for (const entry of pages) violations += await audit(page, address + entry);
    // Validate *actual* keyboard activation of the semantic sidebar button.
    await page.goto(address + pages[0], { waitUntil: "networkidle" });
    const toggle = page.locator("#mdbook-sidebar-toggle");
    const initialState = await toggle.getAttribute("aria-expanded");
    if (!["true", "false"].includes(initialState) || await toggle.getAttribute("role") !== "button") {
      throw new Error("Sidebar lacks a usable button role/aria-expanded state");
    }
    await toggle.focus();
    await page.keyboard.press("Enter");
    await page.waitForTimeout(100);
    const nextState = await toggle.getAttribute("aria-expanded");
    if (nextState === initialState) {
      throw new Error("Enter did not change the sidebar's advertised expanded state");
    }
    await page.keyboard.press("Space");
    await page.waitForTimeout(100);
    if (await toggle.getAttribute("aria-expanded") !== initialState) {
      throw new Error("Space did not restore the sidebar state");
    }
    console.log("PASS: sidebar keyboard Enter/Space and aria-expanded state");
    await page.setViewportSize({ width: 390, height: 844 });
    for (const entry of pages) violations += await audit(page, address + entry);
    // Include the default dark theme on representative user-facing content.
    await page.setViewportSize({ width: 1300, height: 900 });
    for (const entry of samples) violations += await audit(page, address + entry, "navy");
    await page.setViewportSize({ width: 390, height: 844 });
    for (const entry of samples) violations += await audit(page, address + entry, "navy");
    if (violations) throw new Error(violations + " WCAG rule violation(s) across audited pages");
    console.log("PASS: WCAG/Chrome AX tree across " + pages.length + " canonical chapters desktop/mobile plus navy theme samples");
    console.log("LIMIT: this is NOT a screen-reader speech/interaction test");
  } finally {
    await browser.close();
    await new Promise(resolve => server.close(resolve));
  }
}

main().catch(error => { console.error("ERROR: accessibility audit:", error); process.exitCode = 1; });
