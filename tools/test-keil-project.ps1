param(
  [Parameter(Mandatory = $true)]
  [string]$Root
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Assert-Equal {
  param(
    [Parameter(Mandatory = $true)]$Actual,
    [Parameter(Mandatory = $true)]$Expected,
    [Parameter(Mandatory = $true)][string]$Message
  )

  if ($Actual -ne $Expected) {
    throw "$Message (expected '$Expected', got '$Actual')"
  }
}

function Assert-SequenceEqual {
  param(
    [Parameter(Mandatory = $true)][object[]]$Actual,
    [Parameter(Mandatory = $true)][object[]]$Expected,
    [Parameter(Mandatory = $true)][string]$Message
  )

  $actualText = $Actual -join '|'
  $expectedText = $Expected -join '|'
  if ($actualText -cne $expectedText) {
    throw "$Message (expected '$expectedText', got '$actualText')"
  }
}

$projectPath = Join-Path $Root 'stc8h8k64u.uvproj'
$optionsPath = Join-Path $Root 'stc8h8k64u.uvopt'

if (-not (Test-Path -LiteralPath $projectPath -PathType Leaf)) {
  throw "Missing Keil project: $projectPath"
}
if (-not (Test-Path -LiteralPath $optionsPath -PathType Leaf)) {
  throw "Missing Keil target options: $optionsPath"
}

$projectText = Get-Content -Raw -LiteralPath $projectPath
$optionsText = Get-Content -Raw -LiteralPath $optionsPath
[xml]$project = $projectText
[xml]$options = $optionsText

$target = $project.Project.Targets.Target
Assert-Equal $project.Project.SchemaVersion '1.1' 'Unexpected Keil project schema'
Assert-Equal $target.ToolsetNumber '0x0' 'Unexpected toolset number'
Assert-Equal $target.ToolsetName 'MCS-51' 'Unexpected toolset name'
Assert-Equal $target.TargetOption.TargetCommonOption.Device 'STC8H8K64U Series' 'Unexpected MCU device'
Assert-Equal $target.TargetOption.TargetCommonOption.Vendor 'STC' 'Unexpected MCU vendor'
Assert-Equal $target.TargetOption.TargetCommonOption.Cpu 'IRAM(0-0xFF) XRAM(0-0x1FFF) IROM(0-0xFFF8) CLOCK(24000000)' 'Unexpected target CPU or clock'
Assert-Equal $target.TargetOption.TargetCommonOption.OutputDirectory '.\Objects\' 'Unexpected object output directory'
Assert-Equal $target.TargetOption.TargetCommonOption.ListingPath '.\Listings\' 'Unexpected listing output directory'
Assert-Equal $target.TargetOption.Target51.Target51Misc.MemoryModel '2' 'Unexpected C51 memory model'
Assert-Equal $target.TargetOption.Target51.Target51Misc.RTOS '1' 'RTX51 Tiny target setting is not enabled'
Assert-Equal $target.TargetOption.Target51.Target51Misc.RomSize '2' 'Unexpected C51 ROM size'
Assert-Equal $target.TargetOption.Target51.C51.VariousControls.IncludePath '.\App;.\Driver;.\Lib;.\User' 'Unexpected compiler include path'
Assert-Equal $options.ProjectOpt.Target.TargetOption.CLK51 '24000000' 'Unexpected debugger clock'

$expectedGroups = [ordered]@{
  App = @(
    '.\App\App_System.c'
    '.\App\APP_RobotMotion.c'
    '.\App\APP_Command.c'
    '.\App\APP_Safety.c'
  )
  Driver = @(
    '.\Driver\pca9685.c'
    '.\Driver\servo.c'
    '.\Driver\bluetooth.c'
    '.\Driver\ultrasonic.c'
    '.\Driver\battery.c'
    '.\Driver\buzzer.c'
    '.\Driver\light.c'
  )
  Lib = @(
    '.\Lib\Soft_I2C.c'
    '.\Lib\GPIO.c'
    '.\Lib\NVIC.c'
    '.\Lib\UART.c'
    '.\Lib\UART_Isr.c'
    '.\Lib\Timer.c'
    '.\Lib\Timer_Isr.c'
    '.\Lib\Delay.c'
    '.\Lib\ADC.c'
    '.\Lib\STC8H_PWM.c'
  )
  User = @('.\User\main.c')
  OS = @(
    '.\OS\Conf_tny.A51'
    '.\OS\RTX51TNY.LIB'
  )
}

$groups = @($target.Groups.Group)
Assert-SequenceEqual @($groups.GroupName) @($expectedGroups.Keys) 'Keil group names or order differ'

$allPaths = [System.Collections.Generic.List[string]]::new()
foreach ($group in $groups) {
  $actualPaths = @($group.Files.File | ForEach-Object { [string]$_.FilePath })
  $expectedPaths = @($expectedGroups[[string]$group.GroupName])
  Assert-SequenceEqual $actualPaths $expectedPaths "Unexpected files in group '$($group.GroupName)'"

  foreach ($file in @($group.Files.File)) {
    $path = [string]$file.FilePath
    $extension = [System.IO.Path]::GetExtension($path)
    $expectedType = switch -Regex ($extension) {
      '^\.c$' { '1'; break }
      '^\.A51$' { '2'; break }
      '^\.LIB$' { '4'; break }
      default { throw "Non-compilable file is listed in project: $path" }
    }
    Assert-Equal ([string]$file.FileType) $expectedType "Unexpected Keil FileType for '$path'"
    Assert-Equal ([string]$file.FileName) ([System.IO.Path]::GetFileName($path)) "FileName does not match FilePath for '$path'"

    if ($path -notmatch '^\.\\' -or [System.IO.Path]::IsPathRooted($path) -or $path -match '^[A-Za-z]:[\\/]') {
      throw "Keil FilePath is not repository-relative: $path"
    }
    if ($path -match '(?i)(^|\\)(tests?|[^\\]*test[^\\]*)($|\\)') {
      throw "Test source is listed in project: $path"
    }

    $relativePath = $path -replace '^\.\\', ''
    if (-not (Test-Path -LiteralPath (Join-Path $Root $relativePath) -PathType Leaf)) {
      throw "Keil FilePath does not exist: $path"
    }
    $allPaths.Add($path)
  }
}

Assert-Equal $allPaths.Count 24 'Unexpected number of compilable project files'
$duplicates = $allPaths | Group-Object { $_.ToLowerInvariant() } | Where-Object Count -gt 1
if ($duplicates) {
  throw "Duplicate Keil FilePath entries: $(($duplicates.Name | Sort-Object) -join ', ')"
}

$staleNamePattern = '(?i)(motors?|track|(^|\\)key(?:\.c|\.h)?$|blueteeth|APP_(Light|buzzr|ultrasonic|UART1))'
foreach ($documentText in @($projectText, $optionsText)) {
  if ($documentText -match $staleNamePattern) {
    throw "Stale car reference found in Keil XML: $($Matches[0])"
  }
  if ($documentText -match '(?i)(?<![A-Za-z])[A-Za-z]:[\\/]') {
    throw "Absolute path found in Keil XML: $($Matches[0])"
  }
  if ($documentText -match '(?i)\.uvgui\.|\.uvguix\.|(^|[>\\])\.vscode([<\\]|$)') {
    throw "Personal UI reference found in Keil XML: $($Matches[0])"
  }
}

$referencedPaths = @($project.SelectNodes('//FilePath') | ForEach-Object { [string]$_.InnerText }) +
  @($options.SelectNodes('//PathWithFileName') | ForEach-Object { [string]$_.InnerText } | Where-Object { $_ })
foreach ($path in $referencedPaths) {
  if ($path -match $staleNamePattern) {
    throw "Stale car file reference found: $path"
  }
  if ([System.IO.Path]::IsPathRooted($path) -or $path -match '^[A-Za-z]:[\\/]') {
    throw "Absolute Keil path found: $path"
  }
  if ($path -match '(?i)\.uvgui\.|\.uvguix\.|(^|\\)\.vscode($|\\)') {
    throw "Personal UI reference found: $path"
  }

  $relativePath = $path -replace '^\.\\', ''
  if (-not (Test-Path -LiteralPath (Join-Path $Root $relativePath) -PathType Leaf)) {
    throw "Keil referenced path does not exist: $path"
  }
}

Write-Output "Keil project validation passed: 5 groups, $($allPaths.Count) compilable files, 24 MHz RTX51 target."
