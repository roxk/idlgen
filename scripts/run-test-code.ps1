param([string]$config, [string]$gen)

if (!($config -eq "Release" -or $config -eq "Debug")) {
	echo "Uknown config: $config. -config [Release|Debug]"
	exit 1
}

if (!($gen -eq "stdout" -or $gen -eq "default" -or $gen -eq "custom"))
{
	echo "Unknown gen: $gen. -config [stdout|default|custom]"
	exit 1
}

$srcDir = $PSScriptRoot
$testDataDir = "$srcDir\..\test-data"
$testCodeDir = "$testDataDir\src"
$testIncludeDirs = "$testDataDir\include\", "$testDataDir\include B\"
$idlgen = "$srcDir/../dev/out/build/x64-$config/idlgen.exe"
$testCodes = get-childitem -path $testCodeDir
$testIncludeDirs = $testIncludeDirs.Replace("\", "/")
$includes = $testIncludeDirs | ForEach-Object { "--include=`"$_`"" }
foreach($code in $testCodes) {
	if ($code.PSisContainer) {
		continue
	}
	$filePath = $code.FullName
	$headerExtensionIndex = $filePath.IndexOf(".h")
	if ($headerExtensionIndex -eq -1 -or $headerExtensionIndex -ne $filePath.Length - 2) {
		continue
	}
	$out = $filePath.Replace(".h","")
	$out += ".testidl"
	if ($gen -eq "default") {
		$genArgs = "--gen"
	} elseif ($gen -eq "custom") {
		$genArgs = "--gen"
		$genOutArgs = "--gen-out=`"$out`""
	}
	&$idlgen $includes $filePath $genArgs $genOutArgs
}
