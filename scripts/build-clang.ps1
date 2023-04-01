param([string]$config)

if (!($config -eq "Release" -or $config -eq "Debug")) {
	echo "Uknown config: $config. -config [Release|Debug]"
	exit 1
}

$scriptDir = $PSScriptRoot
$rootDir = "$scriptDir/.."
$ninjaBuildDir = "$rootDir/build-ninja/$config"
if (!(test-path $ninjaBuildDir)) {
	echo "build-ninja doesn't 'exist. Run populdate-build-ninja.ps1 first"
	exit 0
}
cd $ninjaBuildDir
cmd /c "vcvars64.bat && ninja"
cd ..
