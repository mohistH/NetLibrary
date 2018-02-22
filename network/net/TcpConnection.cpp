#include "TcpConnection.h"

TcpConnection::TcpConnection(EventLoop* loop, SOCKET socket,
	const InetAddress& localAddr, const InetAddress& peerAddr)
	: channel_(loop, socket)
	, localAddr_(localAddr)
	, peerAddr_(peerAddr)
{
	channel_.SetReadCallback(
		std::bind(&TcpConnection::HandleRead, this, std::placeholders::_1, std::placeholders::_2));
}

TcpConnection::~TcpConnection()
{

}

void TcpConnection::HandleRead(char* buf, DWORD len)
{
	if (len == 0) {
		closeCallback_(this);
		return;
	}

	messageCallback_(this, buf);
	PostRecv();
}

void TcpConnection::PostRecv()
{
	DWORD dwBytes = 0;
	DWORD flags = 0;

	memset(&ctx_, 0, sizeof(ctx_));
	ctx_.ioType = IO_READ;
	ctx_.wsaBuff.buf = ctx_.buffer;
	ctx_.wsaBuff.len = MAX_BUFFER_LEN;

    WSARecv(channel_.GetSocket(), &ctx_.wsaBuff, 1, &dwBytes, &flags, &ctx_.overlapped, NULL);
}

void TcpConnection::OnEstablished()
{
	PostRecv();
	connectionCallback_(this);
}

void TcpConnection::OnDestroyed()
{
	closesocket(channel_.GetSocket());
	connectionCallback_(this);
}