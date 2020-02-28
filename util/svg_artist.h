#pragma once
// Copyright (C) 2018 Josh Bialkowski (josh.bialkowski@gmail.com)

/**
 *  @file
 *  @date   31 October 2018
 *  @author Josh Bialkowski (josh.bialkowski@gmail.com)
 *  @brief  utils and helpers for rendering triangulations
 */

#include <list>
#include <string>

#include <fmt/format.h>
#include <Eigen/Dense>

namespace svg_artist {
extern const char* kPathPrefix;
extern const char* kPathSuffix;
extern const char* kDotFormat;
}  // namespace svg_artist

struct StyleOpts {
  std::string fill_color;
  double fill_opacity;
  std::string stroke_color;
  double stroke_width;
  std::string stroke_linecap;
  std::string stroke_linejoin;
  double stroke_miterlimit;
  double stroke_opacity;
  double dot_radius;

  StyleOpts();
  StyleOpts(uint32_t scale);  // NOLINT
  std::string to_string() const;
};

class SvgArtist {
 public:
  class StyleScope {
   public:
    StyleScope(SvgArtist* artist, const StyleOpts& style);
    ~StyleScope();

   private:
    SvgArtist* artist_;
  };

  SvgArtist(std::ostream* out, uint32_t scale);
  ~SvgArtist();

  template <class Derived>
  void draw_dot(const Eigen::MatrixBase<Derived>& x);

  template <class Derived>
  void draw_edge(const Eigen::MatrixBase<Derived>& x0,
                 const Eigen::MatrixBase<Derived>& x1);

  template <class Derived>
  void draw_triangle(const Eigen::MatrixBase<Derived>& x0,
                     const Eigen::MatrixBase<Derived>& x1,
                     const Eigen::MatrixBase<Derived>& x2);

  template <class Derived>
  void draw_dilated_triangle(const Eigen::MatrixBase<Derived>& x0,
                             const Eigen::MatrixBase<Derived>& x1,
                             const Eigen::MatrixBase<Derived>& x2);

  template <class Derived>
  void draw_quad(const Eigen::MatrixBase<Derived>& x0,
                 const Eigen::MatrixBase<Derived>& x1,
                 const Eigen::MatrixBase<Derived>& x2,
                 const Eigen::MatrixBase<Derived>& x3);

  template <class Derived>
  void draw_dilated_quad(const Eigen::MatrixBase<Derived>& x0,
                         const Eigen::MatrixBase<Derived>& x1,
                         const Eigen::MatrixBase<Derived>& x2,
                         const Eigen::MatrixBase<Derived>& x3);

  StyleOpts& push_style();
  void push_style(const StyleOpts& opts);
  void pop_style();

  void start_path();
  void end_path();

  std::ostream* out() {
    return out_;
  }

  uint32_t scale() const {
    return scale_;
  }

 private:
  const StyleOpts& get_style() const;

  uint32_t scale_;     ///< max dimension of the drawing
  uint32_t next_id_;   ///< net unused id
  std::ostream* out_;  ///< output stream
  std::list<StyleOpts> style_stack_;
  StyleOpts default_style_;
};

// -----------------------------------------------------------------------------
//                    Template Implementations
// -----------------------------------------------------------------------------

template <class Derived>
void SvgArtist::draw_dot(const Eigen::MatrixBase<Derived>& x) {
  using svg_artist::kDotFormat;
  (*out_) << fmt::format(kDotFormat, get_style().dot_radius, x[0], x[1],
                         next_id_++, get_style().to_string());
}

template <class Derived>
void SvgArtist::draw_edge(const Eigen::MatrixBase<Derived>& x0,
                          const Eigen::MatrixBase<Derived>& x1) {
  using svg_artist::kPathPrefix;
  using svg_artist::kPathSuffix;
  (*out_) << fmt::format(kPathPrefix, next_id_++);
  (*out_) << fmt::format("M {:6.2f},{:6.2f}", x0[0], x0[1]);
  (*out_) << fmt::format(" L {:6.2f},{:6.2f}", x1[0], x1[1]);
  (*out_) << fmt::format(kPathSuffix, get_style().to_string());
}

/// Return the point on the ray from xc to x0 which intersects the line which
/// is parallel to (x0, x1) but `dilation` units closer to xc
/**
 * The idea is that x0 and x1 are two verticess of a triangle and xc is the
 * geometric center of all three vertices. If we were to dilate the triangle
 * so that all faces were `dilation` units closer to the geometric center,
 * then the returned vertex is the vertex of the dilated triangle corresponding
 * to the x0.
 */
template <class Derived>
Eigen::Matrix<double, 2, 1> dilate_one(const Eigen::MatrixBase<Derived>& x0,
                                       const Eigen::MatrixBase<Derived>& x1,
                                       const Eigen::MatrixBase<Derived>& x2,
                                       double dilation) {
  const Eigen::Vector2d dx1 = (x1 - x0);
  const Eigen::Vector2d dx2 = (x2 - x0);
  const Eigen::Vector2d dx1n = dx1.normalized();
  const Eigen::Vector2d dx2n = dx2.normalized();
  const Eigen::Vector2d rayn = (dx1n + dx2n).normalized();

  const Eigen::Vector3d v1(dx1n[0], dx1n[1], 0);
  const Eigen::Vector3d v2(rayn[0], rayn[1], 0);
  const Eigen::Vector3d vx = v1.cross(v2);
  const double sin_theta = vx[2];

  // double cross_product = (-dx1[1] * dxc[0]) + (dx1[0] * dxc[1]);
  double radius = std::abs(dilation / sin_theta);
  // if (radius > ray.norm()) {
  //   radius = ray.norm() / 2.0;
  // }
  return x0 + (rayn * radius);
}

template <class Derived>
void SvgArtist::draw_triangle(const Eigen::MatrixBase<Derived>& x0,
                              const Eigen::MatrixBase<Derived>& x1,
                              const Eigen::MatrixBase<Derived>& x2) {
  using svg_artist::kPathPrefix;
  using svg_artist::kPathSuffix;

  (*out_) << fmt::format(kPathPrefix, next_id_++);
  (*out_) << fmt::format("M {:6.2f},{:6.2f}", x0[0], x0[1]);
  (*out_) << fmt::format(" L {:6.2f},{:6.2f}", x1[0], x1[1]);
  (*out_) << fmt::format(" L {:6.2f},{:6.2f}", x2[0], x2[1]);
  (*out_) << "Z";
  (*out_) << fmt::format(kPathSuffix, get_style().to_string());
}

template <class Derived>
void SvgArtist::draw_dilated_triangle(const Eigen::MatrixBase<Derived>& x0,
                                      const Eigen::MatrixBase<Derived>& x1,
                                      const Eigen::MatrixBase<Derived>& x2) {
  const double dilation = 1.0 * scale_ / 200;
  Eigen::Vector2d x0p = dilate_one(x0, x1, x2, dilation);
  Eigen::Vector2d x1p = dilate_one(x1, x2, x0, dilation);
  Eigen::Vector2d x2p = dilate_one(x2, x0, x1, dilation);
  draw_triangle(x0p, x1p, x2p);
}

template <class Derived>
void SvgArtist::draw_quad(const Eigen::MatrixBase<Derived>& x0,
                          const Eigen::MatrixBase<Derived>& x1,
                          const Eigen::MatrixBase<Derived>& x2,
                          const Eigen::MatrixBase<Derived>& x3) {
  using svg_artist::kPathPrefix;
  using svg_artist::kPathSuffix;

  (*out_) << fmt::format(kPathPrefix, next_id_++);
  (*out_) << fmt::format("M {:6.2f},{:6.2f}", x0[0], x0[1]);
  (*out_) << fmt::format(" L {:6.2f},{:6.2f}", x1[0], x1[1]);
  (*out_) << fmt::format(" L {:6.2f},{:6.2f}", x2[0], x2[1]);
  (*out_) << fmt::format(" L {:6.2f},{:6.2f}", x3[0], x3[1]);
  // NOTE(josh): don't close the quad because really it's an infinite simplex
  // (*out_) << "Z";
  (*out_) << fmt::format(kPathSuffix, get_style().to_string());
}

template <class Derived>
void SvgArtist::draw_dilated_quad(const Eigen::MatrixBase<Derived>& x0,
                                  const Eigen::MatrixBase<Derived>& x1,
                                  const Eigen::MatrixBase<Derived>& x2,
                                  const Eigen::MatrixBase<Derived>& x3) {
  const double dilation = 1.0 * scale_ / 200;
  // NOTE(josh): dilate_one order of linputs is
  // (target, vertex-left, vertex-right)
  Eigen::Vector2d x0p = dilate_one(x0, x1, x3, dilation);
  Eigen::Vector2d x1p = dilate_one(x1, x2, x0, dilation);
  Eigen::Vector2d x2p = dilate_one(x2, x3, x1, dilation);
  Eigen::Vector2d x3p = dilate_one(x3, x0, x2, dilation);
  draw_quad(x0p, x1p, x2p, x3p);
}
