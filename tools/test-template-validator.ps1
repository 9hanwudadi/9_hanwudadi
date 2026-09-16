param([Parameter(Mandatory = $true)][string]$Root)
$ErrorActionPreference = 'Stop'
$fixture = Join-Path ([System.IO.Path]::GetTempPath()) ('quadruped-validator-' + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $fixture | Out-Null
try {
  # Copy current tracked source, never real generated output or .git internals.
  $files = & git -C $Root ls-files
  if ($LASTEXITCODE -ne 0) { throw 'Cannot enumerate template source.' }
  foreach ($file in $files) {
    $destination = Join-Path $fixture $file
    New-Item -ItemType Directory -Path (Split-Path $destination -Parent) -Force | Out-Null
    Copy-Item -LiteralPath (Join-Path $Root $file) -Destination $destination
  }
  & git -C $fixture init -q
  & git -C $fixture -c core.safecrlf=false add .
  if ($LASTEXITCODE -ne 0) { throw 'Cannot prepare validator fixture index.' }
  New-Item -ItemType Directory -Path (Join-Path $fixture 'Objects'),(Join-Path $fixture 'Listings') | Out-Null
  $hex = Join-Path $fixture 'Objects/probe.hex'
  [System.IO.File]::WriteAllText($hex, ':00000001FF')
  [System.IO.File]::WriteAllText((Join-Path $fixture 'build.log'), 'fixture build output')
  $validator = Join-Path $Root 'tools/validate-template.ps1'
  & $validator -Root $fixture

  & git -C $fixture add -f Objects/probe.hex
  $rejected = $false
  try { & $validator -Root $fixture } catch {
    if ($_.Exception.Message -notlike 'Tracked build artifacts found:*') { throw }
    $rejected = $true
  }
  if (-not $rejected) { throw 'Validator accepted a tracked HEX artifact.' }
  & git -C $fixture rm --cached -q Objects/probe.hex

  [System.IO.File]::WriteAllText((Join-Path $fixture 'probe.uvguix.user'), 'fixture UI state')
  $rejected = $false
  try { & $validator -Root $fixture } catch {
    if ($_.Exception.Message -notlike 'User-specific files found:*') { throw }
    $rejected = $true
  }
  if (-not $rejected) { throw 'Validator accepted personal Keil UI state.' }
  Write-Output 'PASS: 3 validator artifact and personal-config regression checks'
} finally {
  $resolvedFixture = [System.IO.Path]::GetFullPath($fixture)
  $temporaryRoot = [System.IO.Path]::GetFullPath([System.IO.Path]::GetTempPath())
  if (-not $resolvedFixture.StartsWith($temporaryRoot, [System.StringComparison]::OrdinalIgnoreCase) -or
      (Split-Path $resolvedFixture -Leaf) -notlike 'quadruped-validator-*') {
    throw 'Refusing to remove unexpected fixture directory.'
  }
  Remove-Item -LiteralPath $resolvedFixture -Recurse -Force
}
