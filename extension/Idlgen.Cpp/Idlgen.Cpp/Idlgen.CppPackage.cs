global using Community.VisualStudio.Toolkit;
global using Microsoft.VisualStudio.Shell;
global using System;
global using Task = System.Threading.Tasks.Task;
using Microsoft.VisualStudio;
using System.Runtime.InteropServices;
using System.Threading;

namespace Idlgen.Cpp
{
    [PackageRegistration(UseManagedResourcesOnly = true, AllowsBackgroundLoading = true)]
    [InstalledProductRegistration(Vsix.Name, Vsix.Description, Vsix.Version)]
    [ProvideMenuResource("Menus.ctmenu", 1)]
    [Guid(PackageGuids.IdlgenCppString)]
    [ProvideAutoLoad(PackageGuids.CppHeaderSelectedString, PackageAutoLoadFlags.BackgroundLoad)]
    [ProvideUIContextRule(PackageGuids.CppHeaderSelectedString,
    name: "C++ header file selected",
    expression: "h & building",
    termNames: new[] { "h", "building" },
    termValues: new[] { "HierSingleSelectionName:.h$", VSConstants.UICONTEXT.NotBuildingAndNotDebugging_string })]
    public sealed class IdlgenCppPackage : ToolkitPackage
    {
        protected override async Task InitializeAsync(CancellationToken cancellationToken, IProgress<ServiceProgressData> progress)
    {
        await this.RegisterCommandsAsync();
    }
}
}