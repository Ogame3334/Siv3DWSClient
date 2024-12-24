#include <Siv3D.hpp>
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
	client.open(U"echo.websocket.org", 443);
	

	TextEditState textEdit;


	while (System::Update()) {
		//Print << client.getIsConnectionClosed();
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
