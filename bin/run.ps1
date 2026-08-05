param(
    [string]$Configuration = "Release",
    [string]$BuildDir = "",
    [string]$QtPrefix = "D:\Qt\6.11.0\msvc2022_64",
    [switch]$Build,
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$AppArgs
)

$ErrorActionPreference = "Stop"

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptRoot "..")
if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $repoRoot "target\cpp-build"
}

$exe = Join-Path $BuildDir "$Configuration\labelImgCpp.exe"
if ($Build -or !(Test-Path $exe)) {
    & (Join-Path $scriptRoot "build.ps1") -Configuration $Configuration -BuildDir $BuildDir -QtPrefix $QtPrefix -Target labelImgCpp
}

if (!(Test-Path $exe)) {
    throw "Executable was not found: $exe"
}

$qtBin = Join-Path $QtPrefix "bin"
if (!(Test-Path $qtBin)) {
    throw "Qt bin directory was not found: $qtBin"
}
$env:PATH = "$qtBin;$env:PATH"

$startArgs = @{
    FilePath = $exe
    WorkingDirectory = Split-Path -Parent $exe
}
if ($AppArgs -and $AppArgs.Count -gt 0) {
    $startArgs.ArgumentList = $AppArgs
}
Start-Process @startArgs
