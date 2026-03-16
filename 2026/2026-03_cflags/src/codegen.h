#pragma once
#include <cstdio>
#include <string>

#include "dsl.h"

namespace forge {

// Write generated header to out_h, source to out_cc.
// basename is used for the #include directive (e.g. "my_flags").
// Returns true on error.
bool CodegenWrite( const FlagDef *def, const char *basename, FILE *out_h,
                   FILE *out_cc, std::string *err_msg );

}  // namespace forge
