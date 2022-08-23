#
# msvc-build-wxwidgets.ps1
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
$MsvcBuildDir = Join-Path -Path $BuildDir -ChildPath 'msvc'

if(-Not (Test-Path -Path $MsvcBuildDir -PathType Container)) {
	& $CMake -S $SourceDir --preset msvc
	if ( -Not $? ) {
		exit
	}
}

# Build
$Presets = 'msvc-debug', 'msvc-release'
foreach($preset in $Presets) {
	try {
		Push-Location -Path $SourceDir
		& $CMake --build --preset $preset --target 'package'
		if ( -Not $? ) {
			exit
		}
	}
	finally {
		Pop-Location
	}
}
