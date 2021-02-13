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

#include "GenericJob.h"
#include "character/sGirl.h"
#include "cGirls.h"
#include "buildings/IBuilding.h"

struct sTrainingData {
    const char* TargetTrait;
    const char* HasAlreadyMessage;
    const char* ProceedMessage;
    const char* ResistMessage;
    const char* ProgressMessage;
    Image_Types ImageType;
};

class TrainingJob : public IGenericJob {
public:
    TrainingJob(JOBS job, const char* brief, sTrainingData dat) : IGenericJob(job), m_Data(dat) {
        m_Info.FullTime = true;
        m_Info.ShortName = brief;
    }

    sWorkJobResult DoWork(sGirl& girl, bool is_night) override;
    double GetPerformance(const sGirl& girl, bool estimate) const override;
    eCheckWorkResult CheckWork(sGirl& girl, bool is_night) override;

    virtual void HandleTraining(sGirl& girl, bool is_night) = 0;
    virtual void OnComplete(sGirl& girl) = 0;
    virtual void OnNoProgress(sGirl& girl);
    virtual void OnRegularProgress(sGirl& girl, bool is_night);

protected:
    void update_progress(sGirl& girl);
    void CountTheDays(sGirl& girl, bool is_night);

    EventType TypeOfEvent;
    int Tiredness;
    int Enjoyment;
    int Wages;

    sTrainingData m_Data;

private:
    int m_StartProgress = 0;
};

void TrainingJob::update_progress(sGirl& girl) {
    girl.m_WorkingDay += uniform(10, 20);
    girl.m_WorkingDay += girl.obedience() / 20;
    if (girl.pcfear() > 50)                girl.m_WorkingDay +=uniform(0, girl.pcfear() / 20);        // She will do as she is told
    if (girl.pclove() > 50)                girl.m_WorkingDay += uniform(0, girl.pclove() / 20);        // She will do what you ask

    // Negative Stats/Skills
    girl.m_WorkingDay -= girl.spirit() / 25;
    if (girl.pchate() > 30)                girl.m_WorkingDay -= uniform(0, girl.pchate() / 10);        // She will not do what you want
    if (girl.happiness() < 50)             girl.m_WorkingDay -= uniform(1, 5);                        // She is not feeling like it
    if (girl.health() < 50)                girl.m_WorkingDay -= uniform(1, 5);                        // She is feeling sick
    if (girl.tiredness() > 50)             girl.m_WorkingDay -= uniform(1, 5);                        // She is tired
}

sWorkJobResult TrainingJob::DoWork(sGirl& girl, bool is_night) {
    // TODO pull this up into generic job?
    if (girl.m_YesterDayJob != job()) girl.m_PrevWorkingDay = girl.m_WorkingDay = 0;
    if (girl.m_WorkingDay < 0) girl.m_WorkingDay = 0;

    // Base adjustment
    Wages = 0;
    Tiredness = uniform(5, 15);
    m_StartProgress = girl.m_WorkingDay;

    cGirls::UnequipCombat(girl);

    ss << m_Data.ProceedMessage << "\n";

    TypeOfEvent = is_night ? EVENT_NIGHTSHIFT : EVENT_DAYSHIFT;

    HandleTraining(girl, is_night);

    girl.upd_Enjoyment(ACTION_WORKTRAINING, Enjoyment);
    girl.tiredness(Tiredness);

    girl.AddMessage(ss.str(), m_Data.ImageType, TypeOfEvent);

    return {false, 0, 0, Wages};
}

double TrainingJob::GetPerformance(const sGirl& girl, bool estimate) const {
    if (girl.has_active_trait(m_Data.TargetTrait))    return -1000;
    return 250;
}

IGenericJob::eCheckWorkResult TrainingJob::CheckWork(sGirl& girl, bool is_night) {
    if (girl.has_active_trait(m_Data.TargetTrait))
    {
        girl.AddMessage(m_Data.HasAlreadyMessage, IMGTYPE_PROFILE, EVENT_WARNING);
        girl.FullJobReset(JOB_RESTING);
        girl.m_PrevWorkingDay = girl.m_WorkingDay = 0;
        return eCheckWorkResult::IMPOSSIBLE;    // not refusing
    }

    return eCheckWorkResult::ACCEPTS;
}

void TrainingJob::CountTheDays(sGirl& girl, bool is_night)
{
    if (girl.disobey_check(ACTION_WORKTRAINING, job())) girl.m_WorkingDay /= 2;    // if she disobeys, half her time is wasted

    int total = girl.m_WorkingDay - m_StartProgress;
    if (total <= 0)                                // she lost time so more tired
    {
        Tiredness += uniform(5, 5-total);
        Enjoyment -= uniform(0, 2);
    }
    else if (total > 40)                        // or if she trained a lot
    {
        Tiredness += uniform(total / 4, total / 2);
        Enjoyment += uniform(0, 2);
    }
    else                                        // otherwise just a bit tired
    {
        Tiredness += uniform(0, total / 3);
        Enjoyment -= uniform(-2, 2);
    }

    if (girl.m_WorkingDay <= 0)
    {
        girl.m_WorkingDay = 0;
        TypeOfEvent = EVENT_WARNING;
        OnNoProgress(girl);
    }
    else if (girl.m_WorkingDay >= 100 && is_night)
    {
        girl.m_PrevWorkingDay = girl.m_WorkingDay = 0;
        TypeOfEvent = EVENT_GOODNEWS;
        OnComplete(girl);
        girl.FullJobReset(JOB_RESTING);
        Wages = 200;
    }
    else
    {
        OnRegularProgress(girl, is_night);
        Wages = std::min(100, girl.m_WorkingDay);
    }

    if (girl.is_slave()) Wages /= 2;
}

void TrainingJob::OnNoProgress(sGirl& girl) {
    ss << m_Data.ResistMessage << "\n";
    Tiredness += uniform(5, 15);
}

void TrainingJob::OnRegularProgress(sGirl& girl, bool is_night) {
    ss << m_Data.ProgressMessage;
    if (girl.m_WorkingDay >= 100)
    {
        ss << "almost complete.";
        Tiredness -= (girl.m_WorkingDay - 100) / 2;    // her last day so she rested a bit
    }
    else ss << "in progress (" << girl.m_WorkingDay << "%).";
}

class SoStraight : public TrainingJob {
public:
    SoStraight() : TrainingJob(JOB_SO_STRAIGHT, "SOSt", {
        "Straight", "${name} is already Straight.",
        "You proceed to change ${name}'s sexual orientation to Straight.",
        "She resisted all attempts to make her Straight.",
        "Her Sexual Orientation conversion to Straight is ",
        IMGTYPE_SEX}) {
        m_Info.Description = "You will make sure she only likes having sex with men.";
    }
    void HandleTraining(sGirl& girl, bool is_night) override;
    void OnComplete(sGirl& girl) override;
};

void SoStraight::HandleTraining(sGirl& girl, bool is_night) {
    auto brothel = girl.m_Building;

    // Positive Stats/Skills
    girl.m_WorkingDay += girl.normalsex() / 5;
    girl.m_WorkingDay += girl.group() / 10;
    girl.m_WorkingDay += girl.oralsex() / 20;
    girl.m_WorkingDay += girl.tittysex() / 20;
    girl.m_WorkingDay += girl.anal() / 20;

    update_progress(girl);

    if (girl.has_active_trait("Lesbian"))
    {
        ss << "Her innate disgust of balls and shaft made her pull away from you while trying to teach her to suck it.\n";
        girl.m_WorkingDay -= girl.lesbian() / 5;                    // it is hard to change something you are good at
        Tiredness += girl.lesbian() / 10;
    }
    if (girl.has_active_trait("Bisexual")) girl.m_WorkingDay -= girl.lesbian() / 20;    // it is hard to change something you are good at

    int trait = girl.get_trait_modifier("so.straight");
    girl.m_WorkingDay += uniform(trait / 2, trait + trait / 2);
    if (girl.has_active_trait("Broken Will"))        { ss << "She just sits there doing exactly what you tell her to do, You don't think it is really getting through to her.\n"; }

    if (!brothel->is_sex_type_allowed(SKILL_NORMALSEX))      girl.m_WorkingDay -= uniform(10, 30);

    CountTheDays(girl, is_night);
    int xp = 1 + std::max(0, girl.m_WorkingDay / 20);

    // Improve girl
    int I_lesbian = uniform(-15, -2);
    int I_normalsex = uniform(2, 15);
    int I_group = uniform(1, 5);
    int I_anal = uniform(1, 5);
    int I_oralsex = uniform(1, 5);
    int I_handjob = uniform(1, 5);
    int I_tittysex = uniform(1, 5);

    girl.exp(xp);
    girl.lesbian(I_lesbian);
    girl.normalsex(I_normalsex);
    girl.group(I_group);
    girl.anal(I_anal);
    girl.oralsex(I_oralsex);
    girl.handjob(I_handjob);
    girl.tittysex(I_tittysex);
}

void SoStraight::OnComplete(sGirl& girl) {
    ss << "Her Sexual Orientation conversion is complete. She is now Straight.\n";
    girl.lose_trait("Lesbian");    girl.gain_trait("Bisexual");    girl.lose_trait("Straight");
}

class SoLesbian : public TrainingJob {
public:
    SoLesbian() : TrainingJob(JOB_SO_LESBIAN, "SOLe",
                              {"Lesbian", "${name} is already a Lesbian.",
                               "You proceed to change ${name}'s sexual orientation to Lesbian.",
                               "She resisted all attempts to make her a Lesbian.",
                               "Her Sexual Orientation conversion to Lesbian is ",
                               IMGTYPE_SEX}) {
        m_Info.Description = "You will make sure she only likes having sex with women.";
    }
    void HandleTraining(sGirl& girl, bool is_night) override;
    void OnComplete(sGirl& girl) override;
};

void SoLesbian::HandleTraining(sGirl& girl, bool is_night) {
    auto brothel = girl.m_Building;

    // Positive Stats/Skills
    girl.m_WorkingDay += girl.lesbian() / 5;
    girl.m_WorkingDay += girl.group() / 20;
    girl.m_WorkingDay += girl.oralsex() / 25;
    update_progress(girl);

    if (girl.has_active_trait("Straight"))
    {
        ss << "Being used to working with something long and hard, she wasn't really sure what she was doing with her partner.\n";
        girl.m_WorkingDay -= girl.normalsex() / 5;                // it is hard to change something you are good at
        Tiredness += girl.normalsex() / 10;
    }
    if (girl.has_active_trait("Bisexual")) girl.m_WorkingDay -= girl.normalsex() / 20;                    // it is hard to change something you are good at

    int trait = girl.get_trait_modifier("so.lesbian");
    girl.m_WorkingDay += uniform(trait / 2, trait + trait / 2);

    if (girl.has_active_trait("Broken Will"))    { ss << "She just sits there doing exactly what you tell her to do, You don't think it is really getting through to her.\n"; }

    //    if (girl.check_virginity())                {}

    if (!brothel->is_sex_type_allowed(SKILL_LESBIAN))        girl.m_WorkingDay -= uniform(10, 30);

    int xp = 1 + std::max(0, girl.m_WorkingDay / 20);
    CountTheDays(girl, is_night);

    // Improve girl
    int I_lesbian = uniform(2, 15);
    int I_normalsex = uniform(-2, -15);
    int I_group = uniform(1, -3);
    int I_anal = uniform(-1, -5);
    int I_oralsex = uniform(1, -3);
    int I_handjob = uniform(1, -3);
    int I_tittysex = uniform(-1, -5);

    girl.exp(xp);
    girl.lesbian(I_lesbian);
    girl.normalsex(I_normalsex);
    girl.group(I_group);
    girl.anal(I_anal);
    girl.oralsex(I_oralsex);
    girl.handjob(I_handjob);
    girl.tittysex(I_tittysex);
}

void SoLesbian::OnComplete(sGirl& girl) {
    ss << "\nHer Sexual Orientation conversion is complete. She is now a Lesbian.";
    girl.gain_trait("Lesbian");    girl.lose_trait("Bisexual");    girl.lose_trait("Straight");
}

class SoBi : public TrainingJob {
public:
    SoBi() : TrainingJob(JOB_SO_BISEXUAL, "SOBi",
                              {"Bisexual", "${name} is already Bisexual.",
                               "You proceed to change ${name}'s sexual orientation to Bisexual.",
                               "She resisted all attempts to make her Bisexual.",
                               "Her Sexual Orientation conversion to Bisexual is ",
                               IMGTYPE_LESBIAN}) {
        m_Info.Description = "You will make sure she likes having sex with both men and women.";
    }
    void HandleTraining(sGirl& girl, bool is_night) override;
    void OnComplete(sGirl& girl) override;
};

void SoBi::HandleTraining(sGirl& girl, bool is_night) {
    auto brothel = girl.m_Building;
    if (girl.has_active_trait("Straight"))
    {
        girl.m_WorkingDay += girl.group() / 10;
        girl.m_WorkingDay += girl.normalsex() / 20;
        girl.m_WorkingDay += girl.lesbian() / 5;
        girl.m_WorkingDay += girl.oralsex() / 20;
        girl.m_WorkingDay += girl.tittysex() / 20;
        girl.m_WorkingDay += girl.anal() / 20;
    }
    else if (girl.has_active_trait("Lesbian"))
    {
        girl.m_WorkingDay += girl.group() / 10;
        girl.m_WorkingDay += girl.normalsex() / 5;
        girl.m_WorkingDay += girl.lesbian() / 20;
        girl.m_WorkingDay += girl.oralsex() / 15;
        girl.m_WorkingDay += girl.tittysex() / 15;
        girl.m_WorkingDay += girl.anal() / 15;
    }
    else
    {
        girl.m_WorkingDay += girl.group() / 5;
        girl.m_WorkingDay += girl.normalsex() / 10;
        girl.m_WorkingDay += girl.lesbian() / 10;
        girl.m_WorkingDay += girl.oralsex() / 20;
        girl.m_WorkingDay += girl.tittysex() / 20;
        girl.m_WorkingDay += girl.anal() / 20;
    }
    update_progress(girl);

    int trait = girl.get_trait_modifier("so.bi");
    girl.m_WorkingDay += uniform(trait / 2, trait + trait / 2);
    if (girl.has_active_trait("Broken Will"))    { ss << "She just sits there doing exactly what you tell her to do, You don't think it is really getting through to her.\n"; }

    if (!brothel->is_sex_type_allowed(SKILL_LESBIAN))        girl.m_WorkingDay -= uniform(5, 15);
    if (!brothel->is_sex_type_allowed(SKILL_NORMALSEX))      girl.m_WorkingDay -= uniform(5, 15);
    CountTheDays(girl, is_night);
    int xp = 1 + std::max(0, girl.m_WorkingDay / 20);
    int I_lesbian = uniform(1, 10);
    int I_normalsex = uniform(1, 10);
    int I_group = uniform(2, 15);
    int I_anal = uniform(0, 5);
    int I_oralsex = uniform(0, 5);
    int I_handjob = uniform(0, 5);
    int I_tittysex = uniform(0, 3);

    girl.exp(xp);
    girl.lesbian(I_lesbian);
    girl.normalsex(I_normalsex);
    girl.group(I_group);
    girl.anal(I_anal);
    girl.oralsex(I_oralsex);
    girl.handjob(I_handjob);
    girl.tittysex(I_tittysex);
}

void SoBi::OnComplete(sGirl& girl) {
    ss << "\nHer Sexual Orientation conversion is complete. She is now Bisexual.";
    girl.lose_trait("Lesbian");    girl.gain_trait("Bisexual");    girl.lose_trait("Straight");
}

class FakeOrg : public TrainingJob {
public:
    FakeOrg() : TrainingJob(JOB_FAKEORGASM, "FOEx",
                         {"Fake Orgasm Expert", "${name} is already a \"Fake Orgasm Expert\".",
                          "You teach ${name} how to fake her orgasms.",
                          "She resisted all attempts to make her Bisexual.",
                          "Her Sexual Orientation conversion to Bisexual is ",
                          IMGTYPE_MAST}) {
        m_Info.Description = "You will teach her how to fake her orgasms.";
    }
    void HandleTraining(sGirl& girl, bool is_night) override;
    void OnComplete(sGirl& girl) override;
    void OnNoProgress(sGirl& girl) override;
    void OnRegularProgress(sGirl& girl, bool is_night) override;
};

void FakeOrg::HandleTraining(sGirl& girl, bool is_night) {
    auto brothel = girl.m_Building;

    // Positive Stats/Skills
    girl.m_WorkingDay += girl.performance() / 5;
    girl.m_WorkingDay += girl.group() / 20;
    girl.m_WorkingDay += girl.normalsex() / 20;
    girl.m_WorkingDay += girl.lesbian() / 20;

    update_progress(girl);
    int trait = girl.get_trait_modifier("fake-orgasm");
    girl.m_WorkingDay += uniform(trait / 2, trait + trait / 2);
    if (girl.has_active_trait("Broken Will"))    { ss << "She just sits there doing exactly what you tell her to do, You don't think it is really getting through to her.\n"; }

    if (!brothel->is_sex_type_allowed(SKILL_NORMALSEX))      girl.m_WorkingDay -= uniform(5, 15);

    CountTheDays(girl, is_night);

    // Improve girl
    int xp = 1 + std::max(0, girl.m_WorkingDay / 20);
    int I_performance = uniform(3, 15);
    int I_confidence = uniform(-1, 5);
    int I_constitution = std::max(0, uniform(-2, 1));
    int I_spirit = uniform(-5, 5);
    int I_lesbian = uniform(0, 5);
    int I_normalsex = uniform(0, 5);
    int I_group = uniform(0, 5);
    int I_anal = std::max(0, uniform(-2, 2));

    girl.exp(xp);
    girl.lesbian(I_lesbian);
    girl.normalsex(I_normalsex);
    girl.group(I_group);
    girl.anal(I_anal);
    girl.performance(I_performance);
    girl.confidence(I_confidence);
    girl.constitution(I_constitution);
    girl.spirit(I_spirit);
}

void FakeOrg::OnNoProgress(sGirl& girl) {
    if (girl.has_active_trait("Slow Learner") || girl.has_active_trait("Broken Will") || girl.has_active_trait("Mind Fucked") ||
        girl.has_active_trait("Retarded"))
    {
        ss << "She was not mentally able to learn";
    }
    else if (girl.has_active_trait("Bimbo") || girl.has_active_trait("Fast Orgasms") || girl.has_active_trait("Nymphomaniac"))
    {
        ss << "She was too focused on the sex to learn";
        Tiredness += uniform(5, 15);
    }
    else if (girl.has_active_trait("Blind") || girl.has_active_trait("Deaf"))
    {
        ss << "Her handicap kept her from learning";
    }
    else
    {
        ss << "She resisted all attempts to teach her";
        Tiredness += uniform(5, 15);
    }
    ss << " to fake her orgasms.";
}

void FakeOrg::OnComplete(sGirl& girl) {
    ss << "With her training complete, she is now a \"Fake Orgasm Expert\".";
    girl.lose_trait("Slow Orgasms");    girl.lose_trait("Fast Orgasms");    girl.gain_trait("Fake Orgasm Expert");
}

void FakeOrg::OnRegularProgress(sGirl& girl, bool is_night) {
    if (girl.m_WorkingDay >= 100)        Tiredness -= (girl.m_WorkingDay - 100) / 2;    // her last day so she rested a bit
    else    ss << "Training in progress (" << girl.m_WorkingDay << "%).\n \n";
    if (girl.m_WorkingDay < 25)      ss << "She has no idea what she sounds like during sex but it ain't orgasmic.";
    else if (girl.m_WorkingDay < 50) ss << "When she realizes she should finish, you can see it click in her mind and easily notice her changing things up.";
    else if (girl.m_WorkingDay < 75) ss << "She is still not getting into rhythm with " << (chance(33) ? "you" : "her partner") << " but it still seems enjoyable.";
    else                             ss << "She is almost there but you want her to practice a little more to get it perfect.";

    if (!is_night)
    {
        ss << "\nYou tell her to take a break for lunch and ";
        if (girl.m_WorkingDay < 50)      ss << "clear her mind, she has a lot more work to do.";
        else if (girl.m_WorkingDay < 75) ss << "relax, she has a bit more training to do.";
        else                             ss << "see if she can make anyone say \"I'll have what she's having\".";
    }
    else
    {
        ss << "\nThats all for tonight, ";
        if (girl.m_WorkingDay < 50)    ss << "we have a lot more to do tomorrow (and probably the next few weeks).";
        else                           ss << "we'll pick things up in the morning.";
    }
}

void RegisterTrainingJobs(cJobManager& mgr) {
    mgr.register_job(std::make_unique<SoStraight>());
    mgr.register_job(std::make_unique<SoLesbian>());
    mgr.register_job(std::make_unique<SoBi>());
    mgr.register_job(std::make_unique<FakeOrg>());
}