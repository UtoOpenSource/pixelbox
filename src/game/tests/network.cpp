/*
 * This file is a part of Pixelbox - Infinite 2D sandbox game
 * Copyright (C) 2023 UtoECat
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <https://www.gnu.org/licenses/>
 */

#include "doctest.h"
#include "network.hpp"
#include <memory>
#include <string>
#include <thread>

struct TestData : public pb::ConnectionData {
	char test[4] = "abc";
};

TEST_CASE("enet-simple") {

	std::thread tsrv([](){

	pb::ProtocolInfo info;
	info.ip = nullptr;
	info.port = 20496;
	info.nchannels = 1;
	info.nconnections = 2;
	pb::ENetServer server(info);
	printf("aboba started\n");

	CHECK(server.is_valid());

	ENetEvent ev; bool work = true;
	while (work) {
		while (server.service(ev, 50)) {
			switch(ev.type) {
				case ENET_EVENT_TYPE_CONNECT: {
					pb::ENetConnection con(ev.peer);
					con.set_data(new TestData);
					con.send(0, enet_packet_create("init", 5, ENET_PACKET_FLAG_RELIABLE));
					printf("Client connected");
				}; break;
				case ENET_EVENT_TYPE_DISCONNECT:
				case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
					pb::ENetConnection con(ev.peer);
					con.release_data();
					printf("Client disconnected!");
					work = false;
				}; break;
				case ENET_EVENT_TYPE_RECEIVE: {
					printf("SERVER: %s\n", (char*)enet_packet_get_data(ev.packet));
					pb::ENetConnection con(ev.peer);
					con.disconnect();
				} break;
				server.releaseEvent(ev);
			}
			printf("SERVER: event %i!\n", ev.type);
		}
		printf("SERVER: HELP!!!!!\n");
	}
	
	server.shutdown();

	});

	std::thread tcli([](){
		
		pb::ProtocolInfo info;
		info.ip = "127.0.0.1";
		info.port =20496;
		info.nchannels = 1;
		info.nconnections = 1;
		pb::ENetClient cli(info);

		bool ok = cli.connect();
		if (!ok) ok = cli.connect();
		REQUIRE(ok);
		printf("bobaba started\n");

		REQUIRE(cli.is_connected());

		ENetEvent ev;

		ok = true;
		while (ok) {
			cli.send(0, enet_packet_create("sus", 4, 0));
			while (cli.service(ev, 50)) {
				if (ev.type == ENET_EVENT_TYPE_DISCONNECT ||
						ev.type == ENET_EVENT_TYPE_DISCONNECT_TIMEOUT) {
					ok = false;
				} else if (ev.type == ENET_EVENT_TYPE_RECEIVE) {
					printf("CLI: %s\n", (char*)enet_packet_get_data(ev.packet));
				} else {
					printf("CLI: event %i!", ev.type);
				}
				cli.releaseEvent(ev);
			};
		};

	});

	tsrv.join();
	tcli.join();
}

