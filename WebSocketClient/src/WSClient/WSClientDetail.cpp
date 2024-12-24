#include "WSClientDetail.hpp"

#include "root_certificates.hpp"

namespace ogm
{
	WSClient::WSClientDetail::WSClientDetail()
		: m_ioContext(), m_resolver(m_ioContext), m_ws(m_ioContext, m_ctx)
	{
		load_root_certificates(m_ctx);
	}

	void WSClient::WSClientDetail::onOpen(const std::function<void()> callback)
	{
		m_onOpen = callback;
	}
	void WSClient::WSClientDetail::onClose(const std::function<void()> callback)
	{
		m_onClose = callback;
	}
	void WSClient::WSClientDetail::onError(const std::function<void(const StringView)> callback)
	{
		m_onError = callback;
	}
	void WSClient::WSClientDetail::onMessage(const std::function<void(const StringView)> callback)
	{
		m_onMessage = callback;
	}

	bool WSClient::WSClientDetail::open(const StringView uri)
	{
		return true;
	}
	bool WSClient::WSClientDetail::open(const StringView host, uint16 port, const StringView endpoint)
	{
		if (getIsConnectionClosed()) return false;

		try {
			auto results = m_resolver.resolve(host.toUTF8(), std::to_string(port));

			auto ep = net::connect(m_ws.next_layer().next_layer(), results);

			if (!SSL_set_tlsext_host_name(m_ws.next_layer().native_handle(), host.toUTF8().c_str())) {
				throw beast::system_error(
				beast::error_code(
					static_cast<int>(::ERR_get_error()),
					net::error::get_ssl_category()),
				"Failed to set SNI Hostname");
			}

			std::string hostWithPort = host.toUTF8() + ":" + std::to_string(ep.port());

			m_ws.next_layer().handshake(ssl::stream_base::client);

			m_ws.set_option(websocket::stream_base::decorator(
				[](websocket::request_type& req) {
					req.set(http::field::user_agent,
						std::string(BOOST_BEAST_VERSION_STRING) +
						" websocket-client-coro"
					);
				}
			));

			m_ws.handshake(hostWithPort, endpoint.toUTF8());

			m_task = Async([this] {this->run(); });
			return true;
		}
		catch (const beast::system_error& e) {
			m_onError(Unicode::Widen(e.code().message()));
			return false;
		}
	}

	void WSClient::WSClientDetail::close()
	{
		if (getIsConnectionClosed()) return;

		try {
			m_ws.close(websocket::close_code::normal);
		}
		catch (const beast::system_error& e) {
			if (e.code() == boost::asio::error::eof) {
				m_onClose();
			}
			else {
				m_onError(Unicode::Widen(e.code().message()));
			}
		}

		isContinue = false;
	}

	bool WSClient::WSClientDetail::send(const StringView message)
	{
		if (getIsConnectionClosed()) return false;

		try {
			m_ws.write(net::buffer(message.toUTF8()));
		}
		catch (const beast::system_error& e) {
			m_onError(Unicode::Widen(e.code().message()));
			return false;
		}

		return true;
	}



	void WSClient::WSClientDetail::run()
	{
		m_onOpen();

		try {
			while (isContinue) {
				Print << isContinue;
				m_ws.read(m_buffer);
				std::string message = beast::buffers_to_string(m_buffer.data());
				m_onMessage(Unicode::Widen(message));
				m_buffer.clear();
			}
		}
		catch (const beast::system_error& e) {
			m_onError(Unicode::Widen(e.code().message()));
			return;
		}
		return;
	}
}
