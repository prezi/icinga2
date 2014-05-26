/******************************************************************************
 * Icinga 2                                                                   *
 * Copyright (C) 2012-2014 Icinga Development Team (http://www.icinga.org)    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License                *
 * as published by the Free Software Foundation; either version 2             *
 * of the License, or (at your option) any later version.                     *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software Foundation     *
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ******************************************************************************/

#ifndef APICLIENT_H
#define APICLIENT_H

#include "remote/endpoint.hpp"
#include "base/stream.hpp"
#include "base/timer.hpp"
#include "remote/i2-remote.hpp"

namespace icinga
{

enum ClientRole
{
	ClientInbound,
	ClientOutbound
};

/**
 * An API client connection.
 *
 * @ingroup remote
 */
class I2_REMOTE_API ApiClient : public Object
{
public:
	DECLARE_PTR_TYPEDEFS(ApiClient);

	ApiClient(const String& identity, const Stream::Ptr& stream, ConnectionRole role);

	static void StaticInitialize(void);

	void Start(void);

	String GetIdentity(void) const;
	Endpoint::Ptr GetEndpoint(void) const;
	Stream::Ptr GetStream(void) const;
	ConnectionRole GetRole(void) const;

	void Disconnect(void);

	void SendMessage(const Dictionary::Ptr& request);

private:
	String m_Identity;
	Endpoint::Ptr m_Endpoint;
	Stream::Ptr m_Stream;
	ConnectionRole m_Role;
	double m_Seen;

	bool ProcessMessage(void);
	void MessageThreadProc(void);

	static Timer::Ptr m_KeepAliveTimer;
	static void KeepAliveTimerHandler(void);
};

}

#endif /* APICLIENT_H */