namespace Idlgen.Cpp
{
    [Command(PackageIds.CppGenerateIDL)]
    internal sealed class CppGenerateIDL : BaseCommand<CppGenerateIDL>
    {
        protected override async Task ExecuteAsync(OleMenuCmdEventArgs e)
        {
            await VS.MessageBox.ShowWarningAsync("Idlgen.Cpp", "Button clicked");
        }
    }
}
