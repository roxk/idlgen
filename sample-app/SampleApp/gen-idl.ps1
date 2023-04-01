param([string]$file, [string]$includeDirs)

$idlgen = "./idlgen.exe"
if ($includeDirs -ne $null) {
	$includeDirs = $includeDirs.Replace(";",",")
	$includeDirs = $includeDirs.Replace("\", "/")
	$includeDirArrays = $includeDirs -split "," | where { $_ -ne "" }
	$includes = $includeDirArrays | ForEach-Object { "--include=`"$_`"" }
}
echo $file
echo $includes
echo "$includes $file"
&$idlgen $includes $file
