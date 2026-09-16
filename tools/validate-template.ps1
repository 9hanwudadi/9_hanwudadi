param([Parameter(Mandatory=$true)][string]$Root)
$ErrorActionPreference = 'Stop'
$required = @(
  'App\App.h','App\App_System.c','App\APP_RobotMotion.c',
  'App\APP_Command.c','App\APP_Safety.c',
  'Driver\Driver_Common.h','Driver\pca9685.h','Driver\pca9685.c',
  'Driver\servo.h','Driver\servo.c',
  'Driver\bluetooth.h','Driver\bluetooth.c',
  'Driver\ultrasonic.h','Driver\ultrasonic.c',
  'Driver\battery.h','Driver\battery.c',
  'Driver\buzzer.h','Driver\buzzer.c',
  'Driver\light.h','Driver\light.c','User\main.c',
  'Lib\Config.h','Lib\Type_def.h','Lib\STC8H.h',
  'Lib\Soft_I2C.h','Lib\Soft_I2C.c',
  'OS\Conf_tny.A51','OS\RTX51TNY.LIB',
  'stc8h8k64u.uvproj','stc8h8k64u.uvopt',
  'README.md','docs\architecture.md','docs\hardware-resources.md',
  'docs\interfaces\README.md','docs\interfaces\servo.md',
  'docs\interfaces\robot-motion.md'
)
$missing = $required | Where-Object { -not (Test-Path -LiteralPath (Join-Path $Root $_)) }
if ($missing) { throw "Missing required files: $($missing -join ', ')" }

$forbidden = Get-ChildItem -LiteralPath $Root -Recurse -Force |
  Where-Object { $_.Name -like '*.uvgui.*' -or $_.Name -like '*.uvguix.*' -or $_.Name -eq '.vscode' }
if ($forbidden) { throw "User-specific files found: $($forbidden.FullName -join ', ')" }

$newSources = Get-ChildItem -LiteralPath (Join-Path $Root 'App'),(Join-Path $Root 'Driver'),(Join-Path $Root 'User') -File |
  Where-Object { $_.Extension -in '.c','.h' }
foreach ($file in $newSources) {
  $bytes = [System.IO.File]::ReadAllBytes($file.FullName)
  if ($bytes | Where-Object { $_ -gt 127 }) { throw "Non-ASCII authored source: $($file.FullName)" }
}

$badIncludes = Get-ChildItem -LiteralPath (Join-Path $Root 'Driver'),(Join-Path $Root 'Lib') -Recurse -File -Include *.c,*.h |
  Select-String -Pattern '#include\s+["<]App\.h[">]'
if ($badIncludes) { throw 'Driver or Lib includes App.h' }

$mainIncludes = Select-String -LiteralPath (Join-Path $Root 'User\main.c') -Pattern '^\s*#include'
if (($mainIncludes.Line -join "`n") -ne '#include "App.h"') { throw 'User/main.c must include only App.h' }

[xml]$project = Get-Content -Raw -LiteralPath (Join-Path $Root 'stc8h8k64u.uvproj')
$paths = $project.SelectNodes('//FilePath') | ForEach-Object { $_.'#text' }
foreach ($path in $paths) {
  $normalized = $path -replace '^\.\\',''
  if (-not (Test-Path -LiteralPath (Join-Path $Root $normalized))) {
    throw "Keil FilePath does not exist: $path"
  }
}
Write-Output 'Template validation passed.'
