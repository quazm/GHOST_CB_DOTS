#pragma once

#include "includes.h"
#include "socket.h"

class CTCPStatusBroadcasterSocket;
class CBaseGame;

class CStatusBroadcaster
{
private:
    int *nfds = 0;
    fd_set *fd;
    fd_set *send_fd;

public:
    CTCPServer *connectSocket; // сокет для подключения
    uint16_t hostPort;
    uint16_t statusPort; // config value: порт для статуса
    std::string statString;
    uint8_t slotsNum;

    BYTEARRAY packetSlots                 = BYTEARRAY();
    uint32_t gameSlotsStatusesLastGetTime = 0;

    std::vector<CTCPStatusBroadcasterSocket *> sockets; // вектор сокетов для отсылки статуса
    std::string autoHostGameName;                       // имя игры автохоста

    CStatusBroadcaster(std::string gameName);
    ~CStatusBroadcaster();

    void set_fd(fd_set *n_fd, fd_set *n_send_fd, int *n_ndfs);

    bool StartListen(std::string m_BindAddress);
    std::string MakeGameNameString(CBaseGame *game);
    void SendAll(BYTEARRAY *packet);
    void Send(BYTEARRAY *packet, CTCPStatusBroadcasterSocket *socket);

    //void Send(CTCPStatusBroadcasterSocket *socket, BYTEARRAY *packet);
    void SendGame(CBaseGame *game, CTCPStatusBroadcasterSocket *bs = NULL); // GAME, порт, кол-во игроков, максимум игроков, длина названия игры в байтах, название игры, длина статы в байтах, стата
    void SendNump(CBaseGame *game);                                         // NUMP, кол - во игроков
    void SendName(CBaseGame *game);                                         // NAME, длина названия игры в байтах, название игры
    void SendSlot(CBaseGame *game, CTCPStatusBroadcasterSocket *bs);
};
