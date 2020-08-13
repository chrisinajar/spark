#ifndef __EQU8_SESSION_MANAGER_H_DEF__
#define __EQU8_SESSION_MANAGER_H_DEF__

#include "../equ8_meta/error.h"
#include "../equ8_meta/shared.h"

#ifdef __cplusplus
extern "C" {
#endif

equ8_err_t equ8_sm_initialize(const char *equ8_dir, int production);
void equ8_sm_deinitialize(void);

equ8_action_t equ8_sm_user_status(const char *client_id);
void equ8_sm_on_data_received(const char *client_id, const void *data, uint32_t size);
equ8_event_id_t equ8_sm_poll_event(const char *client_id, equ8_event_t *event);

equ8_err_t equ8_sm_write_metadata(const char *client_id, const char *key, const char *value);
equ8_sm_match_t equ8_sm_allocate_match(const char *match_name);
equ8_sm_team_t equ8_sm_match_add_team(equ8_sm_match_t match, const char *team_name);
void equ8_sm_team_add_client(equ8_sm_team_t team, const char *client_id);
 equ8_err_t equ8_sm_finalize_match(equ8_sm_match_t match);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __EQU8_SESSION_MANAGER_H_DEF__

