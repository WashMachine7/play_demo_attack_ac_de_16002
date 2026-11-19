param([string]$Compiler = 'zig')
$ErrorActionPreference = 'Stop'
Push-Location -LiteralPath $PSScriptRoot
try {
    $symbols = 'reset_match', 'start_attack', 'interrupt_attack', 'resolve_candidate', 'target_hp', 'damage_commits', 'sweep_contact'
    $exports = $symbols | ForEach-Object { "--export=$_" }
    & $Compiler build-exe -target wasm32-freestanding --entry reset_match @exports -O ReleaseSmall '-femit-bin=combat.wasm' -cflags '-std=c++17' -Wall -Wextra -Werror -fno-exceptions -fno-rtti -- combat.cpp
    if ($LASTEXITCODE -ne 0) { throw 'WebAssembly build failed.' }
    $encoded = [Convert]::ToBase64String([IO.File]::ReadAllBytes((Join-Path $PSScriptRoot 'combat.wasm')))
    $html = (Get-Content -LiteralPath .\demo.template.html -Raw -Encoding UTF8).Replace('__WASM_BASE64__', $encoded)
    [IO.File]::WriteAllText((Join-Path $PSScriptRoot 'HitWindowLab.html'), $html, [Text.UTF8Encoding]::new($false))
} finally {
    Pop-Location
}
