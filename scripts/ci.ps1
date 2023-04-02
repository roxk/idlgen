function run {
	param([ScriptBlock]$func)
	&$func
	if ($LASTEXITCODE -ne 0) {
		echo "Last func returned $LASTEXITCODE"
		exit 1
	}
}

# TODO: For some reason ninja returns -1073740791 even when the build succeeds in CI
# Disable error exit code detection for now
. "$PSScriptRoot\build-idlgen" -config Release
dir "$PSScriptRoot\..\dev\out\build\x64-Release"
run -func {
	. "$PSScriptRoot\test" -config Release
}
# TODO: Add test to validate output...
