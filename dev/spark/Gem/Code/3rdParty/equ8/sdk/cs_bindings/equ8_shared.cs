using System;
using System.Runtime.InteropServices;

namespace equ8
{
	public struct equ8_err
	{
		public const UInt32 EQU8_ERR_STATUS_AGENT_TERMINATED = 0x60000000;
		public const UInt32 EQU8_ERR_STATUS_CLIENT_AGENT_PROTOCOL_MISMATCH = 0x60010000;
		public const UInt32 EQU8_ERR_STATUS_SYSTEM_IN_TEST_SIGNING_MODE = 0x60020000;
		public const UInt32 EQU8_ERR_STATUS_DRIVER_REQUIRE_REBOOT = 0x60030000;
		public const UInt32 EQU8_ERR_STATUS_DRIVER_START_FAILURE = 0x60040000;
		public const UInt32 EQU8_ERR_STATUS_BAD_API = 0x60050000;
		public const UInt32 EQU8_ERR_STATUS_CORRUPT_CONFIGURATION = 0x60060000;
		public const UInt32 EQU8_ERR_STATUS_AC_CONNECTION_FAILURE = 0x60070000;

		public equ8_err(UInt64 err)
		{
			error_code = err;
		}
		public bool is_ok()
		{
			return (0 == get_code());
		}
		public UInt64 get_full()
		{
			return error_code;
		}
		public UInt32 get_line()
		{
			return to_line(error_code);
		}
		public UInt32 get_code()
		{
			return to_error_code(error_code);
		}
		public UInt32 get_extended()
		{
			return to_extended(error_code);
		}
		public static UInt32 to_error_code(UInt64 code)
		{
			return (UInt32)(code & 0xFFFF0000);
		}
		public static UInt32 to_extended(UInt64 code)
		{
			return (UInt32)(code >> 32);
		}
		public static UInt32 to_line(UInt64 code)
		{
			return (UInt32)(code & 0xFFFF);
		}
		public static equ8_err create(UInt64 code)
		{
			if(0 != to_error_code(code))
			{
				return new equ8_err(code);
			}
			return ok_err;
		}

		static equ8_err ok_err = new equ8_err(0);
		UInt64 error_code;
	}

    public enum equ8_event_id : UInt32
    {
        none,         // No event
        error,        // Client & server
        send_request, // Client & server
        status        // Server-only
    };

	public class equ8_event
	{
		public equ8_event()
		{
			buf = new byte [0x200];
			gc_handle = GCHandle.Alloc(buf, GCHandleType.Pinned);
			raw_addr = gc_handle.AddrOfPinnedObject();
		}

		~equ8_event()
		{
			gc_handle.Free();
		}

		public IntPtr get_raw()
		{
			return raw_addr;
		}

		public equ8_event_id get_event_id()
		{
			return (equ8_event_id)Marshal.ReadInt32(raw_addr);
		}

		public equ8_err get_error_code()
		{
			return equ8_err.create((UInt64)Marshal.ReadInt64(IntPtr.Add(raw_addr, 4)));
		}

		public UInt32 get_status_action()
		{
			return (UInt32)Marshal.ReadInt32(IntPtr.Add(raw_addr, 4));
		}

		public int get_send_event_size()
		{
			return (int)Marshal.ReadInt32(IntPtr.Add(raw_addr, 4));
		}

		public void copy_send_event_data(byte[] dest, int dest_index)
		{
			Marshal.Copy(IntPtr.Add(raw_addr, 8), dest, dest_index, get_send_event_size());
		}

		public byte [] duplicate_send_event()
		{
			byte [] data = new byte [get_send_event_size()];
			copy_send_event_data(data, 0);
			return data;
		}

		private byte[] buf;
		private GCHandle gc_handle;
		private IntPtr raw_addr;
	}

    public class dll_helper
    {
        [DllImport("kernel32.dll")]
        private static extern IntPtr GetProcAddress(IntPtr module, string function);

        [DllImport("kernel32.dll")]
        private static extern IntPtr LoadLibrary(string filename);

        public static T resolve<T>(string filename, string function)
        {
            IntPtr mod = LoadLibrary(filename);
            if(mod == IntPtr.Zero)
            {
                throw new Exception("No module");
            }
            IntPtr proc = GetProcAddress(mod, function);
            if(proc == IntPtr.Zero)
            {
                throw new Exception("No function");
            }
            return (T)(object)Marshal.GetDelegateForFunctionPointer(proc, typeof(T));
        }
    }
}

