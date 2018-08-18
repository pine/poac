#ifndef POAC_SUBCMD_BUILD_HPP
#define POAC_SUBCMD_BUILD_HPP

#include <iostream>
#include <string>
#include <regex>

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#include "../core/except.hpp"
#include "../io/file.hpp"
#include "../io/cli.hpp"
#include "../util/compiler.hpp"
#include "../util/package.hpp"


namespace poac::subcmd { struct build {
    static const std::string summary() { return "Beta: Compile all sources that depend on this project."; }
    static const std::string options() { return "[-v | --verbose]"; }

    template <typename VS>
    void operator()(VS&& argv) { _main(argv); }
    template <typename VS>
    void _main(VS&& argv) {
        namespace fs     = boost::filesystem;
        namespace except = core::except;

        check_arguments(argv);

        bool verbose = (argv.size() > 0 && (argv[0] == "-v" || argv[0] == "--verbose"));

        // TODO: --backend cmake
        // TODO: poac.yml compiler: -> error version outdated

        const std::string project_name = io::file::yaml::get_node("name").as<std::string>();
        const std::string project_version = io::file::yaml::get_node("version").as<std::string>();
        const std::string project_path = (io::file::path::current_build_bin_dir / project_name).string();

        fs::create_directories(io::file::path::current_build_bin_dir);

        // Generate object file
        util::compiler compiler(project_name);
        // TODO: g++, clang++の選択方法，std::getenv("CXX")など
        compiler.set_system("clang++");
        compiler.set_version(17);
        compiler.set_source_file("src/main.cpp");
        compiler.set_output_path(io::file::path::current_build_bin_dir);

        compiler.add_macro_defn(std::make_pair("POAC_ROOT", std::getenv("PWD")));
        const std::string def_macro_name = boost::to_upper_copy<std::string>(project_name) + "_VERSION";
        compiler.add_macro_defn(std::make_pair(def_macro_name, project_version));

        const auto deps = io::file::yaml::get_node("deps");
        for (YAML::const_iterator itr = deps.begin(); itr != deps.end(); ++itr) {
            const std::string name = itr->first.as<std::string>();
            std::string src = util::package::get_source(itr->second);
            const std::string version = util::package::get_version(itr->second, src);
            const std::string pkgname = util::package::cache_to_current(util::package::github_conv_pkgname(name, version));
            const fs::path pkgpath = io::file::path::current_deps_dir / pkgname;

            if (const fs::path include_dir = pkgpath / "include"; fs::exists(include_dir))
                compiler.add_include_search_path(include_dir.string());
            if (const fs::path lib_dir = pkgpath / "lib"; fs::exists(lib_dir)) {
                compiler.add_library_search_path(lib_dir.string());

                if (io::file::yaml::exists_key(itr->second, "link")) {
                    if (const auto vs = io::file::yaml::get2<std::vector<std::string>>(itr->second, "link", "include")) {
                        for (const auto &s : *vs) {
                            compiler.add_static_link_lib(s);
                        }
                    }
                    else {
                        compiler.add_static_link_lib(pkgname);
//                    for ( const auto& e : boost::make_iterator_range( fs::directory_iterator( pkgpath / "lib" ), { } ) ) {
//                        if (!fs::is_directory(e)) {
//                            const std::string libname = e.path().filename().stem().string();
//                            compiler.add_static_link_lib(libname.substr(3));
//                        }
//                    }
                    }
                }
            }
        }
        if (verbose) std::cout << compiler << std::endl << std::endl;

        if (compiler.compile()) {
            // TODO: compiler.hpp side
            fs::remove(project_path + ".o");
            std::cout << io::cli::green << "Done: " << io::cli::reset
                      << "Output to `" + fs::relative(project_path).string() + "`"
                      << std::endl;
        }
        else { /* error */ }
    }

    void check_arguments(const std::vector<std::string>& argv) {
        namespace except = core::except;

        if (argv.size() >= 2)
            throw except::invalid_second_arg("build");
    }
};} // end namespace
#endif // !POAC_SUBCMD_BUILD_HPP