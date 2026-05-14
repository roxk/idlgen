using System.Diagnostics;
using System.Linq;
using System.Collections.Generic;
using System.Text;

const string binDirPath = "./winLibs/mingw64/bin";
const string devDirPath = "dev/src";
const string outDirPath = "out";
const string nugetInclude = "nuget/include";
const string fakeWinrtGeneratedProjectDirPath = "test-data/include";
const string fakeProjectRootDirPath = "test-data/src";
List<string> includes = [nugetInclude, fakeWinrtGeneratedProjectDirPath, fakeProjectRootDirPath];
var includeParam = includes.Aggregate("", (cur, next) =>
{
    cur += $"-I\"{next}\" ";
    return cur;
});
var compilerExe = $"{binDirPath}/g++.exe";
var pchPath = $"{fakeProjectRootDirPath}/pch.h";
var pchOutputPath = $"{pchPath}.gch";
if (File.Exists(pchOutputPath))
{
    Console.WriteLine("pch.h.gch exists, skipping pch.h...");
}
else
{
    var genPchCmd = $"-std=c++26 -v {includeParam} -x c++-header {pchPath}";
    var pchP = Process.Start(new ProcessStartInfo
    {
        FileName = "powershell",
        Arguments = $"-c \"{compilerExe}\" {genPchCmd}",
        UseShellExecute = false,
    });
    pchP!.WaitForExit();
    if (pchP.ExitCode != 0)
    {
        Console.WriteLine("Failed to generate pch");
        return -1;
    }
}
var input = $"{devDirPath}/main.cpp";
var outputExePath = $"{outDirPath}/main.exe";
var outputIdlPath = $"{outDirPath}/outputIdl.txt";
var outputImplPath = $"{outDirPath}/outputImpl.txt";
var compileCmd = $"-std=c++26 -freflection -static -v {includeParam} -include pch.h -ftime-report {input} -o {outputExePath}";
Directory.CreateDirectory(outDirPath);
var cp = Process.Start(new ProcessStartInfo
{
    FileName = "powershell",
    Arguments = $"-c \"{compilerExe}\" {compileCmd}",
    UseShellExecute = false,
});
cp!.WaitForExit();
if (cp.ExitCode != 0)
{
    Console.WriteLine("Failed to compile. Abort generating idl");
    return -1;
}
else
{
    Console.WriteLine("Generating idl...");
}
var idlp = Process.Start(new ProcessStartInfo
{
    FileName = "powershell",
    Arguments = $"-c \"{outputExePath}\" -idl | Out-File {outputIdlPath} -Encoding utf8",
    UseShellExecute = false,
});
idlp!.WaitForExit();
if (idlp.ExitCode != 0)
{
    Console.WriteLine("Failed to generate idl");
    return -1;
}
Console.WriteLine("Testing output...");
const string expectedOutputPath = "test-data/src/ExpectedOutput.txt";
using var reader1 = new StreamReader(outputIdlPath);
using var reader2 = new StreamReader(expectedOutputPath);
string? line1;
string? line2;
int lineIndex = 0;
while (true)
{
    ++lineIndex;
    line1 = reader1.ReadLine();
    line2 = reader2.ReadLine();
    if (line1 == null && line2 == null)
    {
        Console.WriteLine("Test passed");
        break;
    }
    if (line1 == null ||  line2 == null || !line1.Equals(line2))
    {
        Console.WriteLine($"Test failed at line {lineIndex}.");
        Console.WriteLine($"Received: {line1}");
        Console.WriteLine($"Expected: {line2}");
        return -1;
    }
}
Console.WriteLine("Generating implementation...");
var implp = Process.Start(new ProcessStartInfo
{
    FileName = "powershell",
    Arguments = $"-c \"{outputExePath}\" -implementation-header | Out-File {outputImplPath} -Encoding utf8",
    UseShellExecute = false,
});
implp!.WaitForExit();
if (implp.ExitCode != 0)
{
    Console.WriteLine("Failed to generate implementation");
    return -1;
}
return 0;