$ErrorActionPreference = "Stop"
$scriptDir = $PSScriptRoot
$sampleAppDir = "$scriptDir\..\sample-app"
nuget restore $sampleAppDir
function build {
	param([switch]$clean)
	if ($clean.IsPresent) {
		$cleanFlags = "-t:clean"
	}
	msbuild $sampleAppDir $cleanFlags -t:build -p:Configuration=Debug -p:Platform=x64 | out-host
}
# Test clean build
build -clean
# Test that modifying 1 header doesn't re-compile everything
$blankPageObj = get-childitem "$sampleAppDir\SampleApp\x64\Debug\BlankPage.obj"
$blankPageObjLastWriteTime = $blankPageObj.LastWriteTime
$mainPageHeader = "$sampleAppDir\SampleApp\MainPage.h"
copy-item $mainPageHeader "$mainPageHeader.bak"
try {
	add-content $mainPageHeader -value ""
	build
	$blankPageObj = get-childitem "$sampleAppDir\SampleApp\x64\Debug\BlankPage.obj"
	$blankPageObjNewLastWriteTime = $blankPageObj.LastWriteTime
	if ($blankPageObjLastWriteTime -ne $blankPageObjNewLastWriteTime) {
		write-error "Test failed: BlankPage.obj is modified when only MainPage.h is updated."
		exit 1
	} else {
		echo "Test passed: BlankPage.obj isn't updated when only MainPage.h is updated. "
	}
} finally {
	remove-item $mainPageHeader
	move-item "$mainPageHeader.bak" $mainPageHeader
}
