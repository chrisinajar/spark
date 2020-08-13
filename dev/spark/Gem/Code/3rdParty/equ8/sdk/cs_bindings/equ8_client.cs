using System;
using System.Runtime.InteropServices;

namespace equ8
{
    public class client
    {
	    private static equ8_err resolve_api(string equ8_dir, int production)
		{
			string[] paths =
			{
				equ8_dir,
				System.Environment.Is64BitProcess
					? ((0 != production) ? @"client.x64.equ8.dll" : @"client.null.x64.equ8.dll")
					: ((0 != production) ? @"client.x86.equ8.dll" : @"client.null.x86.equ8.dll")
			};

			UInt64 rc = 0;
			string filename = System.IO.Path.Combine(paths);

			try
			{
				ptr_equ8_client_initialize = dll_helper.resolve<equ8_client_initialize>(filename, "equ8_client_initialize");
				ptr_equ8_client_deinitialize = dll_helper.resolve<equ8_client_deinitialize>(filename, "equ8_client_deinitialize");
				ptr_equ8_client_on_data_received = dll_helper.resolve<equ8_client_on_data_received>(filename, "equ8_client_on_data_received");
				ptr_equ8_client_poll_event = dll_helper.resolve<equ8_client_poll_event>(filename, "equ8_client_poll_event");
				ptr_equ8_client_close_session = dll_helper.resolve<equ8_client_close_session>(filename, "equ8_client_close_session");
			}
			catch(Exception)
			{
				rc = 0x60050000;
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
			err = equ8_err.create(ptr_equ8_client_initialize());
			is_initialized = err.is_ok();
			return err;
		}

		public static void deinitialize()
		{
			if(is_initialized)
			{
				ptr_equ8_client_deinitialize();
			}
		}

		public static void on_data_received(byte[] data)
		{
			if(is_initialized)
			{
				IntPtr addr = Marshal.UnsafeAddrOfPinnedArrayElement(data, 0);
				ptr_equ8_client_on_data_received(addr, data.Length);
			}
		}

		public static equ8_event_id poll_event(equ8_event ev)
		{
			if(is_initialized)
			{
				return ptr_equ8_client_poll_event(ev.get_raw());
			}
			return equ8_event_id.none;
		}

		public static void close_session()
		{
			if(is_initialized)
			{
				ptr_equ8_client_close_session();
			}
		}

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private delegate UInt64 equ8_client_initialize();

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private delegate void equ8_client_deinitialize();

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private delegate void equ8_client_on_data_received(IntPtr raw, int size);

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private delegate equ8_event_id equ8_client_poll_event(IntPtr ev);

		[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
		private delegate void equ8_client_close_session();

		private static equ8_client_initialize ptr_equ8_client_initialize;
		private static equ8_client_deinitialize ptr_equ8_client_deinitialize;
		private static equ8_client_on_data_received ptr_equ8_client_on_data_received;
		private static equ8_client_poll_event ptr_equ8_client_poll_event;
		private static equ8_client_close_session ptr_equ8_client_close_session;

		private static bool is_initialized = false;
	}
}
