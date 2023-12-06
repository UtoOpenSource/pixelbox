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
#include "game.hpp"
#include "chunk.hpp"

namespace pb {

	enum EntityAction {
		/* 
		 * sended by server only, to indicate that new entity was appeared.
		 * not implemented yet
		 */
		ENTITY_SPAWN,

		/*
		 * Sended by server to notify client about entity state changes
		 * Sended by client to notify about cient's interaction
		 */
		ENTITY_GETSET,

		/*
		 * Sended by server to notify about death of entity.
		 * CLient does not send this.
		 */
		ENTITY_REMOVE
	};

	enum ChannelID {
		/* Server (first) sends PROTOCOL_ID\0 SERVER_NAME\0 PLAYERS_COUNT OR -1 IF LOCAL
		 * Local server will respond only for localhost for local playing in singleplayer (and only for one player)
		 *
		 * TODO : for future it may be useful to add SECURE CONNECTION step here, but i am suck at SSL and especially in BearSSL,
		 * so this is for very future.
		 *
		 * Client waits for message to be recieved and send back PLAYER_LOGIN(NICKNAME)\0 CLIENT_UNIQUE_KEY - 64 bytes in length, randomly
		 * generated unique user key (works like a password).
		 * Should use /dev/rand for this, but for now it uses juat a pseudorng, to make things simpler :/
		 */
		CH_AUTH,

		/* TODO request SECURE mode from the server.
		 * May only be called after recieving CH_AUTH and should send nothing to server.
		 *
		 * SERVER should return SSL Sertificate in result, or 0 in case no encryption is supported (default behaviour now)
		 *
		 * After this some non-implemented SSL auth steps goes (handshake, and all this stuff)
		 *
		 * Secure mode will be used to encrypt all chat messages from the user to server and from the server to user
		 * (we can't let people know private client's keys still! but maybe later somehow messages from user will contain a secure hash of it's private key... i don't know)
		 *
		 * Also secure mode will force client to encrypt CH_AUTH message sended to server, that contains a private passord.
		 * ALSO, all commands will be encrypted, both from client and from server.
		 * This will not affect recieved/sended chunks and entities though, delta data recieved and other stuff for perfomance reasons.
		 */
		CH_SECURE, // RESERVED

		/*
		 * Commands. Commands allow user to interact more with the server and other users. For admins they allow to operate significantly more,
		 * allowing to spawn/kill any entities, regenerate chunks and ban players by ip
		 * They may be customised.
		 *
		 * First part of the command message is a command itself in text mode, followed by \0 to indicate end of it.
		 * Rest - is data for the command. Separatation of tem is implementation defined, but using spaces or \0 is preferrable
		 * 
		 * Some commands are directly sended from GUI in client - to make things simpler. But not all of the buttons are commands :)
		 */
		CH_COMMAND,

		/* TODO
		 * Chunk updation channel.
		 * Client sends chunk that he changed. Server and client may lower resources from the player and
		 * validate underflows in some way - Implementation defined.
		 *
		 * Server sends back all the chunks visible to the player (player will send back camera position and size, read below),
		 * And may also send them in "compressed" format - only changed pixels are send.
		 * To prevent desynchronisation, full chunk data will be sended once in a 1second-1munite - server settings defined.
		 * 
		 * Client in other hand, will send changed pixels in similar format - Chunk Transfer Format™ (XD)
		 * Chunk Transfer Format™ XD will be described later, but in short :
		 * - Sended only one piece of chunk data (see struct ChunkData), not both old and new
		 * - if REFRESH To Prevent Desyncronisatio™n requested, threat next step as non-optimal and skip it
		 * - If not, we will try to "COMPRESS™™XD" chunk data in the format : 
		 *   -- put byte 255 at the start of data stream 
		 *   -- if two or ore bytes are same, put them in special format "[COUNT OF BYTES] BYTE"
		 *   -- else put 0 [LENGTH] [BYTES OF DATA WITH SPECIFIED LENGTH] if bytes are too random
		 *   -- if the end of data reached, put 0 0 to indicate end (means - zero bytes left)
		 *   -- error handling is implementaion defined
		 * - Check is "compressed" data size is woth compression.
		 *   (Better is to actually break compression if we are already at 80% of the chunk original size)
		 *	 if it's lower : send compressed
		 *	 else : send 0 + chunk data as they are.
		 * 
		 * chunk coordinates are also put at the start of data packet.
		 */
		CH_CHUNK,

		/* TODO
		 * Message starts from EntityAction + Entity UUID + Payload.
		 * Not implemented yet
		 */
		CH_ENTITY,
		CH_MAX
	};

	struct ChunkNet {
		ChunkPos  pos;
		ChunkData data;
	};

};

