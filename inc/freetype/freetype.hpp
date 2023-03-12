// Copyright (c) 2023 Jacob R. Green
// All Rights Reserved.

#pragma once

#include "freetype/freetype.h"

#include <filesystem>
#include <stdexcept>
#include <string>
#include <strstream>

namespace ft {
using CharCode = FT_ULong;

using GlyphMetrics = FT_Glyph_Metrics;
using Bitmap = FT_Bitmap;

constexpr auto default_dpi = 96;

class Exception : public std::runtime_error {
 public:
  explicit Exception(FT_Error error)
      : std::runtime_error(FT_Error_String(error)) {}
};

class InvalidCharCodeException : public Exception {
  CharCode _code;

 public:
  explicit InvalidCharCodeException(CharCode code)
      : Exception(FT_Err_Invalid_Character_Code), _code{code} {}

  auto code() const { return _code; }
};

class Glyph final {
  FT_GlyphSlot _glyph;

 public:
  explicit Glyph(FT_GlyphSlot slot) : _glyph{slot} {}
  Glyph(Glyph&&) = default;
  Glyph(const Glyph&) = delete;

  auto& metrics() const { return _glyph->metrics; }

  auto& bitmap() const { return _glyph->bitmap; }

  auto bitmap_left() const { return _glyph->bitmap_left; }

  auto bitmap_top() const { return _glyph->bitmap_top; }

  auto render(
      FT_Render_Mode renderMode = FT_Render_Mode::FT_RENDER_MODE_NORMAL) {
    if (const auto error = FT_Render_Glyph(_glyph, renderMode); error) {
      throw Exception{error};
    }
  }
};

class Face final {
  FT_Face _face;

 public:
  Face(FT_Face face) : _face{face} {}
  Face(Face&&) = default;
  Face(const Face&) = delete;
  ~Face() {
    if (const auto error = FT_Done_Face(_face); error) {
      // throw Exception{ error };
    }
  }

  auto set_char_size(uint32_t width, uint32_t height, uint32_t horz_res,
                     uint32_t vert_res) {
    if (const auto error =
            FT_Set_Char_Size(_face, width, height, horz_res, vert_res);
        error) {
      throw Exception{error};
    }
  }

  auto set_char_size(uint32_t pt, uint32_t dpi = default_dpi) {
    set_char_size(0, pt * 64, 0, dpi);
  }

  [[nodiscard]] auto get_char_index(CharCode code) const {
    const auto index = FT_Get_Char_Index(_face, code);
    if (!index) {
      throw InvalidCharCodeException{code};
    }

    return index;
  }

  auto load_glyph(FT_UInt32 index, FT_UInt32 flags = FT_LOAD_DEFAULT) -> Glyph {
    if (const auto error = FT_Load_Glyph(_face, index, flags); error) {
      throw Exception{error};
    }

    return Glyph{_face->glyph};
  }

  const auto& metrics() const { return _face->size->metrics; }
};

class Library final {
  FT_Library _library;

  [[nodiscard]] static auto create_library() -> FT_Library {
    auto lib = FT_Library{};
    if (const auto error = FT_Init_FreeType(&lib); error) {
      throw Exception{error};
    }
    return lib;
  }

 public:
  Library() : _library{create_library()} {}
  Library(Library&&) = default;
  Library(const Library&) = delete;

  ~Library() {
    if (const auto error = FT_Done_FreeType(_library); error) {
      // throw Exception{ error };
    }
  }

  [[nodiscard]] auto new_face(const char* path, FT_Long index = 0) -> Face {
    FT_Face face;
    if (const auto error = FT_New_Face(_library, path, index, &face); error) {
      throw Exception{error};
    }

    return Face{face};
  }

  [[nodiscard]] auto new_face(const std::string& path, FT_Long index = 0)
      -> Face {
    return new_face(path.c_str(), index);
  }

  [[nodiscard]] auto new_face(const std::filesystem::path& path,
                              FT_Long index = 0) -> Face {
    return new_face(path.string(), index);
  }
};

}  // namespace ft