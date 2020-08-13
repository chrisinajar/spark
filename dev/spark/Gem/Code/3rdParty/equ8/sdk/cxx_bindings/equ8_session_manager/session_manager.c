#include "session_manager.h"

static void *equ8_library = NULL;

typedef equ8_err_t (*proto_equ8_sm_initialize)(const char *config_file);
typedef equ8_err_t (*proto_equ8_sm_initialize_from_data)(const void *config_data, uint32_t size);
typedef void (*proto_equ8_sm_deinitialize)(void);
typedef equ8_action_t (*proto_equ8_sm_user_status)(const char *client_id);
typedef void (*proto_equ8_sm_on_data_received)(const char *client_id, const void *data, uint32_t size);
typedef equ8_event_id_t (*proto_equ8_sm_poll_event)(const char *client_id, equ8_event_t *event);
typedef equ8_err_t (*proto_equ8_sm_write_metadata)(const char *client_id, const char *key, const char *value);
typedef equ8_sm_match_t (*proto_equ8_sm_allocate_match)(const char *match_name);
typedef equ8_sm_team_t (*proto_equ8_sm_match_add_team)(equ8_sm_match_t match, const char *team_name);
typedef void (*proto_equ8_sm_team_add_client)(equ8_sm_team_t team, const char *client_id);
typedef equ8_err_t (*proto_equ8_sm_finalize_match)(equ8_sm_match_t match);

static proto_equ8_sm_initialize ptr_equ8_sm_initialize = NULL;
static proto_equ8_sm_initialize_from_data ptr_equ8_sm_initialize_from_data = NULL;
static proto_equ8_sm_deinitialize ptr_equ8_sm_deinitialize = NULL;
static proto_equ8_sm_user_status ptr_equ8_sm_user_status = NULL;
static proto_equ8_sm_on_data_received ptr_equ8_sm_on_data_received = NULL;
static proto_equ8_sm_poll_event ptr_equ8_sm_poll_event = NULL;
static proto_equ8_sm_write_metadata ptr_equ8_sm_write_metadata = NULL;
static proto_equ8_sm_allocate_match ptr_equ8_sm_allocate_match = NULL;
static proto_equ8_sm_match_add_team ptr_equ8_sm_match_add_team = NULL;
static proto_equ8_sm_team_add_client ptr_equ8_sm_team_add_client = NULL;
static proto_equ8_sm_finalize_match ptr_equ8_sm_finalize_match = NULL;

static equ8_err_t resolve_equ8_entrypoints(const char *equ8_dir, int production)
{
	equ8_err_t rc = EQU8_ERR_STATUS_BAD_API;

	if(equ8_library || !equ8_dir)
	{
		goto cleanup;
	}

#ifdef GG_LNX
	#ifdef GG_X64
		#define EQU8_SERVER_LIB_DEV "libsession_manager.null.x64.equ8.so"
		#define EQU8_SERVER_LIB_PROD "libsession_manager.x64.equ8.so"
	#else
		#error "EQU8 does not support 32-bit linux."
	#endif
#else
	#ifdef GG_X64
		#define EQU8_SERVER_LIB_DEV "session_manager.null.x64.equ8.dll"
		#define EQU8_SERVER_LIB_PROD "session_manager.x64.equ8.dll"
	#else
		#define EQU8_SERVER_LIB_DEV "session_manager.null.x86.equ8.dll"
		#define EQU8_SERVER_LIB_PROD "session_manager.x86.equ8.dll"
	#endif
#endif

	if(NULL == (equ8_library = equ8_load_lib(equ8_dir, production ? EQU8_SERVER_LIB_PROD : EQU8_SERVER_LIB_DEV)))
	{
		rc = EQU8_ERR_STATUS_BAD_API;
		goto cleanup;
	}

	equ8_resolve_or_fail(equ8_sm_initialize);
	equ8_resolve_or_fail(equ8_sm_initialize_from_data);
	equ8_resolve_or_fail(equ8_sm_deinitialize);
	equ8_resolve_or_fail(equ8_sm_user_status);
	equ8_resolve_or_fail(equ8_sm_on_data_received);
	equ8_resolve_or_fail(equ8_sm_poll_event);
	equ8_resolve_or_fail(equ8_sm_write_metadata);
	equ8_resolve_or_fail(equ8_sm_allocate_match);
	equ8_resolve_or_fail(equ8_sm_match_add_team);
	equ8_resolve_or_fail(equ8_sm_team_add_client);
	equ8_resolve_or_fail(equ8_sm_finalize_match);

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

equ8_err_t equ8_sm_initialize(const char *equ8_dir, int production)
{
	char path[520];
	equ8_err_t rc = resolve_equ8_entrypoints(equ8_dir, production);

	if(EQU8_OK != rc)
	{
		return rc;
	}

#ifdef GG_WIN
	#define EQU8_DIR_SEPARATOR "\\"
#else
	#define EQU8_DIR_SEPARATOR "/"
#endif

	int len = snprintf(path, sizeof(path), "%s" EQU8_DIR_SEPARATOR "equ8_server.config", equ8_dir);
	if((len >= (int)sizeof(path)) || len < 0)
	{
		return EQU8_ERR_STATUS_BAD_API;
	}

	return ptr_equ8_sm_initialize(path);
}

void equ8_sm_deinitialize(void)
{
	equ8_check_initialized();
	ptr_equ8_sm_deinitialize();
}

equ8_action_t equ8_sm_user_status(const char *client_id)
{
	equ8_check_initialized_rc(0);
	return ptr_equ8_sm_user_status(client_id);
}

void equ8_sm_on_data_received(const char *client_id, const void *data, uint32_t size)
{
	equ8_check_initialized();
	ptr_equ8_sm_on_data_received(client_id, data, size);
}

equ8_event_id_t equ8_sm_poll_event(const char *client_id, equ8_event_t *event)
{
	equ8_check_initialized_rc(0);
	return ptr_equ8_sm_poll_event(client_id, event);
}

equ8_err_t equ8_sm_write_metadata(const char *client_id, const char *key, const char *value)
{
	equ8_check_initialized_rc(EQU8_ERR_UNINITIALZIED);
	return ptr_equ8_sm_write_metadata(client_id, key, value);
}

equ8_sm_match_t equ8_sm_allocate_match(const char *match_name)
{
	equ8_check_initialized_rc(NULL);
	return ptr_equ8_sm_allocate_match(match_name);
}

equ8_sm_team_t equ8_sm_match_add_team(equ8_sm_match_t match, const char *team_name)
{
	equ8_check_initialized_rc(NULL);
	return ptr_equ8_sm_match_add_team(match, team_name);
}

void equ8_sm_team_add_client(equ8_sm_team_t team, const char *client_id)
{
	equ8_check_initialized();
	ptr_equ8_sm_team_add_client(team, client_id);
}

equ8_err_t equ8_sm_finalize_match(equ8_sm_match_t match)
{
	equ8_check_initialized_rc(EQU8_ERR_UNINITIALZIED);
	return ptr_equ8_sm_finalize_match(match);
}





