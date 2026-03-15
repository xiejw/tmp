#include "dsl.h"

#include <cstring>
#include <cstdio>

namespace forge {

namespace {

static void StripComment(char *line) {
  bool in_quote = false;
  for (int i = 0; line[i] != '\0'; i++) {
    if (line[i] == '"') {
      in_quote = !in_quote;
    } else if (!in_quote && line[i] == '/' && line[i + 1] == '/') {
      line[i] = '\0';
      return;
    }
  }
}

static void TrimInPlace(char *s) {
  // Trim leading whitespace
  int start = 0;
  while (s[start] == ' ' || s[start] == '\t') start++;
  if (start > 0) {
    int i = 0;
    while (s[start + i] != '\0') {
      s[i] = s[start + i];
      i++;
    }
    s[i] = '\0';
  }
  // Trim trailing whitespace
  int len = (int)strlen(s);
  while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' ||
                     s[len - 1] == '\n' || s[len - 1] == '\r')) {
    s[--len] = '\0';
  }
}

static FlagSpec *DefFindSpec(FlagDef *def, const char *name) {
  for (int i = 0; i < def->count; i++) {
    if (strcmp(def->specs[i].name, name) == 0) {
      return &def->specs[i];
    }
  }
  return nullptr;
}

} // namespace

bool FlagDefParseDsl(FlagDef *def, const char *dsl_text, std::string *err_msg) {
  def->count = 0;

  // Make a mutable copy for line-by-line parsing
  int text_len = (int)strlen(dsl_text);
  char *buf = new char[text_len + 1];
  memcpy(buf, dsl_text, text_len + 1);

  enum State { OUTSIDE, INSIDE };
  State state = OUTSIDE;

  FlagSpec *cur_spec = nullptr;
  bool has_type = false;

  int line_num = 0;
  char *line = buf;

  while (line < buf + text_len) {
    // Find end of line
    char *end = line;
    while (*end != '\0' && *end != '\n') end++;
    char saved = *end;
    *end = '\0';
    line_num++;

    StripComment(line);
    TrimInPlace(line);

    if (line[0] != '\0') {
      if (state == OUTSIDE) {
        char name[64];
        char brace[4];
        int matched = sscanf(line, "%63s %3s", name, brace);
        if (matched == 2 && strcmp(brace, "{") == 0) {
          // Check for duplicate
          if (DefFindSpec(def, name) != nullptr) {
            *err_msg = std::string("duplicate flag: ") + name;
            delete[] buf;
            return true;
          }
          if (def->count >= kMaxFlags) {
            *err_msg = "too many flags (max 64)";
            delete[] buf;
            return true;
          }
          cur_spec = &def->specs[def->count];
          memset(cur_spec, 0, sizeof(FlagSpec));
          snprintf(cur_spec->name, sizeof(cur_spec->name), "%s", name);
          has_type = false;
          state = INSIDE;
        } else if (line[0] != '\0') {
          *err_msg = std::string("unexpected line outside block: ") + line;
          delete[] buf;
          return true;
        }
      } else {
        // INSIDE state
        if (strcmp(line, "}") == 0) {
          def->count++;
          cur_spec = nullptr;
          state = OUTSIDE;
        } else {
          // Split on first ':'
          char *colon = strchr(line, ':');
          if (colon == nullptr) {
            *err_msg = std::string("expected 'key: value' but got: ") + line;
            delete[] buf;
            return true;
          }
          *colon = '\0';
          char *key = line;
          char *val = colon + 1;
          TrimInPlace(key);
          TrimInPlace(val);

          if (strcmp(key, "type") == 0) {
            if (strcmp(val, "string") == 0) {
              cur_spec->type = FLAG_TYPE_STRING;
            } else if (strcmp(val, "int") == 0) {
              cur_spec->type = FLAG_TYPE_INT;
            } else if (strcmp(val, "long") == 0) {
              cur_spec->type = FLAG_TYPE_LONG;
            } else {
              *err_msg = std::string("unknown type: ") + val;
              delete[] buf;
              return true;
            }
            has_type = true;
          } else if (strcmp(key, "required") == 0) {
            if (strcmp(val, "true") == 0) {
              cur_spec->required = true;
            } else if (strcmp(val, "false") == 0) {
              cur_spec->required = false;
            } else {
              *err_msg = std::string("invalid value for required: ") + val;
              delete[] buf;
              return true;
            }
          } else if (strcmp(key, "default_value") == 0) {
            if (!has_type) {
              *err_msg = "default_value requires type to be declared first";
              delete[] buf;
              return true;
            }
            if (cur_spec->type == FLAG_TYPE_STRING) {
              // Strip surrounding quotes
              int vlen = (int)strlen(val);
              if (vlen >= 2 && val[0] == '"' && val[vlen - 1] == '"') {
                val[vlen - 1] = '\0';
                val++;
              }
            }
            snprintf(cur_spec->default_str, sizeof(cur_spec->default_str), "%s", val);
            cur_spec->has_default = true;
          } else {
            *err_msg = std::string("unknown key: ") + key;
            delete[] buf;
            return true;
          }
        }
      }
    }

    if (saved == '\0') break;
    line = end + 1;
  }

  delete[] buf;

  if (state == INSIDE) {
    *err_msg = std::string("unclosed block for '") + cur_spec->name + "'";
    return true;
  }

  return false;
}

} // namespace forge
