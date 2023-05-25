$srcDir = "$PSScriptRoot"
. "$srcDir\populate-build-ninja" -config Release
$outRootDir = "$srcDir\..\build-ninja\Release"
$zipFile = "$outRootDir\lib.zip"
if (!(test-path $zipFile)) {
	$ProgressPreference = 'SilentlyContinue'
	$url = "https://www.dropbox.com/s/mxb2hez53j21vf7/clang-lib-release-win-x64.zip?dl=1"
	Invoke-WebRequest $url -OutFile $zipFile
}
Expand-Archive $zipFile -DestinationPath $outRootDir -force
