$originalDir = $(get-location)
$scriptDir = $PSScriptRoot
$rootDir = "$scriptDir/.."
$ninjaBuildDir = "$rootDir/build-ninja"
if (!(test-path $ninjaBuildDir)) {
	echo "build-ninja doesn't 'exist. Run populdate-build-ninja.ps1 first"
	exit 0
}
cd $ninjaBuildDir
cmd /c "vcvars64.bat && ninja"
cd ..
