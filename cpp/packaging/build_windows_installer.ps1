param(
    [string]$Configuration = "Release",
    [string]$BuildDir = "",
    [string]$QtBin = "D:\Qt\6.11.0\msvc2022_64\bin",
    [string]$Version = "0.1.0",
    [string]$CMakeExe = "cmake",
    [string]$VcRuntimeDir = "",
    [switch]$PortableOnly
)

$ErrorActionPreference = "Stop"
if ($Version -notmatch '^[A-Za-z0-9._-]+$') {
    throw "Version must be a simple filename component"
}

$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$cppRoot = Resolve-Path (Join-Path $scriptRoot "..")
$repoRoot = Resolve-Path (Join-Path $cppRoot "..")
if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $repoRoot "target\cpp-build"
}

$distRoot = Join-Path $repoRoot "target\dist"
$deployName = "labelImgCpp-$Version-win64"
$deployRoot = Join-Path $distRoot $deployName
$installerWork = Join-Path $distRoot "installer-work"
$payloadZip = Join-Path $installerWork "payload.zip"
$installerExe = Join-Path $distRoot "$deployName-installer.exe"
$portableZip = Join-Path $distRoot "$deployName.zip"
$stagingCab = Join-Path $distRoot "~$deployName-installer.CAB"

if (!(Test-Path (Join-Path $BuildDir "CMakeCache.txt"))) {
    $qtPrefix = Split-Path -Parent $QtBin
    & $CMakeExe -S $cppRoot -B $BuildDir -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="$qtPrefix"
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }
}

& $CMakeExe --build $BuildDir --config $Configuration --target labelImgCpp
if ($LASTEXITCODE -ne 0) { throw "CMake build failed" }

$builtExe = Join-Path $BuildDir "$Configuration\labelImgCpp.exe"
if (!(Test-Path $builtExe)) {
    throw "Built executable was not found: $builtExe"
}

$distFullPath = [IO.Path]::GetFullPath($distRoot).TrimEnd('\') + '\'
foreach ($outputPath in @($deployRoot, $installerWork)) {
    $resolvedOutput = [IO.Path]::GetFullPath($outputPath)
    if (!$resolvedOutput.StartsWith($distFullPath, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to remove a directory outside the distribution directory: $resolvedOutput"
    }
}
Remove-Item -LiteralPath $deployRoot -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $installerWork -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $installerExe -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $portableZip -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $stagingCab -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $deployRoot, $installerWork | Out-Null

Copy-Item -LiteralPath $builtExe -Destination $deployRoot
$windDeployQt = Join-Path $QtBin "windeployqt.exe"
if (!(Test-Path $windDeployQt)) {
    throw "windeployqt was not found: $windDeployQt"
}
& $windDeployQt --release --dir $deployRoot (Join-Path $deployRoot "labelImgCpp.exe")
if ($LASTEXITCODE -ne 0) { throw "Qt deployment failed" }
if ([string]::IsNullOrWhiteSpace($VcRuntimeDir)) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path -LiteralPath $vswhere) {
        $vsInstall = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($vsInstall) {
            $redistBase = Join-Path $vsInstall "VC\Redist\MSVC"
            $redistVersion = Get-ChildItem -LiteralPath $redistBase -Directory |
                Where-Object { $_.Name -match '^\d+\.\d+\.\d+$' } |
                Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1
            if ($redistVersion) { $VcRuntimeDir = Join-Path $redistVersion.FullName "x64\Microsoft.VC143.CRT" }
        }
    }
}
if (![string]::IsNullOrWhiteSpace($VcRuntimeDir) -and (Test-Path -LiteralPath $VcRuntimeDir)) {
    Get-ChildItem -LiteralPath $VcRuntimeDir -Filter *.dll | Copy-Item -Destination $deployRoot
}
foreach ($requiredFile in @("Qt6Core.dll", "Qt6Gui.dll", "Qt6Widgets.dll",
                             "platforms\qwindows.dll", "iconengines\qsvgicon.dll",
                             "vcruntime140.dll", "vcruntime140_1.dll", "msvcp140.dll")) {
    if (!(Test-Path -LiteralPath (Join-Path $deployRoot $requiredFile))) {
        throw "Required deployment component missing: $requiredFile"
    }
}

New-Item -ItemType Directory -Force -Path (Join-Path $deployRoot "resources"), (Join-Path $deployRoot "data") | Out-Null
Copy-Item -LiteralPath (Join-Path $repoRoot "resources\icons") -Destination (Join-Path $deployRoot "resources\icons") -Recurse
Copy-Item -LiteralPath (Join-Path $repoRoot "resources\strings") -Destination (Join-Path $deployRoot "resources\strings") -Recurse
Copy-Item -LiteralPath (Join-Path $repoRoot "data\predefined_classes.txt") -Destination (Join-Path $deployRoot "data\predefined_classes.txt")
Copy-Item -LiteralPath (Join-Path $repoRoot "LICENSE") -Destination (Join-Path $deployRoot "LICENSE.txt")
Copy-Item -LiteralPath (Join-Path $cppRoot "README.md") -Destination (Join-Path $deployRoot "README.txt")
$aiBridge = Join-Path $cppRoot "tools\labelme_ai_bridge.py"
if (!(Test-Path $aiBridge)) {
    throw "AI bridge script was not found: $aiBridge"
}
Copy-Item -LiteralPath $aiBridge -Destination (Join-Path $deployRoot "labelme_ai_bridge.py")
Copy-Item -LiteralPath (Join-Path $cppRoot "tools\onnx_detection_bridge.py") -Destination (Join-Path $deployRoot "onnx_detection_bridge.py")
Copy-Item -LiteralPath (Join-Path $cppRoot "tools\requirements-onnx.txt") -Destination (Join-Path $deployRoot "requirements-onnx.txt")

@'
$ErrorActionPreference = "Stop"
$installRoot = Join-Path $env:LOCALAPPDATA "Programs\labelImgCpp"
$startMenu = [Environment]::GetFolderPath("Programs")
$desktop = [Environment]::GetFolderPath("DesktopDirectory")
$shortcutPaths = @(
    (Join-Path $startMenu "labelImgCpp.lnk"),
    (Join-Path $desktop "labelImgCpp.lnk")
)
foreach ($shortcut in $shortcutPaths) {
    Remove-Item -LiteralPath $shortcut -Force -ErrorAction SilentlyContinue
}
Remove-Item -LiteralPath $installRoot -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $installRoot | Out-Null
Expand-Archive -LiteralPath (Join-Path $PSScriptRoot "payload.zip") -DestinationPath $installRoot -Force

$exe = Join-Path $installRoot "labelImgCpp.exe"
$shell = New-Object -ComObject WScript.Shell
foreach ($shortcut in $shortcutPaths) {
    $link = $shell.CreateShortcut($shortcut)
    $link.TargetPath = $exe
    $link.WorkingDirectory = $installRoot
    $link.IconLocation = "$exe,0"
    $link.Save()
}
'@ | Set-Content -LiteralPath (Join-Path $installerWork "install.ps1") -Encoding UTF8

@'
$ErrorActionPreference = "SilentlyContinue"
$installRoot = Join-Path $env:LOCALAPPDATA "Programs\labelImgCpp"
Remove-Item -LiteralPath (Join-Path ([Environment]::GetFolderPath("Programs")) "labelImgCpp.lnk") -Force
Remove-Item -LiteralPath (Join-Path ([Environment]::GetFolderPath("DesktopDirectory")) "labelImgCpp.lnk") -Force
Remove-Item -LiteralPath $installRoot -Recurse -Force
'@ | Set-Content -LiteralPath (Join-Path $deployRoot "uninstall.ps1") -Encoding UTF8

Compress-Archive -Path (Join-Path $deployRoot "*") -DestinationPath $portableZip -Force
if ($PortableOnly) {
    Write-Host "Deploy directory: $deployRoot"
    Write-Host "Portable zip: $portableZip"
    exit 0
}
Compress-Archive -Path (Join-Path $deployRoot "*") -DestinationPath $payloadZip -Force

$sedPath = Join-Path $installerWork "labelImgCpp.sed"
$installCommand = 'powershell.exe -NoProfile -ExecutionPolicy Bypass -File install.ps1'
@"
[Version]
Class=IEXPRESS
SEDVersion=3

[Options]
PackagePurpose=InstallApp
ShowInstallProgramWindow=0
HideExtractAnimation=1
UseLongFileName=1
InsideCompressed=0
CAB_FixedSize=0
CAB_ResvCodeSigning=0
RebootMode=N
InstallPrompt=%InstallPrompt%
DisplayLicense=%DisplayLicense%
FinishMessage=%FinishMessage%
TargetName=%TargetName%
FriendlyName=%FriendlyName%
AppLaunched=%AppLaunched%
PostInstallCmd=<None>
AdminQuietInstCmd=%AppLaunched%
UserQuietInstCmd=%AppLaunched%
SourceFiles=SourceFiles

[Strings]
InstallPrompt=
DisplayLicense=
FinishMessage=labelImgCpp has been installed.
TargetName=$installerExe
FriendlyName=labelImgCpp
AppLaunched=$installCommand
FILE0=payload.zip
FILE1=install.ps1

[SourceFiles]
SourceFiles0=$installerWork

[SourceFiles0]
%FILE0%=
%FILE1%=
"@ | Set-Content -LiteralPath $sedPath -Encoding ASCII

$iexpress = Start-Process -FilePath (Join-Path $env:SystemRoot "System32\iexpress.exe") `
    -ArgumentList @("/N", "/Q", $sedPath) -WindowStyle Hidden -PassThru
$deadline = (Get-Date).AddSeconds(45)
while (!$iexpress.HasExited -and (Get-Date) -lt $deadline) {
    Start-Sleep -Milliseconds 250
}
if (!$iexpress.HasExited) {
    Stop-Process -Id $iexpress.Id -Force -ErrorAction SilentlyContinue
    throw "IExpress timed out while creating installer: $installerExe"
}
if ($iexpress.ExitCode -ne 0) {
    throw "IExpress failed with exit code $($iexpress.ExitCode): $installerExe"
}
if (!(Test-Path $installerExe)) {
    throw "Installer was not created: $installerExe"
}
$installerSize = (Get-Item -LiteralPath $installerExe).Length
$payloadSize = (Get-Item -LiteralPath $payloadZip).Length
if ($installerSize -lt $payloadSize) {
    throw "Installer is incomplete ($installerSize bytes, expected at least $payloadSize): $installerExe"
}
Remove-Item -LiteralPath $stagingCab -Force -ErrorAction SilentlyContinue

Write-Host "Deploy directory: $deployRoot"
Write-Host "Portable zip: $portableZip"
Write-Host "Installer: $installerExe"
exit 0
