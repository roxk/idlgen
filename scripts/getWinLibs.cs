using System.Net.Http;
using System.IO;
using System.IO.Compression;

const string winLibsUrl = "https://github.com/brechtsanders/winlibs_mingw/releases/download/16.1.0posix-14.0.0-ucrt-r1/winlibs-x86_64-posix-seh-gcc-16.1.0-mingw-w64ucrt-14.0.0-r1.zip";
const string outputDirPath = "./winLibs";
const string outputFilePath = $"{outputDirPath}/winLibs.zip";
if (File.Exists(outputFilePath))
{
    Console.WriteLine($"Reusing ${outputFilePath}, skipping download...");
}
else
{
    Console.WriteLine($"Downloading winlibs from {winLibsUrl}");
    Console.WriteLine($"Output: ${outputFilePath}");
    
    Directory.CreateDirectory(outputDirPath);
    await using var outputStream = File.Create(outputFilePath);

    var client = new HttpClient();
    using var response = await client.GetAsync(winLibsUrl, HttpCompletionOption.ResponseHeadersRead);
    response.EnsureSuccessStatusCode();
    await using var inputStream = await response.Content.ReadAsStreamAsync();
    await inputStream.CopyToAsync(outputStream);

    Console.WriteLine("Download completed");
}

const string mingwDirPath = $"{outputDirPath}/mingw64";
if (Directory.Exists(mingwDirPath))
{
    Console.WriteLine($"{mingwDirPath} exists. Assume it is already unarchived.");
}
else
{
    Console.WriteLine($"{mingwDirPath} not found. Unarchiving {outputFilePath}...");
    ZipFile.ExtractToDirectory(outputFilePath, outputDirPath);
    Console.WriteLine($"Unacrhived to {outputDirPath}");
}
Console.WriteLine("Deleting unused mingw64 files/folders...");
string[] victim = [
    "share",
    "bin/lto-dump.exe",
    "bin/libhogweed-6.dll",
    "bin/libnettle-8.dll",
    "bin/ctest.exe",
    "bin/cpack.exe",
    "bin/cmake.exe",
    "bin/cppcheck.exe",
    "bin/libgfortran-5.dll",
    "bin/libpython3.9.dll",
    "bin/doxygen.exe",
    "bin/gfortran.exe",
    "bin/gdb.exe",
    "bin/gprof.exe",
    "lib/libgfortran.a",
    "lib/libpython3.9.a",
    "lib/python3.9",
    "libexec/gcc/x86_64-w64-mingw32/16.1.0/cc1obj.exe",
    "libexec/gcc/x86_64-w64-mingw32/16.1.0/cc1objplus.exe",
    "libexec/gcc/x86_64-w64-mingw32/16.1.0/f951.exe",
];
foreach (var v in victim)
{
    var path = $"{mingwDirPath}/{v}";
    var isDeleted = false;
    if (Directory.Exists(path))
    {
        Directory.Delete(path, true);
        isDeleted = true;
    }
    else if (File.Exists(path))
    {
        File.Delete(path);
        isDeleted = true;
    }
    if (isDeleted)
    {
        Console.WriteLine($"Deleted {v}");
    }
    else 
    {
        Console.WriteLine($"{v} likely already removed");
    }
}
