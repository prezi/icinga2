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

#include "livestatus/notificationstable.h"
#include "icinga/notification.h"
#include "icinga/host.h"
#include "icinga/service.h"
#include "base/dynamictype.h"
#include <boost/foreach.hpp>

using namespace icinga;

NotificationsTable::NotificationsTable(void)
{
	AddColumns(this);
}

void NotificationsTable::AddColumns(Table *table, const String& prefix,
    const Column::ObjectAccessor& objectAccessor)
{
	table->AddColumn(prefix + "name", Column(&NotificationsTable::NameAccessor, objectAccessor));

}

String NotificationsTable::GetName(void) const
{
	return "notifications";
}

void NotificationsTable::FetchRows(const AddRowFunction& addRowFn)
{
	BOOST_FOREACH(const Notification::Ptr& hg, DynamicType::GetObjects<Notification>()) {
		addRowFn(hg);
	}
}

Value NotificationsTable::NameAccessor(const Value& row)
{
	return static_cast<Notification::Ptr>(row)->GetName();
}


