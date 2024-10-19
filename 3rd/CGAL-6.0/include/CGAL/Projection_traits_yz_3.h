// Copyright (c) 1997-2010  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/v6.0/Kernel_23/include/CGAL/Projection_traits_yz_3.h $
// $Id: include/CGAL/Projection_traits_yz_3.h 50219fc33bc $
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Mariette Yvinec

#ifndef CGAL_PROJECTION_TRAITS_YZ_3_H
#define CGAL_PROJECTION_TRAITS_YZ_3_H

#include <CGAL/Kernel_23/internal/Projection_traits_3.h>

namespace CGAL {

template < class R >
class Projection_traits_yz_3
  : public internal::Projection_traits_3<R,0>
{};

} //namespace CGAL

#endif // CGAL_PROJECTION_TRAITS_YZ_3_H
