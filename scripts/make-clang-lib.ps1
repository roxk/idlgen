$srcDir = "$PSScriptRoot"
$releaseDir = "$srcDir\..\build-ninja\Release"
$includeDir = "$releaseDir\include"
$toolsIncludeDir = "$releaseDir\tools\clang\include"
$clangLibDir = "$releaseDir\lib"
$destinationZip = "$releaseDir\clang-idlgen-lib-release-win-64.zip"
if (test-path "$destinationZip") {
	remove-item "$destinationZip"
}
Compress-Archive -Path "$includeDir" -DestinationPath "$destinationZip"
$zipFile = [System.IO.Compression.ZipFile]::Open("$destinationZip", [System.IO.Compression.ZipArchiveMode]::Update)
$toolsIncludeFiles = get-childitem -path $toolsIncludeDir -Recurse
$clangLibFiles = get-childitem -path $clangLibDir -recurse -include clang*.lib, `
	LLVMAnalysis.lib, `
	LLVMAsmParser.lib, `
	LLVMBinaryFormat.lib, `
	LLVMBitReader.lib, `
	LLVMBitstreamReader.lib , `
	LLVMCore.lib , `
	LLVMDebugInfoDWARF.lib , `
	LLVMDemangle.lib , `
	LLVMFrontendOpenMP.lib , `
	LLVMIRReader.lib , `
	LLVMMC.lib , `
	LLVMMCParser.lib , `
	LLVMObject.lib , `
	LLVMOption.lib , `
	LLVMProfileData.lib , `
	LLVMRemarks.lib , `
	LLVMScalarOpts.lib , `
	LLVMSupport.lib , `
	LLVMTarget.lib , `
	LLVMTargetParser.lib , `
	LLVMTextAPI.lib , `
	LLVMTransformUtils.lib , `
	LLVMWindowsDriver.lib , `
	LLVMWindowsManifest.lib
$fullReleaseDir = [System.IO.Path]::GetFullPath($releaseDir)
function includeFiles {
	param(
		$files
	)
	foreach ($file in $files) {
		if ($file.PSIsContainer) {
			continue
		}
		$fileName = $file.FullName
		echo "adding $fileName"
		$fileName = $fileName.Replace("$fullReleaseDir\", "")
		$toolsIncludeEntry = $zipFile.CreateEntry($fileName)
		$toolsIncludeDstStream = $toolsIncludeEntry.Open()
		try {
			$toolsIncludeSrcStream = [System.IO.File]::Open($file.FullName, [System.IO.FileMode]::Open)
			try {
				$toolsIncludeSrcStream.CopyTo($toolsIncludeDstStream)
			} finally {
				$toolsIncludeSrcStream.Dispose()
			}
			$toolsIncludeDstStream.Flush()
		} finally {
			$toolsIncludeDstStream.Dispose()
		}
	}
}
try {
	includeFiles -files $toolsIncludeFiles
	includeFiles -files $clangLibFiles
} finally {
	$zipFile.Dispose()
}
