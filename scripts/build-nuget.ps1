param(
    [parameter(Mandatory=$true)]
    [string]$version
)
$template = Get-Content $PSScriptRoot\..\nuget\IdlGen.IdlGen.Cpp.nuspec.template
$nuspec = $template.Replace("VERSION", $version)
Set-Content -path $PSScriptRoot\..\nuget\.nuspec -value $nuspec
$srcDir = "$PSScriptRoot"
$projectDir = "$srcDir\..\dev"
$outDir = "$projectDir\out\build\x64-Release"
. "$PSScriptRoot\get-msbuild"
. "$PSScriptRoot\populate-build-ninja" -config Release
. "$PSScriptRoot\build-clang" -config Release
if (!(test-path $outDir)) {
    new-item -path $outDir -itemtype Directory
}
cmd /c "vcvars64.bat && cd `"$outDir`" && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release `"$projectDir`" && ninja"
nuget pack $PSScriptRoot\..\nuget\.nuspec -outputDirectory $PSScriptRoot\..\nuget
