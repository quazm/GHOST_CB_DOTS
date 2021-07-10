#include "ghost.h"
#include "statusbroadcaster.h"
#include "socket.h"
#include <vector>
#include "util.h"
#include "game_base.h"
#include "gameplayer.h"


CStatusBroadcaster::CStatusBroadcaster(string gameName)
{
	autoHostGameName = gameName;
	connectSocket = NULL;
	nfds = 0;
	fd = NULL;
	send_fd = NULL;
}

CStatusBroadcaster  :: ~CStatusBroadcaster( )
{

	delete connectSocket;
	connectSocket = NULL;

	for (vector<CTCPStatusBroadcasterSocket* > ::iterator i = sockets.begin( ); i != sockets.end( ); i++)
	{
		delete* i;
		*i = NULL;
	}
}

void CStatusBroadcaster :: set_fd(fd_set* n_fd, fd_set* n_send_fd, int* n_nfds)
{
	fd = n_fd;
	send_fd = n_send_fd;
	nfds = n_nfds;
}
bool CStatusBroadcaster :: StartListen(string m_BindAddress)
{
	connectSocket = new CTCPServer();
	if (connectSocket->Listen(m_BindAddress, statusPort))
	{
		CONSOLE_Print("[GHOST][STATUS] listening for Gproxy status connects on port " + UTIL_ToString(statusPort));
		return true;
	}
	else
	{
		CONSOLE_Print("[GHOST][STATUS] error listening for Gproxy status connects on port " + UTIL_ToString(hostPort));
		return false;
	}
	
}

string CStatusBroadcaster :: MakeGameNameString( CBaseGame* game  )
{
	string GameName = autoHostGameName;
	if(game == NULL)
		GameName = "|c00FF7F00" + GameName + " (rehost)";
	else if ( !game->GetGameLoadingFlag() && !game->GetGameLoadedFlag() && !game->GetCountDownStarted() )
		GameName = "|c00FF0000" + GameName + " (lobby)";
	else if ( game->GetGameLoadingFlag( ) && !game->GetGameLoadedFlag( ) && game->GetCountDownStarted() )
		GameName = "|c007d7d7d" + GameName + " (loading)";
	else if (  !game->GetGameLoadingFlag() && game->GetGameLoadedFlag( ) && game->GetCountDownStarted( ) )
		GameName = "|c007d7d7d" + GameName + " (started)";

	return GameName;

}

void CStatusBroadcaster :: SendAll(BYTEARRAY *packet)
{
	//отправка дл¤ каждого сокета
	for (vector<CTCPStatusBroadcasterSocket* > ::iterator i = sockets.begin(); i != sockets.end(); i++)
	{
		(*i)->socket->PutBytes(*packet);
		(*i)->socket->DoSend(send_fd);
		(*i)->socket->ClearSendBuffer();
	}
}

void  CStatusBroadcaster :: Send(BYTEARRAY* packet, CTCPStatusBroadcasterSocket* bs)
{
	bs->socket->PutBytes(*packet);
	bs->socket->DoSend(send_fd);
	bs->socket->ClearSendBuffer();
}

//—разу после коннекта, если сменилась карта (в том числе рехост той же самой) или если пришло 0xff, 0xff:
//GAME, порт, кол-во игроков, максимум игроков, длина названи¤ игры в байтах, название игры, длина статы в байтах, стата
void CStatusBroadcaster :: SendGame( CBaseGame *game, CTCPStatusBroadcasterSocket* bs )
{
	BYTEARRAY packet;

	if (game != NULL)
	{
		string gameName = MakeGameNameString(game);


		UTIL_AppendByteArrayFast( packet, string("GAME"), false );																	// "GAME"			char[4]			4 байта
		UTIL_AppendByteArray( packet, hostPort, false);																				// порт				uint16_t		2 байта
		packet.push_back( uint8_t( game->GetNumHumanPlayersFromSlots( ) ) ) ;														// кол. игроков		uint8_t			1 байт
		packet.push_back( uint8_t( game->GetSlotsOpen( ) + game->GetNumHumanPlayersFromSlots( ) ) );								// ??????????		uint8_t			1 байт	
		packet.push_back( uint8_t( game->GetSlots( ).size( ) ) );																	// кол. слотов		uint8_t			1 байт	
		packet.push_back( uint8_t( gameName.size( ) ) );																			// размер			uint8_t			1 байт
		UTIL_AppendByteArrayFast(packet, gameName, false );																			// название			char[]			n байт
		packet.push_back( uint8_t(statString.size() ) );																			// размер			uint8_t			1 байт
		UTIL_AppendByteArrayFast( packet, statString, false );																		// —тат—тринг		char[]			n байт
	}
	else //нет объекта игры - рехост
	{
		string gameName = MakeGameNameString(NULL);

		UTIL_AppendByteArrayFast(packet, string("GAME"), false);										// "GAME"			char[4]			4 байта
		UTIL_AppendByteArray(packet, hostPort, false);													// порт				uint16_t		2 байта
		packet.push_back( uint8_t( 0 ) );																// кол. игроков		uint8_t			1 байт
		packet.push_back( slotsNum );																    // ??????????		uint8_t			1 байт
		packet.push_back( slotsNum );																	// кол. слотов		uint8_t			1 байт
		packet.push_back( uint8_t( gameName.size( ) ) );												// размер			uint8_t			1 байт
		UTIL_AppendByteArrayFast(packet, gameName, false);												// название			char[]			n байт
		packet.push_back(uint8_t(statString.size()));													// размер			uint8_t			1 байт
		UTIL_AppendByteArrayFast(packet, statString, false);											// —тат—тринг		char[]			n байт
	}

	if (bs == NULL)
		SendAll(&packet); //отправка всем
	else
		Send(&packet, bs);	//отправка на конкретный сокет

}

//Ќ≈ »—ѕќЋ№«”≈“—я
//≈сли изменилось кол - во игроков :
//NUMP, кол - во игроков
void CStatusBroadcaster::SendNump(CBaseGame* game)
{
	if (game != NULL)
	{
		BYTEARRAY packet;
		UTIL_AppendByteArrayFast(packet, string("NUMP"), false);									// "NUMP"			char[4]			4 байта
		packet.push_back(uint8_t(game->GetSlots().size() - game->GetSlotsOpen()));					// кол. игроков		uint8_t			1 байт
		SendAll(&packet); //отправка
	}
}

//Ќ≈ »—ѕќЋ№«”≈“—я
//≈сли изменилось название игры;
//NAME, длина названи¤ игры в байтах, название игры
void CStatusBroadcaster::SendName(CBaseGame* game)
{
		BYTEARRAY packet;
		UTIL_AppendByteArrayFast(packet, string("NAME"), false);										// "NAME"			char[4]			4 байта
		string gameName = MakeGameNameString(game);
		packet.push_back(uint8_t(gameName.size()));														// размер			uint8_t			1 байт
		UTIL_AppendByteArrayFast(packet, gameName, false);												// название			char[]			n байт
		SendAll(&packet); //отправка
}


//SLOT
void CStatusBroadcaster::SendSlot(CBaseGame* game, CTCPStatusBroadcasterSocket* bs)
{

	if(GetTime() - gameSlotsStatusesLastGetTime > 1) //получаем слоты, только если они устарели больше чем 1 сек
	{
		packetSlots = BYTEARRAY();
		UTIL_AppendByteArrayFast(packetSlots, string("SLOT"), false);

		if (game == NULL)
		{
			packetSlots.push_back(uint8_t(1));
			UTIL_AppendByteArrayFast(packetSlots, string("REHOST"), true);
		}
		else
		{
			vector<CGameSlot> gameSlots = game->GetSlots();
			vector<CGamePlayer*> gamePlayers = game->GetPlayers();
			packetSlots.push_back(uint8_t(gameSlots.size()));														// количество слотов			uint8_t			1 байт

			int SID = 0; //slot id
			for (vector<CGameSlot> ::iterator i = gameSlots.begin(); i != gameSlots.end(); i++)
			{
				if (i->GetSlotStatus() == SLOTSTATUS_OCCUPIED && i->GetComputer() == 0) //условие, что игрок	
				{
					for (vector<CGamePlayer*> ::iterator p = gamePlayers.begin(); p != gamePlayers.end(); p++) //ищем им¤ игрока
						if (SID == game->GetSIDFromPID((*p)->GetPID()))								
							if ((*p)->GetName() != "")
									UTIL_AppendByteArrayFast(packetSlots, string((*p)->GetName()), false);
				}
				else if (i->GetSlotStatus() == SLOTSTATUS_OCCUPIED && i->GetComputer() == 1) //условие, что комп
					UTIL_AppendByteArrayFast(packetSlots, string("COMPUTER"), false);
				else if (i->GetSlotStatus() == SLOTSTATUS_OPEN)
					UTIL_AppendByteArrayFast(packetSlots, string("OPEN"), false);
				else if (i->GetSlotStatus() == SLOTSTATUS_CLOSED)
					UTIL_AppendByteArrayFast(packetSlots, string("CLOSED"), false);
				else
					UTIL_AppendByteArrayFast(packetSlots, string("OUT OF GAME"), false); //ливер

				if ( distance(i, gameSlots.end()) > 1 )		 // не последний слот
					if( i->GetTeam() != next(i)->GetTeam() ) // тима помен¤лась, добавл¤ем тег в ник, что бы раздел¤ть тимы
						UTIL_AppendByteArrayFast(packetSlots, string("{NEXT_TEAM}"), false);

				packetSlots.push_back(uint8_t(0)); // "\n"

				SID++;
			}
		}
		gameSlotsStatusesLastGetTime = GetTime();	
	}
	if (packetSlots.size() > 0)
		if (bs == NULL)
			SendAll(&packetSlots); //отправка всем
		else
			Send(&packetSlots, bs);	//отправка на конкретный сокет
	
}
