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
  'Lib\GPIO.c','Lib\GPIO.h','Lib\NVIC.c','Lib\NVIC.h',
  'Lib\UART.c','Lib\UART.h','Lib\UART_Isr.c',
  'Lib\Timer.c','Lib\Timer.h','Lib\Timer_Isr.c',
  'Lib\Delay.c','Lib\Delay.h','Lib\ADC.c','Lib\ADC.h',
  'Lib\STC8H_PWM.c','Lib\STC8H_PWM.h','Lib\Switch.h',
  'OS\Conf_tny.A51','OS\RTX51TNY.LIB',
  'stc8h8k64u.uvproj','stc8h8k64u.uvopt',
  'README.md','docs\architecture.md','docs\hardware-resources.md',
  'docs\interfaces\README.md','docs\interfaces\servo.md',
  'docs\interfaces\robot-motion.md'
)
$missing = $required | Where-Object { -not (Test-Path -LiteralPath (Join-Path $Root $_)) }
if ($missing) { throw "Missing required files: $($missing -join ', ')" }

$gitRoot = [System.IO.Path]::GetFullPath((Join-Path $Root '.git'))
$forbidden = Get-ChildItem -LiteralPath $Root -Recurse -Force |
  Where-Object {
    $_.FullName -ne $gitRoot -and
    -not $_.FullName.StartsWith($gitRoot + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase) -and
    ($_.Name -like '*.uvgui.*' -or $_.Name -like '*.uvguix.*' -or
    $_.Name -eq '.vscode')
  }
if ($forbidden) { throw "User-specific files found: $($forbidden.FullName -join ', ')" }

# A real Keil build creates these ignored directories. Reject tracked
# artifacts instead of rejecting the local evidence of a successful build.
$tracked = & git -C $Root ls-files
if ($LASTEXITCODE -ne 0) { throw 'Cannot inspect tracked template files.' }
$trackedArtifacts = $tracked | Where-Object {
  $_ -match '(^|/)(Objects|Listings)/|\.(obj|lst|m51|hex|lnp|build_log\.htm)$|(^|/)build\.log$'
}
if ($trackedArtifacts) { throw "Tracked build artifacts found: $($trackedArtifacts -join ', ')" }
foreach ($generated in @('Objects/', 'Listings/', 'build.log')) {
  if (Test-Path -LiteralPath (Join-Path $Root $generated)) {
    & git -C $Root check-ignore --quiet -- $generated
    if ($LASTEXITCODE -ne 0) { throw "Build output is not ignored: $generated" }
  }
}

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
