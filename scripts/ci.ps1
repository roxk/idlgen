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
    $localPackagesDir = "$PSScriptRoot\..\sample-app\LocalPackages"
	$nugetDest = "$localPackagesDir\IdlGen.IdlGen.Cpp.0.0.1.nupkg"
	if (test-path $nugetDest) {
	    remove-item $nugetDest
	}
	if (!(test-path $localPackagesDir)) {
		new-item $localPackagesDir -itemtype Directory
	}
	$packagesDir = "$PSScriptRoot\..\sample-app\packages"
	if (test-path $packagesDir) {
		remove-item $packagesDir -recurse
	}
	copy-item "$PSScriptRoot\..\nuget\IdlGen.IdlGen.Cpp.0.0.1.nupkg" $nugetDest
	. "$PSScriptRoot\test-sample-app"
}
run -func {
	. "$PSScriptRoot\build-extension" -config Debug
}
