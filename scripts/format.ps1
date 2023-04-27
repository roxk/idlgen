$scriptDir = $PSScriptRoot
$files = get-childitem -path "$scriptDir\..\dev" -recurse | where { $_.extension -in ".h", ".cpp" -and !$_.FullName.Contains('out') } | select -expandproperty FullName
foreach ($file in $files) {
	clang-format -i $file
}
