param(
	[switch]$check
)
$scriptDir = $PSScriptRoot
$files = get-childitem -path "$scriptDir\..\dev" -recurse | where { $_.extension -in ".h", ".cpp" -and !$_.FullName.Contains('out') } | select -expandproperty FullName
if ($check.IsPresent) {
	$checkFlag = @("--Werror", "--dry-run", "--ferror-limit=1")
}
foreach ($file in $files) {
	clang-format -i $file $checkFlag
	if ($LASTEXITCODE -ne 0) {
		exit $LASTEXITCODE
	}
}
if ($checkFlag) {
	echo "All files are formatted"
}
