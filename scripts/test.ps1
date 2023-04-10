param([string]$config, [switch]$verbose)

# TODO: Write in C# instead with Process...? Or just unit test in cpp?

if (!($config -eq "Release" -or $config -eq "Debug")) {
	echo "Uknown config: $config. -config [Release|Debug]"
	exit 1
}

if ($verbose.IsPresent) {
	$verboseFlag = "--verbose"
}

$srcDir = $PSScriptRoot
$testDataDir = "$srcDir\..\test-data"
$testCodeDir = "$testDataDir\src"
$testIncludeDirs = "$testCodeDir", "$testDataDir\include\", "$testDataDir\include B\", "$srcDir\..\nuget\include\idlgen"
$idlgen = "$srcDir/../dev/out/build/x64-$config/idlgen.exe"
$testIncludeDirs = $testIncludeDirs.Replace("\", "/")
$includes = $testIncludeDirs | ForEach-Object { "--include=`"$_`"" }
# Test generated output

function gen {
	param([string]$filePath)
	$LASTEXITCODE = 0
	&$idlgen $includes $verboseFlag $filePath --gen
	if ($LASTEXITCODE -ne 0) {
		echo "idlgen returned $LASTEXITCODE"
		exit 1
	}
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
	$actual = ($src.IndexOf($line) -ne -1)
	if (!$actual) {
		echo $src
	}
	assert "`"$line`" exists" -actual $actual
}

function absent {
	param([string]$src, [string]$line)
	$actual = ($src.IndexOf($line) -eq -1)
	if (!$actual) {
		echo $src
	}
	assert "`"$line`" is absent" -actual $actual
}

# Test BlankPage
$blankPageSrc = "$testCodeDir\BlankPage.h"
gen -filePath $blankPageSrc

$blankPageIdlPath = "$testCodeDir\BlankPage.idl"
$blankPageOutput = get-content $blankPageIdlPath

# Import
exists -src $blankPageOutput -line "import `"SameViewModel.idl`";"
exists -src $blankPageOutput -line "import `"ShallowerViewModel.idl`";"
exists -src $blankPageOutput -line "import `"SiblingViewModel.idl`";"
exists -src $blankPageOutput -line "import `"TestIncludeImpl.idl`";"
exists -src $blankPageOutput -line "import `"TestIncludeInTemplate.idl`";"
# Namespace
exists -src $blankPageOutput -line "namespace Root.A"
# Attributes
exists -src $blankPageOutput -line "[default_interface]"
exists -src $blankPageOutput -line "[bindable]"
# Runtime class name
exists -src $blankPageOutput -line "runtimeclass BlankPage"
# Extend
exists -src $blankPageOutput -line "BlankPage : Windows.UI.Xaml.Controls.Page, Windows.UI.Xaml.Data.INotifyPropertyChanged"
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
exists -src $blankPageOutput -line "void MethodOverload(Boolean a);"
exists -src $blankPageOutput -line "void MethodOverload(UInt64 a);"
exists -src $blankPageOutput -line "void MethodOverload(Boolean a, UInt64 b);"
exists -src $blankPageOutput -line "void MethodOverload(UInt64 a, Boolean b);"
exists -src $blankPageOutput -line "void NamespaceSame(Root.A.SameViewModel a);"
exists -src $blankPageOutput -line "void NamespaceShallower(Root.ShallowerViewModel a);"
exists -src $blankPageOutput -line "void NamespaceSibling(Root.B.SiblingViewModel a);"
exists -src $blankPageOutput -line "void NoSetterOnlyProperty(Boolean a);"
exists -src $blankPageOutput -line "void VoidGetterIsGeneratedAsIs{get;};"
exists -src $blankPageOutput -line "static Windows.UI.Xaml.DependencyProperty DependencyProperty{get;};"
exists -src $blankPageOutput -line "String Property;"
exists -src $blankPageOutput -line "Root.A.SameViewModel ReturnAllowImpl{get;};"
exists -src $blankPageOutput -line "Windows.Foundation.Numerics.Vector2 Struct{get;};"
exists -src $blankPageOutput -line "Root.A.TestIncludeImpl TestIncludeImplWithOnlyImplUse{get;};"
exists -src $blankPageOutput -line "String UnqualifiedType{get;};"
exists -src $blankPageOutput -line "event Windows.Foundation.EventHandler Event;"
exists -src $blankPageOutput -line "event Windows.Foundation.TypedEventHandler<Root.A.BlankPage, UInt32> TypedEvent;"
# Stuff that should be hidden
absent -src $blankPageOutput -line "BlankPage(BlankPage that);"
absent -src $blankPageOutput -line "void operator=(BlankPage that);"
absent -src $blankPageOutput -line "~BlankPage();"
absent -src $blankPageOutput -line "void ImplPropertyOnlyExposeGetter(Root.A.SameViewModel a);"
absent -src $blankPageOutput -line "void EventWithConstRefToken(event_token token);"
absent -src $blankPageOutput -line "ImplStruct"
absent -src $blankPageOutput -line "error-type"
absent -src $blankPageOutput -line "void ParamDisallowImpl(Root.A.SameViewModel a);"
absent -src $blankPageOutput -line "Root.A.SameViewModel ParamDisallowImplEvenReturnAllow(Root.A.SameViewModel a);"
absent -src $blankPageOutput -line "void MethodMixingImplAndProjected(Root.A.SameViewModel a, Root.A.SameViewModel b);"
absent -src $blankPageOutput -line "void PrivateMethod();"
absent -src $blankPageOutput -line "void HideMethod();"
absent -src $blankPageOutput -line "Root.A.factory_implementation";
absent -src $blankPageOutput -line "warning";

$sameVmSrc = "$testCodeDir\SameViewModel.h"
gen -filePath $sameVmSrc

$sameVmIdlPath = "$testCodeDir\SameViewModel.idl"
$sameVmOutput = get-content $sameVmIdlPath

exists -src $sameVmOutput -line "[default_interface]"
exists -src $sameVmOutput -line "runtimeclass SameViewModel"
absent -src $sameVmOutput -line "runtimeclass SameViewModelHide"

$someEnumSrc = "$testCodeDir\SomeEnum.h"
gen -filePath $someEnumSrc

$someEnumIdlPath = "$testCodeDir\SomeEnum.idl"
$someEnumOutput = get-content $someEnumIdlPath

exists -src $someEnumOutput -line "enum SomeEnum"
exists -src $someEnumOutput -line "Active = 0,"
exists -src $someEnumOutput -line "InActive = 1,"
exists -src $someEnumOutput -line "Unknown = 2,"
exists -src $someEnumOutput -line "};"

$someFlagSrc = "$testCodeDir\SomeFlag.h"
gen -filePath $someFlagSrc

$someFlagIdlPath = "$testCodeDir\SomeFlag.idl"
$someFlagOutput = get-content $someFlagIdlPath

exists -src $someFlagOutput -line "[flags]"
exists -src $someFlagOutput -line "enum SomeFlag"
exists -src $someFlagOutput -line "Camera = 0x00000001,"
exists -src $someFlagOutput -line "Microphone = 0x00000002,"
exists -src $someFlagOutput -line "};"

echo "All test passed"

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
