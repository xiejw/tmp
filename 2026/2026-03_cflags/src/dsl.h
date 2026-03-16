#pragma once
#include <string>

namespace forge {

// ── DSL limits
// ────────────────────────────────────────────────────────────────
static const int kMaxFlags        = 64;
static const int kFlagNameSize    = 64;   // FlagSpec::name buffer
static const int kFlagDefaultSize = 256;  // FlagSpec::default_str buffer

enum FlagType { FLAG_TYPE_STRING = 0, FLAG_TYPE_INT = 1, FLAG_TYPE_LONG = 2 };

struct FlagSpec {
    char     name[kFlagNameSize];
    FlagType type;
    bool     required;
    bool     has_default;
    char     default_str[kFlagDefaultSize];  // raw default string from DSL
                                             // (unquoted for string)
};

struct FlagDef {
    FlagSpec specs[kMaxFlags];
    int      count;
};

// Parse DSL text into FlagDef. Returns true on error.
bool FlagDefParseDsl( FlagDef *def, const char *dsl_text,
                      std::string *err_msg );

}  // namespace forge
