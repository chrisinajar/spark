using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace server
{
    public enum packet_type : byte
    {
        connect,
        move,
        position,
        disconnect,
        ping,
        remove_client,
        violation,
        equ8_data,
    }

    public class packet
    {
        public packet(packet_type _id)
        {
            id = _id;
        }

        public packet_type id;
    }

    public class packet_ping : packet
    {
        public packet_ping()
            : base(packet_type.ping)
        {
        }
    }

    public class packet_disconnect : packet
    {
        public packet_disconnect()
            : base(packet_type.disconnect)
        {
        }
    }

    public class packet_remove_client : packet
    {
        public packet_remove_client(string _name)
            : base(packet_type.remove_client)
        {
            name = _name;
        }
        public string name;
    }

    public class packet_connect : packet
    {
        public packet_connect(string _name)
            : base(packet_type.connect)
        {
            name = _name;
        }

        public string name;
    }

    public class packet_move : packet
    {
        public packet_move(Int32 _speed_x, Int32 _speed_y)
            : base(packet_type.move)
        {
            speed_x = _speed_x;
            speed_y = _speed_y;
        }
        public Int32 speed_x;
        public Int32 speed_y;
    }

    public class packet_position : packet
    {
        public packet_position(string _name, Int32 _x, Int32 _y)
            : base(packet_type.position)
        {
            name = _name;
            x = _x;
            y = _y;
        }
        public string name;
        public Int32 x;
        public Int32 y;
    }

    public class packet_violation : packet
    {
        public packet_violation(string _msg)
            : base(packet_type.violation)
        {
            msg = _msg;
        }
        public string msg;
    }

    public class packet_equ8_data : packet
    {
        public packet_equ8_data(byte[] _data)
            : base(packet_type.equ8_data)
        {
            data = _data;
        }
        public byte[] data;
    }

    public class Packet
    {
        static public packet serialize_packet(byte [] bytes)
        {
            BinaryReader br = new BinaryReader(new MemoryStream(bytes));

            packet_type id = (packet_type)br.ReadByte();
            switch(id)
            {
                case packet_type.disconnect:
                    return new packet_disconnect();
                case packet_type.connect:
                    return new packet_connect(br.ReadString());
                case packet_type.ping:
                    return new packet_ping();
                case packet_type.move:
                    Int32 speed_x = br.ReadInt32();
                    Int32 speed_y = br.ReadInt32();
                    return new packet_move(speed_x, speed_y);
                case packet_type.position:
                    string name = br.ReadString();
                    Int32 x = br.ReadInt32();
                    Int32 y = br.ReadInt32();
                    return new packet_position(name, x, y);
                case packet_type.remove_client:
                    return new packet_remove_client(br.ReadString());
                case packet_type.violation:
                    return new packet_violation(br.ReadString());
                case packet_type.equ8_data:
                {
                    UInt32 size = br.ReadUInt32();
                    return new packet_equ8_data(br.ReadBytes((int)size));
                }
                default:
                    throw new System.ArgumentException("No idea how to deserialize that...");
            }
        }

        static public byte [] serialize_packet(packet p)
        {
            MemoryStream stream = new MemoryStream();
            BinaryWriter bw = new BinaryWriter(stream);

            bw.Write((byte)p.id);

            switch (p.id)
            {
                case packet_type.ping:
                case packet_type.disconnect:
                    break;
                case packet_type.connect:
                    bw.Write(((packet_connect)p).name);
                    break;
                case packet_type.move:
                    bw.Write(((packet_move)p).speed_x);
                    bw.Write(((packet_move)p).speed_y);
                    break;
                case packet_type.position:
                    bw.Write(((packet_position)p).name);
                    bw.Write(((packet_position)p).x);
                    bw.Write(((packet_position)p).y);
                    break;
                case packet_type.remove_client:
                    bw.Write(((packet_remove_client)p).name);
                    break;
                case packet_type.violation:
                    bw.Write(((packet_violation)p).msg);
                    break;
                case packet_type.equ8_data:
                    bw.Write(((packet_equ8_data)p).data.Length);
                    bw.Write(((packet_equ8_data)p).data);
                    break;
                default:
                    throw new System.ArgumentException("Unknown packet id");
            }

            return stream.ToArray();
        }
    }
}
