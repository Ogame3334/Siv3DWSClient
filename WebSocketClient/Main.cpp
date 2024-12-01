#include <Siv3D.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// Report a failure
void
fail(beast::error_code ec, char const* what)
{
	Print << U"Failed: " << Unicode::Widen(ec.message()) << U" in " << Unicode::Widen(what);
	std::cerr << what << ": " << ec.message() << "\n";
}

// Sends a WebSocket message and prints the response
class session : public std::enable_shared_from_this<session>
{
	tcp::resolver resolver_;
	websocket::stream<beast::tcp_stream> ws_;
	beast::flat_buffer buffer_;
	std::string host_;
	std::string text_;

public:
	// Resolver and socket require an io_context
	explicit
		session(net::io_context& ioc)
		: resolver_(net::make_strand(ioc))
		, ws_(net::make_strand(ioc))
	{
	}

	// Start the asynchronous operation
	void
		run(
			char const* host,
			char const* port,
			char const* text)
	{
		// Save these for later
		host_ = host;
		text_ = text;

		Print << U"Connecting";

		// Look up the domain name
		resolver_.async_resolve(
			host,
			port,
			beast::bind_front_handler(
				&session::on_resolve,
				shared_from_this()));
	}

	void
		on_resolve(
			beast::error_code ec,
			tcp::resolver::results_type results)
	{
		if (ec)
			return fail(ec, "resolve");

		Print << U"Resolve";

		// Set the timeout for the operation
		beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

		// Make the connection on the IP address we get from a lookup
		beast::get_lowest_layer(ws_).async_connect(
			results,
			beast::bind_front_handler(
				&session::on_connect,
				shared_from_this()));
	}

	void
		on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
	{
		if (ec)
			return fail(ec, "connect");

		Print << U"Connect";

		// Turn off the timeout on the tcp_stream, because
		// the websocket stream has its own timeout system.
		beast::get_lowest_layer(ws_).expires_never();

		// Set suggested timeout settings for the websocket
		ws_.set_option(
			websocket::stream_base::timeout::suggested(
				beast::role_type::client));

		// Set a decorator to change the User-Agent of the handshake
		ws_.set_option(websocket::stream_base::decorator(
			[](websocket::request_type& req)
			{
				req.set(http::field::user_agent,
					std::string(BOOST_BEAST_VERSION_STRING) +
					" websocket-client-async");
			}));

		// Update the host_ string. This will provide the value of the
		// Host HTTP header during the WebSocket handshake.
		// See https://tools.ietf.org/html/rfc7230#section-5.4
		host_ += ':' + std::to_string(ep.port());

		Print << U"Connect done";

		// Perform the websocket handshake
		ws_.async_handshake(host_, "/",
			beast::bind_front_handler(
				&session::on_handshake,
				shared_from_this()));
	}

	void
		on_handshake(beast::error_code ec)
	{
		if (ec)
			return fail(ec, "handshake");

		Print << U"Handshake";

		// Send the message4
		ws_.async_write(
			net::buffer(text_),
			beast::bind_front_handler(
				&session::on_write,
				shared_from_this()));
	}

	void
		on_write(
			beast::error_code ec,
			std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		if (ec)
			return fail(ec, "write");

		Print << U"Send Message";

		// Read a message into our buffer
		ws_.async_read(
			buffer_,
			beast::bind_front_handler(
				&session::on_read,
				shared_from_this()));
	}

	void
		on_read(
			beast::error_code ec,
			std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		if (ec)
			return fail(ec, "read");

		//ws_.read(buffer_);
		std::string message = beast::buffers_to_string(buffer_.data());

		Print << Unicode::Widen(message);

		// Close the WebSocket connection
		//ws_.async_close(websocket::close_code::normal,
		//	beast::bind_front_handler(
		//		&session::on_close,
		//		shared_from_this()));
		ws_.async_read(
			buffer_,
			beast::bind_front_handler(
				&session::on_read,
				shared_from_this()));
	}

	void
		on_close(beast::error_code ec)
	{
		if (ec)
			return fail(ec, "close");

		// If we get here then the connection is closed gracefully
		Print << U"Close";

		// The make_printable() function helps print a ConstBufferSequence
		std::cout << beast::make_printable(buffer_.data()) << std::endl;
	}
};

#include "include/WSClient.hpp"

void Main() {
	System::SetTerminationTriggers(UserAction::CloseButtonClicked);

	ogm::WSClient client;

	client.onMessage([](const StringView message) {
		Print << message;
	});
	client.onClose([] {Print << U"websocket通信は正常に閉じられました"; });
	client.onError([](const StringView message) {
		Print << message;
	});
	client.onOpen([] {Print << U"接続がオープンしました"; });


	//client.open(U"localhost", 8081);
	client.open(U"echo.websocket.org", 80);
	

	TextEditState textEdit;


	while (System::Update()) {
		//Print << U"fuga";
		if (KeyEscape.down()) {
			client.close();
		}

		SimpleGUI::TextBox(textEdit, Vec2{ 100, 100 });
		if (SimpleGUI::Button(U"send", Vec2{ 100, 200 })) {
			client.send(textEdit.text);
			textEdit.clear();
		}
	}
}
