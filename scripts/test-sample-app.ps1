param(
	[switch]$resetNuget
)
$ErrorActionPreference = "Stop"
if ($resetnuget.IsPresent) {
	$localPackagesDir = "$PSScriptRoot\..\sample-app\LocalPackages"
	$nugetDest = "$localPackagesDir\IdlGen.IdlGen.Cpp.0.0.1.nupkg"
	if (test-path $nugetDest) {
	    remove-item $nugetDest
	}
	if (!(test-path $localPackagesDir)) {
		new-item $localPackagesDir -itemtype Directory
	}
	$packagesDir = "$PSScriptRoot\..\sample-app\packages"
	if (test-path $packagesDir) {
		remove-item $packagesDir -recurse
	}
	copy-item "$PSScriptRoot\..\nuget\IdlGen.IdlGen.Cpp.0.0.1.nupkg" $nugetDest
}
$tasks = "$PSScriptRoot\test-sample-app-tasks"
powershell -c "Import-Module `"`"`"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll`"`"`";`
Enter-VsDevShell d5206e10 -SkipAutomaticLocation -DevCmdArguments `"`"`"-arch=x64 -host_arch=x64`"`"`";&$tasks"
