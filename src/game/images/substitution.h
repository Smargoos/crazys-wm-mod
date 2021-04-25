#ifndef WM_SUBSTITUTION_H
#define WM_SUBSTITUTION_H

#include <string>
#include <array>
#include <vector>

namespace tinyxml2 {
    class XMLElement;
}

enum class ESecondaryTagState {
    PRESENT,
    ABSENT,
    IGNORED,        // Only used in substitution rule replacements
    ANY
};

enum class ESecondaryTag {
    SLAVE,
    PREGNANT,
    COUNT
};

constexpr const std::size_t NUM_SECONDARY_TAGS = static_cast<std::size_t>(ESecondaryTag::COUNT);

struct sImageSpec {
    std::string PrimaryTag;
    std::array<ESecondaryTagState, NUM_SECONDARY_TAGS> SecondaryTags;
};

struct sImageResource {
    std::vector<std::string> ImageFiles;
    std::string PrimaryTag;
    std::array<ESecondaryTagState, NUM_SECONDARY_TAGS> SecondaryTags;
};

struct sSubstitutionRule {
    int Cost;

    // match
    std::vector<std::string> PrimaryTags;
    std::array<ESecondaryTagState, NUM_SECONDARY_TAGS> SecondaryTags;

    // replace
    std::string NewPrimaryTag;
    std::array<ESecondaryTagState, NUM_SECONDARY_TAGS> NewSecondaryTags;

    static sSubstitutionRule from_xml(const tinyxml2::XMLElement& source);
};

bool is_matched(const sImageSpec& spec, const sSubstitutionRule& rule);
bool is_matched(const sImageResource& resource, const sImageSpec& spec);
sImageSpec get_substitution(const sImageSpec& source, const sSubstitutionRule& rule);



#endif //WM_SUBSTITUTION_H
