param([string]$config)

if (!($config -eq "Release" -or $config -eq "Debug")) {
	echo "Uknown config: $config. -config [Release|Debug]"
	exit 1
}

$originalDir = $(get-location)
$scriptDir = $PSScriptRoot
$rootDir = "$scriptDir/.."
$ninjaBuildDir = "$rootDir/build-ninja/$config"
if (test-path $ninjaBuildDir) {
	echo "build-ninja exists. Nothing to do"
	exit 0
}
new-item -path $ninjaBuildDir -itemtype Directory
cd $ninjaBuildDir
$pythonPath=$(where.exe python)
echo "pythonPath=$pythonPath"
cmd /c "vcvars64.bat && cmake -G Ninja '-DPython3_EXECUTABLE=$pythonPath' -DCMAKE_BUILD_TYPE=$config -DLLVM_ENABLE_PROJECTS=clang ../../llvm-project/llvm"
cd $originalDir
