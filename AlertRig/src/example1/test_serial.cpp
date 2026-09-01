#include "stdafx.h"
#include "VSGV8.H"
#include "alertlib.h"
#include "BufferedAsyncSerial.h"
//#include <boost/asio.hpp>
#include <string>
#include <iostream>

#define BUFSIZE 256

using namespace std;
using namespace alert;

#ifdef _DEBUG
#pragma comment(lib, "dalert.lib")
#else
#pragma comment(lib, "alert.lib")
#endif

#pragma comment(lib, "vsgv8.lib")

//class simpleserial
//{
//    string m_port;
//    boost::asio::io_service m_io;
//    boost::asio::serial_port m_serial;
//    string m_buffer;
//public:
//    simpleserial() : m_io(), m_serial(m_io) { };
//    virtual ~simpleserial() { if (m_serial.is_open()) m_serial.close(); };
//    void open(const string& port) 
//    {
//        m_port = port;
//        cout << "Opening " << m_port << endl;
//        m_serial.open(m_port);
//
//        // Configure basic serial port parameters: 115.2kBaud, 8N1
//        m_serial.set_option(boost::asio::serial_port_base::baud_rate(115200));
//        m_serial.set_option(boost::asio::serial_port_base::character_size(8 /* data bits */));
//        m_serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
//        m_serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
//    }
//
//    string readStringUntil(const std::string delim = "\n")
//    {
//        char data[BUFSIZE];
//        size_t n = m_serial.read_some(boost::asio::buffer(data, BUFSIZE));
//        return string(data, n);
//    }
//
//    void writeString(const std::string& s)
//    {
//        m_serial.write_some(boost::asio::buffer(s));
//    }
//
//};


int main(int argc, char **argv)
{
    string port = "COM7";
    if (argc == 2)
        port = string(argv[1]);

    BufferedAsyncSerial serial;
    //simpleserial serial;
    serial.open(port, 115200);
        //boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none),
        //boost::asio::serial_port_base::character_size(8 /* data bits */),
        //boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none),
        //boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));


    //boost::asio::io_service io;
    //// Open serial port
    //cout << "Opening " << port << endl;
    //boost::asio::serial_port serial(io, port);

    //// Configure basic serial port parameters: 115.2kBaud, 8N1
    //serial.set_option(boost::asio::serial_port_base::baud_rate(115200));
    //serial.set_option(boost::asio::serial_port_base::character_size(8 /* data bits */));
    //serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
    //serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));

    // Read data in a loop and copy to stdout
    while (true) {
        char data[BUFSIZE];
        string s = serial.readStringUntil(";");
        if (s.empty())
            Sleep(1000);
        else
        {
            // Write data to stdout
            std::cout << s << endl;
            // write data back to serial port
            serial.writeString(s);
        }
    }
	return 0;
}