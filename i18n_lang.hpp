#ifndef I18N_LANG_HPP
#define I18N_LANG_HPP

#include <string>
#include <string_view>
#include <unordered_map>
#include <type_traits>

template <
    typename LangType = std::u32string,
    typename TextType = std::u32string,
    typename TextIdType = uint64_t>
class i18n_lang
{
public:
    using lang_t = LangType;
    using text_t = TextType;
    using text_id_t = TextIdType;
    using text_map = std::unordered_map<text_id_t, text_t>;
    using lang_text_map = std::unordered_map<lang_t, text_map>;

    // 构造函数1: 只设置默认语言
    explicit i18n_lang(lang_t default_lang, text_t default_error_text = U"TEXT_NOT_FOUND")
        : default_lang_(std::move(default_lang)),
          default_error_text_(std::move(default_error_text)) {}
    // 构造函数2: 设置默认语言和列表
    explicit i18n_lang(lang_t default_lang,
                       lang_text_map texts,
                       text_t default_error_text = U"TEXT_NOT_FOUND")
        : default_lang_(std::move(default_lang)),
          texts_(std::move(texts)),
          default_error_text_(std::move(default_error_text)) {}

    // 构造函数3: 设置默认语言和初始化列表
    // 构造函数
    i18n_lang(lang_t default_lang,
              std::initializer_list<std::pair<const lang_t,
                                              std::initializer_list<std::pair<const text_id_t, text_t>>>>
                  init_list,
              text_t default_error = U"TEXT_NOT_FOUND")
        : default_lang_(std::move(default_lang)),
          default_error_text_(std::move(default_error))
    {

        for (const auto &lang_pair : init_list)
        {
            for (const auto &text_pair : lang_pair.second)
            {
                texts_[lang_pair.first][text_pair.first] = text_pair.second;
            }
        }
    }

    // 设置文本
    i18n_lang &set_text(const lang_t &lang, text_id_t text_id, text_t text)
    {
        texts_[lang][text_id] = std::move(text);
        return *this;
    }
    // 设置文本
    i18n_lang &set_texts(const lang_t &lang, const text_map &texts)
    {
        texts_[lang].insert(texts.begin(), texts.end());
        return *this;
    }

    // 获取文本
    const text_t &get_text(const lang_t &lang, text_id_t text_id) const
    {
        if (auto lang_iter = texts_.find(lang); lang_iter != texts_.end())
        {
            if (auto text_iter = lang_iter->second.find(text_id); text_iter != lang_iter->second.end())
            {
                return text_iter->second;
            }
        }

        // 回退到默认语言
        if (auto default_iter = texts_.find(default_lang_); default_iter != texts_.end())
        {
            if (auto text_iter = default_iter->second.find(text_id); text_iter != default_iter->second.end())
            {
                return text_iter->second;
            }
        }

        return default_error_text_;
    }

private:
    lang_t default_lang_;
    lang_text_map texts_;
    text_t default_error_text_;
};
#endif