using System;
using System.IO;
using System.Linq;
using System.Diagnostics;
using System.Reflection;

void BuildNuget()
{
    string? version = null;
    bool testPackage = false;

    var argv = Environment.GetCommandLineArgs().Skip(1).ToArray();
    for (int i = 0; i < argv.Length; i++)
    {
        var a = argv[i];
        if (a == "--testPackage")
        {
            testPackage = true;
            continue;
        }
        if (a == "--version" || a == "-v")
        {
            if (i + 1 < argv.Length)
            {
                version = argv[++i];
                continue;
            }
            else
            {
                Console.Error.WriteLine("Error: --version requires a value");
                Environment.Exit(1);
            }
        }
    }

    if (string.IsNullOrWhiteSpace(version))
    {
        Console.Error.WriteLine("Usage: dotnet run BuildNuget.cs --version <version> [--testPackage]");
        Environment.Exit(1);
    }

    string nugetDir = "nuget";
    string localPackagesDir = "LocalPackages";

    string templatePath = Path.Combine(nugetDir, "IdlGen.IdlGen.Cpp.nuspec.template");
    if (!File.Exists(templatePath))
    {
        throw new FileNotFoundException($"Template file not found: {templatePath}");
    }

    string template = File.ReadAllText(templatePath);
    string nuspec = template.Replace("VERSION", version);
    string nuspecPath = Path.GetFullPath(Path.Combine(nugetDir, ".nuspec"));
    File.WriteAllText(nuspecPath, nuspec);

    string nugetOutDir = testPackage ? localPackagesDir : nugetDir;

    Directory.CreateDirectory(nugetOutDir);
    string nugetExe = "nuget"; // rely on PATH
    string arguments = $"pack \"{nuspecPath}\" -outputDirectory \"{nugetOutDir}\"";

    using (var proc = Process.Start(new ProcessStartInfo
    {
        FileName = nugetExe,
        Arguments = arguments,
        UseShellExecute = false,
    }))
    {
        proc!.WaitForExit();
        if (proc.ExitCode != 0)
        {
            throw new InvalidOperationException();
        }
    }
}
BuildNuget();
