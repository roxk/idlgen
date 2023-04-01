param(
    [boolean]$buildClang,
    [string]$config
)

if (!($config -eq "Release" -or $config -eq "Debug")) {
	echo "Uknown config: $config. -config [Release|Debug]"
	exit 1
}

if ($buildClang) {
    . "$PSScriptRoot\populate-build-ninja" -config $config
    . "$PSScriptRoot\build-clang" -config $config
}
$srcDir = "$PSScriptRoot"
$projectDir = "$srcDir\..\dev"
$outDir = "$projectDir\out\build\x64-$config"
if (!(test-path $outDir)) {
    new-item -path $outDir -itemtype Directory
}
. "$srcDir\get-vcvars"
cmd /c "`"$vcvars`" && cd `"$outDir`" && cmake -G Ninja -DCMAKE_BUILD_TYPE=$config `"$projectDir`" && ninja"
