param([string]$config)

# TODO: Write in C# instead with Process...? Or just unit test in cpp?

if (!($config -eq "Release" -or $config -eq "Debug")) {
	echo "Uknown config: $config. -config [Release|Debug]"
	exit 1
}

$srcDir = $PSScriptRoot
$testDataDir = "$srcDir\..\test-data"
$testCodeDir = "$testDataDir\src"
$testIncludeDirs = "$testDataDir\include\", "$testDataDir\include B\"
$idlgen = "$srcDir/../dev/out/build/x64-$config/idlgen.exe"
$blankPageSrc = "$testCodeDir\BlankPage.h"
$testIncludeDirs = $testIncludeDirs.Replace("\", "/")
$includes = $testIncludeDirs | ForEach-Object { "--include=`"$_`"" }
# Test generated output

function genToStdOut {
	param([string]$filePath)
	&$idlgen $includes $filePath
}

function expect {
	param([string]$desc, [string]$expected, [string]$actual)
	if ($expected -ne $actual) {
		echo "Test failed: $desc"
		echo "Expected $expected, got $actual"
		exit 1
	}
}

function assert {
	param([string]$desc, [boolean]$actual)
	if (!$actual) {
		echo "Assertion failed: $desc"
		exit 1
	}
}

function exists {
	param([string]$src, [string]$line)
	assert "$line exists" -actual ($src.IndexOf($line) -ne -1)
}

function absent {
	param([string]$src, [string]$line)
	assert "$line absent" -actual ($src.IndexOf($line) -eq -1)
}

# Test BlankPage
$blankPageOutput = genToStdOut -filePath $blankPageSrc
$blankPageOutput = $blankPageOutput -join "`n"
echo $blankPageOutput
# Import
exists -src $blankPageOutput -line "import `"SameViewModel.idl`";"
exists -src $blankPageOutput -line "import `"ShallowerViewModel.idl`";"
exists -src $blankPageOutput -line "import `"SiblingViewModel.idl`";"
# Namespace
exists -src $blankPageOutput -line "namespace Root.A"
# Attributes
exists -src $blankPageOutput -line "[default_interface]"
exists -src $blankPageOutput -line "[bindable]"
# Runtime class name
exists -src $blankPageOutput -line "runtimeclass BlankPage"
# Extend
exists -src $blankPageOutput -line "BlankPage : Windows.UI.Xaml.Page, Windows.UI.Xaml.Data.INotifyPropertyChanged"
# Constructors
exists -src $blankPageOutput -line "BlankPage()"
exists -src $blankPageOutput -line "BlankPage()"
exists -src $blankPageOutput -line "BlankPage(UInt64 a, UInt64 b)"
# Enum
exists -src $blankPageOutput -line "Root.A.Category Enum{get;};"
# Methods
exists -src $blankPageOutput -line "Boolean Getter{get;};"
exists -src $blankPageOutput -line "Root.A.SameViewModel ImplPropertyOnlyExposeGetter{get;};"
exists -src $blankPageOutput -line "Boolean Method(Boolean a);"
exists -src $blankPageOutput -line "void MethodBool(Boolean a);"
exists -src $blankPageOutput -line "void MethodConst(String a);"
exists -src $blankPageOutput -line "void MethodFloat(Single a, Double b);"
exists -src $blankPageOutput -line "void MethodInt(Int32 a, Int32 b, Int16 c, Int32 d, Int64 e);"
exists -src $blankPageOutput -line "void MethodObject(Object a);"
exists -src $blankPageOutput -line "void MethodPure();"
exists -src $blankPageOutput -line "void MethodUInt(UInt8 a, UInt16 b, UInt32 c, UInt64 d);"
exists -src $blankPageOutput -line "void NamespaceSame(Root.A.SameViewModel a);"
exists -src $blankPageOutput -line "void NamespaceShallower(Root.ShallowerViewModel a);"
exists -src $blankPageOutput -line "void NamespaceSibling(Root.B.SiblingViewModel a);"
exists -src $blankPageOutput -line "void NoSetterOnlyProperty(Boolean a);"
exists -src $blankPageOutput -line "static Windows.UI.Xaml.DependencyProperty DependencyProperty{get;};"
exists -src $blankPageOutput -line "String Property;"
exists -src $blankPageOutput -line "Root.A.SameViewModel ReturnAllowImpl{get;};"
exists -src $blankPageOutput -line "Windows.Foundation.Numerics.Vector2 Struct{get;};"
exists -src $blankPageOutput -line "String UnqualifiedType{get;};"
exists -src $blankPageOutput -line "event Windows.Foundation.EventHandler Event;"
# Stuff that should be hidden
absent -src $blankPageOutput -line "BlankPage(BlankPage that);"
absent -src $blankPageOutput -line "void operator=(BlankPage that);"
absent -src $blankPageOutput -line "~BlankPage();"
absent -src $blankPageOutput -line "void ImplPropertyOnlyExposeGetter(Root.A.SameViewModel a);"
absent -src $blankPageOutput -line "ImplStruct"
absent -src $blankPageOutput -line "error-type"
absent -src $blankPageOutput -line "void ParamDisallowImpl(Root.A.SameViewModel a);"
absent -src $blankPageOutput -line "Root.A.SameViewModel ParamDisallowImplEvenReturnAllow(Root.A.SameViewModel a);"
absent -src $blankPageOutput -line "void MethodMixingImplAndProjected(Root.A.SameViewModel a, Root.A.SameViewModel b);"
absent -src $blankPageOutput -line "void PrivateMethod();"

#foreach($code in $testCodes) {
#	$out = $filePath.Replace(".h","")
#	$out += ".testidl"
#	if ($gen -eq "default") {
#		$genArgs = "--gen"
#	} elseif ($gen -eq "custom") {
#		$genArgs = "--gen"
#		$genOutArgs = "--gen-out=`"$out`""
#	}
#	&$idlgen $includes $filePath $genArgs $genOutArgs
#}
