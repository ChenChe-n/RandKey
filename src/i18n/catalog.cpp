#include "randkey/i18n/catalog.hpp"

namespace randkey::i18n
{
    Catalog::Catalog()
        : fallback_("en-US")
    {
    }

    std::string_view Catalog::translate(std::string_view language, std::string_view key) const
    {
        auto lang_it = data_.find(std::string(language));
        if (lang_it != data_.end())
        {
            if (auto entry = lang_it->second.find(std::string(key)); entry != lang_it->second.end())
            {
                return entry->second;
            }
        }

        auto fallback_it = data_.find(fallback_);
        if (fallback_it != data_.end())
        {
            if (auto entry = fallback_it->second.find(std::string(key)); entry != fallback_it->second.end())
            {
                return entry->second;
            }
        }

        return "";
    }

    void Catalog::set_fallback(std::string language)
    {
        fallback_ = std::move(language);
    }

    void Catalog::insert(std::string language, std::unordered_map<std::string, std::string> entries)
    {
        data_.emplace(std::move(language), std::move(entries));
    }
}
