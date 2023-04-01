$srcDir = "$PSScriptRoot"
$llvmDir = "$srcDir\..\llvm-project"
if (test-path $llvmDir) {
	echo "llvm-project already exists. Nothing to do"
	exit 0
}
git clone --depth 1 --branch llvmorg-16.0.0 https://github.com/llvm/llvm-project
