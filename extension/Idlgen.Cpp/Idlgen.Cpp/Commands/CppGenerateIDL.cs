using Microsoft.Build.Framework;
using Microsoft.VisualStudio.Shell.Interop;
using System.Collections.Generic;
using static Community.VisualStudio.Toolkit.Windows;

namespace Idlgen.Cpp
{
    [Command(PackageIds.CppGenerateIDL)]
    internal sealed class CppGenerateIDL : BaseCommand<CppGenerateIDL>
    {
        private class OutputWindowLogger : ILogger
        {
            private readonly OutputWindowPane outputWindowPane;
            public OutputWindowLogger(OutputWindowPane pane) { outputWindowPane = pane; }
            public LoggerVerbosity Verbosity { get => LoggerVerbosity.Minimal; set { } }
            public string Parameters { get => ""; set { } }

            public void Initialize(IEventSource eventSource)
            {
                eventSource.AnyEventRaised += (s, e) =>
                {
                    _ = outputWindowPane.WriteLineAsync(e.Message);
                };
            }

            public void Shutdown()
            {
            }
        }

        protected override async Task ExecuteAsync(OleMenuCmdEventArgs e)
        {
            if (await VS.Solutions.GetActiveItemAsync() is not PhysicalFile activeFile)
            {
                await VS.StatusBar.ShowMessageAsync("Failed to find active file");
                return;
            }
            var output = await VS.Windows.GetOutputWindowPaneAsync(VSOutputWindowPane.Build);
            if (output == null)
            {
                await VS.StatusBar.ShowMessageAsync("Failed to get output window pane");
                return;
            }
            try
            {
                var project = activeFile.ContainingProject;
                var evalProject = Microsoft.Build.Evaluation.ProjectCollection.GlobalProjectCollection.LoadProject(project.FullPath);
                evalProject.SetProperty("IdlGenCppGenerateIDL", "true");
                evalProject.SetProperty("IdlGenCppInclude", $"{activeFile.Name}");
                evalProject.SetProperty("IdlGenCppExclude", "");
                evalProject.SetProperty("Platform", "x64");
                var execProject = evalProject.CreateProjectInstance();
                await output.ClearAsync();
                await VS.StatusBar.ShowMessageAsync("Generating IDL...");
                var result = await Task.Run(() =>
                {
                    var result = execProject.Build("IdlGenCppGenerateIDL",
                        new List<ILogger>() { new OutputWindowLogger(output) });
                    return result;
                });
                if (result)
                {
                    await VS.StatusBar.ShowMessageAsync("IDL Generated");
                }
                else
                {
                    await VS.StatusBar.ShowMessageAsync("Failed to generate IDL");
                }
            }
            catch (Exception ex)
            {
                await output.WriteLineAsync("Failed to build IdlGenCppGenerateIDL");
                await output.WriteLineAsync($"{ex}");
                await VS.StatusBar.ShowMessageAsync("Failed to generate IDL");
            }
        }
    }
}
