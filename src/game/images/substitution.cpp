#include "substitution.h"
#include "xml/util.h"
#include "xml/getattr.h"
#include "utils/string.hpp"
#include <tinyxml2.h>

bool is_matched(const sImageResource& resource, const sImageSpec& spec) {
    if(spec.PrimaryTag != resource.PrimaryTag) {
        return false;
    }

    // check that secondaries match
    for(int i = 0; i < NUM_SECONDARY_TAGS; ++i) {
        if(spec.SecondaryTags[i] == ESecondaryTagState::ANY || resource.SecondaryTags[i] == ESecondaryTagState::ANY)
            continue;

        if(resource.SecondaryTags[i] != spec.SecondaryTags[i]) {
            return false;
        }
    }

    return true;
}

bool is_matched(const sImageSpec& spec, const sSubstitutionRule& rule) {
    bool primary_match = false;
    for(auto& pt : rule.PrimaryTags) {
        if(pt == "*" || pt == spec.PrimaryTag) {
            primary_match = true;
        }
    }

    if(!primary_match) return false;

    // check that secondaries match
    for(int i = 0; i < NUM_SECONDARY_TAGS; ++i) {
        if(rule.SecondaryTags[i] != spec.SecondaryTags[i] && rule.SecondaryTags[i] != ESecondaryTagState::ANY) {
            return false;
        }
    }

    return true;
}

sImageSpec get_substitution(const sImageSpec& source, const sSubstitutionRule& rule) {
    sImageSpec new_base{source.PrimaryTag, source.SecondaryTags};
    if(!rule.NewPrimaryTag.empty()) {
        new_base.PrimaryTag = rule.NewPrimaryTag;
    }

    // Now we need to look at the combinations of secondaries.
    // any tag that is IGNORED by the rule has to remain unchanged.
    // every other tag is set to the new value
    for(int i = 0; i < NUM_SECONDARY_TAGS; ++i) {
        if( rule.NewSecondaryTags[i] != ESecondaryTagState::IGNORED) {
            new_base.SecondaryTags[i] = rule.NewSecondaryTags[i];
        }
    }

    return new_base;
}

namespace {
    ESecondaryTag string_to_tag(const std::string& source) {
        if(source == "pregnant") {
            return ESecondaryTag::PREGNANT;
        } else if(source == "slave") {
            return ESecondaryTag::SLAVE;
        } else {
            throw std::runtime_error("Unknown tag");
        }
    }
    ESecondaryTagState string_to_state(const std::string& source) {
        if(source == "yes") {
            return ESecondaryTagState::PRESENT;
        } else if(source == "no") {
            return ESecondaryTagState::ABSENT;
        } else if(source == "any") {
            return ESecondaryTagState::ANY;
        } else if(source == "*") {
            return ESecondaryTagState::IGNORED;
        } else {
            throw std::runtime_error("Unknown tag");
        }
    }
}

sSubstitutionRule sSubstitutionRule::from_xml(const tinyxml2::XMLElement& source) {
    sSubstitutionRule new_rule;
    new_rule.SecondaryTags.fill(ESecondaryTagState::ANY);

    for(auto& primary_el : IterateChildElements(source, "Primary") ) {
        new_rule.PrimaryTags.emplace_back( primary_el.Value() );
    }

    for(auto& secondary_el : IterateChildElements(source, "Secondary") ) {
        std::string tag = tolower(GetStringAttribute(secondary_el, "Tag"));
        std::string value = tolower(GetStringAttribute(secondary_el, "Value"));
        int index = static_cast<int>(string_to_tag(tag));
        new_rule.SecondaryTags[index] = string_to_state(value);
    }

    if(auto* np = source.FirstChildElement("NewPrimary")) {
        new_rule.NewPrimaryTag = np->Value();
    }

    for(auto& secondary_el : IterateChildElements(source, "NewSecondary") ) {
        std::string tag = tolower(GetStringAttribute(secondary_el, "Tag"));
        std::string value = tolower(GetStringAttribute(secondary_el, "Value"));
        int index = static_cast<int>(string_to_tag(tag));
        new_rule.NewSecondaryTags[index] = string_to_state(value);
    }

    new_rule.Cost = GetIntAttribute(source, "Cost");

    return new_rule;
}