// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

// Type definitions expected by the profiler API
typedef UINT_PTR FunctionID;
typedef union { FunctionID functionID; UINT_PTR clientID; } FunctionIDOrClientID;
typedef UINT_PTR COR_PRF_ELT_INFO;

// The assembly stubs declared in asmhelpers.asm
extern "C" {
	void EnterNaked();
	void LeaveNaked();
	void TailcallNaked();
}

// The managed callbacks that will be provided at runtime
namespace {
	void* enterCallback;
	void* leaveCallback;
	void* tailcallCallback;
}

// This is the function that will be called from the managed side to register the managed callbacks
EXTERN_C __declspec(dllexport) void __stdcall RegisterCallbacks(void** enter, void** leave, void** tailcall)
{
	if (enter) {
		enterCallback = *enter;
		*enter = reinterpret_cast<void*>(&EnterNaked);
	}

	if (leave) {
		leaveCallback = *leave;
		*leave = reinterpret_cast<void*>(&LeaveNaked);
	}

	if (tailcall) {
		tailcallCallback = *tailcall;
		*tailcall = reinterpret_cast<void*>(&TailcallNaked);
	}	
}

EXTERN_C void STDMETHODCALLTYPE EnterStub(FunctionIDOrClientID functionId, COR_PRF_ELT_INFO eltInfo)
{
	if (enterCallback) {
		auto callback = reinterpret_cast<void (STDMETHODCALLTYPE*)(FunctionIDOrClientID, COR_PRF_ELT_INFO)>(enterCallback);
		callback(functionId, eltInfo);
	}
}

EXTERN_C void STDMETHODCALLTYPE LeaveStub(FunctionID functionId, COR_PRF_ELT_INFO eltInfo)
{
	if (leaveCallback) {
		auto callback = reinterpret_cast<void (STDMETHODCALLTYPE*)(FunctionID, COR_PRF_ELT_INFO)>(leaveCallback);
		callback(functionId, eltInfo);
	}
}

EXTERN_C void STDMETHODCALLTYPE TailcallStub(FunctionID functionId, COR_PRF_ELT_INFO eltInfo)
{
	if (tailcallCallback) {
		auto callback = reinterpret_cast<void (STDMETHODCALLTYPE*)(FunctionID, COR_PRF_ELT_INFO)>(tailcallCallback);
		callback(functionId, eltInfo);
	}
}