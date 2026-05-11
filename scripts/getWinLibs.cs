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
