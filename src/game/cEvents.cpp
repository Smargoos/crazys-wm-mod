/*
 * Copyright 2009, 2010, The Pink Petal Development Team.
 * The Pink Petal Devloment Team are defined as the game's coders 
 * who meet on http://pinkpetal.org
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <tuple>
#include "cEvents.h"
#include "interface/constants.h"


std::string CEvent::TitleText() const
{

/*
 *    Returns default Title Texts
 *    
 *    Used to simplify code in TurnSummary
 *
 */
    switch (m_Event)
    {
    case EVENT_DAYSHIFT:
        return "Day Shift";
    case EVENT_NIGHTSHIFT:
        return "Night Shift";
    case EVENT_WARNING:
        return "Warning";
    case EVENT_DANGER:
        return "!!!DANGER!!!";
    case EVENT_GOODNEWS:
        return "!!!GOODNEWS!!!";
    case EVENT_LEVELUP:
        return "!!!LEVEL UP!!!";
    case EVENT_SUMMARY:
        return "Summary";
    case EVENT_DUNGEON:
        return "Dungeon";
    case EVENT_MATRON:
        return "Matron";
    case EVENT_GANG:
        return "Gang Report";
    //case EVENT_BROTHEL:
    //    return "Event";
    //    break;
    case EVENT_NOWORK:
        return "Refused Job";
        break;
    case EVENT_BACKTOWORK:
        return "Back to Work";
        break;
    case EVENT_DEBUG:
        return "#DEBUG#";
    default:
        return "Event";
    }
}

unsigned int CEvent::ListboxColour() const
{
/*
 *    Returns default listbox colours
 *    
 *    Used to simplify code in TurnSummary
 *
 */
    switch (m_Event)
    {
    //case EVENT_DAYSHIFT:
    //    return COLOR_BLUE;
    //case EVENT_NIGHTSHIFT:
    //    return COLOR_BLUE;
    case EVENT_WARNING:
        return COLOR_DARKBLUE;
    case EVENT_DANGER:
        return COLOR_RED;
    case EVENT_GOODNEWS:
        return COLOR_GREEN;
    case EVENT_LEVELUP:
        return COLOR_YELLOW;;
    //case EVENT_SUMMARY:
    //    return COLOR_BLUE;
    //case EVENT_DUNGEON:
    //    return COLOR_BLUE;
    //case EVENT_MATRON:
    //    return COLOR_BLUE;
    case EVENT_DEBUG:
        return COLOR_RED;
    //case EVENT_GANG:
    //    return COLOR_BLUE;
    //case EVENT_BROTHEL:
    //    return COLOR_BLUE;
    case EVENT_NOWORK:
        return COLOR_DARKBLUE;
    default:
        return COLOR_BLUE;
    }
}

bool CEvent::IsGoodNews() const
{
    return m_Event == EVENT_GOODNEWS || m_Event == EVENT_LEVELUP;
}

bool CEvent::IsUrgent() const
{
    return m_Event == EVENT_DANGER || m_Event == EVENT_WARNING || m_Event == EVENT_NOWORK ||
           m_Event == EVENT_GOODNEWS || m_Event == EVENT_LEVELUP;
}

bool CEvent::IsDanger() const
{
    return m_Event == EVENT_DANGER;
}

bool CEvent::IsWarning() const
{
    return m_Event == EVENT_WARNING || m_Event == EVENT_NOWORK;
}


void cEvents::Clear()
{
    events.clear();
    m_bSorted = false;
}

bool cEvents::HasGoodNews() const
{
    return std::any_of(begin(events), end(events), [](auto&& event){ return event.IsGoodNews(); });
}


bool cEvents::HasUrgent() const
{
    return std::any_of(begin(events), end(events), [](auto&& event){ return event.IsUrgent(); });
}

bool cEvents::HasDanger() const
{
    return std::any_of(begin(events), end(events), [](auto&& event){ return event.IsDanger(); });
}

bool cEvents::HasWarning() const
{
    return std::any_of(begin(events), end(events), [](auto&& event){ return event.IsWarning(); });
}

bool CEvent::IsCombat() const {
    return m_Report != nullptr;
}


void cEvents::AddMessage(std::string message, std::string image_type, EventType event_type)
{
    events.emplace_back(event_type, std::move(image_type), std::move(message), nullptr);
    m_bSorted = false;
}

void cEvents::AddMessage(std::string summary, EventType event, std::shared_ptr<CombatReport> rep) {
    events.emplace_back(event, IMGTYPE_COMBAT, std::move(summary), std::move(rep));
    m_bSorted = false;
}


const CEvent& cEvents::GetMessage(int id) const
{
    return events.at(id);
}

auto make_sort_tuple(const CEvent& e) {
    return std::make_tuple(!e.IsDanger(), !e.IsWarning(), e.GetEvent() != EVENT_NOWORK, e.GetEvent() != EVENT_MATRON,
            e.GetEvent() != EVENT_DUNGEON);
}

bool CEvent::CmpEventPredicate(const CEvent& eFirst, const CEvent& eSecond)
{
    return make_sort_tuple(eFirst) < make_sort_tuple(eSecond);
}

CEvent::CEvent(EventType event, std::string image, std::string message, std::shared_ptr<CombatReport> rep) :
    m_Event(event), m_ImageType(std::move(image)), m_Message(std::move(message)), m_Report(std::move(rep))
{
}

void cEvents::DoSort()
{
    if (!m_bSorted)
    {
        std::stable_sort(events.begin(), events.end(), CEvent::CmpEventPredicate);
        m_bSorted = true;
    }
}

    
    

