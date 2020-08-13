#ifndef __PACKET_HPP_DEF__
#define __PACKET_HPP_DEF__

#include <equ8_meta/shared.h>

#include <array>
#include <stdint.h>
#include <type_traits>

enum class packet_type : uint8_t
{
	connect,
	move,
	position,
	disconnect,
	ping,
	remove_client,
	violation,
	equ8_data
};

struct packet
{
	packet(packet_type _id)
		: id{_id}
	{
	}
	packet_type id;
};

struct packet_ping : public packet
{
	packet_ping()
		: packet{packet_type::ping}
	{
	}
};

struct packet_disconnect : public packet
{
	packet_disconnect()
		: packet{packet_type::disconnect}
	{
	}
};

struct packet_remove_client : public packet
{
	packet_remove_client(std::string && _name)
		: packet{packet_type::remove_client}
		, name{std::move(_name)}
	{
	}
	std::string name;
};

struct packet_connect : public packet
{
	packet_connect(std::string _name)
		: packet{packet_type::connect}
		, name{std::move(_name)}
	{
	}

	std::string name;
};

struct packet_move : public packet
{
	packet_move(int32_t _speed_x, int32_t _speed_y)
		: packet{packet_type::move}
		, speed_x{_speed_x}
		, speed_y{_speed_y}
	{
	}
	int32_t speed_x;
	int32_t speed_y;
};

struct packet_position : public packet
{
	packet_position(std::string _name, int32_t _x, int32_t _y)
		: packet{packet_type::position}
		, name{_name}
		, x{_x}
		, y{_y}
	{
	}
	std::string name;
	int32_t x;
	int32_t y;
};

struct packet_violation : public packet
{
	packet_violation(std::string && _msg)
		: packet{packet_type::violation}
		, msg{std::move(_msg)}
	{
	}
	std::string msg;
};

struct packet_equ8_data : public packet
{
	packet_equ8_data(const uint8_t *p, uint32_t n)
		: packet{packet_type::equ8_data}
		, size{n}
	{
		if(n > sizeof(data))
		{
			throw std::runtime_error("EQU8 buffer too large!");
		}
		memcpy(data, p, n);
	}

	uint32_t size;
	uint8_t data[256];
};

static void throw_on_oob(const uint8_t *p, const uint8_t *end, size_t n)
{
	if((p + n) > end)
	{
		throw std::runtime_error("Packet malformed");
	}
}

class binary_reader
{
public:
	binary_reader(const uint8_t *start, size_t n)
		: p{start}
		, end{start + n}
	{
	}

	template <typename T>
	T read_int()
	{
		static_assert(std::is_integral<T>::value ||std::is_enum<T>::value, "Only ints in read-int");
		throw_on_oob(p, end, sizeof(T));
		T n = *reinterpret_cast<const T *>(p); 
		p += sizeof(T);
		return n;
	}

	const uint8_t * get_direct(size_t n)
	{
		throw_on_oob(p, end, n);
		auto r = p;
		p += n;
		return r;
	}

	std::string read_string()
	{
		uint8_t len = read_int<uint8_t>();
		throw_on_oob(p, end, len);
		std::string s{};
		s.resize(len);
		memcpy(const_cast<char *>(s.data()), p, len);
		p += len;
		return s;
	}

private:
	const uint8_t *p;
	const uint8_t *end;
};

static packet *serialize_packet(const std::array<uint8_t, 512> &v, int size)
{ 
	binary_reader br{v.data(), static_cast<size_t>(size)};

	switch(br.read_int<packet_type>())
	{
		case packet_type::disconnect:
			return new packet_disconnect{};
		case packet_type::connect:
			return new packet_connect{br.read_string()};
		case packet_type::ping:
			return new packet_ping{};
		case packet_type::move:
		{
			auto speed_x = br.read_int<int32_t>();
			auto speed_y = br.read_int<int32_t>();
			return new packet_move{speed_x, speed_y};
		}
		case packet_type::position:
		{
			auto name{br.read_string()};
			auto x = br.read_int<int32_t>();
			auto y = br.read_int<int32_t>();
			return new packet_position{std::move(name), x, y};
		}
		case packet_type::remove_client:
			return new packet_remove_client{br.read_string()};
		case packet_type::violation:
		{
			return new packet_violation{br.read_string()};
		}
		case packet_type::equ8_data:
		{
			uint32_t size = br.read_int<uint32_t>();
			const uint8_t *p = br.get_direct(size);
			return new packet_equ8_data{ p, size };
		}
		default:
			throw std::runtime_error("Unsupported packet received");
	}
}

class binary_writer
{
public:
	binary_writer()
		: p{raw.data()}
		, end{raw.data() + raw.size()}
	{
	}

	template <typename T>
	void write_int(const T n)
	{
		static_assert(std::is_integral<T>::value || std::is_enum<T>::value, "Only integrals in read-int");
		throw_on_oob(p, end, sizeof(n));

		*reinterpret_cast<T *>(p) = n;
		p += sizeof(n);
	}

	void write_string(const std::string &s)
	{
		write_int(static_cast<uint8_t>(s.size()));
		throw_on_oob(p, end, s.size());
		memcpy(p, s.data(), s.size());
		p += s.size();
	}

	void write_buffer(uint32_t size, const uint8_t *data)
	{
		write_int(size);
		throw_on_oob(p, end, size);
		memcpy(p, data, size);
		p += size;
	}

	std::array<uint8_t, 512> get_raw() const
	{
		return raw;
	}

private:
	std::array<uint8_t, 512> raw;
	uint8_t *p;
	uint8_t *end;
};

static std::array<uint8_t, 512> serialize_packet(const packet *p)
{
	binary_writer bw{};
	bw.write_int(p->id);
	switch(p->id)
	{
		case packet_type::ping:
		case packet_type::disconnect:
			break;
		case packet_type::connect:
			bw.write_string(static_cast<const packet_connect *>(p)->name);
			break;
		case packet_type::move:
			bw.write_int(static_cast<const packet_move *>(p)->speed_x);
			bw.write_int(static_cast<const packet_move *>(p)->speed_y);
			break;
		case packet_type::position:
			bw.write_string(static_cast<const packet_position *>(p)->name);
			bw.write_int(static_cast<const packet_position *>(p)->x);
			bw.write_int(static_cast<const packet_position *>(p)->y);
			break;
		case packet_type::remove_client:
			bw.write_string(static_cast<const packet_remove_client *>(p)->name);
			break;
		case packet_type::violation:
			bw.write_string(static_cast<const packet_violation *>(p)->msg);
			break;
		case packet_type::equ8_data:
			bw.write_buffer(static_cast<const packet_equ8_data *>(p)->size,
				static_cast<const packet_equ8_data *>(p)->data);
			break;
		default:
			throw std::runtime_error("Unsupported packet type");
	}
	return bw.get_raw();
}

#endif // __PACKET_HPP_DEF__
