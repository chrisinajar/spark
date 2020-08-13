using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace client
{
    class Program
    {
        static void Main(string[] args)
        {
            //
            // Initialize the EQU8 client library and exit on failure.
            //
            int production_mode = 0;
            equ8.equ8_err err = equ8.client.initialize(@"equ8_client",
				production_mode);
            if(!err.is_ok())
            {
                Console.WriteLine("EQU8 err {0}", err.get_full());
                return;
            }

            string name = "";
            while(String.IsNullOrEmpty(name))
            {
                Console.Write("Enter your name: ");
                name = Console.ReadLine().Trim();
            }
            Console.Write("Server IP address [127.0.0.1]: ");
            string ip = Console.ReadLine().Trim();
            if(0 == ip.Length)
            {
                ip = "127.0.0.1";
            }

            Console.Clear();
            new Client(name, ip).run();
            //
            // Deinitialize the EQU8 client library.
            //
            equ8.client.deinitialize();
        }
    }
}
