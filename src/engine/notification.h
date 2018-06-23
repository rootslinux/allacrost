///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2016 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ***************************************************************************
*** \file   notification.h
*** \author Tyler Olsen (Roots)
*** \brief  Header file for notification event management
*** **************************************************************************/

#pragma once

#include "utils.h"
#include "defs.h"

//! \brief All calls to the notification code are wrapped inside this namespace
namespace hoa_notification {

//! \brief The singleton pointer responsible for maintaining and updating notifications
extern NotificationEngine* NotificationManager;

//! \brief Determines whether the code in the hoa_notification namespace should print debug statements or not.
extern bool NOTIFICATION_DEBUG;


/** ***************************************************************************
*** \brief A simple container class for creating notifications of important events
***
*** Sometimes when the code is running and detects that some event happens, it wants to notify other
*** code about the occurence. These notification events may be examined by other running code and trigger
*** some action or change to take place depending on the type and properties of the event. This class is
*** the lowest level type of notification event.
***
*** Notifications are identified using two different strings. The first identifies the area of the game that
*** generated the notification. Typically this should be the similar to the namespace that contained the code.
*** So the "hoa_battle" namespace uses "battle" as its identifer. The second string is used to inidicate
*** the type of event that caused the trigger to be generated, which could be anything from "collision" to
*** "equipped_weapon".
***
*** Many notifications will desire more data than these two strings can provide to indicate any particular state
*** or conditions that caused the notification to be generated. This class should be sub-classed appropriately
*** to generate such notification events.
*** **************************************************************************/
class NotificationEvent {
public:
	/** \param category String that represents the area of code creating this notification
	*** \param event Identifier string for the type of this notification
	**/
	NotificationEvent(std::string category_name, std::string event_name) :
		category(category_name), event(event_name) {}

	virtual ~NotificationEvent()
		{}

	/** \brief Returns a string representation of the data stored by this object
	***
	*** As expected, this function is for debugging purposes only. Derived classes should implement their own version
	*** of this function and print out the relevant data in a format that they desire. The string should be only one
	*** line and follow the format: "ClassName::category/event - extra data here" to maintain consistency.
	**/
	virtual const std::string DEBUG_PrintInfo()
		{ return ("NotificationEvent::" + category + "/" + event); }

	//! \brief An identifier that signifies the area of code that generated the notification (ex: "map" for map mode)
	std::string category;

	//! \brief An identifier that signifies the event that caused the notification to be created (ex: "collision")
	std::string event;

private:
	//! \brief Copy constructor is private, because making a copy of a notification object is a \b bad idea.
	NotificationEvent(const NotificationEvent& other);

	//! \brief Copy assignment operator is private, because making a copy of a notification object is a \b bad idea.
	NotificationEvent& operator=(const NotificationEvent& other);
}; // class NotificationEvent


/** ***************************************************************************
*** \brief Maintains a list of all notifications that have occurred since the last game loop update
***
*** This is a very simple engine class that does little more than maintain a list of NotificationEvent
*** objects that it provides access to. Once a NotificationEvent object is passed to this function, it
*** assumes responsibility for making sure that the object gets destroyed appropriately. So you
*** should never call delete on a NotificationEvent object that is contained in the trigger list for this class.
***
*** \note One way to iterate through all triggers in the list is to simply continue to call GetNotificationEvent()
*** with increasing index arguments until it returns nullptr, at which case you've reached the end of the list.
*** **************************************************************************/
class NotificationEngine : public hoa_utils::Singleton<NotificationEngine> {
	friend class hoa_utils::Singleton<NotificationEngine>;

public:
	~NotificationEngine();

	bool SingletonInitialize()
		{ return true; }

	/** \brief Deletes all stored NotificationEvent objects and empties the triggers list
	*** \note This should typically only be called from within the main game loop and the class destructor
	**/
	void DeleteAllNotificationEvents();

	/** \brief Sends a trigger object that has already been created
	*** \param notification A pointer to the NotificationEvent (or subclass) object that was created
	**/
	void Notify(NotificationEvent* notification);

	/** \brief Creates a new NotificationEvent object and immediately adds it to the trigger list
	*** \param category The category identifier of the code generating this trigger
	*** \param event The identifier string for the type of trigger that occurred
	**/
	void CreateAndNotify(const std::string& category, const std::string& event);

	//! \brief Returns the number of notification events currently stored by the class
	uint32 GetNotificationCount() const
		{ return _notification_events.size(); }

	/** \brief Retrieves a pointer to the NotificationEvent object stored at a particular index in the trigger list
	*** \param index The list index to retrieve the object from
	*** \return A pointer to the object, or nullptr if there is no object stored at this index
	**/
	NotificationEvent* GetNotificationEvent(uint32 index) const;

	//! \brief Returns a reference to the list of notification events stored by the class
	std::vector<NotificationEvent*>& GetAllNotificationEvents()
		{ return _notification_events; }

	//! \brief Prints the category and event name for all notifications currently stored by this class
	void DEBUG_PrintNotificationEvents() const;

private:
	NotificationEngine();

	//! \brief The list of game NotificationEvent objects that have been sent to this class since it was last cleared
	std::vector<NotificationEvent*> _notification_events;
}; // class NotificationEngine : public hoa_utils::Singleton<NotificationEngine>

} // namespace hoa_notification
