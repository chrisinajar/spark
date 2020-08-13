#include "client.h"

static void *equ8_library = NULL;

typedef equ8_err_t (*proto_equ8_client_initialize)(void);
typedef void (*proto_equ8_client_deinitialize)(void);
typedef void (*proto_equ8_client_close_session)(void);
typedef void (*proto_equ8_client_on_data_received)(const void *data, uint32_t size);
typedef equ8_event_id_t (*proto_equ8_client_poll_event)(equ8_event_t *event);
typedef void (*proto_equ8_client_unregister_d3d_object)(void *obj);
typedef void (*proto___equ8_client_inner_register_d3d_method)(void *obj, const void * const * const *vt_ptr, uint32_t idx, const char *object_name, const char *method_name);

static proto_equ8_client_initialize ptr_equ8_client_initialize = NULL;
static proto_equ8_client_deinitialize ptr_equ8_client_deinitialize = NULL;
static proto_equ8_client_close_session ptr_equ8_client_close_session = NULL;
static proto_equ8_client_on_data_received ptr_equ8_client_on_data_received = NULL;
static proto_equ8_client_poll_event ptr_equ8_client_poll_event = NULL;
static proto_equ8_client_unregister_d3d_object ptr_equ8_client_unregister_d3d_object  = NULL;
static proto___equ8_client_inner_register_d3d_method ptr___equ8_client_inner_register_d3d_method  = NULL;


static equ8_err_t resolve_equ8_entrypoints(const char *equ8_dir, int production)
{
	equ8_err_t rc = EQU8_ERR_STATUS_BAD_API;

	if(equ8_library)
	{
		rc = EQU8_ERR_STATUS_BAD_API;
		goto cleanup;
	}

#ifdef GG_LNX
	#ifdef GG_X64
		#define EQU8_CLIENT_DEV "libclient.null.x64.equ8.so"
		#define EQU8_CLIENT_LIB "libclient.x64.equ8.so"
	#else
		#error "EQU8 does not support 32-bit linux."
	#endif
#else
	#ifdef GG_X64
		#define EQU8_CLIENT_DEV "client.null.x64.equ8.dll"
		#define EQU8_CLIENT_LIB "client.x64.equ8.dll"
	#else
		#define EQU8_CLIENT_DEV "client.null.x86.equ8.dll"
		#define EQU8_CLIENT_LIB "client.x86.equ8.dll"
	#endif
#endif

	if(NULL == (equ8_library = equ8_load_lib(equ8_dir, production ? EQU8_CLIENT_LIB : EQU8_CLIENT_DEV)))
	{
		rc = EQU8_ERR_STATUS_BAD_API;
		goto cleanup;
	}
	equ8_resolve_or_fail(equ8_client_initialize);
	equ8_resolve_or_fail(equ8_client_deinitialize);
	equ8_resolve_or_fail(equ8_client_close_session);
	equ8_resolve_or_fail(equ8_client_on_data_received);
	equ8_resolve_or_fail(equ8_client_poll_event);
	equ8_resolve_or_fail(equ8_client_unregister_d3d_object);
	equ8_resolve_or_fail(__equ8_client_inner_register_d3d_method);

	rc = EQU8_OK;
cleanup:
	if(EQU8_OK != rc)
	{
		//
		// No point in releasing the library, just reset the pointer.
		//
		equ8_library = NULL;
	}
	return rc;
}

equ8_err_t equ8_client_initialize(const char *equ8_dir, int production)
{
	equ8_err_t rc = resolve_equ8_entrypoints(equ8_dir, production);
	if(EQU8_OK != rc)
	{
		return rc;
	}
	return ptr_equ8_client_initialize();
}

void equ8_client_deinitialize(void)
{
	equ8_check_initialized();
	ptr_equ8_client_deinitialize();
}

void equ8_client_on_data_received(const void *data, uint32_t size)
{
	equ8_check_initialized();
	ptr_equ8_client_on_data_received(data, size);
}

equ8_event_id_t equ8_client_poll_event(equ8_event_t *event)
{
	equ8_check_initialized_rc(0);
	return ptr_equ8_client_poll_event(event);
}

void equ8_client_close_session(void)
{
	equ8_check_initialized();
	ptr_equ8_client_close_session();
}

void equ8_client_unregister_d3d_object(void *obj)
{
	equ8_check_initialized();
	ptr_equ8_client_unregister_d3d_object(obj);
}

void __equ8_client_inner_register_d3d_method(void *obj, const void * const * const *vt_ptr, uint32_t idx, const char *object_name, const char *method_name)
{
	equ8_check_initialized();
	ptr___equ8_client_inner_register_d3d_method(obj, vt_ptr, idx, object_name, method_name);
}

