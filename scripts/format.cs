using System;
using System.IO;
using System.Linq;
using System.Diagnostics;
using System.Reflection;
using System.Collections.Generic;

void Format()
{
    bool check = args.Any(a => a == "--check");
    string devDir = "dev";
    var files = Directory.EnumerateFiles(devDir, "*.*", SearchOption.AllDirectories)
        .Where(f => (f.EndsWith(".h", StringComparison.OrdinalIgnoreCase) || f.EndsWith(".cpp", StringComparison.OrdinalIgnoreCase)))
        .ToList();
    foreach (var file in files)
    {
        Console.WriteLine(file);
        var checkFlags = check ? "--Werror --dry-run --ferror-limit=1" : ""; 
        using (var proc = Process.Start(new ProcessStartInfo
        {
            FileName = "clang-format",
            Arguments = $"{checkFlags} -i \"{file}\"",
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
    if (check)
    {
        Console.WriteLine("All files are formatted");
    }
}

Format();
