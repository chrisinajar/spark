using server;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace client
{
    class Client
    {
        class client_pos
        {
            public client_pos(Int32 _x, Int32 _y)
            {
                x = _x;
                y = _y;
            }
            public Int32 x;
            public Int32 y;
        }

        string my_name;
        Socket socket;
        IPEndPoint server;
        bool quit = false;
        Dictionary<string, client_pos> clients;
		equ8.equ8_event ev;

        static Client instance = null;

        public Client(string name, string ip)
        {
            my_name = name;
			ev = new equ8.equ8_event();
            clients = new Dictionary<string, client_pos>();
            socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            server = new IPEndPoint(IPAddress.Parse(ip), 9999);
            socket.Connect(server);

            Console.CancelKeyPress += new ConsoleCancelEventHandler((object sender, ConsoleCancelEventArgs args) =>
            {
                socket.Send(Packet.serialize_packet(new packet_disconnect()));
            });

            instance = this;
        }

        void game_thread()
        {
            while(!quit)
            {
                ConsoleKeyInfo cki = Console.ReadKey();
                Int32 speed_x = 0;
                Int32 speed_y = 0;

                if (ConsoleKey.LeftArrow == cki.Key) { --speed_x; }
                else if(ConsoleKey.RightArrow == cki.Key) { ++speed_x; }
                else if (ConsoleKey.UpArrow == cki.Key) { --speed_y; }
                else if (ConsoleKey.DownArrow == cki.Key) { ++speed_y; }

                if (0 != speed_x || 0 != speed_y)
                {
                    socket.Send(Packet.serialize_packet(new packet_move(speed_x, speed_y)));
                }
            }
        }

        void remove_player_name(string name, client_pos c)
        {
            Console.SetCursorPosition(c.x, c.y);
            for (int i = 0; i < name.Length; ++i)
            {
                Console.Write(" ");
            }
        }

        void update_position(packet_position pos)
        {
            if (!clients.ContainsKey(pos.name))
            {
                clients[pos.name] = new client_pos(pos.x, pos.y);
            }

            var c = clients[pos.name];
            remove_player_name(pos.name, c);
            c.x = pos.x;
            c.y = pos.y;
            Console.SetCursorPosition(c.x, c.y);
            Console.Write(pos.name);
        }

        void send_to_server(packet p)
        {
            socket.Send(Packet.serialize_packet(p));
        }
 
        private static void notify_and_exit(string format, params object[] args)
        {
            Console.WriteLine(format, args);
			System.Console.ReadKey();
            Environment.Exit(-1);
        }

        public static void print_equ8_status_code(equ8.equ8_err err)
        {
			if(err.is_ok())
			{
				return;
			}

			var rc = err.get_full();

			switch(err.get_code())
			{
				case equ8.equ8_err.EQU8_ERR_STATUS_DRIVER_START_FAILURE:
					notify_and_exit("EQU8 failed to install vital component (err: {0})", rc);
					break;
				case equ8.equ8_err.EQU8_ERR_STATUS_DRIVER_REQUIRE_REBOOT:
					notify_and_exit("Your system has a pending reboot which prevents the Anti-Cheat system from initializing. Please restart your computer.");	
					break;
				case equ8.equ8_err.EQU8_ERR_STATUS_SYSTEM_IN_TEST_SIGNING_MODE:
					notify_and_exit("For security reasons, EQU8 Anti-Cheat will not run in test-signing mode.");
					break;
				case equ8.equ8_err.EQU8_ERR_STATUS_CLIENT_AGENT_PROTOCOL_MISMATCH:
					notify_and_exit("Parts of the Anti-Cheat solution use different version; please re-run the launcher.");
					break;
				case equ8.equ8_err.EQU8_ERR_STATUS_BAD_API:
					notify_and_exit("Failed to load EQU8 library.");
					break;
				case equ8.equ8_err.EQU8_ERR_STATUS_AGENT_TERMINATED:
					notify_and_exit("An integral Anti-Cheat service was terminated, exiting.");
					break;
                case equ8.equ8_err.EQU8_ERR_STATUS_CORRUPT_CONFIGURATION:
                    notify_and_exit("EQU8 detected a corrupt configuration (err: 0x{0:X8}).\n", rc);
                    break;
                case equ8.equ8_err.EQU8_ERR_STATUS_AC_CONNECTION_FAILURE:
                    notify_and_exit("EQU8 failed to connect to ant-cheat backend (err: 0x{0:X8}).\n", rc);
                    break;
                default:
					notify_and_exit("Internal EQU8 error: {0}", rc);
					break;
			}
        }

        public void run()
        {
            send_to_server(new packet_connect(my_name));

            new Thread(game_thread).Start();

            while (!quit)
            {
                byte[] data = new byte [512];
                if (socket.Receive(data) <= 0)
                {
                    continue;
                }
                packet p = Packet.serialize_packet(data);
                if(packet_type.equ8_data == p.id)
                {
                    var s = ((packet_equ8_data)p);
                    equ8.client.on_data_received(s.data);
                }
                else if(packet_type.ping == p.id)
                {
                    socket.Send(Packet.serialize_packet(p));
                }
                else if(packet_type.position == p.id)
                {
                    update_position((packet_position)p);
                }
                else if(packet_type.remove_client == p.id)
                {
                    packet_remove_client remove = ((packet_remove_client)p);

                    client_pos pos;
                    if(clients.TryGetValue(remove.name, out pos))
                    {
                        remove_player_name(remove.name, pos);
                        if (my_name == remove.name)
                        {
							equ8.client.close_session();

                            Console.SetCursorPosition(0, 0);
                            Console.WriteLine("Disconnected...");
                        }
                    }
                }

                //
                // Lastly check if EQU8 has anything to send.
                //
				switch(equ8.client.poll_event(ev))
				{ 
					case equ8.equ8_event_id.error:
						print_equ8_status_code(ev.get_error_code());
						break;
					case equ8.equ8_event_id.send_request:
					{
						packet_equ8_data equ8 = new packet_equ8_data(
                            ev.duplicate_send_event());
						send_to_server(equ8);
					}
					break;
				}
            }

            equ8.client.close_session();
        }
    }
}
