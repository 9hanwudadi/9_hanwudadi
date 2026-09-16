param([string]$Compiler = 'gcc')
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$build = Join-Path ([System.IO.Path]::GetTempPath()) ('quadruped-app-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $build | Out-Null
try {
    # C51 task attributes are the ONLY source text transformed for the host.
    $sources = @('App/App.h', 'App/App_System.c', 'App/APP_RobotMotion.c',
        'App/APP_Command.c', 'App/APP_Safety.c', 'User/main.c')
    foreach ($source in $sources) {
        if (!(Test-Path -LiteralPath (Join-Path $repo $source))) { continue }
        $text = [System.IO.File]::ReadAllText((Join-Path $repo $source))
        $text = [regex]::Replace($text, '\s+_task_\s+(?:TASK_ROBOT_MOTION|TASK_COMMAND|TASK_SAFETY|0)\b', '')
        [System.IO.File]::WriteAllText((Join-Path $build (Split-Path $source -Leaf)), $text)
    }
    $exe = Join-Path $build 'test_app.exe'
    & $Compiler -std=c90 -pedantic-errors -Wall -Wextra -Werror -funsigned-char `
        -I $build -I (Join-Path $PSScriptRoot 'host') -I (Join-Path $repo 'Driver') `
        (Join-Path $PSScriptRoot 'test_app.c') -o $exe
    if ($LASTEXITCODE -ne 0) { throw 'App host compilation failed.' }
    & $exe
    if ($LASTEXITCODE -ne 0) { throw 'App host behavior tests failed.' }
} finally {
    # Remove only named files made in this unique temporary directory.
    foreach ($name in @('App.h', 'App_System.c', 'APP_RobotMotion.c',
        'APP_Command.c', 'APP_Safety.c', 'main.c', 'test_app.exe')) {
        $file = Join-Path $build $name
        if (Test-Path -LiteralPath $file) { Remove-Item -LiteralPath $file }
    }
    Remove-Item -LiteralPath $build
}
