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
    if (document.readyState === "loading") {
        document.addEventListener("DOMContentLoaded", enableAccessibleSidebarToggle);
    } else {
        enableAccessibleSidebarToggle();
    }
})();
