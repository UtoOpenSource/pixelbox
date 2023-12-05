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

#pragma once
#include <base.hpp>
#include <stdexcept>
#include <string>
#include <functional>
#include "enet.h"
#include <string.h>

/**
 * @description ENet Wrapper
 */
namespace pb {

	struct ProtocolInfo {
		const char* ip;
		unsigned short port;
		int nchannels;
		int nconnections;
		public:
		inline ENetAddress getAddress() {
			ENetAddress addr;
			addr.port = port;

			if (ip)
				enet_address_set_host(&addr, ip);	
			else addr.host = ENET_HOST_ANY;
			return addr;
		}
	};

	void HostDefaultConfig(ENetHost* host) {
		// TODO Adjust to better values!
		host->maximumPacketSize = 1024 * 32;
		host->maximumWaitingData = 1024 * 128;
	}

	class ENetClient : public Default {
		ENetHost* host;
		ENetPeer* server = nullptr;
		ProtocolInfo pinfo;
		public:
		ENetClient(ProtocolInfo info) {
			pinfo = info;
			auto addr = info.getAddress();
			host = enet_host_create(NULL, info.nconnections, info.nchannels,
				0, 0);
			if (!host) {
				throw std::runtime_error("Can't create client!");
			}
			HostDefaultConfig(host);
		}
		void disconnect() {
			if (server) {
				enet_peer_reset(server);
				server = nullptr;
			}
		}
		const char* errmsg = nullptr;
		bool service(ENetEvent& ev, int timeout = 1) {
			return enet_host_service(host, &ev, timeout) > 0;	
		}
		bool connect() {
			if (server) disconnect();
			auto addr = pinfo.getAddress();
			server = enet_host_connect(host, &addr, pinfo.nchannels, 0);

			if (!server) {
				errmsg = "can't create peer!";
				return false;
			}
			// process events
			ENetEvent event;
			if (enet_host_service(host, &event, 5000) > 0 &&
				event.type == ENET_EVENT_TYPE_CONNECT) {
				return true;
			}

			errmsg = "Can't connect to server!";
			enet_peer_reset(server);
			server = nullptr;
			return false;
		}
		~ENetClient() {
			disconnect();
			enet_host_destroy(host);
		}
		/* must be called after event is processeed */
		bool is_connected() {
			return server != nullptr;
		}
		void releaseEvent(ENetEvent& ev) {
			if (ev.type == ENET_EVENT_TYPE_RECEIVE && ev.packet) {
				enet_packet_destroy(ev.packet);
			}
		}

		/** @warning channel starts FROM ZERO!!!! */
		void send(uint8_t channel, ENetPacket* data) {
			if (server)
			enet_peer_send(server, channel, data);
		}
		void flush() {
			enet_host_flush(host);
		}
	};

	class ConnectionData {
		public:
		virtual ~ConnectionData() {};
	};

	class ENetConnection {
		ENetPeer* peer;
		public:
		ENetConnection(ENetPeer* p) {
			peer = p;
		}
		~ENetConnection() {} // nothing
		public:
		void disconnect() {
			enet_peer_disconnect_later(peer, 0);
		}

		void reset() { // forcefully disconnect
			release_data();
			enet_peer_reset(peer);
		}

		bool send(uint8_t channel, ENetPacket* data) {
			return enet_peer_send(peer, channel, data) == 0;
		}

		ConnectionData* data() {
			return (ConnectionData*)enet_peer_get_data(peer);
		}

		void set_data(ConnectionData* dat) {
			return enet_peer_set_data(peer, dat);
		}

		void release_data() {
			delete data();
			set_data(nullptr);
		}
	};

	class ENetServer : public Default {
		ENetHost* host;
		public:

		ENetServer(ProtocolInfo info) {
			ENetAddress addr = info.getAddress();
			host = enet_host_create(&addr, info.nconnections, info.nchannels,
				0, 0);
			if (!host) {
				throw std::runtime_error("Can't create server!");
			}
			HostDefaultConfig(host);
		}

		bool service(ENetEvent& ev, int timeout = 1) {
			bool stat = enet_host_service(host, &ev, timeout) > 0;	
			if (stat && ev.type == ENET_EVENT_TYPE_CONNECT) {
				enet_peer_set_data(ev.peer, nullptr); // we rely on this behaviour!
			}
			return stat;
		}

		~ENetServer() {
			enet_host_destroy(host);
		}

		bool is_valid() {
			return host != nullptr;
		}

		void releaseEvent(ENetEvent& ev) {
			if (ev.type == ENET_EVENT_TYPE_RECEIVE && ev.packet) {
				enet_packet_destroy(ev.packet);
			}
		}

		void broadcast(uint8_t channel, ENetPacket* data) {
			if (host)
			enet_host_broadcast(host, channel, data);
		}

		void foreach(std::function<void(ENetConnection)> cb) {
			if (!cb) return;
			ENetPeer *currentPeer;
			for (currentPeer = host->peers;
				currentPeer < &host->peers[host->peerCount]; ++currentPeer) {
				cb(currentPeer);
			}
		}

		void flush() {
			enet_host_flush(host);
		}

		/* shutdown sequence. Behaviour after this is not specified! */
		void shutdown() {
			// send disconnect
			foreach([](ENetConnection c) {
				c.disconnect();
			});
			enet_host_flush(host); 
			
			// queue
			ENetEvent ev;
			while (service(ev, 200)) {
				switch (ev.type) {
					case ENET_EVENT_TYPE_DISCONNECT:
					case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT: {
						ENetConnection con(ev.peer);
						con.release_data();
					}; break;
					case ENET_EVENT_TYPE_CONNECT:
						enet_peer_disconnect_now(ev.peer, 0);
					break;
					case ENET_EVENT_TYPE_RECEIVE:
						enet_packet_destroy(ev.packet);
					break;
					default:
					break;
				};
			}

			// force disconnect and free all data
			foreach([](ENetConnection c) {
				c.reset();
			});

			// now we can safely call destructor!
		}
	};

};

