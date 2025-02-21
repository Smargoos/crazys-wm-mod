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

#include "BasicJob.h"
#include "character/sGirl.h"
#include "buildings/IBuilding.h"
#include "cGirls.h"


namespace {
    struct Cleaning : public cBasicJob {
        using cBasicJob::cBasicJob;

        sWorkJobResult DoWork(sGirl& girl, bool is_night) override;
        eCheckWorkResult CheckWork(sGirl& girl, bool is_night) override;
        void CleaningUpdateGirl(sGirl& girl, bool is_night, int enjoy, int clean_amount);

        virtual void DoneEarly(sGirl& girl) = 0;

        Image_Types ImageType = IMGTYPE_MAID;
    };

    struct CleanArena : public Cleaning {
        CleanArena();
        void DoneEarly(sGirl& girl) override;
    };
    struct CleanCentre : public Cleaning {
        CleanCentre();
        void DoneEarly(sGirl& girl) override;
    };
    struct CleanHouse : public Cleaning {
        CleanHouse();
        void DoneEarly(sGirl& girl) override;
    };
    struct CleanBrothel : public Cleaning {
        CleanBrothel();
        void DoneEarly(sGirl& girl) override;
    };
    struct CleanClinic : public Cleaning {
        CleanClinic();
        void DoneEarly(sGirl& girl) override;
    };
    struct CleanFarm : public Cleaning {
        CleanFarm();
        void DoneEarly(sGirl& girl) override;
    };
}

void Cleaning::CleaningUpdateGirl(sGirl& girl, bool is_night, int enjoy, int clean_amount) {
    girl.m_Building->m_Filthiness -= (int)clean_amount;

    // Base Improvement and trait modifiers
    int xp = 5, skill = 3;
    /* */if (girl.has_active_trait("Quick Learner"))    { skill += 1; xp += 3; }
    else if (girl.has_active_trait("Slow Learner"))    { skill -= 1; xp -= 3; }
    // EXP and Libido
    girl.exp(uniform(1, xp));


    // primary improvement (+2 for single or +1 for multiple)
    girl.service(uniform(0, skill+1));

    // secondary improvement (-1 for one then -2 for others)
    girl.morality(std::max(-1, uniform(-1, skill)));        // possibly go down but mostly go up
    girl.refinement(std::max(-1, uniform(-2, skill)));    // possibly go up or down


    // Update Enjoyment
    girl.upd_Enjoyment(ACTION_WORKCLEANING, enjoy);
    // Gain Traits
    if (chance(girl.service()))
        cGirls::PossiblyGainNewTrait(girl, "Maid", 70, ACTION_WORKCLEANING, "${name} has cleaned enough that she could work professionally as a Maid anywhere.", is_night);
    // Lose Traits
    if (chance(girl.service()))
        cGirls::PossiblyLoseExistingTrait(girl, "Clumsy", 30, ACTION_WORKCLEANING, "It took her spilling hundreds of buckets, and just as many reprimands, but ${name} has finally stopped being so Clumsy.", is_night);
}

IGenericJob::eCheckWorkResult Cleaning::CheckWork(sGirl& girl, bool is_night) {
    if (girl.disobey_check(ACTION_WORKCLEANING, job()))
    {
        add_text("refuse");
        girl.AddMessage(ss.str(), IMGTYPE_REFUSE, EVENT_NOWORK);
        return eCheckWorkResult::REFUSES;
    }

    return eCheckWorkResult::ACCEPTS;
}

sWorkJobResult Cleaning::DoWork(sGirl& girl, bool is_night) {
    ImageType = IMGTYPE_MAID;
    auto brothel = girl.m_Building;
    add_text("work") << "\n \n";

    cGirls::UnequipCombat(girl);    // put that shit away

    double CleanAmt = girl.job_performance(JOB_CLEANARENA, false);
    int enjoy = 0;
    bool playtime = false;

    int roll_a = d100();
    if (roll_a <= 10)
    {
        enjoy -= uniform(1, 3);
        CleanAmt *= 0.8;
        add_text("shift.bad");

    }
    else if (roll_a >= 90)
    {
        enjoy += uniform(1, 3);
        CleanAmt *= 1.1;
        add_text("shift.good");
    }
    else
    {
        enjoy += uniform(0, 1);
        add_text("shift.neutral");
    }
    ss << "\n \n";

    // slave girls not being paid for a job that normally you would pay directly for do less work
    if (girl.is_unpaid())
    {
        CleanAmt *= 0.9;
        m_Wages = 0;
    }
    else
    {
        m_Wages = std::min(30, int(30 + (CleanAmt / 10))); // `J` Pay her based on how much she cleaned
    }

    // `J` if she can clean more than is needed, she has a little free time after her shift
    if (brothel->m_Filthiness < CleanAmt / 2) playtime = true;
    ss << "\n \nCleanliness rating improved by " << (int)CleanAmt;
    if (playtime)    // `J` needs more variation
    {
        DoneEarly(girl);
    }

    // do all the output
    girl.AddMessage(ss.str(), ImageType, is_night ? EVENT_NIGHTSHIFT : EVENT_DAYSHIFT);

    CleaningUpdateGirl(girl, is_night, enjoy, CleanAmt);
    return {false, m_Tips, 0, m_Wages};
}

CleanArena::CleanArena() : Cleaning(JOB_CLEANARENA, "CleanArena.xml") {

}

void CleanArena::DoneEarly(sGirl& girl) {
    ss << "\n \n${name} finished her cleaning early so she played around with some of the equipment.";
    girl.combat(uniform(0, 2));
    girl.agility(uniform(0, 1));
    girl.constitution(uniform(0, 1));
    girl.confidence(uniform(0, 1));
    girl.upd_Enjoyment(ACTION_COMBAT, 1);
}

CleanCentre::CleanCentre() : Cleaning(JOB_CLEANCENTRE, "CleanCentre.xml") {

}

void CleanCentre::DoneEarly(sGirl& girl) {
    if(chance(girl.tiredness())) {
        ss << "\n \n${name} finished her cleaning early so she took nap.";
        girl.happiness(uniform(0, 2));
        girl.tiredness(-uniform(1, 10));
        girl.morality(-uniform(0, 1));
    } else if (chance(50))
    {
        ss << "\n \n${name} finished her cleaning early so took a long bath to clean herself off.";
        girl.happiness(uniform(1, 3));
        girl.tiredness(-uniform(0, 2));
        ImageType = IMGTYPE_BATH;
    }
    else
    {
        ss << "\n \n${name} finished her cleaning early so she played with the children a bit.";
        girl.happiness(uniform(3, 7));
        girl.tiredness(uniform(0, 2));
        girl.morality(uniform(0, 1));
    }
}


CleanHouse::CleanHouse() : Cleaning(JOB_CLEANHOUSE, "CleanHouse.xml") {

}

void CleanHouse::DoneEarly(sGirl& girl) {
    ss << "\n \n${name} finished her cleaning early so she just sat around the house.";
    girl.tiredness(-uniform(2, 6));
    girl.happiness(uniform(2, 6));
}

CleanFarm::CleanFarm() : Cleaning(JOB_FARMHAND, "FarmHand.xml") {

}

void CleanFarm::DoneEarly(sGirl& girl) {
    ss << "\n \n${name} finished her cleaning early so she ";
    int roll_c = d100();
    if (!is_night_shift() && chance(33))    // 33% chance she will watch the sunset when working day shift
    {
        ss << "sat beneath an oak tree and watched the sunset.";
        girl.tiredness(-uniform(2, 6));
    }
    else if (roll_c < 25)
    {
        ss << "played with the baby animals a bit.";
        girl.animalhandling(uniform(1, 3));
    }
    else if (roll_c < 50)
    {
        ss << "played in the dirt a bit.";
        girl.farming(uniform(0, 1));
    }
    else
    {
        ss << "sat in a rocking chair on the farm house front porch whittling.";
        girl.crafting(uniform(0, 2));
        girl.tiredness(-uniform(0, 2));
    }
    girl.happiness(uniform(1, 5));
}

CleanClinic::CleanClinic() : Cleaning(JOB_JANITOR, "Janitor.xml") {

}

void CleanClinic::DoneEarly(sGirl& girl) {
    auto brothel = girl.m_Building;
    ss << "\n \n${name} finished her cleaning early so she ";
    if (girl.is_pregnant() && girl.health() < 90)
    {
        ss << "got a quick checkup and made sure her unborn baby was doing OK.";
        girl.health(10);
        girl.happiness(uniform(2, 5));
    }
    else if (girl.health() < 80)
    {
        ss << "got a quick checkup.";
        girl.health(10);
    }
    else if (girl.is_pregnant() || chance(40))
    {
        ss << "hung out in the maternity ward watching the babies.";
        girl.happiness(uniform(-2, 3));
    }
    else if (girl.tiredness() > 50 && brothel->free_rooms() > 10)
    {
        ss << "found an empty room and took a nap.";
        girl.tiredness(-uniform(5, 15));
    }
    else
    {
        int d = brothel->num_girls_on_job(JOB_DOCTOR, is_night_shift());
        int n = brothel->num_girls_on_job(JOB_NURSE, is_night_shift());
        ss << "watched the ";
        if (d + n < 1)
        {
            ss << "people wander about the Clinic.";
            girl.happiness(uniform(-1, 1));
        }
        else
        {
            if (d > 0)            ss << "doctor" << (d > 1 ? "s" : "");
            if (d > 0 && n > 0)    ss << " and ";
            if (n > 0)            ss << "nurse" << (n > 1 ? "s" : "");
            ss << " do their job" << (d + n > 1 ? "s" : "");
            girl.medicine(uniform(0, 1));
        }
    }
}



CleanBrothel::CleanBrothel() : Cleaning(JOB_CLEANING, "CleanBrothel.xml") {

}

void CleanBrothel::DoneEarly(sGirl& girl) {
    auto brothel = girl.m_Building;
    ss << "\n \n${name} finished her cleaning early so ";
    int choice = uniform(0, 5);
    if (choice == 1 && (!brothel->is_sex_type_allowed(SKILL_ORALSEX) || girl.has_active_trait("Lesbian"))) choice = 0;
    if (choice != 2 && girl.tiredness() >= 80) choice = 2;
    if (choice != 1 && chance(30) && girl.has_active_trait("Cum Addict")) choice = 1;

    switch (choice)
    {
        case 1:
        {
            ss << "she hung out at the brothel, offering to \"clean off\" finished customers with her mouth.\n";//Made it actually use quote marks CRAZY
            int tips = uniform(-1, 4); //how many 'tips' she clean? <6 for now, considered adjusting to amount playtime - didn't seem worth complexity
            if (tips > 0)
            {
                brothel->m_Happiness += (tips);
                girl.oralsex(tips / 2);
                // TODO Tips
                //tips *= 5; //customers tip 5 gold each
                //ss << "She got " << tips << " in tips for this extra service.\n";
                ImageType = IMGTYPE_ORAL;
                girl.m_NumCusts += tips;
            }
            else
            {
                ss << "No one was interested.";
            }
        }break;

        case 2:
        {
            ss << "she had a rest.";
            girl.tiredness(-uniform(1, 10));
        }break;

        case 3:
        {
            ss << "she hung out around the brothel chatting with staff and patrons.\n";
            girl.charisma(uniform(1, 3));
            girl.confidence(uniform(1, 2));
        }break;

        case 4:
        {
            ss << "she spent some time training and getting herself fitter.\n";
            girl.constitution(uniform(0, 1));
            girl.agility(uniform(0, 1));
            girl.beauty(uniform(0, 1));
            girl.spirit(uniform(0, 1));
            girl.combat(uniform(0, 1));
        }break;

        case 5:
        {
            if (girl.has_active_trait("Your Wife") || chance(30) && !girl.has_active_trait("Your Daughter") && !girl.has_active_trait(
                    "Lesbian")) //Flipped to fix the daughter issue

            {
                // TODO adjust tips
                //tips = 20; // you tip her for cleaning you
                ss << "she came to your room and cleaned you.\n \n${name} ran you a hot bath and bathed naked with you.";/* Need a check here so your daughters won't do this zzzzz FIXME*/
                ImageType = IMGTYPE_BATH;

                if (brothel->is_sex_type_allowed(SKILL_TITTYSEX))
                {
                    ss << " Taking care to clean your whole body carefully, She rubbed cleansing oils over your back and chest with her ";
                    if (girl.breast_size() >= 8)
                        ss << "enormous, heaving ";
                    else if (girl.breast_size() >= 5)
                        ss << "big, round ";
                    else if (girl.breast_size() <= 3)
                        ss << "cute little ";
                    else ss << "nice, firm ";
                    ss << "breasts. ";
                }
                if (brothel->is_sex_type_allowed(SKILL_ORALSEX))
                {
                    ss << "She finished by cleaning your cock with her " << (girl.has_active_trait("Dick-Sucking Lips") ? "amazing dick-sucking lips" : "mouth") << ". ";
                    if (girl.has_active_trait("Cum Addict"))
                    {
                        ss << "She didn't stop 'cleaning' until you came in her mouth.\nAfterward, you notice her carefully "
                           << "crawling around and licking up every stray drop of cum. She must really love cleaning.\n";
                        girl.oralsex(uniform(0, 1));
                        girl.spirit(-uniform(0, 1));
                        // tips += (rng % 20);  // tip her for hotness
                    }
                    ImageType = IMGTYPE_ORAL;
                }
                girl.service(uniform(0, 4));
                girl.medicine(uniform(0, 1));
                girl.obedience(uniform(0, 3));
                girl.pclove(uniform(0, 4));
            }
            else
            {
                ss << "she hung out around the brothel, watching the other girls and trying to learn tricks and techniques.\n";
                girl.normalsex(uniform(0, 1));
                girl.anal(uniform(0, 1));
                girl.oralsex(uniform(0, 1));
                girl.bdsm(uniform(0, 1));
                girl.lesbian(uniform(0, 1));
            }
        }break;

        default:
            ss << "she hung out around the brothel a bit.";
            girl.upd_temp_stat(STAT_LIBIDO, uniform(1, 3), true);
            girl.happiness(uniform(1, 3));
            break;
    }
}

void RegisterCleaningJobs(cJobManager& mgr) {
    mgr.register_job(std::make_unique<CleanArena>());
    mgr.register_job(std::make_unique<CleanCentre>());
    mgr.register_job(std::make_unique<CleanHouse>());
    mgr.register_job(std::make_unique<CleanBrothel>());
    mgr.register_job(std::make_unique<CleanClinic>());
    mgr.register_job(std::make_unique<CleanFarm>());
}