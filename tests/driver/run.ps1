param([string]$Compiler = 'gcc')
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$build = Join-Path ([System.IO.Path]::GetTempPath()) ('quadruped-driver-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $build | Out-Null
try {
    $exe = Join-Path $build 'test_drivers.exe'
    $sources = @('pca9685', 'servo', 'bluetooth', 'ultrasonic', 'battery', 'buzzer', 'light') |
        ForEach-Object { Join-Path $repo "Driver/$_.c" }
    & $Compiler -std=c90 -pedantic-errors -Wall -Wextra -Werror -funsigned-char `
        -I (Join-Path $PSScriptRoot 'host') -I (Join-Path $repo 'Driver') `
        (Join-Path $PSScriptRoot 'test_drivers.c') @sources -o $exe
    if ($LASTEXITCODE -ne 0) { throw 'Driver host compilation failed.' }
    & $exe
    if ($LASTEXITCODE -ne 0) { throw 'Driver host behavior tests failed.' }
} finally {
    # Delete only exact generated files, then the newly created empty folder.
    if (Test-Path -LiteralPath $exe) { Remove-Item -LiteralPath $exe }
    Remove-Item -LiteralPath $build
}
