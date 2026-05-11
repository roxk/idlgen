param(
    [parameter(Mandatory=$true)]
    [string]$version,
    [bool]$testPackage
)
$template = Get-Content $PSScriptRoot\..\nuget\IdlGen.IdlGen.Cpp.nuspec.template
$nuspec = $template.Replace("VERSION", $version)
Set-Content -path $PSScriptRoot\..\nuget\.nuspec -value $nuspec
$nugetOutDir = "$PSScriptRoot\..\nuget"
if ($testPackage) {
    $nugetOutDir = "$PSScriptRoot\..\LocalPackages"
}
nuget pack $PSScriptRoot\..\nuget\.nuspec -outputDirectory $nugetOutDir
