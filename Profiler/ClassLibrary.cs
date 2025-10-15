using System.Runtime.InteropServices;
using Silhouette;

namespace Profiler;

public static class ClassLibrary
{
    private static ClassFactory? Instance;

    [UnmanagedCallersOnly(EntryPoint = "DllGetClassObject")]
    public static unsafe HResult DllGetClassObject(Guid* rclsid, Guid* riid, nint* ppv)
    {
        Console.WriteLine("DllGetClassObject called");

        if (*rclsid != new Guid("3B05DC62-0B96-4D8E-88BD-291183E99F32"))
        {
            return HResult.E_NOINTERFACE;
        }

        Instance = new ClassFactory(new CorProfiler());
        *ppv = Instance.IClassFactory;

        return 0;
    }
}
