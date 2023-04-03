param([string]$config)

$srcDir = "$PSScriptRoot"
. "$srcDir\populate-build-ninja" -config Release
$outRootDir = "$srcDir\..\build-ninja\Release"
$zipFile = "$outRootDir\lib.zip"
if (!(test-path $zipFile)) {
	$ProgressPreference = 'SilentlyContinue'
	$url = "https://www.dropbox.com/s/p7tk5xdrtjnr744/clang-lib-release-win-x64.zip?dl=1"
	Invoke-WebRequest $url -OutFile $zipFile
}
Expand-Archive $zipFile -DestinationPath $outRootDir -force
