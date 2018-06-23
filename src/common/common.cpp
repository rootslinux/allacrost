///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    common.cpp
*** \author  Tyler Olsen (Roots)
*** \brief   Source file for common code shared across the source tree
*** ***************************************************************************/

// Allacrost utilties
#include "utils.h"

// Local common headers
#include "common.h"

using namespace std;

namespace hoa_common {

bool COMMON_DEBUG = false;

////////////////////////////////////////////////////////////////////////////////
// CommonRecordGroup class
////////////////////////////////////////////////////////////////////////////////

const int32 CommonRecordGroup::BAD_RECORD = 0x7FFFFFFF;



void CommonRecordGroup::AddNewRecord(const string& record_name, int32 record_value) {
	if (DoesRecordExist(record_name) == true) {
		IF_PRINT_WARNING(COMMON_DEBUG) << "a record with the desired name \"" << record_name << "\" already existed in this group: "
			<< _group_name << endl;
		return;
	}
	_records.insert(make_pair(record_name, record_value));
}



int32 CommonRecordGroup::GetRecord(const string& record_name) {
	map<string, int32>::iterator record_iter = _records.find(record_name);
	if (record_iter == _records.end()) {
		IF_PRINT_WARNING(COMMON_DEBUG) << "a record with the specified name \"" << record_name << "\" did not exist in this group: "
			<< _group_name << endl;
		return BAD_RECORD;
	}
	return record_iter->second;
}



bool CommonRecordGroup::DeleteRecord(const string& record_name) {
	map<std::string, int32>::const_iterator element = _records.find(record_name);
	if (element != _records.end()) {
		_records.erase(element);
		return true;
	}
	else {
		return false;
	}
}


bool CommonRecordGroup::_SetOrModifyRecord(const string& record_name, int32 record_value, bool modify_only) {
	map<string, int32>::iterator record_iter = _records.find(record_name);
	if (record_iter == _records.end()) {
		if (modify_only == true)
			return false;
		else
			_records.insert(make_pair(record_name, record_value));
	}
	else {
		record_iter->second = record_value;
	}

	return true;
}

} // namespace hoa_common
