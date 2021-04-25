#include "cImageManager.h"
#include <algorithm>
#include <deque>

std::string cImageMatcher::find_match(const sImageSpec& query) const {
    struct RatedQuery {
        int Cost;
        sImageSpec Spec;
    };

    struct RatedImage {
        int Cost;
        std::string Image;
    };
    std::deque<RatedQuery> query_list{{0, query}};
    std::vector<RatedImage> image_list;
    image_list.reserve(10);

    int trials = 0;
    while(image_list.size() < 10 && trials < 1000) {
        std::sort(begin(query_list), end(query_list), [](auto& a, auto& b) { return a.Cost < b.Cost; });
        auto current = query_list.front();
        query_list.pop_front();

        for(auto& resource : m_Resources) {
            if(is_matched(resource, current.Spec)) {
                for(auto& img: resource.ImageFiles) {
                    image_list.push_back({current.Cost, img});
                }
            }
        }

        for(auto& rule : *m_Rules) {
            if(is_matched(current.Spec, rule)) {
                query_list.push_front({current.Cost + rule.Cost, get_substitution(current.Spec, rule)});
            }
        }
        ++trials;
    }

    if(image_list.empty())
        return "";

    return image_list[rand() % image_list.size()].Image;
}