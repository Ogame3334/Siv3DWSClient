#pragma once

#include "../../include/WSClient.hpp"

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
//#include <boost/asio/strand.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;		// from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

namespace ogm
{
	class WSClient::WSClientDetail
	{
	public:
		WSClientDetail();
		~WSClientDetail() = default;

		void onOpen(const std::function<void()> callback);
		void onClose(const std::function<void()> callback);
		void onError(const std::function<void(const StringView)> callback);
		void onMessage(const std::function<void(const StringView)> callback);

		bool open(const StringView uri);
		bool open(const StringView host, uint16 port, const StringView endpoint);

		void close();

		bool send(const StringView message);

		void run();

		bool getIsConnectionClosed() const { return m_task.isReady(); }

		bool isContinue = true;
	private:

		net::io_context m_ioContext;
		ssl::context m_ctx{ ssl::context::tlsv12_client };
		tcp::resolver m_resolver;
		websocket::stream<beast::ssl_stream<tcp::socket>> m_ws;
		beast::flat_buffer m_buffer;
		std::string m_host;
		std::string m_text;

		std::function<void()> m_onOpen = []{};
		std::function<void()> m_onClose = [] {};
		std::function<void(const StringView)> m_onError = [](const StringView){};
		std::function<void(const StringView)> m_onMessage = [](const StringView) {};
		AsyncTask<void> m_task;

		//static void Update(const WSClientDetail self);
	};
}
