// Copyright (C) 2018 Josh Bialkowski (josh.bialkowski@gmail.com)

/**
 *  @file
 *  @date   31 October 2018
 *  @author Josh Bialkowski (josh.bialkowski@gmail.com)
 *  @brief  utils and helpers for rendering triangulations
 */

#include "util/svg_artist.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace svg_artist {
const char* kSvgHeader =
    "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
    "<svg\n"
    "   xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n"
    "   xmlns:cc=\"http://creativecommons.org/ns#\"\n"
    "   xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"\n"
    "   xmlns:svg=\"http://www.w3.org/2000/svg\"\n"
    "   xmlns=\"http://www.w3.org/2000/svg\"\n"
    "   version=\"1.1\"\n"
    "   id=\"svg2\"\n"
    "   viewBox=\"0 0 {0} {0}\"\n"
    "   height=\"{0}px\"\n"
    "   width=\"{0}px\">\n"
    "  <defs\n"
    "     id=\"defs4\" />\n"
    "  <metadata\n"
    "     id=\"metadata7\">\n"
    "    <rdf:RDF>\n"
    "      <cc:Work\n"
    "         rdf:about=\"\">\n"
    "        <dc:format>image/svg+xml</dc:format>\n"
    "        <dc:type\n"
    "           rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" />\n"
    "        <dc:title></dc:title>\n"
    "      </cc:Work>\n"
    "    </rdf:RDF>\n"
    "  </metadata>\n"
    "  <g id=\"layer1\">\n";

const char* kSvgFooter =
    "\n"
    "  </g>\n"
    "</svg>\n";

const char* kPathPrefix =
    "<path\n"
    "  id=\"path{:06d}\"\n"
    "  d=\"";

const char* kPathSuffix =
    " \"\n"
    "style=\"{}\" />\n";

const char* kDotFormat =
    "<circle\n"
    " r=\"{}\"\n"
    " cx=\"{:6.4f}\"\n"
    " cy=\"{:6.4f}\"\n"
    " id=\"point{}\"\n"
    " style=\"{}\" />\n";

const char* kStyleFormat =
    "fill:{};fill-opacity:{};stroke:{};stroke-width:{};"
    "stroke-linecap:{};stroke-linejoin:{};stroke-miterlimit:{};"
    "stroke-dasharray:none;stroke-opacity:{}";
}  // namespace svg_artist

StyleOpts::StyleOpts()
    : fill_color("none"),
      fill_opacity{1.0},
      stroke_color("#000000"),
      stroke_width{1.0},
      stroke_linecap("round"),
      stroke_linejoin("round"),
      stroke_miterlimit{4},
      stroke_opacity{1},
      dot_radius{1.0} {}

StyleOpts::StyleOpts(uint32_t scale)
    : fill_color("none"),
      fill_opacity{1.0},
      stroke_color("#000000"),
      stroke_width{1.0 * scale / 200},
      stroke_linecap("round"),
      stroke_linejoin("round"),
      stroke_miterlimit{4.0 * scale / 200},
      stroke_opacity{1},
      dot_radius{1.0 * scale / 100} {}

std::string StyleOpts::to_string() const {
  using svg_artist::kStyleFormat;
  return fmt::format(kStyleFormat, fill_color, fill_opacity, stroke_color,
                     stroke_width, stroke_linecap, stroke_linejoin,
                     stroke_miterlimit, stroke_opacity);
}

SvgArtist::StyleScope::StyleScope(SvgArtist* artist, const StyleOpts& style)
    : artist_(artist) {
  artist->push_style(style);
}

SvgArtist::StyleScope::~StyleScope() {
  artist_->pop_style();
}

SvgArtist::SvgArtist(std::ostream* out, uint32_t scale)
    : scale_{scale}, next_id_{0}, out_{out}, default_style_(scale) {
  using svg_artist::kSvgHeader;
  fmt::print(*out_, kSvgHeader, scale);
}

SvgArtist::~SvgArtist() {
  using svg_artist::kSvgFooter;
  fmt::print(*out_, kSvgFooter);
}

StyleOpts& SvgArtist::push_style() {
  style_stack_.push_back(get_style());
  return style_stack_.back();
}

void SvgArtist::push_style(const StyleOpts& style) {
  style_stack_.push_back(style);
}

void SvgArtist::pop_style() {
  style_stack_.pop_back();
}

const StyleOpts& SvgArtist::get_style() const {
  if (style_stack_.empty()) {
    return default_style_;
  } else {
    return style_stack_.back();
  }
}

void SvgArtist::start_path() {
  (*out_) << fmt::format(svg_artist::kPathPrefix, next_id_++);
}

void SvgArtist::end_path() {
  (*out_) << fmt::format(svg_artist::kPathSuffix, get_style().to_string());
}
