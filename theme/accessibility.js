/* mdBook v0.5.4 uses a <label> as the sidebar toggle but assigns it
   aria-expanded, which is invalid on a generic label. Make it a keyboard
   operable button, retaining the existing checkbox/label behavior. */
(function () {
    function enableAccessibleSidebarToggle() {
        const toggle = document.getElementById("mdbook-sidebar-toggle");
        const input = document.getElementById("mdbook-sidebar-toggle-anchor");
        if (!toggle || !input) return;
        toggle.setAttribute("role", "button");
        toggle.setAttribute("tabindex", "0");
        toggle.addEventListener("keydown", (event) => {
            if (event.key === "Enter" || event.key === " ") {
                event.preventDefault();
                input.click();
            }
        });
    }
    function labelReadOnlyTasks() {
        document.querySelectorAll(".content input[type=checkbox][disabled]").forEach((field) => {
            const itemText = field.closest("li")?.textContent?.trim() || "task";
            field.setAttribute("aria-label", (field.checked ? "Completed: " : "Incomplete: ") + itemText.slice(0, 240));
        });
    }
    function makeLongCodeBlocksKeyboardAccessible() {
        // mdBook sometimes applies overflow to the code element itself.
        // A focused element allows keyboard users (including Safari users)
        // to scroll code blocks otherwise accessible only by mouse/touch.
        document.querySelectorAll("main pre code").forEach((code) => {
            code.setAttribute("tabindex", "0");
        });
    }
    function initialize() {
        enableAccessibleSidebarToggle();
        labelReadOnlyTasks();
        makeLongCodeBlocksKeyboardAccessible();
    }
    if (document.readyState === "loading") {
        document.addEventListener("DOMContentLoaded", initialize);
    } else {
        initialize();
    }
})();
