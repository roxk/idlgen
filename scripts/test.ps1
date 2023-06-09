param([string]$config, [switch]$verbose)
$ErrorActionPreference = "Stop"

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
$rootNamespace = "Root"
$idlgen = "$srcDir/../dev/out/build/x64-$config/idlgen.exe"
$getterTemplates = @("wil::single_threaded_property")
$propertyTemplates = @("wil::single_threaded_rw_property")
$pch = "pch.h"
$pchOutDir = "$testDataDir\out"
$testIncludeDirs = $testIncludeDirs.Replace("\", "/")
$includes = $testIncludeDirs | ForEach-Object { "--include=`"$_`"" }
$getterTemplatesFlags = $getterTemplates | ForEach-Object { "--getter-template=`"$_`"" }
$propertyTemplatesFlags = $propertyTemplates | ForEach-Object { "--property-template=`"$_`"" }
$pchFlags = "--pch=`"$pch`""
$pchOutDirFlags = "--pch-out-dir=$pchOutDir"

function gen {
	param([string]$filePath, [switch]$genPch, [switch]$bootstrap)
	$LASTEXITCODE = 0
	push-location $testCodeDir
	if ($genPch.IsPresent) {
		$genPchFlags = "--gen-pch"
	}
	if ($bootstrap.IsPresent) {
		$genBootstrapFlags = "--gen-bootstrap"
	}
	&$idlgen $includes $verboseFlag $filePath --gen $getterTemplatesFlags $propertyTemplatesFlags $pchFlags $pchOutDirFlags $genPchFlags $genBootstrapFlags | out-host
	pop-location
	if ($LASTEXITCODE -ne 0) {
		echo "idlgen returned $LASTEXITCODE"
		exit 1
	}
}

function get-gen-output {
	param([string]$filePath, [switch]$keepExisting, [switch]$bootstrap)
	$idlPath = $filePath.Replace(".xaml", "").Replace(".h", ".idl")
	if ((test-path $idlPath) -and !$keepExisting.IsPresent) {
		remove-item $idlPath
	}
	gen -filePath $filePath -bootstrap:$bootstrap.IsPresent
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

function test-bootstrap {
	param([string]$filePath, [scriptblock]$func)
	$out = get-gen-output -filePath $filePath -bootstrap
	&$func -out $out
}

# Test only pch generation
if (test-path $pchOutDir) {
	remove-item $pchOutDir -Recurse
}
gen -filePath "" -genPch

# Test bootstrap
test-bootstrap "$testCodeDir\BlankPage.h" -func {
	param([string]$out)
	exists -src $out -line "runtimeclass BlankPage"
	$blankPageCount = $out.Split("BlankPage").Length - 1
	assert -desc "No duplicate class" -actual $blankPageCount -eq 1
}

test-bootstrap "$testCodeDir\SameViewModel.h" -func {
	param([string]$out)
	exists -src $out -line "runtimeclass SameViewModel"
	exists -src $out -line "runtimeclass SealedSameViewModel"
}

test-bootstrap "$testCodeDir\ImplementInterface.h" -func {
	param([string]$out)
	exists -src $out -line "runtimeclass ImplementInterface"
	exists -src $out -line "runtimeclass ImplementInterfaceB"
	exists -src $out -line "runtimeclass ImplementInterfaceC"
	exists -src $out -line "runtimeclass ImplementInterfaceD"
	exists -src $out -line "runtimeclass ImplementMoreInterface"
}

test-bootstrap "$testCodeDir\SomeEnum.h" -func {
	param([string]$out)
	exists -src $out -line "enum SomeEnum"
}

test-bootstrap "$testCodeDir\SomeFlag.h" -func {
	param([string]$out)
	exists -src $out -line "enum SomeFlag"
}

test-bootstrap "$testCodeDir\SomeStruct.h" -func {
	param([string]$out)
	exists -src $out -line "struct SomeStruct"
}

test-bootstrap "$testCodeDir\SomeInterface.h" -func {
	param([string]$out)
	exists -src $out -line "interface SomeInterface"
}

test-bootstrap "$testCodeDir\SomeDelegate.h" -func {
	param([string]$out)
	exists -src $out -line "delegate void SomeEventHandler"
}

gen "$testCodeDir\NonWinRtHeader.h" -bootstrap
$idlExists = test-path "$testCodeDir\NonWinRtHeader.idl"
assert -desc "Non WinRT header does not generate bootstrap idl" -actual ($idlExists -ne $true)

# Test BlankPage
# TODO: Rewrite each test case as lambda so we can write test-gen-output("path", (output) -> { exists -src $output })
$blankPageOutput = get-gen-output "$testCodeDir\BlankPage.h"
# Import
exists -src $blankPageOutput -line "import `"SomeDelegate.idl`";"
exists -src $blankPageOutput -line "import `"SomeEnum.idl`";"
exists -src $blankPageOutput -line "import `"SomeStruct.idl`";"
exists -src $blankPageOutput -line "import `"SomeInterface.idl`";"
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
exists -src $blankPageOutput -line "unsealed runtimeclass BlankPage"
# Extend
exists -src $blankPageOutput -line "BlankPage : Windows.UI.Xaml.Controls.Page, Windows.UI.Xaml.Data.INotifyPropertyChanged"
# Constructors
exists -src $blankPageOutput -line "BlankPage()"
exists -src $blankPageOutput -line "BlankPage()"
exists -src $blankPageOutput -line "BlankPage(UInt64 a, UInt64 b)"
# Enum
exists -src $blankPageOutput -line "Root.A.Category Enum{get;};"
# Methods
exists -src $blankPageOutput -line "Int32[] Array();"
exists -src $blankPageOutput -line "void Array(Int32[] a);"
exists -src $blankPageOutput -line "void ArrayRef(ref Int32[] a);"
exists -src $blankPageOutput -line "void ArrayOut(out Int32[] a);"
exists -src $blankPageOutput -line "Int32[] ArrayProp;"
exists -src $blankPageOutput -line "Root.SomeEnum AuthoredEnum();"
exists -src $blankPageOutput -line "Root.SomeStruct AuthoredStruct();"
exists -src $blankPageOutput -line "Root.SomeInterface AuthoredInterface();"
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
exists -src $blankPageOutput -line "UInt32 MethodPropertyLike();"
exists -src $blankPageOutput -line "void MethodPropertyLike(UInt32 a);"
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
exists -src $blankPageOutput -line "Boolean CppXamlProperty;"
exists -src $blankPageOutput -line "Root.A.SameViewModel WilProp{get;};"
exists -src $blankPageOutput -line "Root.A.SameViewModel WilRwProp;"
exists -src $blankPageOutput -line "event Windows.Foundation.EventHandler<Int32> Event;"
exists -src $blankPageOutput -line "event Windows.Foundation.EventHandler<Int32> EventWithConstRefToken;"
exists -src $blankPageOutput -line "event Windows.Foundation.TypedEventHandler<Root.A.BlankPage, UInt32> TypedEvent;"
exists -src $blankPageOutput -line "event Windows.Foundation.TypedEventHandler<Root.A.TestIncludeInTemplate, UInt32> TypedIncludeEvent;"
exists -src $blankPageOutput -line "event Root.SomeEventHandler TestIncludeDelegate;"
exists -src $blankPageOutput -line "event Windows.Foundation.EventHandler<Int32> WilEvent;"
exists -src $blankPageOutput -line "event Windows.Foundation.TypedEventHandler<Int32, Int32> WilTypedEvent;"
exists -src $blankPageOutput -line "protected void ProtecedMethod();"
exists -src $blankPageOutput -line "protected Boolean ProtecedGetter{get;};"
exists -src $blankPageOutput -line "protected UInt32 ProtecedProp;"
exists -src $blankPageOutput -line "protected Boolean ProtectedCppXamlProperty;"
exists -src $blankPageOutput -line "protected Root.A.SameViewModel ProtectedWilProp{get;};"
exists -src $blankPageOutput -line "protected Root.A.SameViewModel ProtectedWilRwProp;"
exists -src $blankPageOutput -line "protected static void ProtecedStaticMethod();"
exists -src $blankPageOutput -line "protected static Boolean ProtectedStaticCppXamlProperty;"
exists -src $blankPageOutput -line "protected static Root.A.SameViewModel ProtectedStaticWilProp{get;};"
exists -src $blankPageOutput -line "protected static Root.A.SameViewModel ProtectedStaticWilRwProp;"
exists -src $blankPageOutput -line "overridable void OverridableMethod();"
exists -src $blankPageOutput -line "overridable Boolean OverridableGetter{get;};"
exists -src $blankPageOutput -line "overridable Boolean OverridableProp;"
exists -src $blankPageOutput -line "overridable Boolean OverridableCppXamlProperty;"
exists -src $blankPageOutput -line "overridable Root.A.SameViewModel OverridableWilProp{get;};"
exists -src $blankPageOutput -line "overridable Root.A.SameViewModel OverridableWilRwProp;"
exists -src $blankPageOutput -line "protected void AttrProtectedMethod();"
exists -src $blankPageOutput -line "overridable void AttrOverridableMethod();"
# Stuff that should be hidden
absent -src $blankPageOutput -line "include\winrt\Root.idl"
absent -src $blankPageOutput -line "BlankPage(BlankPage that);"
absent -src $blankPageOutput -line "void operator=(BlankPage that);"
absent -src $blankPageOutput -line "~BlankPage();"
absent -src $blankPageOutput -line "ImplPropertyOnlyExposeGetter;"
absent -src $blankPageOutput -line "EventWithConstRefToken(event_token token)"
absent -src $blankPageOutput -line "ImplStruct"
absent -src $blankPageOutput -line "error-type"
absent -src $blankPageOutput -line "ParamDisallowImpl"
absent -src $blankPageOutput -line "ParamDisallowImplEvenReturnAllow"
absent -src $blankPageOutput -line "MethodMixingImplAndProjected"
absent -src $blankPageOutput -line "PrivateMethod"
absent -src $blankPageOutput -line "HideMethod"
absent -src $blankPageOutput -line "HideProtectedMethod"
absent -src $blankPageOutput -line "Root.A.factory_implementation"
absent -src $blankPageOutput -line "warning"

$propertyBagOutput = get-gen-output "$testCodeDir\PropertyBag.h"
exists -src $propertyBagOutput -line "unsealed runtimeclass PropertyBag"
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
exists -src $propertyBagOutput -line "static Boolean StaticCppXamlProperty;"
exists -src $propertyBagOutput -line "Root.A.SameViewModel WilProp{get;};"
exists -src $propertyBagOutput -line "Root.A.SameViewModel WilRwProp;"
# hidden
absent -src $propertyBagOutput -line "Root.A.SameViewModel PrivateCppXamlProperty"
absent -src $propertyBagOutput -line "Root.A.SameViewModel PrivateStaticCppXamlProperty"
absent -src $propertyBagOutput -line "Root.A.SameViewModel PrivateWilProp"
absent -src $propertyBagOutput -line "Root.A.SameViewModel PrivateWilRwProp"

$sameVmOutput = get-gen-output "$testCodeDir\SameViewModel.h"
exists -src $sameVmOutput -line "[default_interface]"
exists -src $sameVmOutput -line "unsealed runtimeclass SameViewModel"
exists -src $sameVmOutput -line "runtimeclass SealedSameViewModel"
absent -src $sameVmOutput -line "unsealed runtimeclass SealedSameViewModel"
absent -src $sameVmOutput -line "runtimeclass SameViewModelHide"

$differentPathVmOutput = get-gen-output -filePath "$testCodeDir\SomeNamespace\DifferentPathViewModel.h"
exists -src $differentPathVmOutput -line "[default_interface]"
exists -src $differentPathVmOutput -line "unsealed runtimeclass DifferentPathViewModel"

$differentPathConsumerVmOutput = get-gen-output -filePath "$testCodeDir\SomeNamespace\DifferentPathConsumerViewModel.h"
exists -src $differentPathConsumerVmOutput -line "unsealed runtimeclass DifferentPathConsumerViewModel"
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
exists -src $someStructOutput -line "import `"SomeFlag.idl`";"
exists -src $someStructOutput -line "[webhosthidden]"
exists -src $someStructOutput -line "struct SomeStruct"
exists -src $someStructOutput -line "Int64 X;"
exists -src $someStructOutput -line "Int64 Y;"
exists -src $someStructOutput -line "Root.ShallowerViewModel DisallowedReferenceTypeGeneratedAsIs;"
exists -src $someStructOutput -line "};"
# hidden
absent -src $someStructOutput -line "HiddenStruct"
absent -src $someStructOutput -line "HiddenProp"
absent -src $someStructOutput -line "PrivateProp"

$someDelegateOutput = get-gen-output "$testCodeDir\SomeDelegate.h"
exists -src $someDelegateOutput -line "import `"ShallowerViewModel.idl`";"
exists -src $someDelegateOutput -line "import `"SomeFlag.idl`";"
exists -src $someDelegateOutput -line "[webhosthidden]"
exists -src $someDelegateOutput -line "delegate void SomeEventHandler(Root.ShallowerViewModel vm, UInt64 e);"
# hidden
absent -src $someDelegateOutput -line "NonWinRtTypeHandler"
absent -src $someDelegateOutput -line "HiddenHandler"
absent -src $someDelegateOutput -line "HiddenStructHandler"

$someInterfaceOutput = get-gen-output "$testCodeDir\SomeInterface.h"
exists -src $someInterfaceOutput -line "import `"SomeFlag.idl`";"
exists -src $someInterfaceOutput -line "interface BaseInterface"
exists -src $someInterfaceOutput -line "[webhosthidden]"
exists -src $someInterfaceOutput -line "interface SomeInterface requires Root.BaseInterface"
exists -src $someInterfaceOutput -line "void Method();"
exists -src $someInterfaceOutput -line "Int32 AnotherMethod(Int32 a);"
exists -src $someInterfaceOutput -line "Int32 PropLikeMethod();"
exists -src $someInterfaceOutput -line "void PropLikeMethod(Int32 a);"
exists -src $someInterfaceOutput -line "Int32 Prop;"
exists -src $someInterfaceOutput -line "Boolean WilProp;"
exists -src $someInterfaceOutput -line "event Windows.Foundation.EventHandler<Boolean> WilEvent;"
# hidden
absent -src $someInterfaceOutput -line "PrivateMethod"
absent -src $someInterfaceOutput -line "HiddenInterface"
absent -src $someInterfaceOutput -line "HiddenMethod"

# Note: Outdated projection test cannot be tested here as fixing that involves generating projection from
# bootstrap IDL. These could be tested in sample-app instead

# Test re-generate wouldn't overwrite
$outdatedProjectionXamlIdl = get-childitem "$testCodeDir" "SomeInterface.idl"
$outdatedProjectionXamlIdlLastModifiedTime = $outdatedProjectionXamlIdl.LastWriteTime
$outdatedProjectionXamlOutput = get-gen-output "$testCodeDir\SomeInterface.h" -keepExisting
$outdatedProjectionXamlIdl = get-childitem "$testCodeDir" "SomeInterface.idl"
$outdatedProjectionXamlIdlNewModifiedTime = $outdatedProjectionXamlIdl.LastWriteTime
assert -desc "Regenerating does not overwrite if content is the same" -actual ($outdatedProjectionXamlIdlLastModifiedTime -eq $outdatedProjectionXamlIdlNewModifiedTime)

$nonWinRtHeaderSrc = "$testCodeDir\NonWinRtHeader.h"
$nonWinRtHeaderIdlPath = "$testCodeDir\NonWinRtHeader.idl"
if (test-path $nonWinRtHeaderIdlPath) {
	remove-item $nonWinRtHeaderIdlPath
}
gen -filePath $nonWinRtHeaderSrc
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
