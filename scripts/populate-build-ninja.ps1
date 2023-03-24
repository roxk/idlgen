$originalDir = $(get-location)
$scriptDir = $PSScriptRoot
$rootDir = "$scriptDir/.."
$ninjaBuildDir = "$rootDir/build-ninja"
if (test-path $ninjaBuildDir) {
	echo "build-ninja exists. Nothing to do"
	exit 0
}
new-item -path $ninjaBuildDir -itemtype Directory
cd $ninjaBuildDir
$pythonPath=$(where.exe python)
echo "pythonPath=$pythonPath"
cmd /c "vcvars64.bat && cmake -G Ninja '-DPython3_EXECUTABLE=$pythonPath' -DLLVM_ENABLE_PROJECTS=clang ../llvm-project/llvm"
cd $originalDir
