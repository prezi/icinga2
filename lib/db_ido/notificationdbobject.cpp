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

#include "db_ido/notificationdbobject.h"
#include "db_ido/dbtype.h"
#include "db_ido/dbvalue.h"
#include "db_ido/dbevents.h"
#include "icinga/host.h"
#include "icinga/service.h"
#include "icinga/notification.h"
#include "icinga/dependency.h"
#include "icinga/checkcommand.h"
#include "icinga/eventcommand.h"
#include "icinga/compatutility.h"
#include "base/convert.h"
#include "base/objectlock.h"
#include <boost/foreach.hpp>

using namespace icinga;

REGISTER_DBTYPE(Notification, "notification", DbObjectTypeNotification, "notification_object_id", NotificationDbObject);

NotificationDbObject::NotificationDbObject(const DbType::Ptr& type, const String& name1, const String& name2)
	: DbObject(type, name1, name2)
{ }

Dictionary::Ptr NotificationDbObject::GetConfigFields(void) const
{
	Dictionary::Ptr fields = make_shared<Dictionary>();
	Notification::Ptr notification = static_pointer_cast<Notification>(GetObject());

	/*
	fields->Set("", );
	fields->Set("", );
	fields->Set("", );
	fields->Set("", );
	fields->Set("", );
	fields->Set("", );
	fields->Set("", );
	fields->Set("", );
	fields->Set("", );
	fields->Set("", ); */


	return fields;
}

Dictionary::Ptr NotificationDbObject::GetStatusFields(void) const
{
	Dictionary::Ptr fields = make_shared<Dictionary>();


	return fields;
}

void NotificationDbObject::OnConfigUpdate(void)
{

}

void NotificationDbObject::OnStatusUpdate(void)
{
}
