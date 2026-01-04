#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace fmt {

// Formatting specification for floats
struct FormatSpec {
    int  width     = 0;
    int  precision = -1;
    char align     = '>';  // default right
};

// Parse inside braces "{:...}"
FormatSpec
parse_spec( const std::string &spec )
{
    FormatSpec  fs;
    std::size_t pos = 0;

    // alignment
    if ( !spec.empty( ) && ( spec[0] == '<' || spec[0] == '>' ) ) {
        fs.align = spec[0];
        pos++;
    }

    // width
    while ( pos < spec.size( ) && isdigit( spec[pos] ) ) {
        fs.width = fs.width * 10 + ( spec[pos] - '0' );
        pos++;
    }

    // precision
    if ( pos < spec.size( ) && spec[pos] == '.' ) {
        pos++;
        fs.precision = 0;
        while ( pos < spec.size( ) && isdigit( spec[pos] ) ) {
            fs.precision = fs.precision * 10 + ( spec[pos] - '0' );
            pos++;
        }
    }

    return fs;
}

// Format a float value according to spec
inline std::string
format_float( double value, const FormatSpec &fs )
{
    std::ostringstream oss;

    if ( fs.align == '<' )
        oss << std::left;
    else if ( fs.align == '>' )
        oss << std::right;

    if ( fs.precision >= 0 )
        oss << std::fixed << std::setprecision( fs.precision );
    if ( fs.width > 0 ) oss << std::setw( fs.width );

    oss << value;
    return oss.str( );
}

// Generic argument formatting
template <typename T>
std::string
format_arg( const std::string &spec, T &&value )
{
    if constexpr ( std::is_floating_point<
                       typename std::decay<T>::type>::value ) {
        FormatSpec fs = parse_spec( spec );
        return format_float( value, fs );
    } else {
        // Non-floats: ignore spec, just stream
        std::ostringstream oss;
        oss << value;
        return oss.str( );
    }
}

// Base case
inline void
vprint( std::ostream &os, const std::string &fmt )
{
    os << fmt;
}

// Recursive variadic template
template <typename T, typename... Args>
void
vprint( std::ostream &os, const std::string &fmt, T &&value, Args &&...args )
{
    std::size_t open  = fmt.find( '{' );
    std::size_t close = fmt.find( '}', open );

    if ( open == std::string::npos || close == std::string::npos ) {
        throw std::runtime_error( "Mismatched braces in format string" );
    }

    // Print text before '{'
    os << fmt.substr( 0, open );

    // Extract format spec inside '{:...}'
    std::string inside = fmt.substr( open + 1, close - open - 1 );
    std::string spec;
    if ( !inside.empty( ) && inside[0] == ':' ) spec = inside.substr( 1 );

    os << format_arg( spec, std::forward<T>( value ) );

    // Recurse with the rest
    vprint( os, fmt.substr( close + 1 ), std::forward<Args>( args )... );
}

// Public interface
template <typename... Args>
void
print( const std::string &fmt, Args &&...args )
{
    vprint( std::cout, fmt, std::forward<Args>( args )... );
}

}  // namespace fmt

// -----------------
// Example usage
// -----------------
int
main( )
{
    using fmt::print;

    print( "Default                 : `{}`\n", 3.14159 );
    print( "Precision (2)           : `{:.2f}`\n", 3.14159 );
    print( "Precision (2f)          : `{:.2f}`\n", 3.14159f );
    print( "Width 10.2 + right align: `{:>10.2f}`\n", 3.14159 );
    print( "Width 10.2 + left align : `{:<10.2f}`\n", 3.14159 );
    print( "Mix types: str={} int={} float(3)={:.3f}\n", "hello", 42, 2.71828 );
    return 0;
}
