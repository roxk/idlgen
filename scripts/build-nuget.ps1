param(
    [parameter(Mandatory=$true)]
    [string]$version,
    [boolean]$buildClang
)
$template = Get-Content $PSScriptRoot\..\nuget\IdlGen.IdlGen.Cpp.nuspec.template
$nuspec = $template.Replace("VERSION", $version)
Set-Content -path $PSScriptRoot\..\nuget\.nuspec -value $nuspec
. "$PSScriptRoot\build-idlgen.ps1" -config Release -buildClang $buildClang
nuget pack $PSScriptRoot\..\nuget\.nuspec -outputDirectory $PSScriptRoot\..\nuget
