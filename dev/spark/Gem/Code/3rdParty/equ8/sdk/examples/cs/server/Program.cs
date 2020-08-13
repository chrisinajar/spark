using equ8;
using System;

namespace server
{
    class Program
    {
        static void Main(string[] args)
        {
            //
            // Initialize the EQU8 server library and exit on failure.
            //
			equ8_err equ8_rc;
            int production_mode = 0;
            equ8_rc = equ8.session_manager.initialize(@"equ8_server",
				production_mode);
            if(!equ8_rc.is_ok())
            {
                Console.WriteLine("EQU8 error: {0}", equ8_rc.get_full());
                return;
            }
            new Server().run();
            //
            // Deinitialize the EQU8 client library.
            //
            equ8.session_manager.deinitialize();
        }
    }
}
