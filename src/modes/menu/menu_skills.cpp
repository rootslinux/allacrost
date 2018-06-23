///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2018 by The Allacrost Project
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/*!****************************************************************************
* \file    menu_skills.cpp
* \author  Daniel Steuernol (Steu)
* \author  Andy Gardner (ChopperDave)
* \brief   Source file for skill menu.
*****************************************************************************/

#include <iostream>
#include <sstream>

#include "audio.h"
#include "input.h"
#include "system.h"

#include "menu.h"

using namespace std;
using namespace hoa_menu::private_menu;
using namespace hoa_utils;
using namespace hoa_audio;
using namespace hoa_video;
using namespace hoa_gui;
using namespace hoa_global;
using namespace hoa_input;
using namespace hoa_system;

namespace hoa_menu {

namespace private_menu {


////////////////////////////////////////////////////////////////////////////////
// SkillsWindow Class
////////////////////////////////////////////////////////////////////////////////

SkillsWindow::SkillsWindow() :
    _active_box(SKILL_ACTIVE_NONE),
    _char_skillset(0)
{
    // Init option boxes
    _InitCharSelect();
    _InitSkillsList();
    _InitSkillsCategories();

    _description.SetOwner(this);
    _description.SetPosition(30.0f, 525.0f);
    _description.SetDimensions(800.0f, 80.0f);
    _description.SetDisplaySpeed(30);
    _description.SetDisplayMode(VIDEO_TEXT_INSTANT);
    _description.SetTextAlignment(VIDEO_X_LEFT, VIDEO_Y_TOP);
    _description.SetTextStyle(TextStyle("text20"));

} // SkillsWindow::SkillsWindow()



void SkillsWindow::Activate(bool new_status) {
    // Activate window and first option box...or deactivate both
    if (new_status) {
        _char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
        _active_box = SKILL_ACTIVE_CHAR;
    }
    else {
        _char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
        _active_box = SKILL_ACTIVE_NONE;
    }
}



void SkillsWindow::_InitSkillsList() {
    // Set up the inventory option box
    _skills_list.SetPosition(500.0f, 170.0f);
    _skills_list.SetDimensions(180.0f, 360.0f, 1, 255, 1, 4);
    _skills_list.SetTextStyle(TextStyle("text20"));
    _skills_list.SetCursorOffset(-52.0f, -20.0f);
    _skills_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
    _skills_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
    _skills_list.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);

    _UpdateSkillList();
    if (_skills_list.GetNumberOptions() > 0)
        _skills_list.SetSelection(0);
    _skills_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);

    // setup the cost option box
    _skill_cost_list.SetPosition(700.0f, 170.0f);
    _skill_cost_list.SetDimensions(180.0f, 360.0f, 1, 255, 1, 4);
    _skill_cost_list.SetTextStyle(TextStyle("text20"));
    _skill_cost_list.SetCursorOffset(-52.0f, -20.0f);
    _skill_cost_list.SetHorizontalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
    _skill_cost_list.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
    _skill_cost_list.SetOptionAlignment(VIDEO_X_RIGHT, VIDEO_Y_CENTER);
    _skill_cost_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}



void SkillsWindow::_InitCharSelect() {
    //character selection set up
    vector<ustring> options;
    uint32 size = GlobalManager->GetActiveParty()->GetPartySize();

    _char_select.SetPosition(72.0f, 109.0f);
    _char_select.SetDimensions(360.0f, 432.0f, 1, 4, 1, 4);
    _char_select.SetCursorOffset(-50.0f, -6.0f);
    _char_select.SetTextStyle(TextStyle("text20"));
    _char_select.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
    _char_select.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
    _char_select.SetOptionAlignment(VIDEO_X_LEFT, VIDEO_Y_CENTER);


    //Use blank strings....won't be seen anyway
    for (uint32 i = 0; i < size; i++) {
        options.push_back(MakeUnicodeString(" "));
    }

    //Set options, selection and cursor state
    _char_select.SetOptions(options);
    _char_select.SetSelection(0);
    _char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
}



void SkillsWindow::_InitSkillsCategories() {
    _skills_categories.SetPosition(458.0f, 120.0f);
    _skills_categories.SetDimensions(448.0f, 30.0f, SKILL_CATEGORY_SIZE, 1, SKILL_CATEGORY_SIZE, 1);
    _skills_categories.SetTextStyle(TextStyle("text20"));
    _skills_categories.SetCursorOffset(-52.0f, -20.0f);
    _skills_categories.SetHorizontalWrapMode(VIDEO_WRAP_MODE_SHIFTED);
    _skills_categories.SetVerticalWrapMode(VIDEO_WRAP_MODE_STRAIGHT);
    _skills_categories.SetOptionAlignment(VIDEO_X_CENTER, VIDEO_Y_CENTER);

    // Create options
    vector<ustring> options;
    options.push_back(UTranslate("All"));
    options.push_back(UTranslate("Field"));
    options.push_back(UTranslate("Battle"));

    // Set options and default selection
    _skills_categories.SetOptions(options);
    _skills_categories.SetSelection(SKILL_ALL);
    _skills_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
} // void SkillsWindow::InitSkillsCategories()



void SkillsWindow::Update() {
    OptionBox *active_option = nullptr;

    //choose correct menu
    switch (_active_box) {
    case SKILL_ACTIVE_CATEGORY:
        active_option = &_skills_categories;
        break;
    case SKILL_ACTIVE_CHAR_APPLY:
    case SKILL_ACTIVE_CHAR:
        active_option = &_char_select;
        break;
    case SKILL_ACTIVE_LIST:
        active_option = &_skills_list;
        break;
    }

    // Handle the appropriate input events
    if (InputManager->ConfirmPress())
    {
        active_option->InputConfirm();
    }
    else if (InputManager->CancelPress())
    {
        active_option->InputCancel();
    }
    else if (InputManager->LeftPress())
    {
        active_option->InputLeft();
    }
    else if (InputManager->RightPress())
    {
        active_option->InputRight();
    }
    else if (InputManager->UpPress())
    {
        active_option->InputUp();
    }
    else if (InputManager->DownPress())
    {
        active_option->InputDown();
    }

    uint32 event = active_option->GetEvent();
    active_option->Update();
    switch (_active_box) {
    case SKILL_ACTIVE_CHAR_APPLY:
        // Handle skill application
        if (event == VIDEO_OPTION_CONFIRM)
        {
            GlobalSkill *skill = _GetCurrentSkill();
            GlobalCharacter* target = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_select.GetSelection()));
            GlobalCharacter* instigator = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_skillset));

            const ScriptObject* script_function = skill->GetFieldExecuteFunction();

            if (script_function == nullptr) {
                IF_PRINT_WARNING(MENU_DEBUG) << "selected skill may not be executed in menus" << endl;
                break;
            }
            if (skill->GetSPRequired() > instigator->GetSkillPoints()) {
                IF_PRINT_WARNING(MENU_DEBUG) << "did not have enough skill points to execute skill " << endl;
                break;
            }
            ScriptCallFunction<void>(*script_function, target, instigator);
            instigator->SubtractSkillPoints(skill->GetSPRequired());
            MenuMode::CurrentInstance()->_menu_sounds["confirm"].Play();
        }
        else if (event == VIDEO_OPTION_CANCEL) {
            _active_box = SKILL_ACTIVE_LIST;
            _skills_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
            _char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
            _char_select.SetSelection(_char_skillset);
            MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
        }
        break;

    case SKILL_ACTIVE_CHAR:
        // Choose character for skillset
        if (event == VIDEO_OPTION_CONFIRM) {
            _active_box = SKILL_ACTIVE_CATEGORY;
            _char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
            _skills_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
            _char_skillset = _char_select.GetSelection();
            MenuMode::CurrentInstance()->_menu_sounds["confirm"].Play();
        }
        else if (event == VIDEO_OPTION_CANCEL) {
            Activate(false);
            _char_select.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
            MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
        }
        break;

    case SKILL_ACTIVE_LIST:
        // Choose skill
        if (event == VIDEO_OPTION_CONFIRM) {
            GlobalSkill *skill = _GetCurrentSkill();
            if (skill->IsExecutableInField())
            {
                _active_box = SKILL_ACTIVE_CHAR_APPLY;
                _skills_list.SetCursorState(VIDEO_CURSOR_STATE_BLINKING);
                _char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
                MenuMode::CurrentInstance()->_menu_sounds["confirm"].Play();
            }
            else
                MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
        }
        else if (event == VIDEO_OPTION_CANCEL) {
            _active_box = SKILL_ACTIVE_CATEGORY;
            MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
            _skills_list.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
            _skills_categories.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
        }
        break;

    case SKILL_ACTIVE_CATEGORY:
        // Choose skill type
        if (event == VIDEO_OPTION_CONFIRM) {
            _skills_list.SetSelection(0);
            if (_skills_list.GetNumberOptions() > 0) {
                _active_box = SKILL_ACTIVE_LIST;
                _skills_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
                _skills_list.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
                MenuMode::CurrentInstance()->_menu_sounds["confirm"].Play();
            }
            else {
                MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
            }
        }
        else if (event == VIDEO_OPTION_CANCEL) {
            _active_box = SKILL_ACTIVE_CHAR;
            MenuMode::CurrentInstance()->_menu_sounds["cancel"].Play();
            _skills_categories.SetCursorState(VIDEO_CURSOR_STATE_HIDDEN);
            _char_select.SetCursorState(VIDEO_CURSOR_STATE_VISIBLE);
            _char_select.SetSelection(_char_skillset);
        }
        break;
    }

    if (_active_box != SKILL_ACTIVE_CHAR_APPLY)
        _UpdateSkillList();

    if (_skills_list.GetNumberOptions() > 0 && _skills_list.GetSelection() >= 0 && static_cast<int32>(_skills_list.GetNumberOptions()) > _skills_list.GetSelection())
    {
        GlobalSkill *skill = _GetCurrentSkill();
        string desc = MakeStandardString(skill->GetName()) + "\n\n" + MakeStandardString(skill->GetDescription());
        _description.SetDisplayText(MakeUnicodeString(desc));
    }

} // void SkillsWindow::Update()

GlobalSkill *SkillsWindow::_GetCurrentSkill()
{
    GlobalCharacter* ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_skillset));

    vector<GlobalSkill *> *menu_skills = new vector<GlobalSkill *>();
    vector<GlobalSkill *> *battle_skills = new vector<GlobalSkill *>();
    vector<GlobalSkill *> *all_skills = new vector<GlobalSkill *>();

    _BuildMenuBattleSkillLists(ch->GetAttackSkills(), menu_skills, battle_skills, all_skills);
    _BuildMenuBattleSkillLists(ch->GetDefenseSkills(), menu_skills, battle_skills, all_skills);
    _BuildMenuBattleSkillLists(ch->GetSupportSkills(), menu_skills, battle_skills, all_skills);

    GlobalSkill *skill;
    switch (_skills_categories.GetSelection())
    {
    case SKILL_ALL:
        skill = all_skills->at(_skills_list.GetSelection());
        break;
    case SKILL_BATTLE:
        skill = battle_skills->at(_skills_list.GetSelection());
        break;
    case SKILL_FIELD:
        skill = menu_skills->at(_skills_list.GetSelection());
        break;
    default:
        skill = nullptr;
        cerr << "MENU ERROR: Invalid skill type in SkillsWindow::_GetCurrentSkill()" << endl;
        break;
    }

    return skill;
}


void SkillsWindow::_UpdateSkillList() {
    GlobalCharacter* ch = dynamic_cast<GlobalCharacter*>(GlobalManager->GetActiveParty()->GetActorAtIndex(_char_select.GetSelection()));
    assert(ch);
    vector<ustring> options;
    vector<ustring> cost_options;

    vector<GlobalSkill *> *menu_skills = new vector<GlobalSkill *>();
    vector<GlobalSkill *> *battle_skills = new vector<GlobalSkill *>();
    vector<GlobalSkill *> *all_skills = new vector<GlobalSkill *>();

    _BuildMenuBattleSkillLists(ch->GetAttackSkills(), menu_skills, battle_skills, all_skills);
    _BuildMenuBattleSkillLists(ch->GetDefenseSkills(), menu_skills, battle_skills, all_skills);
    _BuildMenuBattleSkillLists(ch->GetSupportSkills(), menu_skills, battle_skills, all_skills);

    vector<GlobalSkill *>::iterator i;

    switch (_skills_categories.GetSelection()) {
    case SKILL_ALL:
        // 			_skills_list.SetSize(1, all_skills->size());
        // 			_skill_cost_list.SetSize(1, all_skills->size());

        for (i = all_skills->begin(); i != all_skills->end(); ++i)
        {
            options.push_back((*i)->GetName());
            string cost = NumberToString((*i)->GetSPRequired()) + " SP";
            cost_options.push_back(MakeUnicodeString(cost));
        }
        break;
    case SKILL_BATTLE:
        // 			_skills_list.SetSize(1,battle_skills->size());
        // 			_skill_cost_list.SetSize(1, battle_skills->size());

        for (i = battle_skills->begin(); i != battle_skills->end(); ++i)
        {
            options.push_back((*i)->GetName());
            string cost = NumberToString((*i)->GetSPRequired()) + " SP";
            cost_options.push_back(MakeUnicodeString(cost));
        }
        break;
    case SKILL_FIELD:
        // 			_skills_list.SetSize(1, menu_skills->size());
        // 			_skill_cost_list.SetSize(1, menu_skills->size());

        for (i = menu_skills->begin(); i != menu_skills->end(); ++i)
        {
            options.push_back((*i)->GetName());
            string cost = NumberToString((*i)->GetSPRequired()) + " SP";
            cost_options.push_back(MakeUnicodeString(cost));
        }
        break;
    default:
        // 			_skills_list.SetSize(1,0);
        break;
    }

    _skills_list.SetOptions(options);
    _skill_cost_list.SetOptions(cost_options);

    delete menu_skills;
    delete battle_skills;
    delete all_skills;
}

void SkillsWindow::_BuildMenuBattleSkillLists(vector<GlobalSkill *> *skill_list,
    vector<GlobalSkill *> *field, vector<GlobalSkill *> *battle, vector<GlobalSkill *> *all)
{
    vector<GlobalSkill *>::iterator i;
    for (i = skill_list->begin(); i != skill_list->end(); ++i)
    {
        if ((*i)->IsExecutableInBattle())
            battle->push_back(*i);
        if ((*i)->IsExecutableInField())
            field->push_back(*i);
        all->push_back(*i);
    }
}


void SkillsWindow::Draw() {
    MenuWindow::Draw();

    //Draw option boxes
    _char_select.Draw();
    _skills_categories.Draw();
    if (_active_box == SKILL_ACTIVE_NONE)
        _UpdateSkillList();
    _skills_list.Draw();
    _skill_cost_list.Draw();
}


} // namespace private_menu

} // namespace hoa_menu
