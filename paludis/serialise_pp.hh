#ifndef PALUDIS_GUARD_PALUDIS_SERIALISE_PP_HH
#define PALUDIS_GUARD_PALUDIS_SERIALISE_PP_HH 1

#include <string>

namespace paludis {
    class SerialiserPrettyPrinter {
    public:
        std::string operator()(const std::string& s);
    };
};

#endif
