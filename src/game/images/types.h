#ifndef WM_TYPES_H
#define WM_TYPES_H

#include "Constants.h"

namespace image_types {
    constexpr const char* PROFILE = "profile";
    constexpr const char* FORMAL  = "formal";

    namespace work {
        constexpr const char* REFUSE       = "work:refuse";
        constexpr const char* IMPOSSIBLE   = "work:impossible";
        constexpr const char* DOES_NOTHING = "work:nothing";
        constexpr const char* BACK_TO_WORK = "work:back-to-work";
        constexpr const char* MATRON_MSG   = "work:matron-msg";
        constexpr const char* MATRON_WARN  = "work:matron-warn";

        constexpr const char* MAID       = "work:maid";

        constexpr const char* WAITRESS   = "work:waitress";
        constexpr const char* BARMAID    = "work:barmaid";
        constexpr const char* COOK       = "work:cook";
        constexpr const char* PIANO      = "work:piano";
        constexpr const char* SINGER     = "work:singer";
        constexpr const char* BAR_EMPTY  = "work:bar-empty";

        constexpr const char* CARD_DEALER = "work:card-dealer";
        constexpr const char* ENTERTAINER = "work:entertainer";
        constexpr const char* XXX_ENT     = "work:xxx-entertainer";
        constexpr const char* PEEP_SHOW   = "work:peep-show";
        constexpr const char* SLEAZY_BARMAID = "work:sleazy-barmaid";
        constexpr const char* SLEAZY_WAITRESS = "work:sleazy-waitress";

        constexpr const char* BAKER      = "work:baker";
        constexpr const char* BREWER     = "work:brewer";
        constexpr const char* BUTCHER    = "work:butcher";
        constexpr const char* BLACKSMITH = "work:smith";
        constexpr const char* COBBLER    = "work:cobbler";
        constexpr const char* JEWELER    = "work:jeweler";
        constexpr const char* POTIONS    = "work:potions";
        constexpr const char* TAILOR     = "work:tailor";
        constexpr const char* MAKEITEMS  = "work:make-items";

        constexpr const char* DOCTOR     = "work:doctor";
        constexpr const char* NURSE      = "work:nurse";
        constexpr const char* INTERN     = "work:intern";
        constexpr const char* MECHANIC   = "work:mechanic";
        constexpr const char* CURE       = "work:cure";
        constexpr const char* ABORT      = "work:abortion";
        constexpr const char* JANITOR    = "work:janitor";
        constexpr const char* SURGERY    = "work:get-surgery";
        constexpr const char* NO_DOCTOR  = "work:no-doctor";
        constexpr const char* WAITING_ROOM = "work:waiting-room";

        constexpr const char* FARMER     = "work:farmer";
        constexpr const char* GARDENER   = "work:gardener";
        constexpr const char* MARKETER   = "work:marketer";
        constexpr const char* FARMVET    = "work:veterinarian";
        constexpr const char* SHEPHERD   = "work:shepherd";
        constexpr const char* RANCHER    = "work:rancher";
        constexpr const char* CATRANCH   = "work:catacomb-rancher";
        constexpr const char* MILKER     = "work:milker";
        constexpr const char* GET_MILKED = "work:get-milked";
        constexpr const char* BEASTCAPT  = "work:beast-capture";
        constexpr const char* RESEARCH   = "work:research";
        constexpr const char* FARMHAND   = "work:farmhand";
        constexpr const char* BEAST_CARE = "work:beast-care";       // -> FARM

        constexpr const char* NO_CREW    = "work:no-crew";
        constexpr const char* CREW_REFUSE = "work:crew-refuse";
        constexpr const char* FILMCHEF   = "work:film-chef";
        constexpr const char* FILMTITTY  = "work:film-titty";
        constexpr const char* FILMSTRIP  = "work:film-strip";
        constexpr const char* FILMLES    = "work:film-les";
        constexpr const char* FILMHAND   = "work:film-hand";
        constexpr const char* FILMFOOT   = "work:film-foot";
        constexpr const char* FILMANAL   = "work:film-anal";
        constexpr const char* FILMMAST   = "work:film-mast";
        constexpr const char* FILMBEAST  = "work:film-beast";
        constexpr const char* FILMBONDAGE = "work:film-bondage";
        constexpr const char* FILMPUBBDSM = "work:film-public-bdsm";
        constexpr const char* FILMBUK    = "work:film-bukkake";
        constexpr const char* FILMGROUP  = "work:film-group";
        constexpr const char* FILMORAL   = "work:film-oral";
        constexpr const char* FILMSEX    = "work:film-sex";
        constexpr const char* FILMTHROAT = "work:film-throat";
        constexpr const char* FILMTEASE  = "work:film-tease";
        constexpr const char* FILMACTION = "work:film-action";
        constexpr const char* FILMMUSIC  = "work:film-music";

        constexpr const char* DIRECTOR   = "work:director";
        constexpr const char* CAMMAGE    = "work:camera-mage";
        constexpr const char* CRYSTAL    = "work:crystal-purifier";
        constexpr const char* PROMOTER   = "work:promoter";
        constexpr const char* STAGEHAND  = "work:stagehand";
        constexpr const char* FLUFFER    = "work:fluffer";      // -> ORAL
        constexpr const char* FILM_CREW  = "work:film-crew";

        constexpr const char* MATRON     = "work:matron";

        constexpr const char* THERAPY    = "work:therapy";
        constexpr const char* COUNSELOR  = "work:counselor";
        constexpr const char* FIGHT_COUNSELOR = "work:fight-counselor";
        constexpr const char* NO_COUNSELOR = "work:no-counselor";


        constexpr const char* ADVERTISING  = "work:advertising";
        constexpr const char* RECRUITER    = "work:recruiter";
        constexpr const char* CUST_SERVICE = "work:customer-service";
        constexpr const char* CUST_SERVICE_DONE = "work:customer-service:no-custs";

        constexpr const char* STRIPPER    = "work:stripper";
        constexpr const char* MASSEUSE    = "work:masseuse";
        constexpr const char* ESCORT      = "work:escort";
        constexpr const char* WHORE       = "work:whore";
        constexpr const char* WHORE_ATTACKED = "work:whore:attacked";
        constexpr const char* WHORE_CUSTREF = "work:whore:customer-refused";

        constexpr const char* CITYGUARD    = "work:city-guard";
        constexpr const char* SECURITY     = "work:security";
        constexpr const char* CLEANING     = "work:clean";
        constexpr const char* TRAIN_COMBAT = "work:train-combat";
        constexpr const char* CATACOMBS    = "work:catacombs";
        constexpr const char* TORTURER     = "work:torturer";

        constexpr const char* COMMUNITY   = "work:community-service";
        constexpr const char* FEED_POOR   = "work:feed-poor";

        constexpr const char* MISTRESS    = "work:mistress";
    }

    namespace training {
        constexpr const char* FAKE     = "training:fake";
        constexpr const char* STRAIGHT = "training:straight";
        constexpr const char* BI       = "training:bi";
        constexpr const char* LESBIAN  = "training:lesbian";
        constexpr const char* PUPPY    = "training:puppy";
        constexpr const char* PONY     = "training:pony";
        constexpr const char* CAT      = "training:cat";
        constexpr const char* SOLO     = "training:solo";
        constexpr const char* FORCED   = "training:forced";
    }

    namespace treatments {
        constexpr const char* IMP_AIDS = "treatment:impossible:aids";
        constexpr const char* IMP_HEALTH = "treatment:impossible:health";
        constexpr const char* IMP_NODOC = "treatment:impossible:nodoc";
        constexpr const char* SENT_TO_WAITING = "treatment:waiting-room";
    }

    namespace sex {
        constexpr const char* ANY     = "sex:lesbian:*";

        constexpr const char* LESBIAN     = "sex:lesbian:*";
        constexpr const char* EATOUT      = "sex:lesbian:eatout";

        constexpr const char* TITTY       = "sex:titty";
        constexpr const char* HAND        = "sex:hand";
        constexpr const char* FOOT        = "sex:foot";

        constexpr const char* ANAL        = "sex:anal";

        constexpr const char* ORAL        = "sex:oral:*";
        constexpr const char* BLOWJOB     = "sex:oral:blowjob";
        constexpr const char* DEEPTHROAT  = "sex:oral:deep";

        constexpr const char* VANILLA_ALL = "sex:vanilla:*";
        constexpr const char* MISSIONARY  = "sex:vanilla:missionary";
        constexpr const char* DOGGY       = "sex:vanilla:doggy";

        constexpr const char* FFM       = "sex:group:ffm";
        constexpr const char* MMF       = "sex:group:mmf";
        constexpr const char* ORGY      = "sex:group:orgy";
        constexpr const char* GROUP_ALL = "sex:group:*";

        constexpr const char* MASTURBATE = "sex:soft:mast";
        constexpr const char* FLASH = "sex:soft:flash";  // -> strip
        constexpr const char* ECCHI = "sex:soft:ecchi";
        constexpr const char* STRIP = "sex:soft:strip";
        constexpr const char* NUDE  = "sex:soft:nude";
        constexpr const char* FINGERING = "sex:fingering";

        constexpr const char* BATH = "sex:bath";
        constexpr const char* SLAP_BUTT = "sex:slap-butt";

        constexpr const char* BEAST = "sex:rough:beast";
        constexpr const char* BDSM = "sex:rough:bdsm";

        // Impreg
        constexpr const char* IMPREG_BEAST = "sex:impregnate:beast";
        constexpr const char* IMPREG_HUMAN = "sex:impregnate:human";

        constexpr const char* RAPE_MONSTER = "sex:rape:beast";
        constexpr const char* RAPE_HUMAN   = "sex:rape:human";

        // TODO maybe EVENT category
        constexpr const char* BLOW_FOR_DRUGS = "sex:blow-for-drugs";
        constexpr const char* FUCK_FOR_DRUGS = "sex:fuck-for-drugs";

        // customer doesn't want her
        constexpr const char* REJECTED = "sex:rejected";
    }

    namespace free {
        // what she does in her freetime
        constexpr const char* FREETIME  = "free:freetime";
        constexpr const char* BATH      = "free:bath";
        constexpr const char* SLEEP     = "free:sleep";
        constexpr const char* BED       = "free:bed";
        constexpr const char* TALK      = "free:talk";
        constexpr const char* BUY_DRUGS = "free:buy_drugs";
        constexpr const char* STEALING  = "free:stealing";
        constexpr const char* SWIM      = "free:swim";
        constexpr const char* SHOPPING  = "free:shopping";
        constexpr const char* BOOK      = "free:book";
        constexpr const char* SPORTS    = "free:sports";
        constexpr const char* CLUBBING  = "free:club";
    }

    namespace combat {
        constexpr const char* WON_FIGHT  = "combat:victory";
        constexpr const char* LOST_FIGHT = "combat:defeat";
        constexpr const char* FIGHT      = "combat:fight";
        constexpr const char* DEATH      = "combat:death";
    }

    constexpr const char* BUY_HER      = "bought";
    constexpr const char* CAPTURE_HER  = "captured";
    constexpr const char* SLAVE_MARKET = "slave-market";
    constexpr const char* JAIL         = "jail";
    constexpr const char* DUNGEON      = "dungeon";
    constexpr const char* ASK_FAVOUR   = "ask-favour";

    constexpr const char* DEATH        = "death";
    constexpr const char* TORTURE      = "torture";
    constexpr const char* RUNAWAY      = "runaway";
    constexpr const char* BIRTH        = "birth";
    constexpr const char* SICK         = "sick";
    constexpr const char* ADDICTION    = "addiction";

    constexpr const char* LEVEL_UP     = "level-up";
    constexpr const char* GOOD_NEWS    = "good-news";

    // For now, all building events just use this message
    constexpr const char* BUILDING     = "building";

    const char* skill_to_img(SKILLS skill) {
        switch(skill) {
            case SKILL_LESBIAN:
                return image_types::sex::LESBIAN;
            case SKILL_ANAL:
                return image_types::sex::ANAL;
            case SKILL_ORALSEX:
                return image_types::sex::ORAL;
            case SKILL_TITTYSEX:
                return image_types::sex::TITTY;
            case SKILL_HANDJOB:
                return image_types::sex::HAND;
            case SKILL_FOOTJOB:
                return image_types::sex::FOOT;
            case SKILL_NORMALSEX:
                return image_types::sex::VANILLA_ALL;
            case SKILL_BEASTIALITY:
                return image_types::sex::BEAST;
            case SKILL_GROUP:
                return image_types::sex::GROUP_ALL;
            case SKILL_STRIP:
                return image_types::sex::STRIP;
        }
    }
}

#endif //WM_TYPES_H
