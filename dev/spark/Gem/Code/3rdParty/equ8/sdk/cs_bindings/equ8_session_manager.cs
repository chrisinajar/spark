using System;
using System.Runtime.InteropServices;

namespace equ8
{

public class session_manager
{
    private static equ8_err resolve_api(string equ8_dir, int production)
    {
        string[] paths =
        {
            equ8_dir,
            System.Environment.Is64BitProcess
                ? ((0 != production) ? @"session_manager.x64.equ8.dll" : @"session_manager.null.x64.equ8.dll")
                : ((0 != production) ? @"session_manager.x86.equ8.dll" : @"session_manager.null.x86.equ8.dll")
        };

		UInt64 rc = 0;
        string filename = System.IO.Path.Combine(paths);

        try
        {
            ptr_equ8_sm_initialize = dll_helper.resolve<equ8_sm_initialize>(filename, "equ8_sm_initialize");
            ptr_equ8_sm_deinitialize = dll_helper.resolve<equ8_sm_deinitialize>(filename, "equ8_sm_deinitialize");
            ptr_equ8_sm_user_status = dll_helper.resolve<equ8_sm_user_status>(filename, "equ8_sm_user_status");
			ptr_equ8_sm_on_data_received = dll_helper.resolve<equ8_sm_on_data_received>(filename, "equ8_sm_on_data_received");
			ptr_equ8_sm_poll_event = dll_helper.resolve<equ8_sm_poll_event>(filename, "equ8_sm_poll_event");
        }
        catch(Exception)
        {
            rc = equ8.equ8_err.EQU8_ERR_STATUS_BAD_API;
        }
        return equ8_err.create(rc);
    }

    public static equ8_err initialize(string equ8_dir, int production)
    {
		equ8_err err = resolve_api(equ8_dir, production);
		if(!err.is_ok())
		{
			return err;
		}
		
		string[] paths = { equ8_dir, "equ8_server.config" };
		err = equ8_err.create(ptr_equ8_sm_initialize(System.IO.Path.Combine(paths)));
		is_initialized = err.is_ok();
		return err;
    }

	public static void deinitialize()
	{
	   if(is_initialized)
	   {
			ptr_equ8_sm_deinitialize();
	   }
	}

	public static UInt32 user_status(string client_id)
	{
		if(is_initialized)
		{
			return ptr_equ8_sm_user_status(client_id);
		}
		return 0;
	}

	public static void on_data_received(string client_id, byte[] data)
	{
		if(is_initialized)
		{
			IntPtr addr = Marshal.UnsafeAddrOfPinnedArrayElement(data, 0);
			ptr_equ8_sm_on_data_received(client_id, addr, data.Length);
		}
	}

	public static equ8_event_id poll_event(string client_id, equ8_event ev)
	{
		if(is_initialized)
		{
			return ptr_equ8_sm_poll_event(client_id, ev.get_raw());
		}
		return equ8_event_id.none;
	}

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate UInt64 equ8_sm_initialize(string config_file);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate UInt64 equ8_sm_initialize_from_data(byte[] config, int config_size);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate void equ8_sm_deinitialize();

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate UInt32 equ8_sm_user_status(string client_id);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	private delegate void equ8_sm_on_data_received(string client_id, IntPtr data, int size);

	[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
	private delegate equ8_event_id equ8_sm_poll_event(string client_id, IntPtr e);

	static private equ8_sm_initialize ptr_equ8_sm_initialize;
    static private equ8_sm_deinitialize ptr_equ8_sm_deinitialize;
    static private equ8_sm_user_status ptr_equ8_sm_user_status;
	static private equ8_sm_on_data_received ptr_equ8_sm_on_data_received;
	static private equ8_sm_poll_event ptr_equ8_sm_poll_event;

	static private bool is_initialized = false;
}

} // namespace equ8

