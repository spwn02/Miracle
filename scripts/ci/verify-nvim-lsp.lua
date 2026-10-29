-- G5 integration probe: Neovim's built-in LSP talks to the pinned clangd.
-- Run from the Miracle root after configuring the CMake 'tests' preset:
-- nvim --headless --clean -c 'luafile scripts/ci/verify-nvim-lsp.lua'
-- This does not replace testing a real interactive editor session.
local function run()
  local root = assert(vim.uv.cwd(), "missing workspace directory")
  local target = root .. "/src/private/Error.cpp"
  local compile_db = root .. "/build/tests/compile_commands.json"
  local compiler = assert(os.getenv("CXX26_TOOLCHAIN_ROOT"), "reference toolchain is not selected")
    .. "/bin/clang++"

  assert(vim.fn.filereadable(compile_db) == 1, "configure the 'tests' preset first")
  assert(vim.fn.filereadable(target) == 1, "expected Miracle source file is missing")

  local entries = vim.json.decode(table.concat(vim.fn.readfile(compile_db), "\n"))
  local matching = nil
  for _, entry in ipairs(entries) do
    local path = entry.file
    if path:sub(1, 1) ~= "/" then
      path = entry.directory .. "/" .. path
    end
    if vim.fs.normalize(path) == target then
      matching = entry
      break
    end
  end
  assert(matching, "Miracle's source file has no CMake compilation database entry")
  local command = matching.command or table.concat(matching.arguments or {}, " ")
  assert(command:find(compiler, 1, true), "compilation database did not select pinned C++26 clang++")

  vim.cmd.edit(vim.fn.fnameescape(target))
  local bufnr = vim.api.nvim_get_current_buf()
  vim.bo[bufnr].filetype = "cpp"
  local client_id = vim.lsp.start({
    name = "miracle-g5-clangd",
    cmd = {
      compiler:gsub("/clang%+%+$", "/clangd"),
      "--compile-commands-dir=" .. root .. "/build/tests",
      "--background-index=false",
    },
    root_dir = root,
  }, { bufnr = bufnr })

  assert(client_id, "Neovim could not start the pinned clangd")
  local connected = vim.wait(30000, function()
    local client = vim.lsp.get_client_by_id(client_id)
    return client ~= nil and client.initialized and client.attached_buffers[bufnr] ~= nil
  end, 100)
  assert(connected, "clangd failed to initialize and attach to Miracle's C++ source")

  local client = assert(vim.lsp.get_client_by_id(client_id))
  assert(client.server_capabilities.documentSymbolProvider, "clangd did not advertise document symbols")
  local response, err = client:request_sync("textDocument/documentSymbol", {
    textDocument = { uri = vim.uri_from_bufnr(bufnr) },
  }, 30000, bufnr)
  assert(response, "clangd document-symbol request timed out: " .. tostring(err))
  assert(not response.err, "clangd rejected document-symbol request: " .. vim.inspect(response.err))
  assert(type(response.result) == "table" and #response.result > 0,
    "clangd did not return any source symbols")
  print("PASS: Neovim LSP attached to pinned clangd and received Miracle C++ source symbols")
end

local ok, err = pcall(run)
if not ok then
  io.stderr:write("FAIL: G5 Neovim/clangd LSP: " .. tostring(err) .. "\n")
  vim.cmd("cquit 1")
else
  vim.cmd("qa!")
end
