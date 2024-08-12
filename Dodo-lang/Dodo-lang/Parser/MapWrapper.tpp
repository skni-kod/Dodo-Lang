#ifndef DODO_LANG_MAP_WRAPPER_TPP
#define DODO_LANG_MAP_WRAPPER_TPP

#include <unordered_map>
#include "Parser.hpp"

template <typename TK, typename TV>
class MapWrapper {
private:
    std::unordered_map <TK, TV> map;
public:

    bool isKey(TK key) {
        if (map.find(key) == map.end()) {
            return false;
        }
        return true;
    }

    TV& operator[](const TK& key) {
        if (map.find(key) == map.end()) {
            ParserError("call to undefined token!");
        }
        return map[key];
    }

    void insert(const TK& key, const TV& value) {
        if (map.find(key) == map.end()) {
            map.insert(key, value);
            return;
        }
        ParserError("token redefinition!");
    }

    void insert(const TK& key, TV&& value) {
        if (map.find(key) == map.end()) {
            map.insert(std::pair <TK, TV> (key, value));
            return;
        }
        ParserError("token redefinition!");
    }

    std::size_t size() {
        return map.size();
    }
};

#endif //DODO_LANG_MAP_WRAPPER_TPP
