$srcDir = $PSScriptRoot
$testDataDir = "$srcDir\..\test-data"
$testCodeDir = "$testDataDir\src"
$testIncludeDirs = "$testDataDir\include\", "$testDataDir\include B\"
$idlgen = "$srcDir/../dev/out/build/x64-Debug/idlgen.exe"
$testCodes = get-childitem -path $testCodeDir
$testIncludeDirs = $testIncludeDirs.Replace("\", "/")
$includes = $testIncludeDirs | ForEach-Object { "--include=`"$_`"" }
foreach($code in $testCodes) {
	if ($code.PSisContainer) {
		continue
	}
	&$idlgen $includes $code.FullName
}
