param(
    [boolean]$buildClang,
    [string]$config
)

if (!($config -eq "Release" -or $config -eq "Debug")) {
	echo "Uknown config: $config. -config [Release|Debug]"
	exit 1
}

$srcDir = "$PSScriptRoot"
$extensionProjectDir = "$srcDir\..\extension\IdlGen.Cpp-Vs-2022\IdlGen.Cpp\IdlGen.Cpp.csproj"
msbuild $extensionProjectDir -restore
msbuild $extensionProjectDir -p:Configuration=$config
