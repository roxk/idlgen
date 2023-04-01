param(
    [Parameter(Mandatory=$true)]
    [String]$version
)
nuget setApiKey $env:NUGET_API_KEY
nuget push $PSScriptRoot\..\nuget\IdlGen.IdlGen.Cpp.$version.nupkg -Source https://api.nuget.org/v3/index.json
