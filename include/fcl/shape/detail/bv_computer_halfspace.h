/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011-2014, Willow Garage, Inc.
 *  Copyright (c) 2014-2016, Open Source Robotics Foundation
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Open Source Robotics Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/** \author Jia Pan */

#ifndef FCL_SHAPE_DETAIL_BVCOMPUTERHALFSPACE_H
#define FCL_SHAPE_DETAIL_BVCOMPUTERHALFSPACE_H

#include "fcl/BV/AABB.h"
#include "fcl/BV/OBB.h"
#include "fcl/BV/RSS.h"
#include "fcl/BV/OBBRSS.h"
#include "fcl/BV/kDOP.h"
#include "fcl/BV/kIOS.h"

namespace fcl
{
namespace detail
{

template <typename ScalarT>
struct BVComputer<ScalarT, AABB<ScalarT>, Halfspace<ScalarT>>;

template <typename ScalarT>
struct BVComputer<ScalarT, OBB<ScalarT>, Halfspace<ScalarT>>;

template <typename ScalarT>
struct BVComputer<ScalarT, RSS<ScalarT>, Halfspace<ScalarT>>;

template <typename ScalarT>
struct BVComputer<ScalarT, OBBRSS<ScalarT>, Halfspace<ScalarT>>;

template <typename ScalarT>
struct BVComputer<ScalarT, kIOS<ScalarT>, Halfspace<ScalarT>>;

template <typename ScalarT>
struct BVComputer<ScalarT, KDOP<ScalarT, 16>, Halfspace<ScalarT>>;

template <typename ScalarT>
struct BVComputer<ScalarT, KDOP<ScalarT, 18>, Halfspace<ScalarT>>;

template <typename ScalarT>
struct BVComputer<ScalarT, KDOP<ScalarT, 24>, Halfspace<ScalarT>>;

//============================================================================//
//                                                                            //
//                              Implementations                               //
//                                                                            //
//============================================================================//

//==============================================================================
template <typename ScalarT>
struct BVComputer<ScalarT, AABB<ScalarT>, Halfspace<ScalarT>>
{
  static void compute(const Halfspace<ScalarT>& s, const Transform3<ScalarT>& tf, AABB<ScalarT>& bv)
  {
    Halfspace<ScalarT> new_s = transform(s, tf);
    const Vector3<ScalarT>& n = new_s.n;
    const ScalarT& d = new_s.d;

    AABB<ScalarT> bv_;
    bv_.min_ = Vector3<ScalarT>::Constant(-std::numeric_limits<ScalarT>::max());
    bv_.max_ = Vector3<ScalarT>::Constant(std::numeric_limits<ScalarT>::max());
    if(n[1] == (ScalarT)0.0 && n[2] == (ScalarT)0.0)
    {
      // normal aligned with x axis
      if(n[0] < 0) bv_.min_[0] = -d;
      else if(n[0] > 0) bv_.max_[0] = d;
    }
    else if(n[0] == (ScalarT)0.0 && n[2] == (ScalarT)0.0)
    {
      // normal aligned with y axis
      if(n[1] < 0) bv_.min_[1] = -d;
      else if(n[1] > 0) bv_.max_[1] = d;
    }
    else if(n[0] == (ScalarT)0.0 && n[1] == (ScalarT)0.0)
    {
      // normal aligned with z axis
      if(n[2] < 0) bv_.min_[2] = -d;
      else if(n[2] > 0) bv_.max_[2] = d;
    }

    bv = bv_;
  }
};

//==============================================================================
template <typename ScalarT>
struct BVComputer<ScalarT, OBB<ScalarT>, Halfspace<ScalarT>>
{
  static void compute(const Halfspace<ScalarT>& s, const Transform3<ScalarT>& tf, OBB<ScalarT>& bv)
  {
    /// Half space can only have very rough OBB<ScalarT>
    bv.axis.setIdentity();
    bv.To.setZero();
    bv.extent.setConstant(std::numeric_limits<ScalarT>::max());
  }
};

//==============================================================================
template <typename ScalarT>
struct BVComputer<ScalarT, RSS<ScalarT>, Halfspace<ScalarT>>
{
  static void compute(const Halfspace<ScalarT>& s, const Transform3<ScalarT>& tf, RSS<ScalarT>& bv)
  {
    /// Half space can only have very rough RSS<ScalarT>
    bv.axis.setIdentity();
    bv.Tr.setZero();
    bv.l[0] = bv.l[1] = bv.r = std::numeric_limits<ScalarT>::max();
  }
};

//==============================================================================
template <typename ScalarT>
struct BVComputer<ScalarT, OBBRSS<ScalarT>, Halfspace<ScalarT>>
{
  static void compute(const Halfspace<ScalarT>& s, const Transform3<ScalarT>& tf, OBBRSS<ScalarT>& bv)
  {
    computeBV(s, tf, bv.obb);
    computeBV(s, tf, bv.rss);
  }
};

//==============================================================================
template <typename ScalarT>
struct BVComputer<ScalarT, kIOS<ScalarT>, Halfspace<ScalarT>>
{
  static void compute(const Halfspace<ScalarT>& s, const Transform3<ScalarT>& tf, kIOS<ScalarT>& bv)
  {
    bv.num_spheres = 1;
    computeBV(s, tf, bv.obb);
    bv.spheres[0].o.setZero();
    bv.spheres[0].r = std::numeric_limits<ScalarT>::max();
  }
};

//==============================================================================
template <typename ScalarT>
struct BVComputer<ScalarT, KDOP<ScalarT, 16>, Halfspace<ScalarT>>
{
  static void compute(const Halfspace<ScalarT>& s, const Transform3<ScalarT>& tf, KDOP<ScalarT, 16>& bv)
  {
    Halfspace<ScalarT> new_s = transform(s, tf);
    const Vector3<ScalarT>& n = new_s.n;
    const ScalarT& d = new_s.d;

    const std::size_t D = 8;
    for(std::size_t i = 0; i < D; ++i)
      bv.dist(i) = -std::numeric_limits<ScalarT>::max();
    for(std::size_t i = D; i < 2 * D; ++i)
      bv.dist(i) = std::numeric_limits<ScalarT>::max();

    if(n[1] == (ScalarT)0.0 && n[2] == (ScalarT)0.0)
    {
      if(n[0] > 0) bv.dist(D) = d;
      else bv.dist(0) = -d;
    }
    else if(n[0] == (ScalarT)0.0 && n[2] == (ScalarT)0.0)
    {
      if(n[1] > 0) bv.dist(D + 1) = d;
      else bv.dist(1) = -d;
    }
    else if(n[0] == (ScalarT)0.0 && n[1] == (ScalarT)0.0)
    {
      if(n[2] > 0) bv.dist(D + 2) = d;
      else bv.dist(2) = -d;
    }
    else if(n[2] == (ScalarT)0.0 && n[0] == n[1])
    {
      if(n[0] > 0) bv.dist(D + 3) = n[0] * d * 2;
      else bv.dist(3) = n[0] * d * 2;
    }
    else if(n[1] == (ScalarT)0.0 && n[0] == n[2])
    {
      if(n[1] > 0) bv.dist(D + 4) = n[0] * d * 2;
      else bv.dist(4) = n[0] * d * 2;
    }
    else if(n[0] == (ScalarT)0.0 && n[1] == n[2])
    {
      if(n[1] > 0) bv.dist(D + 5) = n[1] * d * 2;
      else bv.dist(5) = n[1] * d * 2;
    }
    else if(n[2] == (ScalarT)0.0 && n[0] + n[1] == (ScalarT)0.0)
    {
      if(n[0] > 0) bv.dist(D + 6) = n[0] * d * 2;
      else bv.dist(6) = n[0] * d * 2;
    }
    else if(n[1] == (ScalarT)0.0 && n[0] + n[2] == (ScalarT)0.0)
    {
      if(n[0] > 0) bv.dist(D + 7) = n[0] * d * 2;
      else bv.dist(7) = n[0] * d * 2;
    }
  }
};

//==============================================================================
template <typename ScalarT>
struct BVComputer<ScalarT, KDOP<ScalarT, 18>, Halfspace<ScalarT>>
{
  static void compute(const Halfspace<ScalarT>& s, const Transform3<ScalarT>& tf, KDOP<ScalarT, 18>& bv)
  {
    Halfspace<ScalarT> new_s = transform(s, tf);
    const Vector3<ScalarT>& n = new_s.n;
    const ScalarT& d = new_s.d;

    const std::size_t D = 9;

    for(std::size_t i = 0; i < D; ++i)
      bv.dist(i) = -std::numeric_limits<ScalarT>::max();
    for(std::size_t i = D; i < 2 * D; ++i)
      bv.dist(i) = std::numeric_limits<ScalarT>::max();

    if(n[1] == (ScalarT)0.0 && n[2] == (ScalarT)0.0)
    {
      if(n[0] > 0) bv.dist(D) = d;
      else bv.dist(0) = -d;
    }
    else if(n[0] == (ScalarT)0.0 && n[2] == (ScalarT)0.0)
    {
      if(n[1] > 0) bv.dist(D + 1) = d;
      else bv.dist(1) = -d;
    }
    else if(n[0] == (ScalarT)0.0 && n[1] == (ScalarT)0.0)
    {
      if(n[2] > 0) bv.dist(D + 2) = d;
      else bv.dist(2) = -d;
    }
    else if(n[2] == (ScalarT)0.0 && n[0] == n[1])
    {
      if(n[0] > 0) bv.dist(D + 3) = n[0] * d * 2;
      else bv.dist(3) = n[0] * d * 2;
    }
    else if(n[1] == (ScalarT)0.0 && n[0] == n[2])
    {
      if(n[1] > 0) bv.dist(D + 4) = n[0] * d * 2;
      else bv.dist(4) = n[0] * d * 2;
    }
    else if(n[0] == (ScalarT)0.0 && n[1] == n[2])
    {
      if(n[1] > 0) bv.dist(D + 5) = n[1] * d * 2;
      else bv.dist(5) = n[1] * d * 2;
    }
    else if(n[2] == (ScalarT)0.0 && n[0] + n[1] == (ScalarT)0.0)
    {
      if(n[0] > 0) bv.dist(D + 6) = n[0] * d * 2;
      else bv.dist(6) = n[0] * d * 2;
    }
    else if(n[1] == (ScalarT)0.0 && n[0] + n[2] == (ScalarT)0.0)
    {
      if(n[0] > 0) bv.dist(D + 7) = n[0] * d * 2;
      else bv.dist(7) = n[0] * d * 2;
    }
    else if(n[0] == (ScalarT)0.0 && n[1] + n[2] == (ScalarT)0.0)
    {
      if(n[1] > 0) bv.dist(D + 8) = n[1] * d * 2;
      else bv.dist(8) = n[1] * d * 2;
    }
  }
};

//==============================================================================
template <typename ScalarT>
struct BVComputer<ScalarT, KDOP<ScalarT, 24>, Halfspace<ScalarT>>
{
  static void compute(const Halfspace<ScalarT>& s, const Transform3<ScalarT>& tf, KDOP<ScalarT, 24>& bv)
  {
    Halfspace<ScalarT> new_s = transform(s, tf);
    const Vector3<ScalarT>& n = new_s.n;
    const ScalarT& d = new_s.d;

    const std::size_t D = 12;

    for(std::size_t i = 0; i < D; ++i)
      bv.dist(i) = -std::numeric_limits<ScalarT>::max();
    for(std::size_t i = D; i < 2 * D; ++i)
      bv.dist(i) = std::numeric_limits<ScalarT>::max();

    if(n[1] == (ScalarT)0.0 && n[2] == (ScalarT)0.0)
    {
      if(n[0] > 0) bv.dist(D) = d;
      else bv.dist(0) = -d;
    }
    else if(n[0] == (ScalarT)0.0 && n[2] == (ScalarT)0.0)
    {
      if(n[1] > 0) bv.dist(D + 1) = d;
      else bv.dist(1) = -d;
    }
    else if(n[0] == (ScalarT)0.0 && n[1] == (ScalarT)0.0)
    {
      if(n[2] > 0) bv.dist(D + 2) = d;
      else bv.dist(2) = -d;
    }
    else if(n[2] == (ScalarT)0.0 && n[0] == n[1])
    {
      if(n[0] > 0) bv.dist(D + 3) = n[0] * d * 2;
      else bv.dist(3) = n[0] * d * 2;
    }
    else if(n[1] == (ScalarT)0.0 && n[0] == n[2])
    {
      if(n[1] > 0) bv.dist(D + 4) = n[0] * d * 2;
      else bv.dist(4) = n[0] * d * 2;
    }
    else if(n[0] == (ScalarT)0.0 && n[1] == n[2])
    {
      if(n[1] > 0) bv.dist(D + 5) = n[1] * d * 2;
      else bv.dist(5) = n[1] * d * 2;
    }
    else if(n[2] == (ScalarT)0.0 && n[0] + n[1] == (ScalarT)0.0)
    {
      if(n[0] > 0) bv.dist(D + 6) = n[0] * d * 2;
      else bv.dist(6) = n[0] * d * 2;
    }
    else if(n[1] == (ScalarT)0.0 && n[0] + n[2] == (ScalarT)0.0)
    {
      if(n[0] > 0) bv.dist(D + 7) = n[0] * d * 2;
      else bv.dist(7) = n[0] * d * 2;
    }
    else if(n[0] == (ScalarT)0.0 && n[1] + n[2] == (ScalarT)0.0)
    {
      if(n[1] > 0) bv.dist(D + 8) = n[1] * d * 2;
      else bv.dist(8) = n[1] * d * 2;
    }
    else if(n[0] + n[2] == (ScalarT)0.0 && n[0] + n[1] == (ScalarT)0.0)
    {
      if(n[0] > 0) bv.dist(D + 9) = n[0] * d * 3;
      else bv.dist(9) = n[0] * d * 3;
    }
    else if(n[0] + n[1] == (ScalarT)0.0 && n[1] + n[2] == (ScalarT)0.0)
    {
      if(n[0] > 0) bv.dist(D + 10) = n[0] * d * 3;
      else bv.dist(10) = n[0] * d * 3;
    }
    else if(n[0] + n[1] == (ScalarT)0.0 && n[0] + n[2] == (ScalarT)0.0)
    {
      if(n[1] > 0) bv.dist(D + 11) = n[1] * d * 3;
      else bv.dist(11) = n[1] * d * 3;
    }
  }
};

} // namespace detail
} // namespace fcl

#endif
