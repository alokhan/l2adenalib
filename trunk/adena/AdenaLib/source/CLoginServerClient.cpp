/*
 * CLoginServerClient.cpp - Login server client.
 * Created January 2, 2007, by Michael 'Bigcheese' Spencer.
 *
 * Copyright (C) 2007 Michael Spencer
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Michael Spencer - bigcheesegs@gmail.com
 */

#include <CLoginServerClient.h>
#include <CPServerInit.h>
#include <CPGGAuth.h>

using namespace irr;

#define RECV_SIZE 65536 // Max size of a L2 packet.

namespace adena
{

CLoginServerClient::CLoginServerClient(irr::net::IServerClient* client, CLoginServer* server)
: Client(client), Server(server)
{
	char send_buff[155];
	CPServerInit init(Server->ScrambledMod);
	send_buff[0] = (155 & 0xff);
	send_buff[1] = ((155 >> 8) & 0xff);
	memcpy(send_buff + 2, init.getData(), 153);
	client->send(send_buff, 155);
};

CLoginServerClient::~CLoginServerClient()
{

};

void CLoginServerClient::HandlePacket()
{
	c8 buff[RECV_SIZE];
	Client->recv(buff, 2);
	int size = 0;
	size += (unsigned char)buff[0];
	size += ((unsigned char)(buff[1]) * 256);
	int recv_len = Client->recv(buff, RECV_SIZE);
	if(recv_len != (size - 2))
	{
		// Invalid packet.
		Server->Server->kickClient(Client);
	}
	char* dec = Server->BlowfishCipher->decrypt(buff, size - 2);

	IPacket* in;
	IPacket* out;
	switch(dec[0])
	{
			case 0:
				// cout << "Auth request.\n";
				break;
			case 7:
				// cout << "GGAuth request.\n";
				out = new CPGGAuth();
				SendPacket(out);
				delete out;
				break;
			default:
				break;
	}

	delete[] dec;
};

void CLoginServerClient::SendPacket(IPacket* packet)
{
	c8 buff[RECV_SIZE];
	irr::c8* data = packet->getData();
	irr::u32 len = packet->getLen();
	printf("Data\n");
	hexdump(data, len);
	Server->BlowfishCipher->checksum(data, len);
	printf("Data checksumed\n");
	hexdump(data, len);
	irr::c8* enc = Server->BlowfishCipher->crypt(data, len);
	buff[0] = ((len + 2) & 0xff);
	buff[1] = (((len + 2) >> 8) & 0xff);
	memcpy(buff + 2, enc, len);
	Client->send(buff, len + 2);
	delete[] enc;
};

}