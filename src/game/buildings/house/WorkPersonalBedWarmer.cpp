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
#include "cJobManager.h"
#include "buildings/cBuildingManager.h"
#include "cRng.h"
#include "cInventory.h"
#include <sstream>
#include "IGame.h"
#include "character/predicates.h"
#include "character/cPlayer.h"
#include "cGirls.h"

// `J` Job House - General
sWorkJobResult WorkPersonalBedWarmer(sGirl& girl, bool Day0Night1, cRng& rng)
{
    auto brothel = girl.m_Building;

    // `J` moving multiple bed warmers to a group function
    //    if (haremcount > 1)    return girl.disobey_check(ACTION_WORKHAREM, brothel);

    std::stringstream ss;
    int roll_a = rng.d100(), roll_c = rng.d100(), roll_d = rng.d100(), roll;
    //g_Building = BUILDING_HOUSE;
    cGirls::UnequipCombat(girl);    // put that shit away, not needed for sex training
    int imagetype = IMGTYPE_MAID;
    auto msgtype = Day0Night1 ? EVENT_NIGHTSHIFT : EVENT_DAYSHIFT;

    ss << "You tell ${name} she is going to warm your bed tonight";

    //initiate things I need
    int wages = 0, tips = 0;
    int HateLove = girl.pclove() - girl.pchate();


    // Headgirl can influence against refusal
    sGirl* headGirl = random_girl_on_job(*brothel, JOB_HEADGIRL, Day0Night1);
    std::string headName = (headGirl ? headGirl->FullName() : "");

    //    refusal check
    if (girl.disobey_check(ACTION_WORKHAREM, JOB_PERSONALBEDWARMER))
    {
#if 1
        ss << " but she refuses to lay with you.";
        int effectiveness = rng.d100();

        if (headGirl)
        {
            ss << "\n \nHead Girl " << headName << " intervenes";
            if (girl.is_slave())
            {
                //BSIN
                //slave - so cannot refuse.
                //planned to just make easier to persuade slaves, but as this job runs separately on each girl, this meant
                //a girl refused in her own description, but showed up in someone else's harem group.
                //Cannot see an easy way around - short of some kind of global tracking - so for now Slave = compulsory participation.
                //And group harem will only involve slaves. Simple.
                //Headgirl will persuade somehow

                ss << " reminding ${name} that she is your property and must do as you command. ";

                // If headgirl likes girls... Because this will be a rare combo this is at the top, so it has some chance of being seen
                if (headGirl->has_active_trait("Lesbian") && girl.has_active_trait("Lesbian") && effectiveness > 10)
                {
                    ss << "As ${name} continues to refuse " << headName << " suddenly kisses her, putting a hand under her underwear and pulling back to expose her ";
                    if (girl.has_active_trait("Pierced Nipples")) ss << "pierced nipples";
                    else if (girl.has_active_trait("Pierced Clit")) ss << "pierced clit";
                    else if (girl.has_active_trait("Perky Nipples")) ss << "perky nipples";
                    else if (girl.has_active_trait("Puffy Nipples")) ss << "puffy nipples";
                    else if (girl.has_active_trait("Abundant Lactation") || girl.has_active_trait("Cow Tits")) ss << "moist, milky breasts";
                    else if (girl.has_active_trait("Flat Chest") || girl.has_active_trait("Petite Breasts") ||
                             girl.has_active_trait("Small Boobs")) ss << "small breasts";
                    else if (girl.has_active_trait("Busty Boobs") || girl.has_active_trait("Big Boobs") ||
                             girl.has_active_trait("Giant Juggs")) ss << "large breasts";
                    else if (girl.has_active_trait("Massive Melons") || girl.has_active_trait("Abnormally Large Boobs") ||
                             girl.has_active_trait("Titanic Tits")) ss << "extremely large breasts";
                    else ss << "breasts";
                    ss << ", which she eagerly licks. ${name} is initially shocked, but is quickly aroused by "
                        << headName << "'s expert tongue.\nThey're soon putting on a hell of a show together. "
                        << headName << " quickly drives her wild, bringing her to the brink of orgasm several times, but never quite letting her come.\n \n"
                        << headName << " suddenly stops, leaving ${name} on your bed, wet, frustrated, horny as hell and begging to be fucked.";

                    girl.upd_Enjoyment(ACTION_WORKHAREM, 2);
                    girl.upd_temp_stat(STAT_LIBIDO, +10, true);
                    headGirl->lesbian(+4);
                }

                //If headgirl has some charm and turns it on a little...
                else if ((headGirl->has_active_trait("Cool Person") || headGirl->has_active_trait("Charismatic") ||
                          headGirl->has_active_trait("Charming") || headGirl->charisma() > 70) && effectiveness > 20)
                {
                    ss << "Because " << headName << " is so cool with her and makes it all seem exciting, ${name} soon agrees.";
                    girl.happiness(5);
                    girl.upd_Enjoyment(ACTION_WORKHAREM, 2);
                }

                //if slave is an addict
                else if (is_addict(girl, true) && effectiveness > 10)
                {
                    ss << "Noticing the girl's shakes, and the look in her eye, " << headName
                       << " suggests that tonight could help ${name} raise a little \"spending money.\"\nShe agrees to stay for "
                       << effectiveness / 2 << " extra drug money.";
                    tips += (effectiveness / 2);
                    girl.obedience(1);
                }

                //If the reminder of her status changes her mind
                else if (effectiveness > 70)
                {
                    ss << "Remembering she is a slave, ${name} reluctantly submits.\n";
                    girl.happiness(-2);
                }

                //if head is a bully
                else if (headGirl->has_active_trait("Aggressive") && effectiveness > 20)
                {
                    ss << "When she still refuses, ";
                    if (effectiveness > 70)
                    {
                        ss << headName << " loses it and rips off all the slave's clothes. Grabbing her violently by the hair, " << headName << " drags ${name}"
                            << " naked into the streets and chains her over a large crate. She smiles, slapping and spreading ${name}'s exposed ass.\n\"Guess I'll leave you out for the "
                            << "city's dogs and homeless. Have a great night!\"\nAs she starts to walk away, ${name} finally breaks and screams to be let back in. She'll do whatever you want.\n"
                            << headName << " reluctantly agrees.";
                        girl.happiness(-15);
                        girl.obedience(5);
                        girl.spirit(-5);
                        girl.pcfear(5);
                        girl.upd_Enjoyment(ACTION_WORKHAREM, 1);  //relief
                    }
                    else
                    {
                        ss << headName << " grabs her hair and threatens her. She sees no choice but to stay.";
                        girl.happiness(-5);
                        girl.obedience(1);
                        girl.upd_Enjoyment(ACTION_WORKHAREM, -1);
                    }
                }

                // if nothing else can persuade her
                else
                {
                    ss << "After some argument, your Head Girl makes clear there is no choice for a slave in this, and offers "
                        << "${name} something to 'make it all feel like a nice warm dream.'\n${name} finally nods.";
                    girl.obedience(1);
                    girl.spirit(-1);
                    girl.health(-1);
                    g_Game->gold().misc_debit(20); //drug/spell money
                    girl.upd_temp_stat(STAT_LIBIDO, 2, true);
                    girl.pchate(5); //she'll hate you later
                    HateLove = 50;  //probably best fit for next bit...
                }
            }
            else // not slave
            {
                // If headgirl likes girls. Again because rare combo, is at the top, so it has some chance of being seen - this time can fail
                if (headGirl->has_active_trait("Lesbian") && girl.has_active_trait("Lesbian") && effectiveness > 10)
                {
                    ss << " suddenly kissing ${name} and putting a hand under her underwear, pulling back to expose her ";
                    if (girl.has_active_trait("Pierced Nipples")) ss << "pierced nipples";
                    else if (girl.has_active_trait("Pierced Clit")) ss << "pierced clit";
                    else if (girl.has_active_trait("Perky Nipples")) ss << "perky nipples";
                    else if (girl.has_active_trait("Puffy Nipples")) ss << "puffy nipples";
                    else if (girl.has_active_trait("Abundant Lactation") || girl.has_active_trait("Cow Tits")) ss << "moist, milky breasts";
                    else if (girl.has_active_trait("Flat Chest") || girl.has_active_trait("Petite Breasts") ||
                             girl.has_active_trait("Small Boobs")) ss << "small breasts";
                    else if (girl.has_active_trait("Busty Boobs") || girl.has_active_trait("Big Boobs") ||
                             girl.has_active_trait("Giant Juggs")) ss << "large breasts";
                    else if (girl.has_active_trait("Massive Melons") || girl.has_active_trait("Abnormally Large Boobs") ||
                             girl.has_active_trait("Titanic Tits")) ss << "extremely large breasts";
                    else ss << "breasts";
                    ss << ", which she eagerly licks. ${name} is shocked, but is quickly aroused by "
                        << headName << "'s expert tongue.\n \nThey're soon putting on a hell of a show together";

                    girl.upd_Enjoyment(ACTION_WORKHAREM, 2);
                    if (effectiveness < 60)
                    {
                        ss << ", licking, rubbing, and bringing each other to orgasm after orgasm there on your bed.\n \n"
                            << "You're a bit annoyed when ${name} leaves, exhausted, to spend the night in " << headName
                            << "'s room.\nStill they did put on quite a show, and she probably learned something tonight.\n";
                        headGirl->lesbian(+2);
                        girl.lesbian(+2);
                        girl.AddMessage(ss.str(), IMGTYPE_LESBIAN, EVENT_NOWORK);
                        return {true, 0, 0, 0};
                    }
                    else
                    {
                        ss << ". " << headName << " quickly drives her wild, bringing her right to the brink of orgasm, but never quite letting her come. ";
                        ss << headName << " suddenly stops, leaving ${name} on your bed, wet, frustrated, horny as hell and begging to be fucked.\n";
                        girl.upd_temp_stat(STAT_LIBIDO, +10, true);
                        headGirl->lesbian(+4);
                    }
                }

                //if headgirl is VERY persuasive, or much smarter, or persuasive and charming...
                else if (effectiveness >= 80 || headGirl->intelligence() >= girl.intelligence() + 20
                    || (effectiveness > 30 && (headGirl->has_active_trait("Charismatic") ||
                                               headGirl->has_active_trait("Charming") || girl.charisma() > 80)))
                {
                    ss << " and talks her into taking part.\n";
                }

                //if girl is an addict
                else if (is_addict(girl, true) && effectiveness > 20)
                {
                    ss << " and offers her some drug money if she'll stay. ${name} agrees to stay for " << effectiveness << " extra gold.\n";
                    tips += effectiveness;
                }

                //if headgirl is a bully
                else if (headGirl->has_active_trait("Aggressive") && effectiveness > 50)
                {
                    ss << " grabbing her hair and threatening her";
                    // IF she refuses to go along, and she's tough enough to physically resist
                    if ((girl.spirit() > 60 || girl.has_active_trait("Aggressive") || girl.has_active_trait("Fearless") ||
                         girl.has_active_trait("Iron Will"))
                        && girl.combat() > (headGirl->combat() - 30))
                    {
                        ss << ". ${name} snaps, grabbing " << headName << " by the throat and telling her to go fuck herself.\n";
                        girl.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_NOWORK);
                        return {true, 0, 0, 0};
                    }
                    else
                    {
                        ss << " leaving her no choice but to stay.\n";
                        girl.happiness(-20);
                        girl.upd_Enjoyment(ACTION_WORKHAREM, -5);
                    }
                }

                //if she's worth more
                else if (girl.beauty() > 95 || girl.has_active_trait("Princess") || girl.has_active_trait("Queen") ||
                         girl.has_active_trait("Goddess"))
                {
                    ss << " stopping her. Because she's rather special you offer extra money. ";
                    //
                    if (rng.percent(50))
                    {
                        int pay = 2 * girl.askprice();
                        ss << "${name} agrees to stay for " << pay << " extra gold.\n";
                        tips += pay;
                    }

                    else
                    {
                        ss << "She refuses.\n";
                        girl.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_NOWORK);
                        return {true, 0, 0, 0};
                    }
                }

                //if nothing can change her mind
                else
                {
                    ss << " but is unable to change her mind.\n";
                    girl.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_NOWORK);
                    return {true, 0, 0, 0};
                }
            }
        }
        else  // no headgirl
        {
            if (girl.is_slave())
            {
                //slave - so cannot refuse.
                //planned to just make easier to persuade slaves, but as this job runs separately on each girl, this meant
                //a girl refused in her own description, but showed up in someone else's harem group.
                //Cannot see an easy way around - short of some kind of global tracking - so for now Slave = compulsory participation.
                //And group harem will only involve slaves. Simple.
                //You will persuade somehow

                ss << "You remind ${name} that slaves are property and should do as their master commands";

                //does she like you
                if (HateLove > 80)
                {
                    ss << ". Thinking again, she agrees as you have been an excellent master.";
                }
                //do you have anything she wants?
                else if (girl.has_active_trait("Cum Addict") && effectiveness > 30)
                {
                    ss << ". Blushing, she says she'll do it... as long as she gets to finish you off with her mouth."
                        << "\nYou do not have a problem with this.";
                    girl.happiness(5);
                    girl.upd_Enjoyment(ACTION_WORKHAREM, 2);
                }
                else if (girl.has_active_trait("Nymphomaniac") && effectiveness > 30)
                {
                    ss << ". She raises her eyebrows and drops her clothes to the floor.\n"
                        << "\"Then get on and fuck me already... 'Master.'\"";
                    girl.happiness(5);
                    girl.upd_Enjoyment(ACTION_WORKHAREM, 2);
                }
                else if (is_addict(girl, true) && effectiveness > 10)
                {
                    ss << ". Noticing her shakes and the look in her eye, you suggest tonight could help her raise a little \"spending money.\"\n"
                        << "She agrees to stay for " << effectiveness / 2 << " extra drug money.";
                    tips += (effectiveness / 2);
                    girl.obedience(1);
                }
                //Are you psychopathic
                else if ((g_Game->player().disposition() < -80) && effectiveness > 60)
                {
                    ss << ", but add that you are a kind master and she has nothing to fear from you. Over a drink, you explain how you respect your slaves and their choices, "
                        << "and that her choice to defy you, led to your choice to drug her.\nShe smiles dreamily, rubbing her breasts against you as you strip away her clothes."
                        << "\nYour drug delivers the perfect blend of horniness and suggestibility. ${name} will participate. Fully.\n";
                    g_Game->gold().misc_debit(100); //drug money
                    girl.upd_temp_stat(STAT_LIBIDO, 10, true);
                    girl.pchate(+10); //she'll hate you later
                    HateLove = 50;  //probably best fit for next bit...
                }
                //Lucky?
                else if (effectiveness > 75)
                {
                    ss << ". You are very persuasive and recognizing the truth of your words, "
                        << "${name} apologizes, promising to do better in future.\n";
                    girl.obedience(4);
                    girl.spirit(-4);
                    girl.upd_Enjoyment(ACTION_WORKHAREM, 2);
                }
                //does she like you
                else if (HateLove > 40)
                {
                    ss << ". She agrees to stay because you've mostly been good to her.";
                }
                //are you nice
                else if (g_Game->player().disposition() > 40)
                {
                    ss << ", adding that you are a kind master she should trust that you would only ever act in her best interests.\nWhile you respect that she is uncomfortable, ${name}"
                        << " must broaden her mind, adjust to this life and learn the skills.\nShe WILL share your bed tonight, and she will learn something too.\n"
                        << "\"One day you will thank me,\" you tell her.\nShe looks up at you, closes her eyes and nods.\n";
                    girl.pcfear(-2);
                    girl.pclove(+1);
                    girl.happiness(-1);
                }
                //or just mean?
                else if (g_Game->player().disposition() < -50)
                {
                    ss << ", adding that slaves who defy you here, get to enjoy a much more intimate and much less pleasant night down in the dungeon...\n";
                    ss << "\"Less pleasant for YOU, at least,\" you smile.\n \n";
                    girl.pcfear(5);
                    if (effectiveness > 35)
                    {
                        ss << "Terrified, she agrees to spend the night.\n";
                        girl.obedience(2);
                        girl.spirit(-2);
                        girl.happiness(-10);
                    }
                    else
                    {
                        ss << "She doesn't say a word. She doesn't leave either.\n";
                        girl.obedience(1);
                        girl.spirit(-1);
                        girl.happiness(-5);
                    }
                }
                else
                {
                    ss << ". Finally, you explain that as a slave she doesn't have a choice in this. You offer "
                        << "${name} a little something to 'make it all feel like a nice warm dream.'\n${name} finally nods.";
                    girl.obedience(1);
                    girl.spirit(-1);
                    girl.health(-1);
                    g_Game->gold().misc_debit(20); //drug/spell money
                    girl.upd_temp_stat(STAT_LIBIDO, 2, true);
                    girl.pchate(5); //she'll hate you later
                    HateLove = 50;  //probably best fit for next bit...
                }
            }
            else // not a slave
            {

                ss << "${name} is a free woman, so you cannot legally force her. ";

                //  Lucky
                if (effectiveness > 90 || ((girl.intelligence() < 30) && effectiveness > 30))
                {
                    ss << "However, you successfully talk her into taking part.\n";
                }
                //if you have anything she wants?
                else if (girl.has_active_trait("Cum Addict") && effectiveness > 50)
                {
                    ss << "Blushing, she says she might do it... if you'll finish in her mouth."
                        << "\nYou agree to this.\n";
                    girl.happiness(5);
                    girl.upd_Enjoyment(ACTION_WORKHAREM, 2);
                }
                else if ((girl.has_active_trait("Nymphomaniac") || girl.has_active_trait("Slut")) && effectiveness > 40)
                {
                    ss << "Aware of her love for sex, you quickly snake your hand between her legs and start to rub on her panties. "
                        << "She doesn't try to stop you, instead just moving her panties aside and guiding your hand.\nShe is quickly naked, panting and aroused and is now happy to stay.\n";
                    girl.happiness(2);
                    girl.upd_Enjoyment(ACTION_WORKHAREM, 2);
                }
                //if she's an addict
                else if (is_addict(girl, true) && effectiveness > 40)
                {
                    ss << "You offer her some drug money if she'll stay. ${name} agrees to stay for " << 2 * effectiveness << " extra gold.\n";
                    tips += (2 * effectiveness);
                }
                //if she's worth more
                else if (girl.beauty() > 95 || girl.has_active_trait("Princess") || girl.has_active_trait("Queen") ||
                         girl.has_active_trait("Goddess"))
                {
                    ss << "Because she's rather special you offer some extra money.\n";
                    //
                    if (rng.percent(65))
                    {
                        int pay = 2 * girl.askprice();
                        ss << "${name} agrees to stay for " << pay << " extra gold.\n";
                        tips += pay;
                    }
                    else
                    {
                        ss << "She refuses.\n";
                        girl.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_NOWORK);
                        return {true, 0, 0, 0};
                    }
                }
                //if you are pure evil and very lucky
                else if ((g_Game->player().disposition() < -90) && effectiveness > 80)
                {
                    ss << "You persuade her to stay for a few minutes, for a casual chat over a drink. She agrees. Her words soon start to slur as the drug takes effect."
                        << "\nShe smiles dreamily, rubbing her nipples against you as you strip away her clothes."
                        << "\nYour drug delivers a perfect blend of horniness and suggestibility. ${name} will now participate. Fully.\n";
                    g_Game->gold().misc_debit(100); //drug money
                    girl.upd_temp_stat(STAT_LIBIDO, 10, true);
                    girl.pchate(+20); //she'll hate you later
                    HateLove = 50;  //probably best fit for next bit...
                }
                //if nothing can change her mind
                else
                {
                    ss << "She leaves.\n";
                    girl.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_NOWORK);
                    return {true, 0, 0, 0};
                }
            }
        }
#endif
    }
    else ss << ".\n \n";

    //sin
    bool diseased = false, risky = false;

    //Disease refusal.
    if (has_disease(girl))
    {
        if (HateLove > 40)  //if she doesn't want you dead...
        {
            ss << "${name} is diseased and refuses to put you at risk.\n";
            girl.morality(2);
            girl.AddMessage(ss.str(), IMGTYPE_PROFILE, Day0Night1 ? EVENT_NIGHTSHIFT : EVENT_DAYSHIFT);
            return {true, 0, 0, 0};
        }
        else if (HateLove > -40)  //if she doesn't care...
        {
            ss << "${name} is diseased, but doesn't care if you don't.\n";
            diseased = true;
        }
        else  //if she actively wants you dead
        {
            ss << "${name} is happy for the chance to share her disease with you.\n";
            girl.morality(-2);
            girl.happiness(2);
            diseased = true;
        }
    }

    if (girl.is_slave())
    {
        wages += 20;  //slaves just get pocket money for being here - more if they do more
        /* */if (HateLove < -80)    ss << "She hates you more then anything but you own her so she does what she is told";
        else if (HateLove < -60)    ss << "She hates you but knows she must listen";
        else if (HateLove < -40)    ss << "She doesn't like you but she is your slave and does what she is told";
        else if (HateLove < -20)    ss << "She finds you annoying but knows she must listen";
        else if (HateLove < 0)        ss << "She finds you to be annoying but you own her";
        else if (HateLove < 20)        ss << "She finds you be a decent master";
        else if (HateLove < 40)        ss << "She finds you be a good master";
        else if (HateLove < 60)        ss << "She finds you to be attractive";
        else if (HateLove < 80)        ss << "She has really strong feelings for you";
        else                        ss << "She loves you more than anything";
    }
    else
    {
        wages += 100; // non slaves get paid - more later if they do more
        /* */if (HateLove < -80)    { wages += 150;    ss << "She can't stand the sight of you and demands way more money to lay with you at night"; }
        else if (HateLove < -60)    { wages += 100;    ss << "She don't like you at all and wants more money to lay with you at night"; }
        else if (HateLove < -40)    { wages += 75;    ss << "She doesn't like you so she wants extra for the job"; }
        else if (HateLove < -20)    { wages += 50;    ss << "She finds you annoying so she wants extra for the job"; }
        else if (HateLove < 0)        { wages += 25;    ss << "She finds you to be annoying so she wants extra for the job"; }
        else if (HateLove < 20)        { ss << "She finds you to be okay"; }
        else if (HateLove < 40)        { wages -= 20;    ss << "She finds you to be nice so she gives you a discount"; }
        else if (HateLove < 60)        { wages -= 40;    ss << "She finds you attractive so she gives you a discount"; }
        else if (HateLove < 80)        { wages -= 60;    ss << "Shes has really strong feelings for you so she lays with you for less money"; }
        else                        { wages -= 80;    ss << "She is totally in love with you and doesn't want as much money"; }
    }
    ss << ".\n \n";

    if (roll_a <= 15)
    {
        ss << "\nYou did something to piss her off.\n \n";
        girl.upd_Enjoyment(ACTION_WORKHAREM, -1);
        girl.pclove(-1);
    }
    else if (roll_a >= 90)
    {
        ss << "\nShe had a pleasant time with you.\n \n";
        girl.upd_Enjoyment(ACTION_WORKHAREM, +3);
        girl.pclove(2);
    }
    else
    {
        ss << "\nIt was as she expected.\n \n";
        girl.upd_Enjoyment(ACTION_WORKHAREM, +1);
    }











#if 1

    //*****************************************NEW
    //One by one ain't a harem!
    //If we have lots of slaves let's put them all together...

    //**********ONE: check for lots of bedwarmers
    std::vector<sGirl*> harem = brothel->girls_on_job(JOB_PERSONALBEDWARMER, Day0Night1);

    //total girls on job
    int warmers = harem.size();

    //Slaves are the only ones we can guarantee haven't refused
    //so this is just going to include slaves
    bool other_notslave = false, other_sick = false;
    for (int i = warmers; i-- > 0; other_notslave = other_sick = false)
        // need to walk backwards through vector, as working forward while erasing no good
    {

        if (harem[i]->is_slave())
        {
        }
        else
        {    //If the other girl is not slave, she will not join group action. Pop this girl off the list and check the next
            other_notslave = true;
        }
        if (has_disease(*harem[i]))
        {
            other_sick = true;
        }

        if (other_sick || other_notslave)
        {
            harem.erase(harem.begin() + i);
        }
    }

    //slaves left for activity
    int sz = harem.size();

    //if there's still a group, and THIS girl is a healthy slave, follow this logic - otherwise use the old logic.
    //Diseased & hatefilled girl can still do you one-to-one, but won't join group (and risk other girls' health)
    if (!diseased && sz > 1 && girl.is_slave())
    {
        //to simplify the upcoming code
        //Scenarios are options for the first Case struct, actions are for the 2nd
        int NUMSCENARIOS = 9;
        int NUMACTIONS = 11;

        wages += 50;

        //SCENARIOS
        //Was gonna use a random number for this, but since script runs separately for each girl
        //resulted in one member of group reporting one kind of orgy, another reporting completely different one
        //using date as a common field. should result in same psuedo-random option for all
        if (Day0Night1)    roll = g_Game->date().day % NUMSCENARIOS;
        else roll = (g_Game->date().day + 1) % NUMSCENARIOS;

        ss << "\nYou order the " << sz << " slaves warming your bed to put on a show.\n";
        switch (roll)
        {
        case 0:
            ss << "You put some toys on the bed and ask your girls to warm each other up. You watch for a while as around you ";
            break;
        case 1:
            ss << "You get them to bathe together. Then you watch as ";
            break;
        case 2:
            if (headGirl) ss << "Your Head Girl sits with you to watch as ";
            else ss << "There are squeals, yelps, shouts and groans as ";
            break;
        case 3:
            ss << "You ply them with drinks and watch as ";
            break;
        case 4:
            ss << "You have work to take care of. Over on the bed ";
            break;
        case 5:
            ss << "One of your gangs turns up to give you an update. The newest member openly stares at your bed where ";
            break;
        case 6:
            ss << "A police chief comes to visit. You negotiate an excellent deal while he is clearly distracted by the scene in the corner, where ";
            g_Game->player().suspicion(-10);
            break;
        case 7:
            ss << "You are tired. You sit on the sofa with a drink, barely bothering to watch as ";
            break;
        case 8:
            if ((warmers - sz) > 1) ss << "The other girls in your house also watch as ";
            else if ((warmers - sz) == 1) ss << "The other girl in your house watches as ";
            else ss << "You notice a neighbour staring through your window as ";
            break;
        default:
            ss << "Something impossible happens while ";
            break;
        }

        //Grammar fork: if just two girls
        if (sz == 2) ss << harem[0]->FullName() << " and " << harem[1]->FullName();
        else
        {
            //if more than two
            ss << harem[0]->FullName();
            for (int i = 1; i < (harem.size() - 1); i++)
            {
                ss << ", " << harem[i]->FullName();
            }
            ss << " and " << harem[harem.size() - 1]->FullName();
        }

        //ACTIONS
        //Was gonna use a random number for this, but since script runs seperately for each girl
        //resulted in one member of group reporting one kind of orgy, another reporting completely different one
        //using date as a common field. should result in same psuedo-random option for all
        if (Day0Night1)    roll = g_Game->date().day % NUMACTIONS;
        else roll = (g_Game->date().day + 1) % NUMACTIONS;
        switch (roll)
        {
        case 0:
            ss << " share some impressively large sex toys. ";
            if (girl.has_active_trait("Fast Orgasms"))
            {
                ss << "${name} orgasms loudly and repeatedly.\n \n";
                girl.upd_Enjoyment(ACTION_WORKHAREM, 1);
                girl.happiness(1);
            }
            else ss << "\n \n";
            break;
        case 1:
            ss << " play with anal beads and magical growth oils.\n \n";
            break;
        case 2:
            ss << " lick, touch and tease one another.\n \n";
            break;
        case 3:
            ss << " compete to whip each other across the bed. ";
            if (girl.has_active_trait("Sadistic"))
            {
                ss << "${name} loves this and loves the squeal every time she manages to crack her whip right on another girl's clit or nipples.\n \n";
                girl.upd_Enjoyment(ACTION_WORKTORTURER, 2);
            }
            else if (girl.has_active_trait("Masochist"))
            {
                ss << "${name} is awful at this, with a stance like she wants to be hit. She seems to be enjoying it though.\n \n";
                girl.upd_Enjoyment(ACTION_WORKHAREM, 1);
            }
            else ss << "\n \n";
            break;
        case 4:
            ss << " oil up and do some rough naked wrestling.\n \n";
            break;
        case 5:
            ss << " intimately 'clean' each other.\n \n";
            break;
        case 6:
            ss << " finger-fuck and fist each other.\n \n";
            break;
        case 7:
            ss << " crawl across your bed mouth-to-pussy in a writhing human-cunterpillar.\n \n";
            break;
        case 8:
            ss << " strip off and dance intimately together.\n \n";
            break;
        case 9:
            ss << " make excellent use of your 12-ended tentacle-beast dildo.\n \n";
            girl.beastiality(+1);
            break;
        case 10:
            ss << " train together, inserting and then 'squeezing out' eggs.\n \n";
            girl.normalsex(+1);
            break;
        default:
            ss << " do something unimaginable. (error)\n \n";
            break;
        }

        //Benefits of group
        girl.lesbian(+(harem.size() / 2));
        girl.performance(+(harem.size() / 4));
        girl.tiredness(+(harem.size() / 2));
        int libido = (girl.has_active_trait("Nymphomaniac")) ? 4 : 2;
        girl.upd_temp_stat(STAT_LIBIDO, libido);
        girl.upd_Enjoyment(ACTION_WORKHAREM, libido);

        // now you get involved...
        if (is_virgin(girl))
        {
            ss << "${name} is a virgin";
            wages += 20;

            if (roll_d <= 20)
            {
                ss << " so you fuck her gently and with extra care.\n";
                girl.normalsex(1);
                girl.pclove(1);
                girl.upd_Enjoyment(ACTION_WORKHAREM, +1);
                girl.lose_trait("Virgin");
                ss << "She is no longer a virgin.\n";
                imagetype = IMGTYPE_SEX;
                //girl.m_Events.AddMessage(ss.str(), IMGTYPE_SEX, Day0Night1);

                girl.calc_pregnancy(&g_Game->player(), 1.0);
            }
            else if (roll_d <= 35)
            {
                ss << " with a tight pussy you can't resist. You break her in hard.\n";
                girl.normalsex(2);
                girl.pcfear(2);
                girl.lose_trait("Virgin");
                ss << "She is no longer a virgin.\n";
                imagetype = IMGTYPE_SEX;
                //girl.m_Events.AddMessage(ss.str(), IMGTYPE_SEX, Day0Night1);

                girl.calc_pregnancy(&g_Game->player(), 1.0);
            }
            else
            {
                ss << ". You decide to keep her virginity intact for now";
                if (roll_c < 30 || girl.has_active_trait("Lesbian"))
                {
                    ss << ", and instead have her play around with another girl in your harem.";
                    girl.lesbian(2);
                    imagetype = IMGTYPE_LESBIAN;
                    //girl.m_Events.AddMessage(ss.str(), IMGTYPE_LESBIAN, Day0Night1);
                }
                else if (roll_c < 60)
                {
                    ss << ", and instead fuck her ass.";
                    girl.anal(2);
                    imagetype = IMGTYPE_ANAL;
                    //girl.m_Events.AddMessage(ss.str(), IMGTYPE_ANAL, Day0Night1);
                }
                else
                {
                    ss << ", and instead have her 'clean it off' as you enjoy the other girls.";
                    girl.oralsex(2);
                    imagetype = IMGTYPE_ORAL;
                    //girl.m_Events.AddMessage(ss.str(), IMGTYPE_ORAL, Day0Night1);
                }
            }
        }
        else   // not virgin
        {
            if ((roll_d <= 20) || (girl.has_active_trait("Cum Addict") && roll_d <= 50))
            {
                ss << "While you enjoy another girl, ${name} grabs your cock and finishes you off with her mouth.\n";
                girl.oralsex(2);
                imagetype = IMGTYPE_ORAL;
                //girl.m_Events.AddMessage(ss.str(), IMGTYPE_ORAL, Day0Night1);
            }
            else if (roll_d <= 40)
            {
                ss << "You watch ${name} play with another girl in the harem.";
                girl.lesbian(2);
                imagetype = IMGTYPE_LESBIAN;
                //girl.m_Events.AddMessage(ss.str(), IMGTYPE_LESBIAN, Day0Night1););
            }
            else if (roll_d <= 60)
            {
                ss << "You enjoy the pleasures of your harem, including fucking ${name}.\n";
                girl.normalsex(2);
                girl.lesbian(1);
                imagetype = IMGTYPE_SEX;
                //girl.m_Events.AddMessage(ss.str(), IMGTYPE_SEX, Day0Night1);

                girl.calc_pregnancy(&g_Game->player(), 1.0);
            }
            else if (roll_d <= 80)
            {
                ss << "You tie up ${name} and get another girl to spank her as you fuck her.\n";
                girl.bdsm(2);
                imagetype = IMGTYPE_BDSM;
                //girl.m_Events.AddMessage(ss.str(), IMGTYPE_BDSM, Day0Night1);

                girl.calc_pregnancy(&g_Game->player(), 1.0);
            }
            else if (roll_d <= 100)
            {
                ss << "You spin ${name} around, push her down and fuck her ass.";
                girl.anal(2);
                imagetype = IMGTYPE_ANAL;
                //girl.m_Events.AddMessage(ss.str(), IMGTYPE_ANAL, Day0Night1);
            }
        }
        //Some group action randomness...
        if (rng.percent(20) && girl.has_active_trait("Shy"))
        {
            ss << "\n${name} doesn't seem so shy tonight!\n";
            girl.charisma(1);
        }
        if (rng.percent(20) && girl.has_active_trait("Cum Addict"))
        {
            roll = rng % 6;
            switch (roll)
            {
            case 0:
                ss << "\n${name} gets upset when she sees you cum inside another girl. "
                    << "She acts quickly, spreading the girl's pussy and getting her tongue deep inside to lick and suck all the cum out.\n";
                girl.spirit(-1);
                break;
            case 1:
                ss << "\n${name} gets upset when she sees you cum in another girl's mouth. She aggressively kisses the girl, trying to lick the cum right out of her mouth.\n";
                break;
            case 2:
                ss << "\n${name} gets upset when she sees you cum inside another girl's ass. She quickly spreads the girl's ass and tries to suck the cum out.\n";
                girl.spirit(-1);
                girl.dignity(-1);
                break;
            case 3:
                ss << "\n${name} gets upset when she sees you cum on a girl's face. She immediately pounces on the girl, licking the cum off the girl's eyes, cheek and nose.\n";
                break;
            case 4:
                ss << "\n${name} gets upset when she sees you cum on a girl's breasts. She immediately pounces on the girl, licking the cum off her tits.\n";
                break;
            case 5:
                ss << "\n${name} tries to kiss you afterwards, cum dripping from her mouth.\nYou decline.\n";
                break;
            default:
                ss << "\n${name} is a Cum Addict doing something totally wild and unexpected (error).";
            }
            girl.lesbian(2);
            girl.upd_Enjoyment(ACTION_WORKHAREM, 2);
        }
        else if (rng.percent(30) && HateLove > 0 && girl.has_active_trait("Good Kisser"))
        {
            ss << "\n${name} gives you a mindblowing kiss afterwards.\n";
            girl.charisma(1);
            girl.pclove(1);
        }
        if (rng.percent(20) && (girl.has_active_trait("Masochist")))
        {
            ss << "\n\"Next time spank me please";
            if (girl.is_slave()) ss << " master";
            ss << ",\" she whispers.\n";
        }
        else if (rng.percent(5) && (girl.has_active_trait("Sadistic") || girl.has_active_trait("Aggressive")))
        {
            ss << "\n\"If you made me Head Girl, I'd keep these bitches in line for you,";
            if (girl.is_slave()) ss << " master";
            ss << ",\" she tells you.\n";
        }
    }
    else // if no harem - follow existing logic, copy-paste from prev version
    {
        if (HateLove >= 80) //loves you
        {
            if (is_virgin(girl))        // 25% decline
            {
                ss << "She is a virgin so you ask her if she wants to let you be her first.\nShe loves you greatly ";
                if (roll_d <= 25)    { ss << "but says she isn't ready so instead she "; roll_d *= 2; }
                else                { ss << "and agrees so she ";    roll_d = 77; } // normal sex
            }
            else
            {
                ss << "She loves you greatly so she ";
            }
        }
        else if (HateLove >= 60)            //find u attractive
        {
            if (is_virgin(girl))        // 50 % decline
            {
                ss << "She is a virgin so you ask her if she wants to let you be her first.\nShe finds you to be attractive ";
                if (roll_d <= 50)        { ss << "but declines so instead she ";    roll_d *= 2; }
                else                    { ss << "so she agrees and she ";    roll_d = 77; } // normal sex
            }
            else
            {
                ss << "She finds you to be attractive so she ";
            }
        }
        else if (HateLove > 20)            //your okay
        {
            if (is_virgin(girl))        // 70% decline
            {
                ss << "She is a virgin so you ask her if she wants to let you be her first.\nShe finds you to be an okay guy ";
                if (roll_d <= 70)    { ss << "but declines anyway. So she "; roll_d = 100; }
                else                { ss << "so she agrees and she ";    roll_d = 77; } // normal sex
            }
            else
            {
                ss << "She finds you to be okay so she ";
            }
        }
        else if (HateLove >= -20)            //annoying
        {
            if (is_virgin(girl))        // 80% decline
            {
                ss << "She is a virgin so you ask her if she wants to let you be her first.\nShe finds you to be rather annoying ";
                if (roll_d <= 80)    { ss << "so she declines and she "; roll_d = 100; }
                else                { ss << "but agrees none the less. She ";    roll_d = 77; } // normal sex
            }
            else
            {
                ss << "She finds you to be annoying so she ";
            }
        }
        else if (HateLove > -60)            //dont like u at all
        {
            if (is_virgin(girl))        // 95% decline
            {
                ss << "She is a virgin so you ask her if she wants to let you be her first.\nShe doesn't really like you ";
                if (roll_d <= 95)    { ss << "and declines so she "; roll_d = 100; }
                else                { ss << "but for whatever reason she agrees so she ";    roll_d = 77; } // normal sex
            }
            else ss << "She don't like you at all so she ";
            roll_d = girl.libido() > 60 ? 88 : 100;
        }
        else                                            //cant stand the site of u
        {
            if (is_virgin(girl))        // 100% decline
            {
                ss << "She is a virgin so you ask her if she wants to let you be her first.\nShe ";
                ss << "laughs hard at the thought of you touching her.\nShe ";
            }
            else
                ss << "She can't stand the sight of you so she ";
            roll_d = girl.libido() > 60 ? 88 : 100;
        }


        if (roll_d <= 10)
        {
            girl.strip(2);
            ss << "does a little strip show for you.\n \n";
            imagetype = IMGTYPE_STRIP;
            //girl.m_Events.AddMessage(ss.str(), IMGTYPE_STRIP, Day0Night1);
        }
        else if (roll_d <= 20 && brothel->is_sex_type_allowed(SKILL_TITTYSEX))
        {
            girl.tittysex(2);
            ss << "uses her tits on you.\n \n";
            imagetype = IMGTYPE_TITTY;
            //girl.m_Events.AddMessage(ss.str(), IMGTYPE_TITTY, Day0Night1);
        }
        else if (roll_d <= 30 && brothel->is_sex_type_allowed(SKILL_HANDJOB))
        {
            girl.handjob(2);
            ss << "gives you a hand job.\n \n";
            imagetype = IMGTYPE_HAND;
            //girl.m_Events.AddMessage(ss.str(), IMGTYPE_HAND, Day0Night1);
        }
        else if (roll_d <= 40 && brothel->is_sex_type_allowed(SKILL_FOOTJOB))
        {
            girl.footjob(2);
            ss << "gives you a foot job.\n \n";
            imagetype = IMGTYPE_FOOT;
            //girl.m_Events.AddMessage(ss.str(), IMGTYPE_FOOT, Day0Night1);
        }
        else if (roll_d <= 50 && brothel->is_sex_type_allowed(SKILL_ORALSEX))
        {
            girl.oralsex(2);
            ss << "decided to suck your cock.\n \n";
            if (girl.has_active_trait("Herpes")) risky = true;
            imagetype = IMGTYPE_ORAL;
            //girl.m_Events.AddMessage(ss.str(), IMGTYPE_ORAL, Day0Night1);

        }
        else if (roll_d <= 60 && brothel->is_sex_type_allowed(SKILL_ANAL))
        {
            girl.anal(2);
            ss << "lets you use her ass.\n \n";
            risky = true;
            imagetype = IMGTYPE_ANAL;
            //girl.m_Events.AddMessage(ss.str(), IMGTYPE_ANAL, Day0Night1);
        }
        else if (roll_d <= 70 && brothel->is_sex_type_allowed(SKILL_BDSM))
        {
            girl.bdsm(2);
            ss << "lets you tie her up.\n \n";
            risky = true;
            if (girl.lose_trait("Virgin"))
            {
                ss << "She is no longer a virgin.\n";
            }
            imagetype = IMGTYPE_BDSM;
            //girl.m_Events.AddMessage(ss.str(), IMGTYPE_BDSM, Day0Night1);
            girl.calc_pregnancy(&g_Game->player(), 1.0);
        }
        else if (roll_d <= 80 && brothel->is_sex_type_allowed(SKILL_NORMALSEX))
        {
            girl.normalsex(2);
            ss << "has sex with you.\n \n";
            risky = true;
            if (girl.lose_trait("Virgin"))
            {
                ss << "She is no longer a virgin.\n";
            }
            imagetype = IMGTYPE_SEX;
            //girl.m_Events.AddMessage(ss.str(), IMGTYPE_SEX, Day0Night1);
            girl.calc_pregnancy(&g_Game->player(), 1.0);
        }
        else if (roll_d <= 90)
        {
            if (HateLove > 20)
            {
                ss << "let you watch her Masturbate.\n \n";
            }
            else
            {
                ss << "was horny so she just Masturbated.\n \n";
            }
            girl.upd_Enjoyment(ACTION_SEX, +1);
            girl.upd_temp_stat(STAT_LIBIDO, -15, true);
            imagetype = IMGTYPE_MAST;
            //girl.m_Events.AddMessage(ss.str(), IMGTYPE_MAST, Day0Night1);
        }
        else
        {
            if (HateLove > 20)
            {
                ss << "is to tired to fool around so she went to sleep.\n \n";
            }
            else
            {
                ss << "just went to sleep.\n \n";
            }
            //girl.m_Events.AddMessage(ss.str(), IMGTYPE_ECCHI, Day0Night1);
            imagetype = IMGTYPE_ECCHI;
        }
    }

#else //BSIN - this was the prev version - have linked this up to 'no harem' action
    if (HateLove >= 80) //loves you
    {
        if (is_virgin(girl))        // 25% decline
        {
            ss << "She is a virgin so you ask her if she wants to let you be her first.\nShe loves you greatly ";
            if (roll_d <= 25)    { ss << "but says she isn't ready so instead she "; roll_d *= 2; }
            else                { ss << "and agrees so she ";    roll_d = 77; } // normal sex
        }
        else
        {
            ss << "She loves you greatly so she ";
        }
    }
    else if (HateLove >= 60)            //find u attractive
    {
        if (is_virgin(girl))        // 50 % decline
        {
            ss << "She is a virgin so you ask her if she wants to let you be her first.\nShe finds you to be attractive ";
            if (roll_d <= 50)        { ss << "but declines so instead she ";    roll_d *= 2; }
            else                    { ss << "so she agrees and she ";    roll_d = 77; } // normal sex
        }
        else
        {
            ss << "She finds you to be attractive so she ";
        }
    }
    else if (HateLove > 20)            //your okay
    {
        if (is_virgin(girl))        // 70% decline
        {
            ss << "She is a virgin so you ask her if she wants to let you be her first.\nShe finds you to be an okay guy ";
            if (roll_d <= 70)    { ss << "but declines anyway. So she "; roll_d = 100; }
            else                { ss << "so she agrees and she ";    roll_d = 77; } // normal sex
        }
        else
        {
            ss << "She finds you to be okay so she ";
        }
    }
    else if (HateLove >= -20)            //annoying
    {
        if (is_virgin(girl))        // 80% decline
        {
            ss << "She is a virgin so you ask her if she wants to let you be her first.\nShe finds you to be rather annoying ";
            if (roll_d <= 80)    { ss << "so she declines and she "; roll_d = 100; }
            else                { ss << "but agrees none the less. She ";    roll_d = 77; } // normal sex
        }
        else
        {
            ss << "She finds you to be annoying so she ";
        }
    }
    else if (HateLove > -60)            //dont like u at all
    {
        if (is_virgin(girl))        // 95% decline
        {
            ss << "She is a virgin so you ask her if she wants to let you be her first.\nShe doesn't really like you ";
            if (roll_d <= 95)    { ss << "and declines so she "; roll_d = 100; }
            else                { ss << "but for whatever reason she agrees so she ";    roll_d = 77; } // normal sex
        }
        else
            ss << "She don't like you at all so she ";
        roll_d = girl.libido() > 60 ? 88 : 100;
    }
    else                                            //cant stand the site of u
    {
        if (is_virgin(girl))        // 100% decline
        {
            ss << "She is a virgin so you ask her if she wants to let you be her first.\nShe ";
            ss << "laughs hard at the thought of you touching her.\nShe ";
        }
        else
            ss << "She can't stand the sight of you so she ";
        roll_d = girl.libido() > 60 ? 88 : 100;
    }




    if (roll_d <= 10)
    {
        girl.strip(2);
        ss << "does a little strip show for you.\n \n";
        girl.AddMessage(ss.str(), IMGTYPE_STRIP, Day0Night1);
    }
    else if (roll_d <= 20 && is_sex_type_allowed(SKILL_TITTYSEX, brothel))
    {
        girl.tittysex(2);
        ss << "uses her tits on you.\n \n";
        girl.AddMessage(ss.str(), IMGTYPE_TITTY, Day0Night1);
    }
    else if (roll_d <= 30 && is_sex_type_allowed(SKILL_HANDJOB, brothel))
    {
        girl.handjob(2);
        ss << "gives you a hand job.\n \n";
        girl.AddMessage(ss.str(), IMGTYPE_HAND, Day0Night1);
    }
    else if (roll_d <= 40 && is_sex_type_allowed(SKILL_FOOTJOB, brothel))
    {
        girl.footjob(2);
        ss << "gives you a foot job.\n \n";
        girl.AddMessage(ss.str(), IMGTYPE_FOOT, Day0Night1);
    }
    else if (roll_d <= 50 && is_sex_type_allowed(SKILL_ORALSEX, brothel))
    {
        girl.oralsex(2);
        ss << "decided to suck your cock.\n \n";
        girl.AddMessage(ss.str(), IMGTYPE_ORAL, Day0Night1);
    }
    else if (roll_d <= 60 && is_sex_type_allowed(SKILL_ANAL, brothel))
    {
        girl.anal(2);
        ss << "lets you use her ass.\n \n";
        girl.AddMessage(ss.str(), IMGTYPE_ANAL, Day0Night1);
    }
    else if (roll_d <= 70 && is_sex_type_allowed(SKILL_BDSM, brothel))
    {
        girl.bdsm(2);
        ss << "lets you tie her up.\n \n";
        if (is_virgin(girl))
        {
            girl.lose_virginity();    // `J` updated for trait/status
            ss << "She is no longer a virgin.\n";
        }
        girl.AddMessage(ss.str(), IMGTYPE_BDSM, Day0Night1);
        if (!girl.calc_pregnancy(&g_Game->player(), false, 1.0))
        {
            stringstream ssm;
            ssm << girlName << " has gotten pregnant.\n";
            ss << ssm.str();
            g_Game->push_message(ssm.str(), 0);
        }
    }
    else if (roll_d <= 80 && is_sex_type_allowed(SKILL_NORMALSEX, brothel))
    {
        girl.normalsex(2);
        ss << "has sex with you.\n \n";
        if (is_virgin(girl))
        {
            girl.lose_virginity();    // `J` updated for trait/status
            ss << "She is no longer a virgin.\n";
        }
        girl.AddMessage(ss.str(), IMGTYPE_SEX, Day0Night1);
        if (!girl.calc_pregnancy(&g_Game->player(), false, 1.0))
        {
            stringstream ssm;
            ssm << girlName << " has gotten pregnant.\n";
            ss << ssm.str();
            g_Game->push_message(ssm.str(), 0);
        }
    }
    else if (roll_d <= 90)
    {
        if (HateLove > 20)
        {
            ss << "let you watch her Masturbate.\n \n";
        }
        else
        {
            ss << "was horney so she just Masturbated.\n \n";
        }
        girl.upd_Enjoyment(ACTION_SEX, +1);
        girl.upd_temp_stat(STAT_LIBIDO, -15, true);
        girl.AddMessage(ss.str(), IMGTYPE_MAST, Day0Night1);
    }
    else
    {
        if (HateLove > 20)
        {
            ss << "is to tried to fool around so she went to sleep.\n \n";
        }
        else
        {
            ss << "just went to sleep.\n \n";
        }
        girl.AddMessage(ss.str(), IMGTYPE_ECCHI, Day0Night1);
    }
#endif
#if 1
    //BSIN bit more randomness
    if (is_addict(girl, true) && rng.percent(20))
    {
        int theft = rng.in_range(5, 50);
        ss << "\nWhile you're not looking, she steals " << theft << " gold from your room to feed her addiction.\n";
        wages += theft;
    }
    if (rng.percent(30) && girl.has_active_trait("Natural Pheromones"))
    {
        int wowfactor = rng.in_range(5, 55);
        ss << "\nSomething about her drives you wild. You pay her " << wowfactor << " gold extra.\n";
        wages += wowfactor;
    }
    if (rng.percent(30) && (girl.has_active_trait("Exotic") || girl.has_active_trait("Furry")))
    {
        ss << "\n${name} mentions that this reminds her of a 'coming of age' ritual back home.";
        girl.upd_Enjoyment(ACTION_WORKHAREM, 1);
    }
    else if (rng.percent(20) && (girl.has_active_trait("Nymphomaniac") || girl.has_active_trait("Slut") ||
                                 girl.has_active_trait("Open Minded")))
    {
        ss << "\n${name} actually seems suited to this.\n\"Can I do this again";
        if (girl.is_slave()) ss << ", master?\"\n";
        else ss << "?\"\n";
        girl.upd_Enjoyment(ACTION_WORKHAREM, 2);
    }

    //BSIN - disease risk, mostly a cost, but also an expensive way to cure a girl of serious disease
    if (diseased && risky && !girl.m_UseAntiPreg)
    {
        if (girl.lose_trait("Chlamydia", 30))
        {
            g_Game->gold().misc_debit(500);
            const char* ssm = "${name} gave you Chlamydia.\nYou spend 500 gold getting your shit cleaned up.\nYou use the medicine to clear her up too.\n";
            ss << ssm;
            g_Game->push_message(ssm, 0);
        }
        else if (girl.lose_trait("Herpes",30))
        {
            g_Game->gold().misc_debit(1000);
            girl.pclove(1);
            girl.pchate(-1);
            const char* ssm = "${name} gave you herpes.\nMedicine for it costs you 1,000 gold.\nYou use the medicine to clear her up too.\n";
            ss << ssm;
            g_Game->push_message(ssm, 0);
        }
        else if (girl.lose_trait("Syphilis", 30))
        {
            g_Game->gold().misc_debit(3500);
            girl.pclove(3);
            girl.pchate(-3);
            const char* ssm = "${name} gave you syphilis.\nMedicine for it is hard to track down, costing you 3,500 gold.\nYou share it with her.\n";
            ss << ssm;
            g_Game->push_message(ssm, 0);
        }
        else if (girl.lose_trait("AIDS", 30))
        {
            g_Game->gold().misc_debit(8000);
            girl.pclove(6);
            girl.pchate(-6);
            const char* ssm = "${name} gave you AIDS.\n8,000 gold later and the wizards' incantations have finally cleansed you both.\n";
            ss << ssm;
            g_Game->push_message(ssm, 0);
        }
    }

#endif
    if (wages < 0) wages = 0;
    g_Game->gold().girl_support(wages);  // wages come from you

    // Improve stats
    int xp = 10;

    if (girl.has_active_trait("Quick Learner"))        { xp += 3; }
    else if (girl.has_active_trait("Idiot Savant"))    { xp += 5; }
    else if (girl.has_active_trait("Slow Learner"))    { xp -= 3; }
    else if (girl.has_active_trait("Retarded"))        { xp -= 5; }

    girl.exp(xp);
    girl.upd_temp_stat(STAT_LIBIDO, 2);
    girl.AddMessage(ss.str(), imagetype, msgtype);
    return {false, std::max(0, tips), 0, wages};
}


double JP_PersonalBedWarmer(const sGirl& girl, bool estimate)// not used
{
    return 0;
}
