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
}
run -func {
	. "$PSScriptRoot\test-sample-app" -resetNuget
}
run -func {
	. "$PSScriptRoot\build-extension" -config Debug
}
