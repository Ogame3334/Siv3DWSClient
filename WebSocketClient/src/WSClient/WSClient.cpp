#include "../../include/WSClient.hpp"
#include "WSClientDetail.hpp"

namespace ogm
{
	WSClient::WSClient()
		: pImpl(std::make_shared<WSClientDetail>()) {}

	void WSClient::onOpen(const std::function<void()> callback)
	{
		return pImpl->onOpen(callback);
	}
	void WSClient::onClose(const std::function<void()> callback)
	{
		return pImpl->onClose(callback);
	}
	void WSClient::onError(const std::function<void(const StringView)> callback)
	{
		return pImpl->onError(callback);
	}
	void WSClient::onMessage(const std::function<void(const StringView)> callback)
	{
		return pImpl->onMessage(callback);
	}

	bool WSClient::open(const StringView uri)
	{
		return pImpl->open(uri);
	}
	bool WSClient::open(const StringView host, uint16 port)
	{
		return pImpl->open(host, port, U"/");
	}
	bool WSClient::open(const StringView host, uint16 port, const StringView endpoint)
	{
		return pImpl->open(host, port, endpoint);
	}

	void WSClient::close()
	{
		return pImpl->close();
	}

	bool WSClient::send(const StringView message)
	{
		return pImpl->send(message);
	}

	bool WSClient::getIsConnectionClosed() {
		return pImpl->getIsConnectionClosed();
	}
}
