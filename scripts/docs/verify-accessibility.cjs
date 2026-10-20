#!/usr/bin/env node
"use strict";

// Browser-accessibility regression probe, not an actual screen-reader session.
// Tests rendered mdBook against WCAG axe rules and Chrome's accessibility tree.
const fs = require("node:fs");
const http = require("node:http");
const path = require("node:path");
const { chromium } = require("playwright-core");
const AxeBuilder = require("@axe-core/playwright").default;

const pages = ["/index.html", "/infrastructure/G3-PROGRESS.html"];
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

async function audit(page, url) {
  await page.goto(url, { waitUntil: "networkidle", timeout: 45000 });
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
      issue.nodes.slice(0, 6).map(node => node.target.join(" "))
    );
  }
  console.log(
    "AXE summary:",
    JSON.stringify({ url, violations: results.violations.length, passes: results.passes.length, incomplete: results.incomplete.length })
  );
  if (results.violations.length) {
    throw new Error(results.violations.length + " WCAG rule(s) failed at " + url);
  }
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
    const page = await browser.newPage({ viewport: { width: 1300, height: 900 } });
    for (const entry of pages) await audit(page, address + entry);
    await page.setViewportSize({ width: 390, height: 844 });
    await audit(page, address + pages[0]);
    console.log("PASS: generated Miracle Book WCAG semantics and Chrome accessibility tree");
    console.log("LIMIT: this is NOT a screen-reader speech/interaction test");
  } finally {
    await browser.close();
    await new Promise(resolve => server.close(resolve));
  }
}

main().catch(error => { console.error("ERROR: accessibility audit:", error); process.exitCode = 1; });
