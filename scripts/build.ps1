[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')]
    [string]$Configuration = 'Debug',
    [string]$BuildDirectory = 'build',
    [string[]]$Target = @('sphere_game', 'sphere_headless', 'sphere_tests'),
    [switch]$RunTests,
    [string]$CMakePath
)

$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$buildPath = if ([System.IO.Path]::IsPathRooted($BuildDirectory)) {
    $BuildDirectory
} else {
    Join-Path $projectRoot $BuildDirectory
}

if (-not $CMakePath) {
    $cmakeCommand = Get-Command cmake -CommandType Application -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($cmakeCommand) {
        $CMakePath = $cmakeCommand.Source
    } else {
        $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
        if (Test-Path -LiteralPath $vswhere) {
            $CMakePath = & $vswhere -latest -products '*' -find 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe' | Select-Object -First 1
        }
    }
}
if (-not $CMakePath -or -not (Test-Path -LiteralPath $CMakePath)) {
    throw 'CMake was not found. Use Developer PowerShell or pass -CMakePath.'
}
$CMakePath = (Resolve-Path -LiteralPath $CMakePath).Path
$ctestPath = Join-Path (Split-Path $CMakePath -Parent) 'ctest.exe'
if ($RunTests -and -not (Test-Path -LiteralPath $ctestPath)) {
    throw "CTest was not found beside CMake: $ctestPath"
}

function Invoke-BuildTool {
    param([string]$Executable, [string[]]$ToolArguments)
    & $Executable @ToolArguments
    if ($LASTEXITCODE -ne 0) {
        throw "$(Split-Path $Executable -Leaf) failed with exit code $LASTEXITCODE."
    }
}

# CMake 3.31.6-msvc6 and 3.31.12 can abort in find_program() when
# the working directory contains Japanese characters. Keep all source
# and output paths intact, but run the build tools from an ASCII path.
$toolWorkingDirectory = $env:SystemRoot
if ($toolWorkingDirectory -match '[^\x00-\x7F]') {
    throw 'The Windows directory must have an ASCII path for this workaround.'
}
Push-Location -LiteralPath $toolWorkingDirectory
try {
    Write-Host "CMake: $CMakePath"
    Invoke-BuildTool $CMakePath @('-S', $projectRoot, '-B', $buildPath, '-DBUILD_TESTING=ON')
    Invoke-BuildTool $CMakePath (@('--build', $buildPath, '--config', $Configuration, '--target') + $Target)
    if ($RunTests) {
        Invoke-BuildTool $ctestPath @('--test-dir', $buildPath, '-C', $Configuration, '--output-on-failure', '--no-tests=error')
    }
} finally {
    Pop-Location
}
