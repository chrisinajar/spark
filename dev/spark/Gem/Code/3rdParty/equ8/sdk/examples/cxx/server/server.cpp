#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <stdint.h>

#include <array>
#include <memory>
#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <chrono>

#include <equ8_session_manager/session_manager.c>

#include "packet.hpp"

#pragma comment(lib, "ws2_32.lib")

static uint64_t get_ms()
{
	return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()).count());
}

bool g_exit = false;

BOOL WINAPI console_handler(DWORD signal)
{
	if(signal <= CTRL_CLOSE_EVENT)
	{
		g_exit = true;
	}
	return TRUE;
}

class server
{
private:
	struct game_client
	{
		game_client(const sockaddr_in &sa, const std::string &_name, int32_t _x, int32_t _y)
			: remote{sa}
			, name{_name}
			, x{_x}
			, y{_y}
			, ping_ok{true}
		{
		}

		void move_x(int32_t speed_x)
		{
			x += speed_x;
			if(x < 0)
			{
				x = 0;
			}
			else if((x + name.length()) > 80)
			{
				x = static_cast<int>(80 - name.length());
			}
		}
		void move_y(int32_t speed_y)
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

		bool equal(const sockaddr_in &in) const
		{
			return in.sin_family == remote.sin_family
				&& in.sin_addr.s_addr == remote.sin_addr.s_addr
				&& in.sin_port == remote.sin_port;
		}

		const sockaddr *remote_addr() const
		{
			return reinterpret_cast<const sockaddr *>(&remote);
		}

		const sockaddr_in &remote_in() const
		{
			return remote;
		}

		int remote_addr_len() const
		{
			return static_cast<int>(sizeof(remote));
		}
		const char *unique_id() const
		{
			return name.c_str();
		}
		std::string name;
		int x;
		int y;
		bool ping_ok;
		sockaddr_in remote;
	};

private:
	static SOCKET bind_socket(uint16_t port)
	{
		SOCKET sd = socket(AF_INET, SOCK_DGRAM, 0);
		if(SOCKET_ERROR == sd)
		{
			throw std::runtime_error("Kernel is out of ram");
		}

		uint32_t timeout_ms = 3000;

		if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char *>(&timeout_ms), sizeof(timeout_ms)) < 0)
		{
			throw std::runtime_error("Failed to set socket timeout");
		}

		sockaddr_in sa{};
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = INADDR_ANY;
		sa.sin_port = htons(port);

		if(bind(sd, (const sockaddr *)&sa, sizeof(sa)) < 0)
		{
			closesocket(sd);
			throw std::runtime_error("Failed to bind socket");
		}
		return sd;
	}
private:
	packet *read_network(sockaddr_in &cl)
	{
		std::array<uint8_t, 512> v;

		int len = sizeof(cl);
		auto n = recvfrom(m_socket, reinterpret_cast<char *>(v.data()), static_cast<int>(v.size()), 0, reinterpret_cast<sockaddr *>(&cl), &len);
		if(n < 1)
		{
			const auto error = WSAGetLastError();
			if(10060 != error && 10054 != error)
			{
				throw std::runtime_error("Unknown socket error: " + std::to_string(error));
			}
			return nullptr;
		}
		return serialize_packet(v, n);
	}

	typedef std::vector<game_client>::iterator gc_iterator;

	gc_iterator remove_client(gc_iterator it)
	{
		std::cout << "Client disconnect '" << it->name << "'" << std::endl;

		packet_remove_client p{ std::move(it->name) };
		send_to_all(&p);
	
		return m_clients.erase(it);
	}

	void remove_clients_from(gc_iterator it)
	{
		while(it < m_clients.end())
		{
			it = remove_client(it);
		}
	}

	void send_packet_to_client(const char* client_id, const packet *p)
	{
		const std::string name{ client_id };
		for(const auto &c : m_clients)
		{
			if(name == c.unique_id())
			{
				send_packet(c, p);
				return;
			}
		}
	}

	void send_to_all(const packet *p)
	{
		const std::array<uint8_t, 512> v{serialize_packet(p)};
		for(auto &ref : m_clients)
		{
			sendto(m_socket, reinterpret_cast<const char *>(v.data()), static_cast<int>(v.size()), 0, ref.remote_addr(), ref.remote_addr_len());
		}
	}

	void send_packet(const sockaddr *sa, int size, const packet *p)
	{
		const std::array<uint8_t, 512> v{ serialize_packet(p) };
		sendto(m_socket, reinterpret_cast<const char *>(v.data()), static_cast<int>(v.size()), 0, sa, size);
	}

	void send_packet(const game_client &gc, const packet *p)
	{
		send_packet(gc.remote_addr(), gc.remote_addr_len(), p);
	}

	void send_positions()
	{
		for(const auto &c : m_clients)
		{
			packet_position pos{ c.name, c.x, c.y };
			send_to_all(&pos);
		}
	}

	bool user_is_allowed_to_play(const sockaddr_in &from,
		const std::string &unique_id, uint32_t action)
	{
		auto from_sa = reinterpret_cast<const sockaddr *>(&from);

		if(equ8_action_ban == action)
		{
			packet_violation pv{ "You are banned." };
			send_packet(from_sa, sizeof(from), &pv);
			return false;
		}
		else if(equ8_action_timeout_15m == action)
		{
			packet_violation pv{ "You are timed-out." };
			send_packet(from_sa, sizeof(from), &pv);
			return false;
		}
		return true;
	}

	void update_clients(const sockaddr_in &from, const packet *p)
	{
		auto it = m_clients.begin();
		while(m_clients.end() != it)
		{
			if(it->equal(from))
			{
				break;
			}

			++it;
		}

		if(it == m_clients.end())
		{
			if(packet_type::connect == p->id)
			{
				const std::string &unique_id = 
					static_cast<const packet_connect *>(p)->name;
				
			    uint32_t status = equ8_sm_user_status(unique_id.c_str());
				if(!user_is_allowed_to_play(from, unique_id, status))
				{
					std::cout << "Client '" << unique_id
						<< "' not allowed to play (action: "
						<< status << ")" << std::endl;
					return;
				}

				m_clients.emplace_back(game_client{
					from, unique_id.c_str(), rand() % 8, rand() % 25 });

				std::cout << "Client connect '" << unique_id << "'" << std::endl;

				send_positions();
			}
			return;
		}

		switch(p->id)
		{
			case packet_type::equ8_data:
				std::cout << "EQU8 Received Data from user: '"
					<< it->unique_id() << "' size: "
					<< static_cast<const packet_equ8_data *>(p)->size
					<< std::endl;
				equ8_sm_on_data_received(it->unique_id(),
					static_cast<const packet_equ8_data *>(p)->data,
					static_cast<const packet_equ8_data *>(p)->size);
				return;
			case packet_type::disconnect:
				remove_client(it);
				return;
			case packet_type::move:
				it->move_x(static_cast<const packet_move *>(p)->speed_x);
				it->move_y(static_cast<const packet_move *>(p)->speed_y);
				break;
			case packet_type::ping:
				it->ping_ok = true;
				return;
		}

		send_positions();
	}

public:
	server()
		: m_socket{bind_socket(9999)}
	{
	}

	~server()
	{
		closesocket(m_socket);
	}

	int run()
	{
		SetConsoleCtrlHandler(console_handler, TRUE);

		auto ping_at = get_ms() + 1000;

		std::cout << "Server waiting for clients...  Ctrl-c to exit" << std::endl;

		while(!g_exit)
		{
			equ8_event_t equ8_data;

			remove_clients_from(std::partition(m_clients.begin(), m_clients.end(), [this, &equ8_data](game_client &c)
			{
				switch(equ8_sm_poll_event(c.unique_id(), &equ8_data))
				{
					case equ8_event_id_send_request:
					{
						std::cout << "EQU8 Data Send Request: user: '"
							<< c.unique_id() << "' size: "
							<< equ8_data.send_request.size << std::endl;
						packet_equ8_data session{
							equ8_data.send_request.payload,
							equ8_data.send_request.size };
						//
						// Send the EQU8 data to the client
						//
						send_packet(c, &session);
						break;
					}
					case equ8_event_id_status:
					{
						if(!user_is_allowed_to_play(c.remote_in(), c.unique_id(),
							equ8_data.status.action))
						{
							std::cout << "Client '" << c.unique_id()
								<< "' kicked (action: "
								<< equ8_data.status.action << ")" << std::endl;
							return false;
						}
						break;
					}
				}
				return true;
			}));

			if(get_ms() >= ping_at)
			{
				remove_clients_from(std::partition(m_clients.begin(), m_clients.end(), [](game_client &c)
				{
					if(c.ping_ok)
					{
						c.ping_ok = false;
						return true;
					}
					return false;
				}));
				packet_ping ping{};
				send_to_all(&ping);
				ping_at = get_ms() + 1000;
			}

			sockaddr_in client;
			std::unique_ptr<packet> data{read_network(client)};
			if(data)
			{
				update_clients(client, data.get());
			}
		}

		std::cout << "Server is exiting..." << std::endl;

		return 0;
	}

private:
	SOCKET m_socket;
	std::vector<game_client> m_clients;
};

int main()
{
	int rc = -1;
	equ8_err_t equ8_rc;
	//
	// Initialize the EQU8 server library and exit on failure.
	//
	int production_mode = 1;
	if(!EQU8_SUCCESS(equ8_rc = equ8_sm_initialize("equ8_server", production_mode)))
	{
		std::cout << "EQU8 server init failed: " << std::hex
			<< equ8_rc << std::endl;
		return rc;
	}

	try
	{
		WSADATA data;
		WSAStartup(0x202, &data);

		server s{};
		rc = s.run();
	}
	catch(const std::exception &e)
	{
		std::cout << "EXCEPTION: " << e.what();
	}
	//
	// Deinitialize the EQU8 client library.
	//
	equ8_sm_deinitialize();

	return rc;
}




