// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.
//
// This file has been generated. Do not modify it by hand or your changes might
// be lost.
//
#include "CathodeSegmentationCreator.h"

namespace o2
{
namespace mch
{
namespace mapping
{
namespace impl4
{
CathodeSegmentation* createSegType20(bool isBendingPlane)
{
  if (isBendingPlane) {
    return new CathodeSegmentation{
      20,
      true,
      /* PG */
      { { 1, 1, 0, -80, -20 },   { 2, 5, 0, -70, -20 },
        { 3, 2, 0, -60, -20 },   { 103, 5, 0, 40, -20 },
        { 104, 5, 0, 60, -20 },  { 107, 5, 0, 0, -20 },
        { 108, 5, 0, 20, -20 },  { 111, 5, 0, -40, -20 },
        { 112, 5, 0, -20, -20 }, { 201, 0, 0, 60, -4 },
        { 202, 4, 0, 50, 4 },    { 203, 3, 0, 40, -4 },
        { 206, 0, 0, 20, -4 },   { 207, 4, 0, 10, 4 },
        { 208, 3, 0, 0, -4 },    { 211, 0, 0, -20, -4 },
        { 212, 4, 0, -30, 4 },   { 213, 3, 0, -40, -4 },
        { 303, 4, 0, -60, 4 },   { 304, 4, 0, -80, 4 } },
      /* PGT */
      { /* L10 */ { 2, 48, { 35, 36, 38, 33, 41, 34, 44, 37, 45, 32, 47, 39,
                             50, 40, 51, 42, 53, 43, 56, 46, 57, 48, 59, 49,
                             60, 52, 61, 54, 62, 55, 63, 58, -1, 31, -1, 30,
                             -1, 29, -1, 28, -1, 27, -1, 26, -1, 24, -1, 23,
                             -1, 20, -1, 21, -1, 16, -1, 19, -1, 12, -1, 14,
                             -1, 11, -1, 13, -1, 7,  -1, 8,  -1, 5,  -1, 2,
                             -1, 6,  -1, 1,  -1, 3,  -1, 0,  -1, 4,  -1, 9,
                             -1, 10, -1, 15, -1, 17, -1, 18, -1, 22, -1, 25 } },
        /* L19 */ { 2, 48, { 25, -1, 22, -1, 18, -1, 17, -1, 15, -1, 10, -1,
                             9,  -1, 4,  -1, 0,  -1, 3,  -1, 1,  -1, 6,  -1,
                             2,  -1, 5,  -1, 8,  -1, 7,  -1, 13, -1, 11, -1,
                             14, -1, 12, -1, 19, -1, 16, -1, 21, -1, 20, -1,
                             23, -1, 24, -1, 26, -1, 27, -1, 28, -1, 29, -1,
                             30, -1, 31, -1, 58, 63, 55, 62, 54, 61, 52, 60,
                             49, 59, 48, 57, 46, 56, 43, 53, 42, 51, 40, 50,
                             39, 47, 32, 45, 37, 44, 34, 41, 33, 38, 36, 35 } },
        /* L20 */ { 2, 48, { -1, 63, -1, 62, -1, 61, -1, 60, -1, 59, -1, 57,
                             -1, 56, -1, 53, -1, 51, -1, 50, -1, 47, -1, 45,
                             -1, 44, -1, 41, -1, 38, -1, 35, -1, 36, -1, 33,
                             -1, 34, -1, 37, -1, 32, -1, 39, -1, 40, -1, 42,
                             -1, 43, -1, 46, -1, 48, -1, 49, -1, 52, -1, 54,
                             -1, 55, -1, 58, 25, 31, 22, 30, 18, 29, 17, 28,
                             15, 27, 10, 26, 9,  24, 4,  23, 0,  20, 3,  21,
                             1,  16, 6,  19, 2,  12, 5,  14, 8,  11, 7,  13 } },
        /* L9 */ { 2, 48, { 13, 7,  11, 8,  14, 5,  12, 2,  19, 6,  16, 1,
                            21, 3,  20, 0,  23, 4,  24, 9,  26, 10, 27, 15,
                            28, 17, 29, 18, 30, 22, 31, 25, 58, -1, 55, -1,
                            54, -1, 52, -1, 49, -1, 48, -1, 46, -1, 43, -1,
                            42, -1, 40, -1, 39, -1, 32, -1, 37, -1, 34, -1,
                            33, -1, 36, -1, 35, -1, 38, -1, 41, -1, 44, -1,
                            45, -1, 47, -1, 50, -1, 51, -1, 53, -1, 56, -1,
                            57, -1, 59, -1, 60, -1, 61, -1, 62, -1, 63, -1 } },
        /* O11 */ { 2, 32, { 58, 31, 55, 30, 54, 29, 52, 28, 49, 27, 48, 26, 46,
                             24, 43, 23, 42, 20, 40, 21, 39, 16, 32, 19, 37, 12,
                             34, 14, 33, 11, 36, 13, 35, 7,  38, 8,  41, 5,  44,
                             2,  45, 6,  47, 1,  50, 3,  51, 0,  53, 4,  56, 9,
                             57, 10, 59, 15, 60, 17, 61, 18, 62, 22, 63, 25 } },
        /* O12 */
        { 2, 32, { 25, 63, 22, 62, 18, 61, 17, 60, 15, 59, 10, 57, 9,
                   56, 4,  53, 0,  51, 3,  50, 1,  47, 6,  45, 2,  44,
                   5,  41, 8,  38, 7,  35, 13, 36, 11, 33, 14, 34, 12,
                   37, 19, 32, 16, 39, 21, 40, 20, 42, 23, 43, 24, 46,
                   26, 48, 27, 49, 28, 52, 29, 54, 30, 55, 31, 58 } } },
      /* PS */
      { { 10, 0.5 } }
    };
  } else {
    return new CathodeSegmentation{
      20,
      false,
      /* PG */
      { { 1028, 3, 0, -60, -20 },
        { 1029, 2, 0, -80, -20 },
        { 1125, 3, 0, 60, -20 },
        { 1126, 2, 0, 40, -20 },
        { 1129, 3, 0, 20, -20 },
        { 1130, 2, 0, 0, -20 },
        { 1133, 3, 0, -20, -20 },
        { 1134, 2, 0, -40, -20 },
        { 1228, 0, 0, 40, 0 },
        { 1229, 1, 0, 60, 0 },
        { 1233, 0, 0, 0, 0 },
        { 1234, 1, 0, 20, 0 },
        { 1238, 0, 0, -40, 0 },
        { 1239, 1, 0, -20, 0 },
        { 1325, 0, 0, -80, 0 },
        { 1326, 1, 0, -60, 0 } },
      /* PGT */
      { /* O5 */ { 28, 2, { 25, 18, 15, 9,  0,  1,  2,  8,  13, 14, 19, 21,
                            23, 26, 28, 30, 58, 54, 49, 46, 42, 39, 37, 33,
                            35, 41, 45, 50, 22, 17, 10, 4,  3,  6,  5,  7,
                            11, 12, 16, 20, 24, 27, 29, 31, 55, 52, 48, 43,
                            40, 32, 34, 36, 38, 44, 47, 51 } },
        /* O6 */ { 28, 2, { 0,  1,  2,  8,  13, 14, 19, 21, 23, 26, 28, 30,
                            58, 54, 49, 46, 42, 39, 37, 33, 35, 41, 45, 50,
                            53, 57, 60, 62, 3,  6,  5,  7,  11, 12, 16, 20,
                            24, 27, 29, 31, 55, 52, 48, 43, 40, 32, 34, 36,
                            38, 44, 47, 51, 56, 59, 61, 63 } },
        /* O7 */ { 28, 2, { 63, 61, 59, 56, 51, 47, 44, 38, 36, 34, 32, 40,
                            43, 48, 52, 55, 31, 29, 27, 24, 20, 16, 12, 11,
                            7,  5,  6,  3,  62, 60, 57, 53, 50, 45, 41, 35,
                            33, 37, 39, 42, 46, 49, 54, 58, 30, 28, 26, 23,
                            21, 19, 14, 13, 8,  2,  1,  0 } },
        /* O8 */ { 28, 2, { 51, 47, 44, 38, 36, 34, 32, 40, 43, 48, 52, 55,
                            31, 29, 27, 24, 20, 16, 12, 11, 7,  5,  6,  3,
                            4,  10, 17, 22, 50, 45, 41, 35, 33, 37, 39, 42,
                            46, 49, 54, 58, 30, 28, 26, 23, 21, 19, 14, 13,
                            8,  2,  1,  0,  9,  15, 18, 25 } } },
      /* PS */
      { { 0.714285714, 10 } }
    };
  }
}
class CathodeSegmentationCreatorRegisterCreateSegType20
{
 public:
  CathodeSegmentationCreatorRegisterCreateSegType20()
  {
    registerCathodeSegmentationCreator(20, createSegType20);
  }
} aCathodeSegmentationCreatorRegisterCreateSegType20;

} // namespace impl4
} // namespace mapping
} // namespace mch
} // namespace o2
