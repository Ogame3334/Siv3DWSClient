#pragma once

#include <Siv3D.hpp>

namespace ogm
{
	class WSClient
	{
	public:
		WSClient();
		~WSClient() = default;

		void onOpen(const std::function<void()> callback);
		void onClose(const std::function<void()> callback);
		void onError(const std::function<void(const StringView)> callback);
		void onMessage(const std::function<void(const StringView)> callback);

		bool open(const StringView uri);
		bool open(const StringView host, uint16 port);
		bool open(const StringView host, uint16 port, const StringView endpoint);

		void close();

		bool send(const StringView message);

		bool getIsConnectionClosed();
	private:
		class WSClientDetail;
		std::shared_ptr<WSClientDetail> pImpl;
	};
}
