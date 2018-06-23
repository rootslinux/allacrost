///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2016 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file   notification.cpp
*** \author Tyler Olsen (Roots)
*** \brief  Source file for notification event management
*** **************************************************************************/

#include "notification.h"

using namespace std;

using namespace hoa_utils;

template<> hoa_notification::NotificationEngine* Singleton<hoa_notification::NotificationEngine>::_singleton_reference = nullptr;

namespace hoa_notification {

NotificationEngine* NotificationManager = nullptr;
bool NOTIFICATION_DEBUG = false;

////////////////////////////////////////////////////////////////////////////////
// NotificationEngine class methods
////////////////////////////////////////////////////////////////////////////////

NotificationEngine::NotificationEngine() {
	IF_PRINT_DEBUG(NOTIFICATION_DEBUG) << "constructor invoked" << endl;
}



NotificationEngine::~NotificationEngine() {
	IF_PRINT_DEBUG(NOTIFICATION_DEBUG) << "destructor invoked" << endl;

	// Make sure to delete any triggers that the class is still holding onto
	DeleteAllNotificationEvents();
}



void NotificationEngine::DeleteAllNotificationEvents() {
	for (uint32 i = 0; i < _notification_events.size(); ++i) {
		delete _notification_events[i];
	}
	_notification_events.clear();
}



void NotificationEngine::Notify(NotificationEvent* notification) {
	if (notification == nullptr) {
		IF_PRINT_WARNING(NOTIFICATION_DEBUG) << "function received nullptr argument" << endl;
		return;
	}

	_notification_events.push_back(notification);
}



void NotificationEngine::CreateAndNotify(const string& category, const string& event) {
	NotificationEvent* new_trigger = new NotificationEvent(category, event);
	_notification_events.push_back(new_trigger);
}



NotificationEvent* NotificationEngine::GetNotificationEvent(uint32 index) const {
	if (index >= _notification_events.size())
		return nullptr;
	else
		return _notification_events[index];
}



void NotificationEngine::DEBUG_PrintNotificationEvents() const {
	if (_notification_events.size()) PRINT_DEBUG << "printing list of all stored notifications" << endl;
	for (uint32 i = 0; i < _notification_events.size(); ++i) {
		PRINT_DEBUG << i << ": " << _notification_events[i]->DEBUG_PrintInfo() << endl;
	}
}

} // namespace hoa_notification
