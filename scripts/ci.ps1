function run {
	param([ScriptBlock]$func)
	&$func
	if ($LASTEXITCODE -ne 0) {
		exit $LASTEXITCODE
	}
}
run -func {
	. "$PSScriptRoot\format" -check
}
run -func {
	. "$PSScriptRoot\build-idlgen" -config Release
}
run -func {
	. "$PSScriptRoot\test" -config Release
}
run -func {
	. "$PSScriptRoot\build-nuget" -version 0.0.1
	$nugetDest = "$PSScriptRoot\..\sample-app\LocalPackages\IdlGen.IdlGen.Cpp.0.0.1.nupkg"
	if (test-path $nugetDest) {
		remove-item $nugetDest
	}
	copy-item "$PSScriptRoot\..\nuget\IdlGen.IdlGen.Cpp.0.0.1.nupkg" $nugetDest
	. "$PSScriptRoot\test-sample-app"
}
run -func {
	. "$PSScriptRoot\build-extension" -config Debug
}
