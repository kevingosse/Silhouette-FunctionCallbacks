using Silhouette;
using System.ComponentModel;
using System.Runtime.InteropServices;

namespace Profiler;

internal unsafe class CorProfiler : CorProfilerCallback11Base
{
    private static CorProfiler Instance;

    private static int _mainThreadId;
    private static int _depth;

    protected override HResult Initialize(int iCorProfilerInfoVersion)
    {
        Instance = this;
        _mainThreadId = Environment.CurrentManagedThreadId;

        var functionEnter = (IntPtr)(delegate* unmanaged<FunctionId, IntPtr, void>)&FunctionEnterCallback;
        var functionLeave = (IntPtr)(delegate* unmanaged<FunctionId, IntPtr, void>)&FunctionLeaveCallback;
        var functionTailCall = IntPtr.Zero;

        RegisterCallbacks(ref functionEnter, ref functionLeave, ref functionTailCall);

        if (iCorProfilerInfoVersion < 13)
        {
            Console.WriteLine($"This profiler requires ICorProfilerInfo13 ({iCorProfilerInfoVersion})");
            return HResult.E_FAIL;
        }   

        var eventMask = COR_PRF_MONITOR.COR_PRF_MONITOR_ENTERLEAVE
            | COR_PRF_MONITOR.COR_PRF_ENABLE_FUNCTION_ARGS
            | COR_PRF_MONITOR.COR_PRF_ENABLE_FUNCTION_RETVAL
            | COR_PRF_MONITOR.COR_PRF_ENABLE_FRAME_INFO;

        var result = ICorProfilerInfo5.SetEventMask2(eventMask, COR_PRF_HIGH_MONITOR.COR_PRF_HIGH_MONITOR_NONE);

        if (!result)
        {
            Console.WriteLine($"SetEventMask2 failed with {result}");
            return result;
        }

        result = ICorProfilerInfo5.SetEnterLeaveFunctionHooks3WithInfo(functionEnter, functionLeave, functionTailCall);

        if (!result)
        {
            Console.WriteLine($"SetEnterLeaveFunctionHooks3WithInfo failed with {result}");
            return result;
        }

        return HResult.S_OK;
    }

    [DllImport("__Internal")]
    private static extern void RegisterCallbacks(ref IntPtr enter, ref IntPtr leave, ref IntPtr tailCall);

    [UnmanagedCallersOnly]
    private static void FunctionEnterCallback(FunctionId functionId, IntPtr eltInfo)
    {
        if (Environment.CurrentManagedThreadId == _mainThreadId)
        {
            Instance.FunctionEnter(functionId, eltInfo);
        }
    }

    [UnmanagedCallersOnly]
    private static void FunctionLeaveCallback(FunctionId functionId, IntPtr eltInfo)
    {
        if (Environment.CurrentManagedThreadId == _mainThreadId)
        {
            Instance.FunctionLeave(functionId, eltInfo);
        }
    }

    private void FunctionEnter(FunctionId functionId, IntPtr eltInfo)
    {
        var name = GetFunctionFullName(functionId);
        Console.WriteLine($"{new string(' ', _depth * 2)} - {name}");
        _depth++;
    }

    private void FunctionLeave(FunctionId functionId, IntPtr eltInfo)
    {
        _depth--;
    }

    private string GetFunctionFullName(FunctionId functionId)
    {
        try
        {
            var functionInfo = ICorProfilerInfo2.GetFunctionInfo(functionId).ThrowIfFailed();
            var metaDataImport = ICorProfilerInfo2.GetModuleMetaDataImport(functionInfo.ModuleId, CorOpenFlags.ofRead).ThrowIfFailed();
            var methodProperties = metaDataImport.GetMethodProps(new MdMethodDef(functionInfo.Token)).ThrowIfFailed();
            var typeDefProps = metaDataImport.GetTypeDefProps(methodProperties.Class).ThrowIfFailed();

            return $"{typeDefProps.TypeName}.{methodProperties.Name}";
        }
        catch (Win32Exception ex)
        {
            return $"Failed ({ex.NativeErrorCode})";
        }
    }
}
