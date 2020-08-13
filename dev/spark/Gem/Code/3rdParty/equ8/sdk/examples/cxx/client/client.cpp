#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS // inet_addr.
#include <windows.h>
#include <winsock2.h>
#include <stdint.h>

#include <iostream>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>

#include "../server/packet.hpp"
#include <equ8_client/client.c>

#pragma comment(lib, "ws2_32.lib")

void print_equ8_status_code(equ8_err_t code);

static bool quit = false;

class client;
client *instance = nullptr;

class client
{
private:
	struct client_pos
	{
		client_pos(int32_t _x, int32_t _y)
			: x{_x}
			, y{_y}
		{
		}
		int32_t x;
		int32_t y;
	};
private:
	static sockaddr_in to_address(const std::string &ip)
	{
		sockaddr_in sa{};
		sa.sin_family = AF_INET;
		sa.sin_addr.s_addr = inet_addr(ip.c_str());
		sa.sin_port = htons(9999);
		return sa;
	}
	static SOCKET create_socket()
	{
		SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
		if(SOCKET_ERROR == s)
		{
			throw new std::runtime_error("Kernel out of ram");
		}
		return s;
	}

	void game_thread()
	{
		auto h = GetStdHandle(STD_INPUT_HANDLE);

		while(!quit)
		{
			INPUT_RECORD ir;
			DWORD n;
			if(ReadConsoleInputW(h, &ir, 1, &n)
			&& 1 == n)
			{
				if(KEY_EVENT != ir.EventType || !ir.Event.KeyEvent.bKeyDown)
				{
					continue;
				}

				int32_t speed_x = 0;
				int32_t speed_y = 0;

				auto vk = ir.Event.KeyEvent.wVirtualKeyCode;

				if(VK_LEFT == vk) { --speed_x; }
				else if(VK_RIGHT == vk) { ++speed_x; }
				else if(VK_UP == vk) { --speed_y; }
				else if(VK_DOWN == vk) { ++speed_y; }

				if(0 != speed_x || 0 != speed_y)
				{
					packet_move m{speed_x, speed_y};
					send_to_server(&m);
				}
			}
		}
	}

	void set_cursor_pos(int x, int y)
	{
		COORD c;
		c.X = x;
		c.Y = y;
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
	}

	void set_cursor_pos(const client_pos &pos)
	{
		set_cursor_pos(pos.x, pos.y);
	}

	void remove_player_name(const std::string &name, const client_pos &pos)
	{
		set_cursor_pos(pos);
		for(size_t i = 0; i < name.length(); ++i)
		{
			putchar(' ');
		}
	}

	void update_position(const packet_position *pos)
	{
		auto it = m_clients.find(pos->name);
		if(m_clients.end() == it)
		{
			it = m_clients.emplace(pos->name, client_pos(pos->x, pos->y)).first;
		}

		auto &c = it->second;
		remove_player_name(it->first, c);
		c.x = pos->x;
		c.y = pos->y; 
		set_cursor_pos(c); 
		std::cout << it->first;
	}

	static BOOL WINAPI exit_handler(DWORD event)
	{
		if(CTRL_C_EVENT == event)
		{
			packet_disconnect disc;
			instance->send_to_server(&disc);
			return TRUE;
		}
		return FALSE;
	}

public:
	client(std::string && name, const std::string &ip)
		: m_name{std::move(name)}
		, m_socket{create_socket()}
		, m_server{to_address(ip)}
		, m_thread{&client::game_thread, this}
	{
		instance = this;
		SetConsoleCtrlHandler(exit_handler, TRUE);
	}
	~client()
	{
		SetConsoleCtrlHandler(exit_handler, FALSE);
		quit = true;
		m_thread.join();
		closesocket(m_socket);
	}

	void send_to_server(const packet *p) const
	{
		auto v{ serialize_packet(p) };
		sendto(m_socket, reinterpret_cast<const char *>(v.data()),
			static_cast<int>(v.size()), 0,
			reinterpret_cast<const sockaddr *>(&m_server), sizeof(m_server));
	}

	int run()
	{
		{
			packet_connect conn{m_name}; 
			send_to_server(&conn);
		}
		while(!quit)
		{
			std::array<uint8_t, 512> buf;
			int server_len = sizeof(m_server);

			int len = recvfrom(m_socket, reinterpret_cast<char *>(buf.data()),
				static_cast<int>(buf.size()), 0, reinterpret_cast<sockaddr *>(&m_server), &server_len);
			if(len < 1)
			{
				continue;	
			} 

			std::unique_ptr<packet> p{serialize_packet(buf, len)};
			if(packet_type::equ8_data == p->id)
			{
				auto equ8 = static_cast<const packet_equ8_data *>(p.get());
				equ8_client_on_data_received(equ8->data, equ8->size);
			}
			else if(packet_type::ping == p->id)
			{
				send_to_server(p.get());
			}
			else if(packet_type::position == p->id)
			{
				update_position(static_cast<const packet_position *>(p.get()));
			}
			else if(packet_type::remove_client == p->id)
			{
				auto rem = static_cast<const packet_remove_client *>(p.get());
				auto it = m_clients.find(rem->name);
				if(m_clients.end() != it)
				{
					remove_player_name(it->first, it->second);
				}
				if(m_name == rem->name)
				{
					equ8_client_close_session();

					set_cursor_pos(0, 0);
					std::cout << "Disconnected...";
					break;
				}
			}

			//
			// Lastly check if equ8 has any pending event for use
			//
			equ8_event_t data;
			switch(equ8_client_poll_event(&data))
			{
				case equ8_event_id_error:
				{
					print_equ8_status_code(data.error.code);
					break;
				}
				case equ8_event_id_send_request:
				{
					packet_equ8_data equ8{
						static_cast<uint8_t *>(data.send_request.payload),
						data.send_request.size };
					send_to_server(&equ8);
					break;
				}
			}
		}

		equ8_client_close_session();

		return 0;
	}
private:
	std::string m_name;
	SOCKET m_socket;
	sockaddr_in m_server;
	std::thread m_thread;

	std::unordered_map<std::string, client_pos> m_clients;
};

static void clear_screen()
{
	COORD zero = { 0, 0 };
	auto h = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if(!GetConsoleScreenBufferInfo(h, &csbi))
	{
		return;
	}

	DWORD out;
	FillConsoleOutputCharacterA(h, ' ', csbi.dwSize.X *csbi.dwSize.Y, zero, &out);
}

static void notify_and_exit(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	getchar();
	exit(-1);
}

static void print_equ8_status_code(equ8_err_t code)
{
	switch(EQU8_GET_ERR_CODE(code))
	{
	case EQU8_OK:
		break;

	case EQU8_ERR_STATUS_AGENT_TERMINATED:
		notify_and_exit("An integral Anti-Cheat service was terminated, exiting.");
		break;

	case EQU8_ERR_STATUS_CLIENT_AGENT_PROTOCOL_MISMATCH:
		notify_and_exit("Parts of the Anti-Cheat solution use different version; please re-run the launcher.");
		break;

	case EQU8_ERR_STATUS_SYSTEM_IN_TEST_SIGNING_MODE:
		notify_and_exit("For security reasons, EQU8 Anti-Cheat will not run in test-signing mode.");
		break;

	case EQU8_ERR_STATUS_DRIVER_REQUIRE_REBOOT:
		notify_and_exit("Your system has a pending reboot which prevents the Anti-Cheat system from initializing. Please restart your computer.\n");
		break;

	case EQU8_ERR_STATUS_CORRUPT_CONFIGURATION:
		notify_and_exit("EQU8 detected a corrupt configuration (err: %08llX).\n", code);
		break;

	case EQU8_ERR_STATUS_AC_CONNECTION_FAILURE:
		notify_and_exit("EQU8 failed to connect to ant-cheat backend (err: %08llX).\n", code);
		break;

	case EQU8_ERR_STATUS_DRIVER_START_FAILURE:
		notify_and_exit("EQU8 failed to install vital component (err: %08llX).\n", code);
		break;

	default:
		notify_and_exit("Internal EQU8 failure error: %08llX\n", code);
		break;
	}
}

int main()
{
	int rc = -1;
	int production_mode = 1;
	equ8_err_t equ8_rc = equ8_client_initialize("equ8_client", production_mode);
	if(!EQU8_SUCCESS(equ8_rc))
	{
		print_equ8_status_code(equ8_rc);
		return rc;
	}

	try
	{
		WSADATA data;
		WSAStartup(0x202, &data);

		std::string name;
		std::string ip;

		while(name.empty())
		{
			std::cout << "Enter your name: ";
			std::getline(std::cin, name);
		}
		std::cout << "Server IP address [127.0.0.1]: ";
		std::getline(std::cin, ip);
		std::cout << "'" << ip << "'" << std::endl;
	
		if(ip.empty())
		{
			ip = "127.0.0.1";
		}

		clear_screen();
		client c{std::move(name), ip};
		rc = c.run();
	}
	catch(const std::exception &e)
	{
		std::cout << "EXCEPTION: " << e.what();
	}

	equ8_client_deinitialize();
	return rc;
}



