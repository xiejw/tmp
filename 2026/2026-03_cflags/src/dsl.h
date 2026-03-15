#pragma once
#include <string>

namespace forge {

enum FlagType { FLAG_TYPE_STRING = 0, FLAG_TYPE_INT = 1, FLAG_TYPE_LONG = 2 };

struct FlagSpec {
  char     name[64];
  FlagType type;
  bool     required;
  bool     has_default;
  char     default_str[256];  // raw default string from DSL (unquoted for string)
};

static const int kMaxFlags = 64;

struct FlagDef {
  FlagSpec specs[kMaxFlags];
  int      count;
};

// Parse DSL text into FlagDef. Returns true on error.
bool FlagDefParseDsl(FlagDef *def, const char *dsl_text, std::string *err_msg);

} // namespace forge
