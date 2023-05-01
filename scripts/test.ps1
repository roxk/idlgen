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
	push-location $testCodeDir
	&$idlgen $includes $verboseFlag $filePath --gen | out-host
	pop-location
	if ($LASTEXITCODE -ne 0) {
		echo "idlgen returned $LASTEXITCODE"
		exit 1
	}
}

function get-gen-output {
	param([string]$filePath)
	$idlPath = $filePath.Replace(".h", ".idl")
	if (test-path $idlPath) {
		remove-item $idlPath
	}
	gen -filePath $filePath
	$idlPath = $filePath.Replace(".h", ".idl")
	return get-content $idlPath
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
# TODO: Rewrite each test case as lambda so we can write test-gen-output("path", (output) -> { exists -src $output })
$blankPageOutput = get-gen-output "$testCodeDir\BlankPage.h"
# Import
exists -src $blankPageOutput -line "import `"SameViewModel.idl`";"
exists -src $blankPageOutput -line "import `"ShallowerViewModel.idl`";"
exists -src $blankPageOutput -line "import `"SiblingViewModel.idl`";"
exists -src $blankPageOutput -line "import `"SomeNamespace\DifferentPathViewModel.idl`";"
exists -src $blankPageOutput -line "import `"TestIncludeImpl.idl`";"
exists -src $blankPageOutput -line "import `"TestIncludeInTemplate.idl`";"
# Namespace
exists -src $blankPageOutput -line "namespace Root.A"
# Attributes
exists -src $blankPageOutput -line "[default_interface]"
exists -src $blankPageOutput -line "[bindable]"
exists -src $blankPageOutput -line "[Windows.UI.Xaml.Markup.ContentProperty(`"Property`")]"
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
exists -src $blankPageOutput -line "void MethodConstRefBool(Boolean a);"
exists -src $blankPageOutput -line "void MethodConst(String a);"
exists -src $blankPageOutput -line "void MethodFloat(Single a, Double b);"
exists -src $blankPageOutput -line "void MethodConstRefFloat(Single a, Double b);"
exists -src $blankPageOutput -line "void MethodInt(Int32 a, Int32 b, Int16 c, Int32 d, Int64 e);"
exists -src $blankPageOutput -line "void MethodConstRefInt(Int32 a, Int32 b, Int16 c, Int32 d, Int64 e);"
exists -src $blankPageOutput -line "void MethodObject(Object a);"
exists -src $blankPageOutput -line "void MethodDateTime(Windows.Foundation.DateTime a);"
exists -src $blankPageOutput -line "void MethodTimeSpan(Windows.Foundation.TimeSpan a);"
exists -src $blankPageOutput -line "void MethodPure();"
exists -src $blankPageOutput -line "void MethodUInt(UInt8 a, UInt16 b, UInt32 c, UInt64 d);"
exists -src $blankPageOutput -line "void MethodConstRefUInt(UInt8 a, UInt16 b, UInt32 c, UInt64 d);"
exists -src $blankPageOutput -line "void MethodOverload(Boolean a);"
exists -src $blankPageOutput -line "void MethodOverload(UInt64 a);"
exists -src $blankPageOutput -line "void MethodOverload(Boolean a, UInt64 b);"
exists -src $blankPageOutput -line "void MethodOverload(UInt64 a, Boolean b);"
exists -src $blankPageOutput -line "void MethodOverriden(UInt32 a);"
exists -src $blankPageOutput -line "void MethodEnum(Root.A.TestEnumInMethod a);"
exists -src $blankPageOutput -line "void MethodConstRefEnum(Root.A.TestEnumInMethod a);"
exists -src $blankPageOutput -line "Windows.Foundation.IAsyncOperation<Windows.Foundation.TypedEventHandler<UInt32, UInt32> > MethodTemplateInTemplate();"
exists -src $blankPageOutput -line "void NamespaceSame(Root.A.SameViewModel a);"
exists -src $blankPageOutput -line "void NamespaceShallower(Root.ShallowerViewModel a);"
exists -src $blankPageOutput -line "void NamespaceSibling(Root.B.SiblingViewModel a);"
exists -src $blankPageOutput -line "void NoSetterOnlyProperty(Boolean a);"
exists -src $blankPageOutput -line "void VoidGetterIsMethod();"
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
absent -src $blankPageOutput -line "Root.A.factory_implementation"
absent -src $blankPageOutput -line "warning"

$propertyBagOutput = get-gen-output "$testCodeDir\PropertyBag.h"
exists -src $propertyBagOutput -line "runtimeclass PropertyBag"
exists -src $propertyBagOutput -line "UInt32 UInt32Prop;"
exists -src $propertyBagOutput -line "Root.A.SameViewModel ClassProp{get;};"
exists -src $propertyBagOutput -line "void Method(UInt32 a, UInt32 b);"
exists -src $propertyBagOutput -line "Boolean IsErrored{get;};"
exists -src $propertyBagOutput -line "Boolean IsLoading{get;};"
exists -src $propertyBagOutput -line "Boolean IsIdle{get;};"
exists -src $propertyBagOutput -line "Windows.Foundation.IAsyncAction MethodAsync();"
exists -src $propertyBagOutput -line "void MethodPure();"
exists -src $propertyBagOutput -line "void MethodBool(Boolean a);"
exists -src $propertyBagOutput -line "Boolean CppXamlProperty;"

$sameVmOutput = get-gen-output "$testCodeDir\SameViewModel.h"
exists -src $sameVmOutput -line "[default_interface]"
exists -src $sameVmOutput -line "runtimeclass SameViewModel"
absent -src $sameVmOutput -line "runtimeclass SameViewModelHide"

$differentPathVmOutput = get-gen-output -filePath "$testCodeDir\SomeNamespace\DifferentPathViewModel.h"
exists -src $differentPathVmOutput -line "[default_interface]"
exists -src $differentPathVmOutput -line "runtimeclass DifferentPathViewModel"

$differentPathConsumerVmOutput = get-gen-output -filePath "$testCodeDir\SomeNamespace\DifferentPathConsumerViewModel.h"
exists -src $differentPathConsumerVmOutput -line "runtimeclass DifferentPathConsumerViewModel"
exists -src $differentPathConsumerVmOutput -line "import `"SomeNamespace\DifferentPathViewModel.idl`";"

$someEnumOutput = get-gen-output -filePath "$testCodeDir\SomeEnum.h"
exists -src $someEnumOutput -line "enum SomeEnum"
exists -src $someEnumOutput -line "Active = 0,"
exists -src $someEnumOutput -line "InActive = 1,"
exists -src $someEnumOutput -line "Unknown = 2,"
exists -src $someEnumOutput -line "};"

$someFlagOutput = get-gen-output "$testCodeDir\SomeFlag.h"
exists -src $someFlagOutput -line "[flags]"
exists -src $someFlagOutput -line "enum SomeFlag"
exists -src $someFlagOutput -line "Camera = 0x00000001,"
exists -src $someFlagOutput -line "Microphone = 0x00000002,"
exists -src $someFlagOutput -line "};"

$someStructOutput = get-gen-output "$testCodeDir\SomeStruct.h"
exists -src $someStructOutput -line "struct SomeStruct"
exists -src $someStructOutput -line "Int64 X;"
exists -src $someStructOutput -line "Int64 Y;"
exists -src $someStructOutput -line "};"

$someDelegateOutput = get-gen-output "$testCodeDir\SomeDelegate.h"
exists -src $someDelegateOutput -line "delegate void SomeEventHandler(Root.ShallowerViewModel vm, UInt64 e);"

$nonWinRtHeaderSrc = "$testCodeDir\NonWinRtHeader.h"
gen -filePath $nonWinRtHeaderSrc
$nonWinRtHeaderIdlPath = "$testCodeDir\NonWinRtHeader.idl"
if (test-path $nonWinRtHeaderIdlPath) {
	$doesWinRtHeaderIdlExist = $true
} else {
	$doesWinRtHeaderIdlExist = $false
}
assert "Non WinRT header doesn't generate idl" -actual ($doesWinRtHeaderIdlExist -eq $false)

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
