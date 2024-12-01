#include "WSClientDetail.hpp"

namespace ogm
{
	WSClient::WSClientDetail::WSClientDetail()
		: m_ioContext(), m_resolver(m_ioContext), m_ws(m_ioContext) {}

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

			net::connect(m_ws.next_layer(), results);

			m_ws.handshake(host.toUTF8(), endpoint.toUTF8());

			m_task = Async([this, &host, &port] {this->run(); });
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

		m_ws.write(net::buffer(message.toUTF8()));

		return true;
	}



	void WSClient::WSClientDetail::run()
	{
		m_onOpen();

		while (isContinue) {
			m_ws.read(m_buffer);
			std::string message = beast::buffers_to_string(m_buffer.data());
			m_onMessage(Unicode::Widen(message));
			m_buffer.clear();
		}
		return;
	}
}
