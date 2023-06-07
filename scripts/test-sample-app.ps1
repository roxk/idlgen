$ErrorActionPreference = "Stop"
$scriptDir = $PSScriptRoot
$sampleAppDir = "$scriptDir\..\sample-app"
nuget restore $sampleAppDir
$buildCmd = "msbuild $sampleAppDir -t:clean -t:build -p:Configuration=Debug -p:Platform=x64 | out-host"
powershell -c "Import-Module `"`"`"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Microsoft.VisualStudio.DevShell.dll`"`"`";`
Enter-VsDevShell d5206e10 -SkipAutomaticLocation -DevCmdArguments `"`"`"-arch=x64 -host_arch=x64`"`"`";$buildCmd"
