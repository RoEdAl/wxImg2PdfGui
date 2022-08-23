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
	if ( -Not $? ) {
		exit
	}
}

# Build
$Presets = 'mingw64-debug', 'mingw64-release'
foreach($preset in $Presets) {
	try {
		Push-Location -Path $SourceDir
		& $CMake --build --preset $preset
		if ( -Not $? ) {
			exit
		}
	}
	finally {
		Pop-Location
	}
}
