#
# build-wxwidgets.ps1
#

# CMake
$CMakeDir = Join-Path -Path $Env:ProgramFiles -ChildPath 'CMake'
$CMakeBinDir = Join-Path -Path $CMakeDir -ChildPath 'bin'
$CMake = Join-Path -Path $CMakeBinDir -ChildPath 'cmake'

# Source directory
$UpperDir = Split-Path -Path $PSScriptRoot
$SourceDir = $PSScriptRoot

# Build directory
$BuildDir = Join-Path -Path $UpperDir -ChildPath 'build'
$MingwBuildDir = Join-Path -Path $BuildDir -ChildPath 'mingw64'

if(-Not (Test-Path -Path $MingwBuildDir -PathType Container)) {
	& $CMake -S $SourceDir --preset mingw64
}

# Build
$Presets = 'mingw64-debug', 'mingw64-release'
foreach($preset in $Presets) {
	$CMakeArgs = @(
		'--build'
		'--preset'
		$preset
	)
	$processOptions = @{
		FilePath = $CMake
		WorkingDirectory = $SourceDir
		ArgumentList = $CMakeArgs
	}
	Start-Process @processOptions -NoNewWindow -Wait
}
