// TODO: Use include when that lands in 10.300
using System.Diagnostics;

void Run(string file)
{
    var p = Process.Start(new ProcessStartInfo("dotnet", $"run {file}"));
    p!.WaitForExit();
    if (p.ExitCode != 0)
    {
        throw new InvalidOperationException();
    }
}

Run("./scripts/Format.cs --check");
Run("./scripts/Test.cs");
Run("./scripts/BuildNuget.cs --version 0.0.1");
