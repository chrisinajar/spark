using server;
using System;
using System.Collections.Generic;
using System.Text;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;

namespace server
{
    class game_client
    {
        public game_client(IPEndPoint _addr, string _name, Int32 _x, Int32 _y)
        {
            addr = _addr;
            name = _name;
            x = _x;
            y = _y;
            ping_ok = true;
        }

        public void move_x(Int32 speed_x)
        {
            x += speed_x;
            if(x < 0)
            {
                x = 0;
            }
            else if(x + name.Length > 80)
            {
                x = 80 - name.Length;
            }
        }
        public void move_y(Int32 speed_y)
        {
            y += speed_y;
            if(y < 0)
            {
                y = 0;
            }
            else if(y > 24)
            {
                y = 24;
            }
        }

        public string unique_id() { return name; }
        public string key() { return addr.ToString(); }

        public IPEndPoint addr;
        public string name;
        public Int32 x;
        public Int32 y;
        public bool ping_ok;
    }

    class Server
    {
        private Socket socket;
        private bool exit = false;
        List<game_client> clients;
        Random random;
		private equ8.equ8_event ev;

        public Server()
        {
            clients = new List<game_client>();
            socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            socket.Bind(new IPEndPoint(IPAddress.Any, 9999));
            socket.ReceiveTimeout = 1000;
            random = new Random();
			ev = new equ8.equ8_event();
        }

        private void remove_client(int index)
        {
            Console.WriteLine("Client disconnected '{0}'", clients[index].name);
            packet p = new packet_remove_client(clients[index].name);
            send_to_all(p);
            clients.RemoveAt(index);
        }

        private bool remove_client(string remove)
        {
            for(int i = clients.Count - 1; i >= 0; i--)
            {
                if(clients[i].key() == remove)
                {
                    remove_client(i);
                    return true;
                }
            }
            return false;
        }

        private game_client get_client_by_id(string client_id)
        {
            for (int i = 0; i < clients.Count; ++i)
            {
                if (clients[i].unique_id() == client_id)
                {
                    return clients[i];
                }
            }
            return null;
        }

        private Tuple<IPEndPoint, packet> read_network()
        {
            byte [] data = new byte[512];
            EndPoint remote = new IPEndPoint(0, 0);
            try
            {
                int received = socket.ReceiveFrom(data, ref remote);
                if (received < 1)
                {
                    remove_client(remote.ToString());
                }
                else
                {
                    return Tuple.Create((IPEndPoint)remote, Packet.serialize_packet(data));
                }
            }
            catch (SocketException)
            {
            }
            return null;
        }

        void send_to_all(packet p)
        {
            byte[] raw = Packet.serialize_packet(p);
            foreach (var dest in clients)
            {
                socket.SendTo(raw, dest.addr);
            }
        }

        void send_packet(IPEndPoint addr, packet p)
        {
            byte[] raw = Packet.serialize_packet(p);
            socket.SendTo(raw, addr);
        }

        void send_packet(game_client c, packet p)
        {
            send_packet(c.addr, p);
        }

        long get_ms()
        {
            return DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond;
        }

        void send_positions()
        {
            foreach (var c in clients)
            {
                send_to_all(new packet_position(c.name, c.x, c.y));
            }
        }

        bool user_is_allowed_to_play(IPEndPoint from, string id, UInt32 ac)
        {
            var action = (equ8.equ8_action)ac;
            if (equ8.equ8_action.ban == action)
            {
                send_packet(from, new packet_violation("You are banned."));
                return false;
            }
            else if (equ8.equ8_action.timeout_15m == action)
            {
                send_packet(from, new packet_violation("You are timed-out."));
                return false;
            }
            return true;
        }

        void update_clients(IPEndPoint from, packet p)
        {
            string key = from.ToString();
            game_client gc = null;

            foreach(var c in clients)
            {
                if (c.key() == key)
                {
                    gc = c;
                }
            }

            if (null == gc)
            {
                if(packet_type.connect == p.id)
                {
                    string unique_id = ((packet_connect)p).name;
					var status = equ8.session_manager.user_status(unique_id);

                    if(!user_is_allowed_to_play(from, unique_id, status))
                    {
						Console.WriteLine(
                            "Client '{0}' not allowed to play (status: {1})",
                            unique_id, status);
                        return;
                    }

                    gc = new game_client(from, unique_id,
                        random.Next(0, 80), random.Next(0, 25));
                    clients.Add(gc);

                    Console.WriteLine("Client connect '{0}'", unique_id);
                    send_positions();
                }

                return;
            }

            switch (p.id)
            {
                case packet_type.equ8_data:
                    var payload = ((packet_equ8_data)p).data;
                    Console.WriteLine("EQU8 Received Data: user: '{0}' size: {1}",
						gc.unique_id(), payload.Length);
                    equ8.session_manager.on_data_received(gc.unique_id(), payload);
                    break;
                case packet_type.disconnect:
                    remove_client(gc.key());
                    return;
                case packet_type.move:
                    if (null != gc)
                    {
                        gc.move_x(((packet_move)p).speed_x);
                        gc.move_y(((packet_move)p).speed_y);
                    }
                    break;
                case packet_type.ping:
                    if (null != gc)
                    {
                        gc.ping_ok = true;
                    }
                    return;
            }

            send_positions();
        }
        public void run()
        {
            long ping_at = get_ms() + 5000;

            Console.TreatControlCAsInput = false;
            Console.CancelKeyPress += new ConsoleCancelEventHandler((object sender, ConsoleCancelEventArgs args) =>
            {
                args.Cancel = true;
                exit = true;
            });

            Console.WriteLine("Server waiting for clients... Ctrl-c to exit");

            while (!exit)
            {
                for (int i = clients.Count - 1; i >= 0; i--)
                {
                    game_client c = clients[i];
                    switch (equ8.session_manager.poll_event(c.name, ev))
                    {
                        case equ8.equ8_event_id.send_request:
                        {
                            Console.WriteLine(
								"EQU8 Data Send Request: user: '{0}' size: {1}",
								c.name, ev.get_send_event_size());
                            send_packet(c, new packet_equ8_data(
								ev.duplicate_send_event()));
                        }
                        break;
                        case equ8.equ8_event_id.status:
                        {
                            var action = ev.get_status_action();
                            if (!user_is_allowed_to_play(c.addr, c.name, action))
                            {
                                Console.WriteLine("Client '{0}' kicked (action: {1})",
									c.name, action);
                                remove_client(i);
                            }
                        }
                        break;
                    }
                }

                if (get_ms() >= ping_at)
                {
                    for (int i = clients.Count - 1; i >= 0; i--)
                    {
                        game_client c = clients[i];
                        if (!c.ping_ok)
                        {
                            Console.WriteLine("Kicking client {0} (late ping reply)", c.name);
                            remove_client(i);
                        }
                        else
                        {
                            c.ping_ok = false;
                        }
                    }
                    send_to_all(new packet_ping());
                    ping_at = get_ms() + 5000;
                }
                Tuple<IPEndPoint, packet> data = read_network();
                if (null != data)
                {
                    update_clients(data.Item1, data.Item2);
                }
            }
            Console.WriteLine("Server is exiting...");
        }
    }
}
