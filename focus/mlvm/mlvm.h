// vim: ft=cpp
#pragma once

#include <cassert>
#include <string_view>

// =============================================================================
namespace mlvm::base {

enum class ErrorCode { kOK, kInvalidInput };

template <class T>
struct ResultOr {
    ResultOr( ErrorCode ec )
    {
        assert( ec != ErrorCode::kOK );
        mEc = ec;
    }

    ResultOr( T t )
    {
        mEc = ErrorCode::kOK;
        mT  = t;
    }

    bool IsOk( ) { return mEc == ErrorCode::kOK; }

    T &Value( ) { return mT; }

  private:
    ErrorCode mEc;
    T         mT;
};

};  // namespace mlvm::base
// =============================================================================

// =============================================================================
namespace mlvm::internal {

enum class TokenKind {
    Id,
    Eq,    // =
    At,    // @
    Plus,  // +
    Semi,  // ;
    Eof,
};

struct Token {
    TokenKind        Kind;
    std::string_view Token;
};

struct Lexer {
    Lexer( std::string_view );
    base::ResultOr<Token> NextToken( );

  private:
    void SkipWhiteSpaces( );
};
}  // namespace mlvm::internal
// =============================================================================
