///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    common.h
*** \author  Tyler Olsen (Roots)
*** \brief   Header file for common code shared across the source tree
*** ***************************************************************************/

#pragma once

#include <string.h>

namespace hoa_common {

//! \brief Determines whether the code in the hoa_common namespace should print debug statements or not.
extern bool COMMON_DEBUG;


/** ****************************************************************************
*** \brief A container that manages the occurences of several related game records
***
*** Records in Allacrost are nothing more than a string-integer pair. The string
*** represents the name of the record while the integer takes on various meanings
*** about the record. Here are a few examples of how records are used:
***
*** - Record if the player has already seen a certain event sequence on a map.
*** - Whether the player chose option A, B, C, or D in a particular dialogue
*** - Record the number of hidden treasures a player has found on a map
***
*** Keeping so many records across the entire course of the game can result in a very
*** large data structure, and likewise a slower than desirable lookup time. To mitigate
*** this and also avoid record name collision between two distant areas of gameplay,
*** all records are stored within a record group, represented by this class. All record
*** groups are named, which is used to retrieve the appropriate group in the GlobalManager.
*** As an example, every map script file has their own record group name.
***
*** \note The GameGlobal class maintains a container of CommonRecordGroup objects and
*** provides methods to allow the creation, modification, and retrieval of these objects.
*** ***************************************************************************/
class CommonRecordGroup {
public:
	//! \brief A return value used for when a specified record name fails to be found
	static const int32 BAD_RECORD;

	//! \param group_name The name of the group to create. This can not be changed later
	CommonRecordGroup(const std::string& group_name) :
		_group_name(group_name) {}

	~CommonRecordGroup() {}

	/** \brief Queries whether or not a record of a given name exists in the group
	*** \param record_name The name of the record to check for
	*** \return True if the record name was found in the group, false if it was not
	**/
	bool DoesRecordExist(const std::string& record_name)
		{ if (_records.find(record_name) != _records.end()) return true; else return false; }

	/** \brief Adds a new record to the group
	*** \param record_name The name of the record to add
	*** \param record_value The value of the record to add (default value is zero)
	*** \note If an record by the given name already exists, a warning will be printed and no addition
	*** or modification of any kind will take place
	**/
	void AddNewRecord(const std::string& record_name, int32 record_value = 0);

	/** \brief Retrieves the value of a specific record in the group
	*** \param record_name The name of the record to retrieve
	*** \return The value of the record, or GLOBAL_BAD_RECORD if there is no record corresponding to
	*** the requested record named
	**/
	int32 GetRecord(const std::string& record_name);

	/** \brief Sets the value for an existing record, or creates a new record if one matching the record name does not exist
	*** \param record_name The name of the record whose value should be changed
	*** \param record_value The value to set for the record
	**/
	void SetRecord(const std::string& record_name, int32 record_value)
		{ _SetOrModifyRecord(record_name, record_value, false); }

	/** \brief Modifies the value of an existing record
	*** \param record_name The name of the record whose value should be changed
	*** \param record_value The value to set for the record
	*** \return True if a record was modified, false if no change took place
	*** \note This is idential to SetRecord, except that if the record does not exist,
	*** then a new record will NOT be created
	**/
	bool ModifyRecord(const std::string& record_name, int32 record_value)
		{ return _SetOrModifyRecord(record_name, record_value, true); }

	/** \brief Completely removes an existing record from the group
	*** \param record_name The name of the record to remove
	*** \return True if a record was deleted, false if no matching record was found
	**/
	bool DeleteRecord(const std::string& record_name);

	//! \brief Returns the number of records currently stored within the group
	uint32 GetNumberRecords() const
		{ return _records.size(); }

	//! \brief Returns a copy of the name of this group
	std::string GetGroupName() const
		{ return _group_name; }

	//! \brief Returns an immutable reference to the private _records container
	const std::map<std::string, int32>& GetRecords() const
		{ return _records; }

private:
	//! \brief The name given to this group of records
	std::string _group_name;

	/** \brief The map container for all the records in the group
	*** The string is the name of the record, which is unique within the group. The integer value
	*** represents the record's state and can take on multiple meanings depending on the context
	*** of this specific record.
	**/
	std::map<std::string, int32> _records;

	/** \brief Helper function that implements the functionality of SetRecord and ModifyRecord
	*** \param record_name The name of the record whose value should be set or modified
	*** \param record_value The value to set for the record
	*** \param modify_only If true, no changes will take place if an existing record  matching record_name does not exist
	*** \return True if any change to _records took place, false if no changes where made
	**/
	bool _SetOrModifyRecord(const std::string& record_name, int32 record_value, bool modify_only);
}; // class CommonRecordGroup

} // namespace hoa_common
