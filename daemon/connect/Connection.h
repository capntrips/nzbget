/*
 *  This file is part of nzbget
 *
 *  Copyright (C) 2004 Sven Henkel <sidddy@users.sourceforge.net>
 *  Copyright (C) 2007-2015 Andrey Prygunkov <hugbug@users.sourceforge.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * $Revision$
 * $Date$
 *
 */


#ifndef CONNECTION_H
#define CONNECTION_H

#include "NString.h"

#ifndef HAVE_GETADDRINFO
#ifndef HAVE_GETHOSTBYNAME_R
#include "Thread.h"
#endif
#endif
#ifndef DISABLE_TLS
#include "TlsSocket.h"
#endif

class Connection
{
public:
	enum EStatus
	{
		csConnected,
		csDisconnected,
		csListening,
		csCancelled
	};

protected:
	CString				m_host;
	int					m_port;
	SOCKET				m_socket;
	bool				m_tls;
	CString				m_cipher;
	CharBuffer			m_readBuf;
	int					m_bufAvail;
	char*				m_bufPtr;
	EStatus				m_status;
	int					m_timeout;
	bool				m_suppressErrors;
	BString<100>		m_remoteAddr;
	int					m_totalBytesRead;
	bool				m_broken;
	bool				m_gracefull;

	struct SockAddr
	{
		int				ai_family;
		int				ai_socktype;
		int				ai_protocol;
		bool			operator==(const SockAddr& rhs) const
							{ return memcmp(this, &rhs, sizeof(SockAddr)) == 0; }
	};

#ifndef DISABLE_TLS
	class ConTlsSocket: public TlsSocket
	{
	private:
		Connection*		m_owner;
	protected:
		virtual void	PrintError(const char* errMsg) { m_owner->PrintError(errMsg); }
	public:
						ConTlsSocket(SOCKET socket, bool isClient, const char* certFile,
							const char* keyFile, const char* cipher, Connection* owner):
							TlsSocket(socket, isClient, certFile, keyFile, cipher), m_owner(owner) {}
	};

	std::unique_ptr<ConTlsSocket>	m_tlsSocket;
	bool							m_tlsError;
#endif
#ifndef HAVE_GETADDRINFO
#ifndef HAVE_GETHOSTBYNAME_R
	static Mutex*		m_mutexGetHostByName;
#endif
#endif

						Connection(SOCKET socket, bool tls);
	void				ReportError(const char* msgPrefix, const char* msgArg, bool PrintErrCode, int herrno = 0, const char* herrMsg = nullptr);
	virtual void		PrintError(const char* errMsg);
	bool				DoConnect();
	bool				DoDisconnect();
	bool				InitSocketOpts();
	bool				ConnectWithTimeout(void* address, int address_len);
#ifndef HAVE_GETADDRINFO
	in_addr_t			ResolveHostAddr(const char* host);
#endif
#ifndef DISABLE_TLS
	int					recv(SOCKET s, char* buf, int len, int flags);
	int					send(SOCKET s, const char* buf, int len, int flags);
	void				CloseTls();
#endif

public:
						Connection(const char* host, int port, bool tls);
	virtual 			~Connection();
	static void			Init();
	static void			Final();
	virtual bool 		Connect();
	virtual bool		Disconnect();
	bool				Bind();
	bool				Send(const char* buffer, int size);
	bool				Recv(char* buffer, int size);
	int					TryRecv(char* buffer, int size);
	char*				ReadLine(char* buffer, int size, int* bytesRead);
	void				ReadBuffer(char** buffer, int *bufLen);
	int					WriteLine(const char* buffer);
	Connection*			Accept();
	void				Cancel();
	const char*			GetHost() { return m_host; }
	int					GetPort() { return m_port; }
	bool				GetTls() { return m_tls; }
	const char*			GetCipher() { return m_cipher; }
	void				SetCipher(const char* cipher) { m_cipher = cipher; }
	void				SetTimeout(int timeout) { m_timeout = timeout; }
	EStatus				GetStatus() { return m_status; }
	void				SetSuppressErrors(bool suppressErrors);
	bool				GetSuppressErrors() { return m_suppressErrors; }
	const char*			GetRemoteAddr();
	bool				GetGracefull() { return m_gracefull; }
	void				SetGracefull(bool gracefull) { m_gracefull = gracefull; }
#ifndef DISABLE_TLS
	bool				StartTls(bool isClient, const char* certFile, const char* keyFile);
#endif
	int					FetchTotalBytesRead();
};

#endif
