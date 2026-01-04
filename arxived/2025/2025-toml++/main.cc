// Download https://github.com/marzer/tomlplusplus/releases
// Doc https://marzer.github.io/tomlplusplus/
//
#include <iostream>
#include <toml++/toml.hpp>
using namespace std::string_view_literals;

int
main( )
{
    static constexpr auto source = R"(
        str = "hello world"

        numbers = [ 1, 2, 3, "four", 5.0 ]
        vegetables = [ "tomato", "onion", "mushroom", "lettuce" ]
        minerals = [ "quartz", "iron", "copper", "diamond" ]

        [animals]
        cats = [ "tiger", "lion", "puma" ]
        birds = [ "macaw", "pigeon", "canary" ]
        fish = [ "salmon", "trout", "carp" ]
    )"sv;
    toml::table           tbl    = toml::parse( source );

    // different ways of directly querying data
    std::optional<std::string_view> str1 =
        tbl["str"].value<std::string_view>( );
    std::optional<std::string> str2 = tbl["str"].value<std::string>( );
    std::string_view           str3 = tbl["str"].value_or( ""sv );
    std::string &str4 = tbl["str"].ref<std::string>( );  // ~~dangerous~~

    std::cout << *str1 << "\n";
    std::cout << *str2 << "\n";
    std::cout << str3 << "\n";
    std::cout << str4 << "\n";

    // get a toml::node_view of the element 'numbers' using operator[]
    auto numbers = tbl["numbers"];
    std::cout << "table has 'numbers': " << !!numbers << "\n";
    std::cout << "numbers is an: " << numbers.type( ) << "\n";
    std::cout << "numbers: " << numbers << "\n";

    // get the underlying array object to do some more advanced stuff
    if ( toml::array *arr = numbers.as_array( ) ) {
        // visitation with for_each() helps deal with heterogeneous data
        arr->for_each( []( auto &&el ) {
            if constexpr ( toml::is_number<decltype( el )> )
                ( *el )++;
            else if constexpr ( toml::is_string<decltype( el )> )
                el = "five"sv;
        } );

        // arrays are very similar to std::vector
        arr->push_back( 7 );
        arr->emplace_back<toml::array>( 8, 9 );
        std::cout << "numbers: " << numbers << "\n";
    }

    // node-views can be chained to quickly query deeper
    std::cout << "cats: " << tbl["animals"]["cats"] << "\n";
    std::cout << "fish[1]: " << tbl["animals"]["fish"][1] << "\n";

    // can also be retrieved via absolute path
    std::cout << "cats: " << tbl.at_path( "animals.cats" ) << "\n";
    std::cout << "fish[1]: " << tbl.at_path( "animals.fish[1]" ) << "\n";

    // ...even if the element doesn't exist
    std::cout << "dinosaurs: " << tbl["animals"]["dinosaurs"]
              << "\n";  // no dinosaurs :(

    return 0;
}
