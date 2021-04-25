#ifndef WM_CIMAGEMANAGER_H
#define WM_CIMAGEMANAGER_H

#include <memory>
#include "substitution.h"

class cImageMatcher {
public:
    std::string find_match(const sImageSpec& query) const;
private:
    std::shared_ptr<std::vector<sSubstitutionRule>> m_Rules;
    std::vector<sImageResource> m_Resources;
};

class cImageManager {
public:
private:
    std::shared_ptr<std::vector<sSubstitutionRule>> m_GlobalRules;
};

#endif //WM_CIMAGEMANAGER_H
