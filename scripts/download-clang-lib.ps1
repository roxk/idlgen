$srcDir = "$PSScriptRoot"
. "$srcDir\populate-build-ninja" -config Release
$outRootDir = "$srcDir\..\build-ninja\Release"
$zipFile = "$outRootDir\lib.zip"
if (!(test-path $zipFile)) {
	$ProgressPreference = 'SilentlyContinue'
	$url = "https://www.dropbox.com/s/4wf0851vwpvj2p7/clang-idlgen-lib-release-win-64.zip?dl=1"
	Invoke-WebRequest $url -OutFile $zipFile
}
Expand-Archive $zipFile -DestinationPath $outRootDir -force
