// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <iostream>

typedef UINT_PTR FunctionID;
typedef union { FunctionID functionID; UINT_PTR clientID; } FunctionIDOrClientID;
typedef UINT_PTR COR_PRF_ELT_INFO;

extern "C" {
	void EnterNaked();
	void LeaveNaked();
	void TailcallNaked();
}

namespace {
	void* enterCallback;
	void* leaveCallback;
	void* tailcallCallback;
}

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
		typedef void (STDMETHODCALLTYPE* EnterCallbackType)(FunctionIDOrClientID, COR_PRF_ELT_INFO);
		EnterCallbackType callback = reinterpret_cast<EnterCallbackType>(enterCallback);
		callback(functionId, eltInfo);
	}
}

EXTERN_C void STDMETHODCALLTYPE LeaveStub(FunctionID functionId, COR_PRF_ELT_INFO eltInfo)
{
	if (leaveCallback) {
		typedef void (STDMETHODCALLTYPE* LeaveCallbackType)(FunctionID, COR_PRF_ELT_INFO);
		LeaveCallbackType callback = reinterpret_cast<LeaveCallbackType>(leaveCallback);
		callback(functionId, eltInfo);
	}
}

EXTERN_C void STDMETHODCALLTYPE TailcallStub(FunctionID functionId, COR_PRF_ELT_INFO eltInfo)
{
	if (tailcallCallback) {
		typedef void (STDMETHODCALLTYPE* TailcallCallbackType)(FunctionID, COR_PRF_ELT_INFO);
		TailcallCallbackType callback = reinterpret_cast<TailcallCallbackType>(tailcallCallback);
		callback(functionId, eltInfo);
	}
}