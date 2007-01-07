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
#include <CPRequestLogin.h>
#include <CPLoginOk.h>
#include <CPPlayOk.h>
#include <CPLoginFailed.h>
#include <CPRequestPlay.h>
#include <irrCrypt.h>

using namespace irr;

#define RECV_SIZE 65536 // Max size of a L2 packet.

namespace adena
{

CLoginServerClient::CLoginServerClient(irr::net::IServerClient* client, CLoginServer* server)
: Client(client), Server(server), SessionId(0)
{
	char send_buff[155];
	CPServerInit init(Server->ScrambledMod);
	send_buff[0] = (155 & 0xff);
	send_buff[1] = ((155 >> 8) & 0xff);
	memcpy(send_buff + 2, init.getData(), 153);
	client->send(send_buff, 155);
	Server->ServerStatus.Connections++;
};

CLoginServerClient::~CLoginServerClient()
{

};

void CLoginServerClient::HandlePacket()
{
	c8 buff[RECV_SIZE];
	c8 dec[RECV_SIZE];
	Client->recv(buff, 2);
	int size = 0;
	size += (unsigned char)buff[0];
	size += ((unsigned char)(buff[1]) * 256);
	int recv_len = Client->recv(buff, size - 2);
	if(recv_len != (size - 2))
	{
		// Invalid packet.
		Server->Server->kickClient(Client);
	}

	Server->BlowfishCipher->decrypt(buff, size - 2, dec, RECV_SIZE);

	IPacket* in;
	if(dec[0] == 0)
	{
		Server->ServerStatus.LoginAttempts++;
		// cout << "Auth request.\n";
		in = new CPRequestLogin(dec, Server->RsaCipher);
		puts(((CPRequestLogin*)in)->Username.c_str());
		puts(((CPRequestLogin*)in)->Password.c_str());
		irr::db::Query q = irr::db::Query(irr::core::stringc("SELECT password FROM accounts WHERE username = '$user'"));
		q.setVar(irr::core::stringc("$user"), ((CPRequestLogin*)in)->Username);
		irr::db::CQueryResult qr = Server->DataBase->query(q);
		if(qr.RowCount != 1)
		{
			// Don't tell the client that the username does not exist.
			CPLoginFailed clf(CPLoginFailed::REASON_PASS_WRONG);
			SendPacket(&clf);
			Server->Server->kickClient(Client);
		}else
		{
			irr::crypt::CHashMD5 md5 = irr::crypt::CHashMD5();
			irr::core::stringc hash = md5.quickHash(((CPRequestLogin*)in)->Password);
			if(qr[0][irr::core::stringc("password")] == hash)
			{
				SessionId = Server->Rng->getRandU32();
				CPLoginOk clo(SessionId);
				SendPacket(&clo);
				Server->ServerStatus.LoginSuccesses++;
			}else
			{
				CPLoginFailed clf(CPLoginFailed::REASON_PASS_WRONG);
				SendPacket(&clf);
				Server->Server->kickClient(Client);
			}
		}
	}else if(dec[0] == 2)
	{
		//CPRequestPlay rp(dec);
		CPPlayOk po(SessionId);
		SendPacket(&po);
	}else if(dec[0] == 5)
	{
		// Request server list.
		if(SessionId)// User has logged in.
		{
			SendPacket(Server->ServerListPacket);
		}else
		{
			// Should never get here unless not using l2 client.
			Server->Server->kickClient(Client);
			Server->ServerStatus.SuspectedHackAttempts++;
		}
	}else if(dec[0] == 7)
	{
		// cout << "GGAuth request.\n";
		CPGGAuth gga = CPGGAuth();
		SendPacket(&gga);
	}
};

void CLoginServerClient::SendPacket(IPacket* packet)
{
	c8 buff[RECV_SIZE];
	c8 enc[RECV_SIZE];
	irr::c8* data = packet->getData();
	irr::u32 len = packet->getLen();
	printf("Data\n");
	hexdump(data, len);
	Server->BlowfishCipher->checksum(data, len);
	printf("Data checksumed\n");
	hexdump(data, len);
	Server->BlowfishCipher->crypt(data, len, enc, RECV_SIZE);
	buff[0] = ((len + 2) & 0xff);
	buff[1] = (((len + 2) >> 8) & 0xff);
	memcpy(buff + 2, enc, len);
	Client->send(buff, len + 2);
};

}
