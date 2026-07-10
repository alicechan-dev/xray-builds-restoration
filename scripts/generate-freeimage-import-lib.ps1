param(
    [Parameter(Mandatory = $true)]
    [string] $DllPath,

    [Parameter(Mandatory = $true)]
    [string] $OutDir,

    [string] $DumpbinPath = "",
    [string] $LibPath = ""
)

$ErrorActionPreference = "Stop"

function Resolve-Tool {
    param(
        [string] $RequestedPath,
        [string] $ToolName
    )

    if ($RequestedPath) {
        if (!(Test-Path -LiteralPath $RequestedPath)) {
            throw "$ToolName was not found at '$RequestedPath'"
        }
        return (Resolve-Path -LiteralPath $RequestedPath).Path
    }

    $command = Get-Command $ToolName -ErrorAction SilentlyContinue
    if ($command) {
        return $command.Source
    }

    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path -LiteralPath $vswhere) {
        $install = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($install) {
            $toolsRoot = Join-Path $install "VC\Tools\MSVC"
            $latest = Get-ChildItem -LiteralPath $toolsRoot -Directory -ErrorAction SilentlyContinue |
                Sort-Object Name -Descending |
                Select-Object -First 1
            if ($latest) {
                $candidate = Join-Path $latest.FullName "bin\Hostx64\x86\$ToolName.exe"
                if (Test-Path -LiteralPath $candidate) {
                    return $candidate
                }
            }
        }
    }

    throw "$ToolName.exe was not found. Run from a Visual Studio developer prompt or pass -${ToolName}Path."
}

if (!(Test-Path -LiteralPath $DllPath)) {
    throw "FreeImage DLL was not found at '$DllPath'"
}

$resolvedDll = (Resolve-Path -LiteralPath $DllPath).Path
$dumpbin = Resolve-Tool -RequestedPath $DumpbinPath -ToolName "dumpbin"
$lib = Resolve-Tool -RequestedPath $LibPath -ToolName "lib"

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null
$resolvedOut = (Resolve-Path -LiteralPath $OutDir).Path

$repoRoot = ""
$git = Get-Command git -ErrorAction SilentlyContinue
if ($git) {
    $repoRoot = (& git rev-parse --show-toplevel 2>$null)
}
if ($repoRoot) {
    $repoRoot = (Resolve-Path -LiteralPath $repoRoot).Path
    if ($resolvedOut.StartsWith($repoRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to write generated FreeImage import files inside the repository: '$resolvedOut'"
    }
}

$exports = & $dumpbin /exports $resolvedDll |
    ForEach-Object {
        if ($_ -match '^\s*\d+\s+[0-9A-Fa-f]+\s+[0-9A-Fa-f]+\s+(\S+)\s*$') {
            $Matches[1]
        }
    } |
    Where-Object { $_ } |
    Sort-Object -Unique

if (!$exports) {
    throw "No exports were parsed from '$resolvedDll'"
}

$defPath = Join-Path $resolvedOut "FreeImage.def"
$libPath = Join-Path $resolvedOut "FreeImage.lib"

$defExports = $exports | ForEach-Object {
    if ($_ -match '^_(.+@\d+)$') {
        "    $($Matches[1])=$_"
    } else {
        "    $_"
    }
}

@('LIBRARY "FreeImage.dll"', 'EXPORTS') + $defExports |
    Set-Content -Path $defPath -Encoding ASCII

& $lib "/def:$defPath" /machine:x86 "/out:$libPath"
if ($LASTEXITCODE -ne 0) {
    throw "lib.exe failed with exit code $LASTEXITCODE"
}

Write-Host "Generated:"
Write-Host "  $defPath"
Write-Host "  $libPath"
Write-Host ""
Write-Host "Use with CMake:"
Write-Host "  -DXR_DO_LIGHT_FREEIMAGE_LIB=$libPath"
