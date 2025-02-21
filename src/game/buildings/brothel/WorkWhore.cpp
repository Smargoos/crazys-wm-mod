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

#include "buildings/cBuildingManager.h"
#include "character/cCustomers.h"
#include "character/cPlayer.h"
#include "cRival.h"
#include "cRng.h"
#include "cInventory.h"
#include <sstream>
#include "cGangs.h"
#include "cGangManager.hpp"
#include <algorithm>
#include "IGame.h"
#include "CLog.h"
#include "scripting/GameEvents.h"
#include "cJobManager.h"
#include "cGirls.h"


sWorkJobResult WorkWhore(sGirl& girl, bool Day0Night1, cRng& rng) {
    /*
    *    WD:    Modified to fix customer service problems.. I hope :)
    *
    *    Change logic as original code is based on linked list of customers
    *    not random generation for each call to GetCustomer()
    *
    *    Pricing issues seem to be resolved with getting lots of money
    *    from customer that cant pay
    *
    *    The numbers I have added need to be tested
    *
    *    Limit number customers a girl can fuck to 10 max
    *
    *    Limit the number of customers a girl can see if they will
    *    fuck her from 5 to Max Customers * 2
    *
    *    Redid the code for deadbeat customers
    *
    *    % Chance of customers without any money getting service is
    *  percent(50 - INTELLIGENCE) / 5) where  20 < INTELLIGENCE < 100
    *    If caught will set deadbeat flag
    *
    *    GetCustomer() is generating a lot of poor customers changed
    *    code to add pay to customers funds instead of generating
    *    New customer.
    *
    *    % Chance of customer refusing to pay despite having funds is
    *    percent((40 - HAPPINESS) / 2) && percent(CONFIDENCE - 25)
    *    If caught by guards they will pay
    *
    *    Only decrement filthiness when service is performed in brothel
    *
    *    Street Work will not decrement number customers
    *  Street work will only service 66% number of customers
    *    Street Work will only generate 66% of brothel income
    *    Street Work Really needs its own NumCustomers() counter
    *
    *    Rival gangs can damage girls doing Street Work
    *  % Chance of destroying rival gang is depended on best of
    *    SKILL_COMBAT & SKILL_MAGIC / 5
    *
    *    Changed message for rival gangs attacking street girls to give
    *    EVENT_WARNING
    *
    *    GROUP sex code caculations now consolidated to one place
    *
    *  Fixed end of shift messages
    *
    *    Fame only to be done in GirlFucks()
    *
    *    Now Base Customer HAPPINESS = 60, code conslidated from 2 places in file
    *
    */

    auto brothel = girl.m_Building;

    const Action_Types actiontype = ACTION_SEX;
    // put that shit away, you'll scare off the customers!
    cGirls::UnequipCombat(girl);


    std::stringstream fuckMessage;
    std::stringstream ss;
    int iNum = 0;
    int iOriginal = 0;
    int    AskPrice = girl.askprice();
    int pay = 0;                    // pay from a single customer
    int tip = 0;                    // tip from a single customer
    int wages = 0;                    // pay from all customers
    int tips = 0;                    // tips from all customers
    int LoopCount;
    bool group = false;                // Group sex flag
    bool knowwife = false;            // if the girl is your daughter and the customer knows this
    bool knowdaughter = false;        // if the girl is your wife and the customer knows this
    bool bCustCanPay = true;        // Customer has enough money to pay
    bool custout = false;    // if a male customer tries running and gets caught
    bool femalecustcaught = false;    // if a female customer tries running and gets caught
    bool acceptsGirl = false;        // Customer will sleep girl

    int oralcount = 0;        // how much oral she gave for use with AdjustTraitGroupGagReflex
    const JOBS job = girl.get_job(Day0Night1);
    const bool bStreetWork = (job == JOB_WHORESTREETS);

    // work out how many customers the girl can service

    // Changed the number of custmers stats add.. reasone was that old value,
    // it was only adding 1 customer per stat, unless stat was 100 for beauty and Charisma. Fame would add a max of 3. and only if was = 10
    // there would be NO point in doing this, if it defaults to NumCusts++ since it is basically the same effect.    -PP

    // Max number on customers the girl can fuck
    int b = girl.beauty(), c = girl.charisma(), f = girl.fame();
    int NumCusts = std::min(8, 2 + ((b + c + f + 1) / 50));
    int NumSleptWith = 0;        // Total num customers she fucks this session

    if (bStreetWork)
    {
        NumCusts = NumCusts * 2 / 3;
        AskPrice = AskPrice * 2 / 3;
    }

    /*
    *    WD:    Rival Gang is incompleate
    *
    *    Chance of defeating gang is based on  combat / magic skill
    *    Added Damage and Tiredness
    *    ToDo Girl fightrivalgang() if its implemented
    *
    */
    std::string summary = "";
    if (bStreetWork && rng.percent(5))
    {
        cRival* rival = g_Game->random_rival();
        if (rival && rival->m_NumGangs > 0)
        {
            ss.str("");
            summary += "${name} was attacked by enemy goons. \n";
            ss << "${name} ran into some enemy goons and was attacked.\n";

            // WD: Health loss, Damage 0-15, 25% chance of 0 damage
            iNum = std::max(rng % 20 - 5, 0);
            iOriginal = girl.health();
            girl.health(-iNum);
            iNum = iOriginal - girl.health();

            ss << "She fought back ";
            if (iNum <= 0) ss << "taking no";
            else ss << "and was hurt taking " << iNum << " points of";
            ss << " damage.\n";

            // WD:    Tiredness (5 + 2 * damage) points avg is (6 + Health Damage) is bell curve
            iNum = rng % (iNum) + rng % (iNum) + 5;
            girl.tiredness(iNum);

            // WD:    If girl used magic to defend herself she will use mana
            if (girl.mana() > 20 && girl.magic() > girl.combat())
            {
                girl.mana(-20);
                iNum = girl.magic() / 5;        // WD: Chance to destroy rival gang
            }
            else iNum = girl.combat() / 5;    // WD: Chance to destroy rival gang

            if (rng.percent(iNum)) rival->m_NumGangs--;            // WD:    Destroy rival gang

            girl.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_WARNING);
        }
    }

    // WD: Set the limits on the Number of customers a girl can try and fuck
    LoopCount = std::max(NumCusts * 2, 5);

    // WD: limit to number of customers left
    if (LoopCount > g_Game->GetNumCustomers()) LoopCount = g_Game->GetNumCustomers();

    for (int i = 0; i < LoopCount; i++)    // Go through all customers
    {
        fuckMessage.str("");
        // WD:    Move exit test to top of loop
        // if she has already slept with the max she can attact then stop processing her fucking routine
        if (NumSleptWith >= NumCusts) break;
        // Stop if she has worked the bare minimum and tiredness is high enough to get a warning, pushing too hard is bad for the business too
        if ((girl.tiredness() > 80 || girl.health() < 20) && NumSleptWith >= 2) break;

        // WD:    Init Loop variables
        pay = AskPrice;
        group = false;
        acceptsGirl = false;
        custout = false;
        femalecustcaught = false;
        // WD:    Create Customer
        sCustomer Cust = g_Game->GetCustomer(*brothel);

        // `J` check for disease
        if (cGirls::detect_disease_in_customer(brothel, girl, &Cust)) continue;

        // filter out unwanted sex types (unless it is street work)
        if (!bStreetWork && !brothel->is_sex_type_allowed(Cust.m_SexPref) && !brothel->is_sex_type_allowed(Cust.m_SexPrefB))
        {
            brothel->m_RejectCustomersRestrict++;
            continue;    // `J` if both their sexprefs are banned then they leave
        }

        SKILLS SexType{};
        if (!bStreetWork && !brothel->is_sex_type_allowed(Cust.m_SexPref)) // it their first sexpref is banned then switch to the second
        {
            Cust.m_SexPref = Cust.m_SexPrefB;
            SexType = (SKILLS)Cust.m_SexPref;
            Cust.set_stat(STAT_HAPPINESS, 32 + rng % 9 + rng % 9);  // `J` and they are less happy
        }
        else    // `J` otherwise they are happy with their first choice.
        {
            // WD:    Set the customers begining happiness/satisfaction
            Cust.set_stat(STAT_HAPPINESS, 42 + rng % 10 + rng % 10); // WD: average 51 range 42 to 60
            SexType = (SKILLS)Cust.m_SexPref;
        }


        // WD:    Consolidate GROUP Sex Calcs here
        //        adjust price by number of parcitipants
        if (Cust.m_Amount > 1)
        {
            group = true;
            if (Cust.m_SexPref == SKILL_GROUP)    pay = (Cust.m_Amount * pay);                    // full price for all of them
            if (Cust.m_SexPref == SKILL_STRIP)    pay += (int)((Cust.m_Amount - 1) * pay / 2);    // full price for the first then half price for the rest
            // WD: this is complicated total for 1.7 * pay * num of customers
            // pay += (int)((float)(pay*(Cust.m_Amount))*0.7f);
        }

        // WD: Has the customer have enough money
        bCustCanPay = Cust.m_Money >= pay;

        // WD:    TRACE Customer Money = {Cust->m_Money}, Pay = {pay}, Can Pay = {bCustCanPay}

        // WD:    If the customer doesn't have enough money, he will only sleep with her if he is stupid
        if (!bCustCanPay && !rng.percent((50 - Cust.intelligence()) / 5))
        {
            //continue;
            // WD: Hack to avoid many newcustomer() calls
            Cust.m_Money += pay;
            bCustCanPay = true;
        }

        // test for specific girls
        if (girl.has_active_trait("Skeleton"))
        {
            fuckMessage << "The customer sees that you are offering up a Skeleton for sex and is scared, if you allow that kind of thing in your brothels, what else do you allow? They left in a hurry, afraid of what might happen if they stay.\n \n";
            brothel->m_Fame -= 5;
            g_Game->player().customerfear(2);
            acceptsGirl = false;
            continue;
        }
        if (Cust.m_Fetish == FETISH_SPECIFICGIRL)
        {
            if (Cust.m_ParticularGirl == brothel->get_girl_index(girl))
            {
                fuckMessage << "This is the customer's favorite girl.\n \n";
                acceptsGirl = true;
            }
        }
        else if (girl.has_active_trait("Zombie") && Cust.m_Fetish == FETISH_FREAKYGIRLS && rng.percent(10))
        {
            fuckMessage << "This customer is intrigued to fuck a Zombie girl.\n \n";
            acceptsGirl = true;
        }
        else
        {
            // 50% chance of getting something a little weirder during the night
            if (Day0Night1 && Cust.m_Fetish < NUM_FETISH - 2 && rng.percent(50)) Cust.m_Fetish += 2;

            // Check for fetish match
            if (cGirls::CheckGirlType(girl, (Fetishs)Cust.m_Fetish))
            {
                fuckMessage << "The customer loves this type of girl.\n \n";
                acceptsGirl = true;
            }
        }

        // Other ways the customer will accept the girl
        if (!acceptsGirl)
        {
            if (girl.has_active_trait("Zombie"))
            {
                fuckMessage << "The customer sees that you are offering up a Zombie girl and is scared, if you allow that kind of thing in your brothels, what else do you allow? They left in a hurry, afraid of what might happen if they stay.\n \n";
                brothel->m_Fame -= 10;
                g_Game->player().customerfear(5);
                acceptsGirl = false;
            }
            else if (girl.has_active_trait("Lesbian") && Cust.m_IsWoman && rng.percent(50))
            {
                fuckMessage << "The female customer chooses her because she is a Lesbian.\n \n";
                acceptsGirl = true;
            }
            else if (girl.has_active_trait("Straight") && Cust.m_IsWoman && rng.percent(10))
            {
                fuckMessage << "${name} refuses to accept a female customer because she is Straight.\n \n";
                brothel->m_Fame -= 2;
                acceptsGirl = false;
            }
            else if (girl.has_active_trait("Lesbian") && !Cust.m_IsWoman && rng.percent(10))
            {
                fuckMessage << "${name} refuses to accept a male customer because she is a Lesbian.\n \n";
                brothel->m_Fame -= 5;
                acceptsGirl = false;
            }
            else if (girl.dignity() >= 70 && Cust.m_SexPref == SKILL_BEASTIALITY && rng.percent(20))    //
            {
                fuckMessage << "${name} refuses to sleep with a beast because she has too much dignity for that.\n \n";
                brothel->m_Fame -= 5;
                acceptsGirl = false;
            }
            else if ((girl.has_active_trait("Queen") || girl.has_active_trait("Princess")) && Cust.m_SexPref == SKILL_BEASTIALITY && rng.percent(20))
            {
                fuckMessage << "${name} refuses to sleep with a beast because one of Royal blood is above that.\n \n";
                brothel->m_Fame -= 5;
                acceptsGirl = false;
            }
            else if (girl.is_pregnant() && Cust.m_SexPref == SKILL_BEASTIALITY && rng.percent(35))
            {
                fuckMessage << "${name} refuses because she shouldn't fuck beasts in her condition.\n \n";
                brothel->m_Fame -= 5;
                acceptsGirl = false;
            }
            else if (girl.health() < 33 && rng.percent(50))
            {
                fuckMessage << "The customer refuses because ${name} looks sick and he doesn't want to catch anything.\n \n";
                brothel->m_Fame -= 10;
                acceptsGirl = false;
            }
            else if (girl.has_active_trait("Your Daughter") && rng.percent(20))
            {
                fuckMessage << "The customer chooses her because " << (Cust.m_IsWoman ? "she" : "he") << " wants to fuck your daughter.\n \n";
                knowdaughter = true;
                acceptsGirl = true;
            }
            else if (girl.has_active_trait("Your Wife") && rng.percent(20))
            {
                fuckMessage << "The customer chooses her because " << (Cust.m_IsWoman ? "she" : "he") << " wants to fuck your wife.\n \n";
                knowwife = true;
                acceptsGirl = true;
            }
            else if (girl.has_active_trait("Porn Star") && rng.percent(15))
            {
                fuckMessage << "The customer chooses her because " << (Cust.m_IsWoman ? "she" : "he") << " has seen her in porn.\n \n";
                acceptsGirl = true;
            }
            else if ((girl.has_active_trait("Queen") || girl.has_active_trait("Princess")) && rng.percent(10))
            {
                fuckMessage << "The customer chooses her because she is former royalty.\n \n";
                acceptsGirl = true;
            }
            else if (girl.has_active_trait("Teacher") && rng.percent(10))
            {
                fuckMessage << "The customer chooses her because " << (Cust.m_IsWoman ? "she" : "he") << " used to daydream about this back when "
                            << (Cust.m_IsWoman ? "she" : "he") << " was in ${name}'s class.\n \n";
                acceptsGirl = true;
            }
            else if (girl.has_active_trait("Old") && rng.percent(20))
            {
                fuckMessage << "The customer chooses her because " << (Cust.m_IsWoman ? "she" : "he") << " likes mature women.\n \n";
                acceptsGirl = true;
            }
            else if (girl.has_active_trait("Natural Pheromones") && rng.percent(20))
            {
                fuckMessage << "The customer chooses her for reasons " << (Cust.m_IsWoman ? "she" : "he") << " can't explain. There's something about her.\n \n";
                acceptsGirl = true;
            }
            else if (rng.percent(10) && girl.has_active_trait("Lolita"))
            {
                fuckMessage << "The customer chooses her because "
                            << (Cust.m_IsWoman ? "she wants a young woman, uncorrupted by men.\n" : "he's hoping for a virgin, and she looks like one.\n") << "\n";
                acceptsGirl = true;
            }
            else if (rng.percent(20) && girl.has_active_trait("Social Drinker"))
            {
                fuckMessage << "The customer chooses her because she's fun, flirty and half-cut.\n \n";
                acceptsGirl = true;
            }
            else if (rng.percent(40) && girl.has_active_trait("Exhibitionist") && girl.beauty() >= 50)
            {
                fuckMessage << "The customer chooses her because she walks into the waiting room naked and the customer likes what "
                            << (Cust.m_IsWoman ? "she sees.\n" : "he sees.\n") << "\n";
                acceptsGirl = true;
            }
            else if (rng.percent(5) && (girl.has_active_trait("Slut") || girl.dignity() >= 70))
            {
                fuckMessage << "${name} gets bored of waiting for someone to step up and starts " << (Cust.m_IsWoman ? "fingering this lady" : "giving this guy a handjob")
                            << " right there in the waiting room. The customer quickly chooses her.\n \n";
                acceptsGirl = true;
            }
            else if (Cust.libido() >= 80)
            {
                fuckMessage << "Customer chooses her because they are very horny.\n \n";
                acceptsGirl = true;
            }
            else if (((girl.beauty() + girl.charisma()) / 2) >= 90)    // if she is drop dead gorgeous
            {
                fuckMessage << "Customer chooses her because they are stunned by her beauty.\n \n";
                acceptsGirl = true;
            }
            else if (girl.fame() >= 80)    // if she is really famous
            {
                fuckMessage << "Customer chooses her because she is so famous.\n \n";
                acceptsGirl = true;
            }
                // WD:    Use Magic only as last resort
            else if (girl.magic() > 50 && girl.mana() >= 20)    // she can use magic to get him
            {
                fuckMessage << "${name} uses magic to get the customer to choose her.\n \n";
                girl.mana(-20);
                acceptsGirl = true;
            }
        }

        if (!acceptsGirl)    // if the customer will not sleep with her
        {
            if (fuckMessage.str().length() > 0)    // if there is a reason, include it in her messages.
            {
                girl.AddMessage(fuckMessage.str(), IMGTYPE_PROFILE, EVENT_WARNING);
            }
            continue;    // otherwise just move on
        }

        // Horizontal boogy
        std::string fm;
        /// TODO this has the possibility of the girl running away. In that case, the job should stop.
        cGirls::GirlFucks(&girl, Day0Night1, &Cust, group, fm, SexType);
        fuckMessage << fm;

        /* */if (SexType == SKILL_ORALSEX)        oralcount += 5;
        else if (SexType == SKILL_GROUP)        oralcount += 5;
        else if (SexType == SKILL_BEASTIALITY)    oralcount += rng % 3;
        else if (SexType == SKILL_LESBIAN)        oralcount += rng % 2;
        else if (SexType == SKILL_TITTYSEX)        oralcount += rng % 2;
        else if (SexType == SKILL_HANDJOB)        oralcount += rng % 2;

        NumSleptWith++;
        if (!bStreetWork) brothel->m_Filthiness++;

        // update how happy the customers are on average
        brothel->m_Happiness += Cust.happiness();

        // Time for the customer to fork over some cash

        // WD:    Customer can not pay
        sGang* guardgang = g_Game->gang_manager().GetRandomGangOnMission(MISS_GUARDING);
        if (!bCustCanPay)
        {
            if (rng.percent(Cust.confidence() - 25))    // Runner
            {
                fuckMessage << " The customer couldn't pay and ";
                if (guardgang)
                {
                    if (rng%Cust.agility() > guardgang->m_Num + rng%guardgang->agility())
                    {
                        fuckMessage << "managed to elude your guards.";
                        pay = 0;
                    }
                    else
                    {
                        pay = (int)Cust.m_Money;    // WD: Take what customer has
                        Cust.m_Money = 0;
                        fuckMessage << "tried to run off.";
                        if (Cust.m_IsWoman)    femalecustcaught = true;
                        else
                        {
                            fuckMessage << " Your men caught him before he got out the door.";
                            custout = true;
                        }
                    }
                }
                else
                {
                    fuckMessage << "ran off. There were no guards!";
                    pay = 0;
                }
            }
            else
            {
                // offers to pay the girl what he has
                fuckMessage << " The customer couldn't pay the full amount";
                if (rng.percent(girl.intelligence()))    // she turns him over to the goons
                {
                    fuckMessage << " so your girl turned them over to your men";
                    if (Cust.m_IsWoman)    femalecustcaught = true;
                    else/*            */    custout = true;
                }
                fuckMessage << ".";
                pay = Cust.m_Money;
                Cust.m_Money = 0;
            }
        }
            // WD:    Unhappy Customer tries not to pay and does a runner
        else if (rng.percent((40 - Cust.happiness()) / 2) && rng.percent(Cust.confidence() - 25))
        {
            fuckMessage << " The customer refused to pay and ";
            if (guardgang)
            {
                if (rng%Cust.agility() > guardgang->m_Num + rng%guardgang->agility())
                {
                    fuckMessage << "managed to elude your guards.";
                    pay = 0;
                }
                else if (Cust.m_Money > pay + 10)
                {
                    Cust.m_Money -= pay; // WD: ??? not needed Cust record is not saved when this fn ends!  Leave for now just in case ???
                    int extra = 10 + rng%Cust.m_Money;
                    fuckMessage << "tried to run off. Your men caught him before he got out the door and forced him to pay the full amount plus " << extra << " extra for not throwing them in the dungeon.";
                    pay += extra;
                }
                else
                {
                    pay = (int)Cust.m_Money;    // WD: Take what customer has
                    Cust.m_Money = 0;
                    fuckMessage << "tried to run off.";
                    if (Cust.m_IsWoman)    femalecustcaught = true;
                    else
                    {
                        fuckMessage << " Your men caught him before he got out the door.";
                        custout = true;
                    }
                }
            }
            else
            {
                fuckMessage << " ran off. There were no guards!";
                pay = 0;
            }
        }
        else if ((knowwife || knowdaughter) && rng.percent(Cust.confidence() / 5))
        {
            fuckMessage << " The customer wanted to screw you and your " << (knowwife ? "wife" : "daughter") << " so they made a break for it";
            if (guardgang)
            {
                if (rng%Cust.agility() > guardgang->m_Num + rng%guardgang->agility())
                {
                    fuckMessage << " and managed to elude your guards.";
                    pay = 0;
                }
                else if (Cust.m_Money > pay + 10)
                {
                    Cust.m_Money -= pay; // WD: ??? not needed Cust record is not saved when this fn ends!  Leave for now just in case ???
                    int extra = 10 + rng%Cust.m_Money;
                    fuckMessage << " tried to run off. Your men caught him before he got out the door and forced him to pay the full amount plus " << extra << " extra for not throwing them in the dungeon.";
                    pay += extra;
                }
                else
                {
                    pay = (int)Cust.m_Money;    // WD: Take what customer has
                    Cust.m_Money = 0;
                    fuckMessage << " tried to run off.";
                    if (Cust.m_IsWoman)    femalecustcaught = true;
                    else
                    {
                        fuckMessage << " Your men caught him before he got out the door.";
                        custout = true;
                    }
                }
            }
            else
            {
                fuckMessage << ". They got away because there were no guards!";
                pay = 0;
            }
        }
        else  // Customer has enough money
        {
            Cust.m_Money -= (unsigned)pay; // WD: ??? not needed Cust record is not saved when this fn ends!  Leave for now just in case ??? // Yes this is necessary for TIP calculation.
            if (girl.has_active_trait("Your Daughter") && knowdaughter && Cust.m_Money >= 20 && rng.percent(50))
            {
                fuckMessage << "The customer tosses your daughter a bag of gold";
                switch (rng % 3)
                {
                case 0:        fuckMessage << " saying no dad should do this to their daughter.";                    break;
                case 1:        fuckMessage << ". They seem to enjoy the thought of fucking the boss's daughter.";    break;
                default:    fuckMessage << " with a wink and a smile.";                                            break;
                }
                Cust.m_Money -= 20;
                tip += 20;
            }
            else if (girl.has_active_trait("Your Wife") && knowwife && Cust.m_Money >= 20 && rng.percent(50))
            {
                fuckMessage << "The customer tosses your wife a bag of gold";
                switch (rng % 3)
                {
                case 0:        fuckMessage << " and tells her she can do better.";        break;
                case 1:        fuckMessage << " and asks who is better in the sack.";    break;
                default:    fuckMessage << " with a wink and a smile.";                break;
                }
                Cust.m_Money -= 20;
                tip += 20;
            }
            else if (girl.has_active_trait("Your Daughter") && Cust.m_Money >= 20 && rng.percent(15))
            {
                if (rng.percent(50))
                {
                    fuckMessage << "Learning that she was your daughter the customer tosses some extra gold down saying no dad should do this to their daughter.";
                }
                else
                {
                    fuckMessage << "A smile crossed the customer's face upon learning that she is your daughter and they threw some extra gold down. They seem to enjoy the thought of fucking the boss's daughter.";
                }
                Cust.m_Money -= 20;
                tip += 20;
            }
            else if (girl.has_active_trait("Your Wife") && Cust.m_Money >= 20 && rng.percent(15))
            {
                if (rng.percent(50))
                {
                    fuckMessage << "Learning that she was your wife the customer tosses some extra gold down saying no husband should do this to their wife.";
                }
                else
                {
                    fuckMessage << "A smile crossed the customer's face upon learning that she is your wife and they threw some extra gold down. They seem to enjoy the thought of fucking the boss's wife.";
                }

                Cust.m_Money -= 20;
                tip += 20;
            }
            // if he is happy and has some extra gold he will give a tip
            if (Cust.m_Money >= 20 && Cust.happiness() > 90)
            {
                tip = (int)Cust.m_Money;
                if (tip > 20)
                {
                    Cust.m_Money -= 20;
                    tip = 20;
                }
                else Cust.m_Money = 0;

                fuckMessage << "\nShe received a tip of " << tip << " gold.";
                tips += tip;

                // If the customer is a government official
                if (Cust.m_Official == 1)
                {
                    g_Game->player().suspicion(-5);
                    fuckMessage << " It turns out that the customer was a government official, which lowers your suspicion.";
                }
            }
        }
        fuckMessage << "\n";

        if (custout)
        {
            g_Game->PushEvent(EDefaultEvent::CUSTOMER_NOPAY);    // male customers use the old code
        }
        else if (femalecustcaught)
        {
            cJobManager::CatchGirl(girl, fuckMessage, guardgang);
        }
        else
        {
            // chance of customer beating or attempting to beat girl
            if (cJobManager::work_related_violence(girl, Day0Night1, false)) {
                pay = 0;        // WD TRACE WorkRelatedViloence {girl.m_Name} earns nothing
                // if gravely injured, stop working
                if(girl.health() < 10) {
                    i = 1000;
                }
            }
        }



        // Match image type to the deed done
        int imageType = IMGTYPE_SEX;
        /* */if (SexType == SKILL_ANAL)         imageType = IMGTYPE_ANAL;
        else if (SexType == SKILL_BDSM)         imageType = IMGTYPE_BDSM;
        else if (SexType == SKILL_NORMALSEX)    imageType = IMGTYPE_SEX;
        else if (SexType == SKILL_BEASTIALITY)  imageType = IMGTYPE_BEAST;
        else if (SexType == SKILL_GROUP)        imageType = IMGTYPE_GROUP;
        else if (SexType == SKILL_LESBIAN)      imageType = IMGTYPE_LESBIAN;
        else if (SexType == SKILL_ORALSEX)      imageType = IMGTYPE_ORAL;
        else if (SexType == SKILL_TITTYSEX)     imageType = IMGTYPE_TITTY;
        else if (SexType == SKILL_HANDJOB)      imageType = IMGTYPE_HAND;
        else if (SexType == SKILL_FOOTJOB)      imageType = IMGTYPE_FOOT;
        else if (SexType == SKILL_STRIP)        imageType = IMGTYPE_STRIP;


        // WD:    Save gold earned
        wages += pay;
        tips += tip;
        girl.AddMessage(fuckMessage.str(), imageType, Day0Night1 ? EVENT_NIGHTSHIFT : EVENT_DAYSHIFT);
    }

    // WD:    Reduce number of availabe customers for next whore
    iNum = g_Game->GetNumCustomers();        // WD: Should not happen but lets make sure
    if (iNum < NumSleptWith)    g_Game->customers().AdjustNumCustomers(-iNum);
    else                        g_Game->customers().AdjustNumCustomers(-NumSleptWith);

    // End of shift messages
    ss.str("");
    ss << "${name} saw " << NumSleptWith << " customers this shift.";
    if (g_Game->GetNumCustomers() == 0)    { ss << "\n \nThere were no more customers left."; }
    else if (NumSleptWith < NumCusts)        { ss << "\n \nShe ran out of customers who like her."; }
    summary += ss.str();

    girl.AddMessage(summary, IMGTYPE_PROFILE, Day0Night1 ? EVENT_NIGHTSHIFT : EVENT_DAYSHIFT);

    //gain
    cGirls::PossiblyGainNewTrait(girl, "Good Kisser", 50, actiontype, "${name} has had a lot of practice kissing and as such as become a Good Kisser.", Day0Night1);
    cGirls::PossiblyGainNewTrait(girl, "Slut", 60, actiontype, "${name} has turned into quite a slut.", Day0Night1, EVENT_WARNING);
    cGirls::PossiblyGainNewTrait(girl, "Nymphomaniac", 70, actiontype, "${name} has been having so much sex she is now wanting sex all the time.", Day0Night1);

    //SIN: use a few of the new traits
    if (rng.percent(1) && rng.percent(girl.oralsex()) && (girl.has_active_trait("Nymphomaniac")))
        cGirls::PossiblyGainNewTrait(girl, "Cum Addict", 90, actiontype, "${name} has tasted so much cum she now craves it at all times.", Day0Night1);

    if (girl.oralsex() > 30 && rng.percent(oralcount))
        cGirls::AdjustTraitGroupGagReflex(girl, +1, true);

    return {false, std::max(0, tips), std::max(0, wages), 0};
}


double JP_Whore(const sGirl& girl, bool estimate)// not used
{
    double jobperformance = 0.0;
    if (estimate)// for third detail string
    {
        jobperformance += cGirls::GetAverageOfSexSkills(girl) + (girl.charisma() + girl.beauty()) / 2;
    }
    else// for the actual check
    {
    }
    return jobperformance;
}
