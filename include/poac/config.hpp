#ifndef POAC_CONFIG_HPP
#define POAC_CONFIG_HPP

#if !defined(POAC_PROJECT_ROOT)
#   warning "POAC_PROJECT_ROOT is not defined"
#endif
#if !defined(POAC_VERSION)
#   warning "POAC_VERSION is not defined"
#endif
namespace poac {
    static constexpr char const* POAC_API_URL =
            "https://poac.pm/api/";
    static constexpr char const* POAC_PACKAGES_API_URL =
            "https://poac.pm/api/packages/";
}

#endif // !POAC_CONFIG_HPP