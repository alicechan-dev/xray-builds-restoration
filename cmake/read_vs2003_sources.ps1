param(
    [Parameter(Mandatory = $true)]
    [string]$ProjectPath,

    [Parameter(Mandatory = $true)]
    [string]$BaseDir,

    [ValidateSet("sources", "no-pch")]
    [string]$Mode = "sources",

    [string]$Configuration = "Debug|Win32"
)

[xml]$project = Get-Content -LiteralPath $ProjectPath
$items = New-Object System.Collections.Generic.List[string]

foreach ($file in $project.SelectNodes("//File")) {
    $relative = [string]$file.RelativePath
    if ($relative -notmatch "\.(c|cpp|rc)$") {
        continue
    }

    $configs = @($file.FileConfiguration)
    $excludedFromAll = $false
    if ($configs.Count -gt 0) {
        $excluded = @($configs | Where-Object { $_.ExcludedFromBuild -eq "TRUE" })
        $excludedFromAll = ($excluded.Count -eq $configs.Count)
    }

    if ($Mode -eq "sources") {
        if ($excludedFromAll) {
            continue
        }
    }
    elseif ($Mode -eq "no-pch") {
        $config = $configs | Where-Object { $_.Name -eq $Configuration } | Select-Object -First 1
        if (-not $config -or -not $config.Tool -or $config.Tool.UsePrecompiledHeader -ne "0") {
            continue
        }
    }

    $normalized = $relative -replace "^[.][\\/]", ""
    $path = [System.IO.Path]::GetFullPath((Join-Path $BaseDir $normalized))
    $items.Add(($path -replace "\\", "/"))
}

[Console]::Out.Write(($items -join ";"))
