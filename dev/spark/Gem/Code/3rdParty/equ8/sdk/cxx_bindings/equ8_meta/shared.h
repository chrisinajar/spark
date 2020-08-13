#ifndef __EQU8_SHARED_TYPES_HPP_DEF__
#define __EQU8_SHARED_TYPES_HPP_DEF__

#include <stdint.h>

#include "../equ8_session_manager/equ8_actions.h"

#define EQU8_MAX_PACKET_SIZE 256

// #define EQU8_PARANOID

#ifdef EQU8_PARANOID
	#define equ8_check_initialized() if(!equ8_library) { return; }
	#define equ8_check_initialized_rc(v) if(!equ8_library) { return v; }
#else
	#define equ8_check_initialized()
	#define equ8_check_initialized_rc(v)
#endif

#ifndef EQU8_ORCUS_BUILD

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4505) // warning C4505: 'equ8_load_lib': unreferenced local function has been removed
#endif

static void *equ8_load_lib(const char *path, const char *library)
{
	char combined[MAX_PATH];
	wchar_t wide_path[MAX_PATH];
	int len	= snprintf(combined, sizeof(combined), "%s\\%s", path, library);
	if((len >= (int)sizeof(combined)) || len < 0)
	{
		return NULL;
	}

  	if(0 == MultiByteToWideChar(CP_UTF8, 0, combined, -1, wide_path, MAX_PATH))
  	{
		return NULL;  		
  	}

	return LoadLibraryW(wide_path);
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define equ8_resolve(lib, name) GetProcAddress(lib, name)

typedef HMODULE module_ptr;

#else

#include <dlfcn.h>
#include <string.h>

typedef void * module_ptr;

static module_ptr equ8_load_lib(const char *path, const char *lib)
{
	char full[500];
	if((strlen(path) + strlen(lib) + 1) > sizeof(full))
	{
		return NULL;
	}
	strcpy(full, path);
	strcat(full, "/");
	strcat(full, lib);
	return dlopen(full, RTLD_NOW);
}

#define equ8_resolve(lib, name) dlsym(lib, name)

#endif

#define equ8_resolve_or_fail(name) \
	if(NULL == (ptr_##name = (proto_##name)equ8_resolve((module_ptr)equ8_library, #name))) \
	{ \
		rc = EQU8_ERR_STATUS_BAD_API; \
		goto cleanup; \
	}

#endif

#if !defined(GG_WIN) && !defined(GG_LNX)
	#if defined(__linux__)
		#define GG_LNX
	#elif defined(_WIN32)
		#define GG_WIN
	#endif
#endif

#if !defined(GG_X64) && !defined(GG_X86)
	#if defined(GG_LNX)
		#ifdef __LP64__
			#define GG_X64
		#else
			#define GG_X86
		#endif
	#elif defined(GG_WIN)
		#ifdef _WIN64
			#define GG_X64
		#else
			#define GG_X86
		#endif
	#endif
#endif

#if !defined(GG_WIN) && !defined(GG_LNX)
	#error Unable to solve architecture, please specify either GG_WIN or GG_LNX
#endif

#if !defined(GG_X64) && !defined(GG_X86)
	#error Unable to solve architecture, please specify either GG_X64 or GG_X86
#endif

#ifdef _MSC_VER
#ifdef GG_BUILDING_LIB
	#define GG_EXPORT __declspec(dllexport)
#else
	#define GG_EXPORT __declspec(dllimport)
#endif
#else
#ifdef GG_BUILDING_LIB
	#define GG_EXPORT __attribute__ ((visibility("default")))
#else
	#define GG_EXPORT
#endif
#endif

#ifdef GG_X64
	#define gg_cdecl
	#define gg_stdcall
	#define gg_fastcall
	#define gg_thiscall

	#define gg_cc
#else
	#if defined(__GNUC__) && !defined(__clang__) && !defined(__MINGW32__)
		#define __cdecl __attribute__((__cdecl__))
		#define __stdcall __attribute__((__stdcall__))
		#define __fastcall __attribute__((__fastcall__))
		#define __thiscall __attribute__((__thiscall__))
	#endif

	#define gg_cdecl	__cdecl
	#define gg_stdcall	__stdcall
	#define gg_fastcall __fastcall
	#define gg_thiscall	__thiscall
	
	#define gg_cc		gg_cdecl
#endif

typedef enum equ8_event_id
{
	equ8_event_id_none,			// No event
	equ8_event_id_error,		// Client & server
	equ8_event_id_send_request,	// Client & server
	equ8_event_id_status		// Server-only
}
equ8_event_id_t;

#pragma pack(push, 1)

typedef struct equ8_event
{
	equ8_event_id_t event_id;
	union
	{
		struct
		{
			uint32_t size;
			uint8_t payload[EQU8_MAX_PACKET_SIZE];
		}
		send_request;
		//
		// Server-only
		//
		struct
		{
			equ8_action_t action;
		}
		status;
		//
		// Client-only
		//
		struct
		{
			uint64_t code;
		}
		error;
	};
}
equ8_event_t;

#pragma pack(pop)

typedef uint64_t equ8_challenge_key_t;

typedef void * equ8_sm_match_t;
typedef void * equ8_sm_team_t;

#endif // __EQU8_SHARED_TYPES_HPP_DEF__


