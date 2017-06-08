/*
 * RJSAsyncTcpClient.h
 *
 *  Created on: Sep 15, 2016
 *      Author: dan
 */

#ifndef LIBRENDER_SRC_JSONQ_RJSASYNCTCPCLIENT_H_
#define LIBRENDER_SRC_JSONQ_RJSASYNCTCPCLIENT_H_

#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <sstream>

namespace render {


class RJSAsyncTcpClient {
	boost::asio::ip::tcp::resolver m_resolver;
	boost::asio::ip::tcp::socket m_socket;
	boost::array<char, 1024> m_buf;
	boost::asio::streambuf m_streambufRequest;
	boost::filesystem::ifstream m_ifstreamSourceFile;
	std::istringstream m_stringstreamSourceString;
	bool m_bUseFile;
	bool m_bUseString;

	void handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
	void handle_connect(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
	void handle_write_file(const boost::system::error_code& err);
	void initiate_resolve(const std::string& server);
	std::istream& getIStream();

public:
	RJSAsyncTcpClient(boost::asio::io_service& io_service, const std::string& server, const std::string& sendme);
	RJSAsyncTcpClient(boost::asio::io_service& io_service, const std::string& server, const boost::filesystem::path& file);
	virtual ~RJSAsyncTcpClient() {};
};

} /* namespace render */

#endif /* LIBRENDER_SRC_JSONQ_RJSASYNCTCPCLIENT_H_ */
