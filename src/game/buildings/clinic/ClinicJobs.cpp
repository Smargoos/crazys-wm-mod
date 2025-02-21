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

#include "jobs/BasicJob.h"
#include "character/sGirl.h"
#include "character/cCustomers.h"
#include "character/predicates.h"
#include "cGirls.h"
#include "buildings/IBuilding.h"
#include "IGame.h"

extern const char* const CarePointsBasicId;
extern const char* const CarePointsGoodId;
extern const char* const DoctorInteractionId;

struct DoctorJob : public cBasicJob {
    DoctorJob();
    sWorkJobResult DoWork(sGirl& girl, bool is_night) override;
    eCheckWorkResult CheckWork(sGirl& girl, bool is_night) override;
};

DoctorJob::DoctorJob() : cBasicJob(JOB_DOCTOR, "Doctor.xml") {
    m_Info.FullTime = true;
    m_Info.FreeOnly = true;
    m_Info.Provides.emplace_back(DoctorInteractionId);
}

sWorkJobResult DoctorJob::DoWork(sGirl& girl, bool is_night) {
    Action_Types actiontype = ACTION_WORKDOCTOR;
    add_text("work");

    cGirls::UnequipCombat(girl);    // put that shit away, you'll scare off the patients!

    int enjoy = 0;
    m_Wages = 100;

    // this will be added to the clinic's code eventually - for now it is just used for her pay
    int patients = 0;            // `J` how many patients the Doctor can see in a shift


    // Doctor is a full time job now
    girl.m_DayJob = girl.m_NightJob = JOB_DOCTOR;

    ProvideInteraction(DoctorInteractionId, 1);

    //enjoyed the work or not
    int roll = d100();
    if (roll <= 10)
    {
        enjoy -= uniform(1, 3);
        m_Performance *= 0.9;
        ss << "Some of the patients abused her during the shift.\n";
    }
    else if (roll >= 90)
    {
        enjoy += uniform(1, 3);
        m_Performance *= 1.1;
        ss << "She had a pleasant time working.\n";
    }
    else
    {
        enjoy += uniform(0, 1);
        ss << "Otherwise, the shift passed uneventfully.\n";
    }

    girl.AddMessage(ss.str(), IMGTYPE_NURSE, is_night ? EVENT_NIGHTSHIFT : EVENT_DAYSHIFT);
    patients += (int)(m_Performance / 10);        // `J` 1 patient per 10 point of performance

    // Improve stats
    girl.upd_Enjoyment(actiontype, enjoy);
    apply_gains(girl, m_Performance);
    return {false, m_Tips, 0, m_Wages};
}

IGenericJob::eCheckWorkResult DoctorJob::CheckWork(sGirl& girl, bool is_night) {
    if (girl.has_active_trait("AIDS"))
    {
        ss << "Health laws prohibit anyone with AIDS from working in the Medical profession so ${name} was sent to get treated.";
        girl.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_WARNING);
        girl.m_DayJob = girl.m_NightJob = JOB_CUREDISEASES;
        return eCheckWorkResult::IMPOSSIBLE;
    }
    if (girl.is_slave())
    {
        ss << "Slaves are not allowed to be Doctors so ${name} was reassigned to being a Nurse.";
        girl.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_WARNING);
        girl.m_DayJob = girl.m_NightJob = JOB_NURSE;
        return eCheckWorkResult::IMPOSSIBLE;
    }
    if (girl.medicine() < 50 || girl.intelligence() < 50)
    {
        ss << "${name} does not have enough training to work as a Doctor. She has been reassigned to Internship so she can learn what she needs.";
        girl.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_WARNING);
        girl.m_DayJob = girl.m_NightJob = JOB_INTERN;
        return eCheckWorkResult::IMPOSSIBLE;
    }
    return eCheckWorkResult::ACCEPTS;
}

struct NurseJob : public cBasicJob {
    NurseJob();
    sWorkJobResult DoWork(sGirl& girl, bool is_night) override;
    eCheckWorkResult CheckWork(sGirl& girl, bool is_night) override;
};

NurseJob::NurseJob() : cBasicJob(JOB_NURSE, "Nurse.xml") {
    m_Info.FullTime = true;
    m_Info.Provides.emplace_back(CarePointsBasicId);
    m_Info.Provides.emplace_back(CarePointsGoodId);
}

sWorkJobResult NurseJob::DoWork(sGirl& girl, bool is_night) {
    auto brothel = girl.m_Building;
#pragma region //    Job setup                //
    Action_Types actiontype = ACTION_WORKNURSE;
    std::stringstream ss;
    int roll_a = d100(), roll_b = d100();
    ss << get_text("work") << "\n \n";
    cGirls::UnequipCombat(girl);    // put that shit away, you'll scare off the patients!

    int enjoy = 0, fame = 0;
    bool hand = false, sex = false, les = false;

    int imagetype = IMGTYPE_NURSE;

#pragma endregion
#pragma region //    Job Performance            //



    // this will be added to the clinic's code eventually - for now it is just used for her pay
    int patients = 0;            // `J` how many patients the Doctor can see in a shift

    //Adding cust here for use in scripts...
    sCustomer Cust = cJobManager::GetMiscCustomer(*brothel);

    int basic_care = 0;
    int quality_care = 0;
    if (m_Performance >= 245)
    {
        basic_care = 20;
        quality_care = 10;
        m_Wages += 155;
        add_text("work.perfect");
    }
    else if (m_Performance >= 185)
    {
        basic_care = 16;
        quality_care = 6;
        m_Wages += 95;
        add_text("work.great");
    }
    else if (m_Performance >= 135)
    {
        basic_care = 12;
        quality_care = 2;
        m_Wages += 55;
        add_text("work.good");
    }
    else if (m_Performance >= 85)
    {
        basic_care = 8;
        m_Wages += 15;
        add_text("work.ok");
    }
    else if (m_Performance >= 65)
    {
        basic_care = 6;
        m_Wages -= 5;
        add_text("work.bad");
    }
    else
    {
        basic_care = 4;
        add_text("work.worst");
        m_Wages -= 15;
    }
    ss << "\n \n";

    ProvideResource(CarePointsBasicId, basic_care);
    ProvideResource(CarePointsGoodId, quality_care);

    //try and add randomness here
    if (girl.beauty() > 85 && chance(20))
    {
        m_Tips += 25;
        ss << "Stunned by her beauty a customer left her a great tip.\n";
    }

    if (girl.has_active_trait("Clumsy") && chance(20))
    {
        m_Wages -= 15;
        ss << "Her clumsy nature caused her to spill some medicine everywhere.\n";
    }

    if (girl.has_active_trait("Pessimist") && chance(5))
    {
        if (m_Performance < 125)
        {
            m_Wages -= 10;
            ss << "Her pessimistic mood depressed the patients making them tip less.\n";
        }
        else
        {
            m_Tips += 10;
            ss << "${name} was in a poor mood so the patients gave her a bigger tip to try and cheer her up.\n";
        }
    }

    if (girl.has_active_trait("Optimist") && chance(20))
    {
        if (m_Performance < 125)
        {
            m_Wages -= 10;
            ss << "${name} was in a cheerful mood but the patients thought she needed to work more on her services.\n";
        }
        else
        {
            m_Tips += 10;
            ss << "Her optimistic mood made patients cheer up increasing the amount they tip.\n";
        }
    }

    if (girl.intelligence() < 45 && chance(30))//didnt put a check on this one as we could use some randomness and its an intel check... guess we can if people keep bitching
    {
        hand = true;
        ss << "An elderly fellow managed to convince ${name} that her touch can heal! She ended up giving him a hand job!\n";
    }

    if (chance(30) && !is_virgin(girl) && !girl.has_active_trait("Lesbian")
        && (girl.has_active_trait("Nymphomaniac") || girl.has_active_trait("Slut") || girl.has_active_trait("Succubus") ||
            girl.has_active_trait("Bimbo")))
    {
        if (girl.libido() > 65 && (brothel->is_sex_type_allowed(SKILL_NORMALSEX) || brothel->is_sex_type_allowed(SKILL_ANAL)))
        {
            m_Tips += 50;
            sex = true;
            enjoy += 1;
            ss << "When giving a sponge bath to one of her male patients she couldn't look away from his enormous manhood. The man took advantage and fucked her brains out!\n";
        }
        else
        {
            ss << "When giving a sponge bath to one of her male patients she couldn't look away from his enormous manhood. But she wasn't in the mood so she left.\n";
        }
    }

    if (girl.has_active_trait("Lesbian") && girl.has_active_trait("Aggressive") &&
        girl.libido() > 65 && chance(10))
    {
        les = true;
        enjoy += 1;
        ss << "When giving a sponge bath to one of her female patients she couldn't help herself and took advantage of the situation.\n";
    }
    ss << "\n";

#pragma endregion
#pragma region    //    Enjoyment and Tiredness        //
    //enjoyed the work or not
    if (roll_a <= 5)
    {
        enjoy -= uniform(1, 3);
        m_Performance *= 0.9;
        ss << "Some of the patrons abused her during the shift.";
    }
    else if (roll_a <= 25)
    {
        enjoy += uniform(1, 3);
        m_Performance *= 1.1;
        ss << "She had a pleasant time working.";
    }
    else
    {
        enjoy += uniform(0, 1);
        ss << "Otherwise, the shift passed uneventfully.";
    }

    if (sex)
    {
        if (brothel->is_sex_type_allowed(SKILL_NORMALSEX) && (roll_b <= 50 || brothel->is_sex_type_allowed(SKILL_ANAL))) //Tweak to avoid an issue when roll > 50 && anal is restricted
        {
            imagetype = IMGTYPE_SEX;
            girl.normalsex(2);
            if (girl.lose_trait("Virgin"))
            {
                ss << "She is no longer a virgin.\n";
            }
            if (!girl.calc_pregnancy(Cust, 1.0))
            {
                g_Game->push_message(girl.FullName() + " has gotten pregnant", 0);
            }
        }
        else if (brothel->is_sex_type_allowed(SKILL_ANAL))
        {
            imagetype = IMGTYPE_ANAL;
            girl.anal(2);
        }
        brothel->m_Happiness += 100;
        girl.upd_temp_stat(STAT_LIBIDO, -20, true);
        girl.upd_Enjoyment(ACTION_SEX, +3);
    }
    else if (hand)
    {
        brothel->m_Happiness += uniform(60, 130);
        girl.handjob(2);
        imagetype = IMGTYPE_HAND;
    }
    else if (les)
    {
        brothel->m_Happiness += uniform(30, 100);
        imagetype = IMGTYPE_LESBIAN;
        girl.lesbian(2);
    }

    if (girl.is_unpaid())
    {
        m_Performance *= 0.9;
        patients += (int)(m_Performance / 5);        // `J` 1 patient per 5 point of performance
        m_Wages = 0;
    }
    else
    {
        patients += (int)(m_Performance / 5);        // `J` 1 patient per 5 point of performance
        m_Wages += patients * 2;                // `J` pay her 2 for each patient you send to her
    }

#pragma endregion
#pragma region    //    Finish the shift            //


    girl.AddMessage(ss.str(), imagetype, is_night ? EVENT_NIGHTSHIFT : EVENT_DAYSHIFT);
    /* `J` this will be a place holder until a better payment system gets done
    *  this does not take into account any of your girls in surgery
    */
    int earned = 0;
    for (int i = 0; i < patients; i++)
    {
        earned += uniform(5, 40); // 5-40 gold per customer
    }
    brothel->m_Finance.clinic_income(earned);
    ss.str("");    ss << "${name} earned " << earned << " gold from taking care of " << patients << " patients.";
    girl.AddMessage(ss.str(), IMGTYPE_PROFILE, is_night ? EVENT_NIGHTSHIFT : EVENT_DAYSHIFT);


    // Improve stats
    if (girl.fame() < 10 && m_Performance >= 70)        { fame += 1; }
    if (girl.fame() < 20 && m_Performance >= 100)        { fame += 1; }
    if (girl.fame() < 40 && m_Performance >= 145)        { fame += 1; }
    if (girl.fame() < 50 && m_Performance >= 185)        { fame += 1; }
    girl.fame(fame);
    apply_gains(girl, m_Performance);
    girl.upd_Enjoyment(actiontype, enjoy);

#pragma endregion
    return {false, m_Tips, 0, m_Wages};
}

IGenericJob::eCheckWorkResult NurseJob::CheckWork(sGirl& girl, bool is_night) {
    if (girl.has_active_trait("AIDS"))
    {
        ss << "Health laws prohibit anyone with AIDS from working in the Medical profession so ${name} was sent to the waiting room.";
        girl.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_WARNING);
        girl.FullJobReset(JOB_RESTING);
        return IGenericJob::eCheckWorkResult::IMPOSSIBLE;
    }

    if (girl.disobey_check(ACTION_WORKNURSE, JOB_NURSE))            // they refuse to work
    {
        ss << "${name} refused to see any patients during the " << (is_night ? "night" : "day") << " shift.";
        if (girl.tiredness() > 50 && chance(girl.tiredness() - 30))
        {
            ss << "\nShe was found sleeping " << rng().select_text({"in a supply closet.", "in an empty patient bed."});
            girl.tiredness(-uniform(0, 40));
        }
        girl.AddMessage(ss.str(), IMGTYPE_REFUSE, EVENT_NOWORK);
        return IGenericJob::eCheckWorkResult::REFUSES;
    }
    return IGenericJob::eCheckWorkResult::ACCEPTS;
}

struct InternJob : public cBasicJob {
    InternJob();
    sWorkJobResult DoWork(sGirl& girl, bool is_night) override;
    double GetPerformance(const sGirl& girl, bool estimate) const override;
    eCheckWorkResult CheckWork(sGirl& girl, bool is_night) override;
};

InternJob::InternJob() : cBasicJob(JOB_INTERN, "Intern.xml") {
    m_Info.Provides.emplace_back(CarePointsBasicId);
}

double InternJob::GetPerformance(const sGirl& girl, bool estimate) const {
    if (estimate)// for third detail string
    {
        double jobperformance =
                (100 - girl.medicine()) +
                (100 - girl.intelligence()) +
                (100 - girl.charisma());

        // traits she could gain/lose
        if (girl.has_active_trait("Nervous")) jobperformance += 20;
        if (!girl.has_active_trait("Charismatic")) jobperformance += 20;
        return jobperformance;
    }
    return 0.0;
}

sWorkJobResult InternJob::DoWork(sGirl& girl, bool is_night) {
    auto brothel = girl.m_Building;

    Action_Types actiontype = ACTION_WORKTRAINING;
    ss << get_text("work") << "\n \n";

    cGirls::UnequipCombat(girl);    // put that shit away

    // less than even a bad nurse, but still something
    ProvideResource(CarePointsBasicId, 2);

    int enjoy = 0;                                                //
    int train = 0;                                                // main skill trained
    int tmed = girl.medicine();                                // Starting level - train = 1
    int tint = girl.intelligence();                            // Starting level - train = 2
    int tcha = girl.charisma();                                // Starting level - train = 3
    bool gaintrait = false;                                        // posibility of gaining a trait
    bool promote = false;                                        // posibility of getting promoted to Doctor or Nurse
    int skill = 0;                                                // gian for main skill trained
    int dirtyloss = brothel->m_Filthiness / 100;                // training time wasted with bad equipment
    int sgMed = 0, sgInt = 0, sgCha = 0;                        // gains per skill
    int roll_a = d100();                                    // roll for main skill gain
    int roll_b = d100();                                    // roll for main skill trained
    int roll_c = d100();                                    // roll for enjoyment


    /* */if (roll_a <= 5)    skill = 7;
    else if (roll_a <= 15)    skill = 6;
    else if (roll_a <= 30)    skill = 5;
    else if (roll_a <= 60)    skill = 4;
    else /*             */    skill = 3;
    /* */if (girl.has_active_trait("Quick Learner"))    { skill += 1; }
    else if (girl.has_active_trait("Slow Learner"))    { skill -= 1; }
    skill -= dirtyloss;
    ss << "The Clinic is ";
    if (dirtyloss <= 0) ss << "clean and tidy";
    if (dirtyloss == 1) ss << "dirty and the equipment has not been put back in its place";
    if (dirtyloss == 2) ss << "messy. The equipment is damaged and strewn about the building";
    if (dirtyloss == 3) ss << "filthy and some of the equipment is broken";
    if (dirtyloss >= 4) ss << "in complete disarray and the equipment barely usable";
    ss << ".\n \n";
    if (skill < 1) skill = 1;    // always at least 1

    do{        // `J` New method of selecting what job to do
        /* */if (roll_b < 40  && tmed < 100)    train = 1;    // medicine
        else if (roll_b < 70  && tint < 100)    train = 2;    // intelligence
        else if (roll_b < 100 && tcha < 100)    train = 3;    // charisma
        roll_b -= 10;
    } while (train == 0 && roll_b > 0);
    if (train == 0 || chance(5)) gaintrait = true;
    if (train == 0 && girl.medicine() > 70 && girl.intelligence() > 70)    promote = true;

    if (train == 1) { sgMed = skill; ss << "She learns how to work with medicine better.\n"; }    else sgMed = uniform(0, 2);
    if (train == 2) { sgInt = skill; ss << "She got smarter today.\n"; }                        else sgInt = uniform(0, 1);
    if (train == 3) { sgCha = skill; ss << "She got more charismatic today.\n"; }                else sgCha = uniform(0, 1);

    if (sgMed + sgInt + sgCha > 0)
    {
        ss << "She managed to gain:\n";
        if (sgMed > 0) { ss << sgMed << " Medicine.\n";        girl.medicine(sgMed); }
        if (sgInt > 0) { ss << sgInt << " Intelligence.\n";    girl.intelligence(sgInt); }
        if (sgCha > 0) { ss << sgCha << " Charisma.\n";        girl.charisma(sgCha); }
    }

    int trycount = 10;
    while (gaintrait && trycount > 0)    // `J` Try to add a trait
    {
        trycount--;
        switch (uniform(0, 10))
        {
            case 0:
                if (girl.lose_trait( "Nervous"))
                {
                    ss << "She seems to be getting over her Nervousness with her training.";
                    gaintrait = false;
                }
                break;
            case 1:
                if (girl.lose_trait( "Meek"))
                {
                    ss << "She seems to be getting over her Meekness with her training.";
                    gaintrait = false;
                }
                break;
            case 2:
                if (girl.lose_trait( "Dependant"))
                {
                    ss << "She seems to be getting over her Dependancy with her training.";
                    gaintrait = false;
                }
                break;
            case 3:
                if (girl.gain_trait( "Charismatic"))
                {
                    ss << "Dealing with patients and talking with them about their problems has made ${name} more Charismatic.";
                    gaintrait = false;
                }
                break;
            case 4:
                break;
            case 5:

                break;
            case 6:

                break;

            default:    break;    // no trait gained
        }
    }



    //enjoyed the work or not
    /* */if (roll_c <= 10)    { enjoy -= uniform(1, 3);    ss << "Some of the patrons abused her during the shift."; }
    else if (roll_c >= 90)    { enjoy += uniform(1, 3);    ss << "She had a pleasant time working."; }
    else /*             */    { enjoy += uniform(0, 1);        ss << "Otherwise, the shift passed uneventfully."; }
    girl.upd_Enjoyment(actiontype, enjoy);

    girl.AddMessage(ss.str(), IMGTYPE_PROFILE, is_night ? EVENT_NIGHTSHIFT : EVENT_DAYSHIFT);

    if (girl.is_unpaid()) { m_Wages = 0; }
    else { m_Wages = 25 + (skill * 5); } // `J` Pay her more if she learns more

    // Improve stats
    int xp = 5 + skill;

    if (girl.has_active_trait("Quick Learner"))        { xp += 2; }
    else if (girl.has_active_trait("Slow Learner"))    { xp -= 2; }

    girl.exp(uniform(1, xp));
    girl.upd_temp_stat(STAT_LIBIDO, skill / 2);

    if (girl.medicine() + girl.intelligence() + girl.charisma() >= 300) promote = true;
    if (promote)
    {
        ss.str("");
        ss << "${name} has completed her Internship and has been promoted to ";
        if (girl.is_slave())    { ss << "Nurse.";    girl.m_DayJob = girl.m_NightJob = JOB_NURSE; }
        else /*            */    { ss << "Doctor.";    girl.m_DayJob = girl.m_NightJob = JOB_DOCTOR; }
        girl.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_GOODNEWS);
    }

    return {false, 0, 0, m_Wages};
}

IGenericJob::eCheckWorkResult InternJob::CheckWork(sGirl& girl, bool is_night) {
    if (girl.has_active_trait("AIDS"))
    {
        ss << "Health laws prohibit anyone with AIDS from working in the Medical profession so ${name} was sent to the waiting room.";
        girl.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_WARNING);
        girl.FullJobReset(JOB_RESTING);
        return IGenericJob::eCheckWorkResult::IMPOSSIBLE;
    }
    if (girl.medicine() + girl.intelligence() + girl.charisma() >= 300)
    {
        ss << "There is nothing more she can learn here so she is promoted to ";
        if (girl.is_slave())    { ss << "Nurse.";    girl.m_DayJob = girl.m_NightJob = JOB_NURSE; }
        else /*            */    { ss << "Doctor.";    girl.m_DayJob = girl.m_NightJob = JOB_DOCTOR; }
        girl.AddMessage(ss.str(), IMGTYPE_PROFILE, EVENT_GOODNEWS);
        return IGenericJob::eCheckWorkResult::IMPOSSIBLE;
    }

    return SimpleRefusalCheck(girl, ACTION_WORKTRAINING);
}

void RegisterClinicJobs(cJobManager& mgr) {
    mgr.register_job(std::make_unique<DoctorJob>());
    mgr.register_job(std::make_unique<NurseJob>());
    mgr.register_job(std::make_unique<InternJob>());
}