#
# make-workdir.ps1
#

$CMakeDir = Join-Path -Path $Env:ProgramFiles -ChildPath 'CMake'
$CMakeBinDir = Join-Path -Path $CMakeDir -ChildPath 'bin'
$CMake = Join-Path -Path $CMakeBinDir -ChildPath 'cmake'

$ScriptPath = Join-Path -Path $PSScriptRoot -ChildPath 'make-workdir.cmake'

& $CMake -P $ScriptPath
