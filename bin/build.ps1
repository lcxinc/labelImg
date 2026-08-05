param(
    [string]$Configuration = "Release",
    [string]$BuildDir = "",
    [string]$QtPrefix = "D:\Qt\6.11.0\msvc2022_64",
    [string]$Generator = "Visual Studio 17 2022",
    [string]$Platform = "x64",
    [string]$Target = "",
    [switch]$RunTests
)

$ErrorActionPreference = "Stop"

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Resolve-Path (Join-Path $scriptRoot "..")
$cppRoot = Join-Path $repoRoot "cpp"
if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $repoRoot "target\cpp-build"
}

$qtBin = Join-Path $QtPrefix "bin"
if (!(Test-Path $qtBin)) {
    throw "Qt bin directory was not found: $qtBin"
}

New-Item -ItemType Directory -Force -Path (Join-Path $repoRoot "target") | Out-Null
$env:PATH = "$qtBin;$env:PATH"

if (!(Test-Path (Join-Path $BuildDir "CMakeCache.txt"))) {
    & cmake -S $cppRoot -B $BuildDir -G $Generator -A $Platform -DCMAKE_PREFIX_PATH="$QtPrefix"
}

$buildArgs = @("--build", $BuildDir, "--config", $Configuration)
if (![string]::IsNullOrWhiteSpace($Target)) {
    $buildArgs += @("--target", $Target)
}
& cmake @buildArgs

if ($RunTests) {
    & ctest --test-dir $BuildDir -C $Configuration --output-on-failure
}
