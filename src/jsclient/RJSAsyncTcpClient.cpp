/*
 * RJSAsyncTcpClient.cpp
 *
 *  Created on: Sep 15, 2016
 *      Author: dan
 */

#include "RJSAsyncTcpClient.h"
#include <exception>

namespace render {

void RJSAsyncTcpClient::initiate_resolve(const std::string& server)
{
	// parse server and port

	size_t pos = server.find(":");
	if (pos == std::string::npos)
		throw std::runtime_error(std::string("Cannot parse server:port from string \"") + server + std::string("\""));
	std::string port_string = server.substr(pos + 1);
	std::string server_ip_or_host = server.substr(0, pos);

	// Start an asynchronous resolve to translate the server and service names
	// into a list of endpoints.

	boost::asio::ip::tcp::resolver::query query(server_ip_or_host, port_string);
	m_resolver.async_resolve(query,
			boost::bind(&RJSAsyncTcpClient::handle_resolve, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::iterator));
}

//RJSAsyncTcpClient::RJSAsyncTcpClient(boost::asio::io_service& io_service, const std::string& server, const std::string& path)


RJSAsyncTcpClient::RJSAsyncTcpClient(boost::asio::io_service& io_service, const std::string& server, const boost::filesystem::path& file)
: m_resolver(io_service)
, m_socket(io_service)
, m_bUseFile(false)
, m_bUseString(false)
{
	// Open source file and deduce size.

	m_ifstreamSourceFile.open(file,
			std::ios_base::binary | std::ios_base::ate);
	if (!m_ifstreamSourceFile)
	{
		throw std::runtime_error(std::string("RJSAsyncTcpClient cannot open file"));
	}
	size_t file_size = m_ifstreamSourceFile.tellg();
	m_ifstreamSourceFile.seekg(0);
	m_bUseFile = true;

	// first send size followed by a single "\n". Its not sent here, actually.... it will
	// be sent in the connect_handler.

	std::ostream request_stream(&m_streambufRequest);
	request_stream << file_size << "\n\n";

	// parse server:port string and start the resolver

	initiate_resolve(server);
}


RJSAsyncTcpClient::RJSAsyncTcpClient(boost::asio::io_service& io_service, const std::string& server, const std::string& sendme)
: m_resolver(io_service)
, m_socket(io_service)
, m_bUseFile(false)
, m_bUseString(false)
{
	// Create istringstream
	m_stringstreamSourceString.str(sendme);
	m_stringstreamSourceString.seekg(0);
	size_t string_size = sendme.size();
	m_bUseString = true;

	// first send size followed by a single "\n". Its not sent here, actually.... it will
	// be sent in the connect_handler.

	std::ostream request_stream(&m_streambufRequest);
	request_stream << string_size;

	// parse server:port string and start the resolver

	initiate_resolve(server);
}

std::istream& RJSAsyncTcpClient::getIStream()
{
	if (m_bUseFile) return m_ifstreamSourceFile;
	else if (m_bUseString) return m_stringstreamSourceString;
	else throw std::runtime_error(std::string("Unknown IStream type in RJSAsyncTcpClient::getIStream()"));
}

void RJSAsyncTcpClient::handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
	if (!err)
	{
		// Attempt a connection to the first endpoint in the list. Each endpoint
		// will be tried until we successfully establish a connection.
		boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
		m_socket.async_connect(endpoint,
				boost::bind(&RJSAsyncTcpClient::handle_connect, this,
						boost::asio::placeholders::error,
						++endpoint_iterator));
	}
	else
	{
		throw std::runtime_error("RJSAsyncTcpClient::handle_resolve error: " + err.message());
	}
}

void RJSAsyncTcpClient::handle_connect(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
	if (!err)
	{
		// The connection was successful. Send the file size "request".
		// After this is written, we will land in handle_write_file, where
		// the file itself (or string, etc) will be written.
		boost::asio::async_write(m_socket, m_streambufRequest,
				boost::bind(&RJSAsyncTcpClient::handle_write_file, this,
						boost::asio::placeholders::error));
	}
	else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
	{
		// The connection failed. Try the next endpoint in the list.
		m_socket.close();
		boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
		m_socket.async_connect(endpoint,
				boost::bind(&RJSAsyncTcpClient::handle_connect, this,
						boost::asio::placeholders::error,
						++endpoint_iterator));
	}
	else
	{
		throw std::runtime_error("RJSAsyncTcpClient::handle_connect error: " + err.message());
	}
}

void RJSAsyncTcpClient::handle_write_file(const boost::system::error_code& err)
{
	if (!err)
	{
		if (getIStream().eof() == false)
		{
			getIStream().read(m_buf.c_array(), (std::streamsize) m_buf.size());
			if (getIStream().gcount() <= 0)
			{
				throw std::runtime_error("RJSAsyncTcpClient::handle_write read error: cannot read.");
			}
			std::cout << "send " << getIStream().gcount() << " bytes, total:"
					<< getIStream().tellg() << " bytes.\n";
			boost::asio::async_write(m_socket,
					boost::asio::buffer(m_buf.c_array(),
							getIStream().gcount()),
					boost::bind(&RJSAsyncTcpClient::handle_write_file, this,
							boost::asio::placeholders::error));
			if (err)
			{
				throw std::runtime_error("RJSAsyncTcpClient::handle_write send error: " + err.message());
			}
		}
		else
		{
			// transfer is complete!
			return;
		}
	}
	else
	{
		throw std::runtime_error("RJSAsyncTcpClient::handle_write error: " + err.message());
	}

}

} /* namespace render */
