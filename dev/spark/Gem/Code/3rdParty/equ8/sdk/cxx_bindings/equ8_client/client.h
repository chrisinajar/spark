#ifndef __EQU8_CLIENT_H_DEF__
#define __EQU8_CLIENT_H_DEF__

#include "../equ8_meta/shared.h"
#include "../equ8_meta/error.h"

#ifdef __cplusplus
	#include "vtable_manip.hpp"

	#define equ8_register_d3d_method(device, type, method) \
		__equ8_client_inner_register_d3d_method( \
			static_cast<void *>(device), \
			gg::vtable_manip::vtable_pointer(device), \
			gg::vtable_manip::method_index(device, &type::method), \
			#type, \
			#method)
	//
	// @andox: Serves more as an example than something useful?
	//
	#define equ8_client_register_d3d9_device(device) \
		equ8_register_d3d_method(device, IDirect3DDevice9, SetRenderState); \
		equ8_register_d3d_method(device, IDirect3DDevice9, Present); \
		equ8_register_d3d_method(device, IDirect3DDevice9, EndScene)

	#define equ8_client_register_d3d9_swapchain(swapchain) \
		equ8_register_d3d_method(swapchain, IDirect3DSwapChain9, Present)

	#define equ8_client_register_d3d9_swapchain_ex(swapchain) \
		equ8_register_d3d_method(swapchain, IDirect3DSwapChain9Ex, Present)

#endif

#ifdef __cplusplus
extern "C" {
#endif

equ8_err_t equ8_client_initialize(const char *equ8_dir, int production);
void equ8_client_deinitialize(void);

void equ8_client_on_data_received(const void *data, uint32_t size);
equ8_event_id_t equ8_client_poll_event(equ8_event_t *event);
void equ8_client_close_session(void);

void equ8_client_unregister_d3d_object(void *obj);

//
// Never call this function directly, use equ8_register_d3d_method() defined above.
//
void __equ8_client_inner_register_d3d_method(void *obj, const void * const * const *vt_ptr, uint32_t idx, const char *object_name, const char *method_name);

#ifdef __cplusplus
}
#endif

#endif // __EQU8_CLIENT_H_DEF__




