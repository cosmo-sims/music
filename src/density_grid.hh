// This file is part of MUSIC
// A software package to generate ICs for cosmological simulations
// Copyright (C) 2010-2024 by Oliver Hahn
// 
// monofonIC is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// monofonIC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <vector>
#include <array>

/*!
 * @class DensityGrid
 * @brief provides infrastructure for computing the initial density field
 *
 * This class provides access and data management member functions that
 * are used when computing the initial density field by convolution with
 * transfer functions.
 */
template <typename real_t>
class DensityGrid
{
public:
  size_t nx_;  //!< number of grid cells in x-direction
  size_t ny_;  //!< number of grid cells in y-direction
  size_t nz_;  //!< number of grid cells in z-direction
  size_t nzp_; //!< number of cells in memory (z-dir), used for Nyquist padding

  std::array<size_t,3> nv_;

  int ox_; //!< offset of grid in x-direction
  int oy_; //!< offset of grid in y-direction
  int oz_; //!< offset of grid in z-direction

  std::array<int,3> ov_;

  //! the actual data container in the form of a 1D array
  std::vector<real_t> data_;

  //! constructor
  /*! constructs an instance given the dimensions of the density field
   * @param nx the number of cells in x
   * @param ny the number of cells in y
   * @param nz the number of cells in z
   */
  DensityGrid(unsigned nx, unsigned ny, unsigned nz)
      : nx_(nx), ny_(ny), nz_(nz), nzp_(2 * (nz_ / 2 + 1)), ox_(0), oy_(0), oz_(0)
  {
    data_.assign((size_t)nx_ * (size_t)ny_ * (size_t)nzp_, 0.0);
    nv_ = {nx_,ny_,nz_};
    ov_ = {ox_,oy_,oz_};
    // nv_[0] = nx_;
    // nv_[1] = ny_;
    // nv_[2] = nz_;
    // ov_[0] = ox_;
    // ov_[1] = oy_;
    // ov_[2] = oz_;
  }

  DensityGrid(unsigned nx, unsigned ny, unsigned nz, int ox, int oy, int oz)
      : nx_(nx), ny_(ny), nz_(nz), nzp_(2 * (nz_ / 2 + 1)), ox_(ox), oy_(oy), oz_(oz)
  {
    data_.assign((size_t)nx_ * (size_t)ny_ * (size_t)nzp_, 0.0);
    nv_ = {nx_,ny_,nz_};
    ov_ = {ox_,oy_,oz_};
    // nv_[0] = nx_;
    // nv_[1] = ny_;
    // nv_[2] = nz_;
    // ov_[0] = ox_;
    // ov_[1] = oy_;
    // ov_[2] = oz_;
  }

  //! copy constructor
  explicit DensityGrid(const DensityGrid<real_t> &g)
      : nx_(g.nx_), ny_(g.ny_), nz_(g.nz_), nzp_(g.nzp_),
        ox_(g.ox_), oy_(g.oy_), oz_(g.oz_)
  {
    data_ = g.data_;
    nv_ = {nx_,ny_,nz_};
    ov_ = {ox_,oy_,oz_};
    // nv_[0] = nx_;
    // nv_[1] = ny_;
    // nv_[2] = nz_;
    // ov_[0] = ox_;
    // ov_[1] = oy_;
    // ov_[2] = oz_;
  }

  //! destructor
  ~DensityGrid()
  {
  }

  //! clears the density object
  /*! sets all dimensions to zero and frees the memory
   */
  void clear(void)
  {
    nx_ = ny_ = nz_ = nzp_ = 0;
    ox_ = oy_ = oz_ = 0;
    nv_[0] = nv_[1] = nv_[2] = 0;
    ov_[0] = ov_[1] = ov_[2] = 0;

    data_.clear();
    std::vector<real_t>().swap(data_);
  }

  //! query the 3D array sizes of the density object
  /*! returns the size of the 3D density object along a specified dimension
   * @param i the dimension for which size is to be returned
   * @returns array size along dimension i
   */
  size_t size(int i) const
  {
    return nv_[i];
  }

  int offset(int i) const
  {
    return ov_[i];
  }

  //! zeroes the density object
  /*! sets all values to 0.0
   */
  void zero(void)
  {
    data_.assign(data_.size(), 0.0);
  }

  //! assigns the contents of another DensityGrid to this
  DensityGrid &operator=(const DensityGrid<real_t> &g)
  {
    nx_ = g.nx_;
    ny_ = g.ny_;
    nz_ = g.nz_;
    nzp_ = g.nzp_;
    ox_ = g.ox_;
    oy_ = g.oy_;
    oz_ = g.oz_;
    data_ = g.data_;

    return *this;
  }

  //! 3D index based data access operator
  inline real_t &operator()(size_t i, size_t j, size_t k)
  {
    return data_[((size_t)i * ny_ + (size_t)j) * nzp_ + (size_t)k];
  }

  //! 3D index based const data access operator
  inline const real_t &operator()(size_t i, size_t j, size_t k) const
  {
    return data_[((size_t)i * ny_ + (size_t)j) * nzp_ + (size_t)k];
  }

  //! recover the pointer to the 1D data array
  inline real_t *get_data_ptr(void)
  {
    return &data_[0];
  }

#if 0
  //! fills the density field with random number values
  /*! given a pointer to a random_numbers object, fills the field with random values
   *  @param prc pointer to a random_numbers object
   *  @param variance the variance of the random numbers (the values returned by prc are multiplied by this)
   *  @param i0 x-offset (shift) in cells of the density field with respect to the random number field
   *  @param j0 y-offset (shift) in cells of the density field with respect to the random number field
   *  @param k0 z-offset (shift) in cells of the density field with respect to the random number field
   *  @param setzero boolean, if true, the global mean will be subtracted
   */
  void fill_rand(/*const*/ random_numbers<real_t> *prc, real_t variance, int i0, int j0, int k0, bool setzero = false)
  {
    long double sum = 0.0;

#pragma omp parallel for reduction(+ \
                                   : sum)
    for (int i = 0; i < nx_; ++i)
      for (int j = 0; j < ny_; ++j)
        for (int k = 0; k < nz_; ++k)
        {
          (*this)(i, j, k) = (*prc)(i0 + i, j0 + j, k0 + k) * variance;
          sum += (*this)(i, j, k);
        }

    sum /= nx_ * ny_ * nz_;

    if (setzero)
    {
#pragma omp parallel for
      for (int i = 0; i < nx_; ++i)
        for (int j = 0; j < ny_; ++j)
          for (int k = 0; k < nz_; ++k)
            (*this)(i, j, k) -= sum;
    }
  }
#endif

  //! copies the data from another field with 3D index-based access operator
  template <class array3>
  void copy(array3 &v)
  {
#pragma omp parallel for
    for (int ix = 0; ix < (int)nx_; ++ix)
      for (int iy = 0; iy < (int)ny_; ++iy)
        for (int iz = 0; iz < (int)nz_; ++iz)
          v(ix, iy, iz) = (*this)(ix, iy, iz);
  }

  //! adds the data from another field with 3D index-based access operator
  template <class array3>
  void copy_add(array3 &v)
  {
#pragma omp parallel for
    for (int ix = 0; ix < (int)nx_; ++ix)
      for (int iy = 0; iy < (int)ny_; ++iy)
        for (int iz = 0; iz < (int)nz_; ++iz)
          v(ix, iy, iz) += (*this)(ix, iy, iz);
  }
};

template <typename real_t>
class PaddedDensitySubGrid : public DensityGrid<real_t>
{
public:
  using DensityGrid<real_t>::nx_;
  using DensityGrid<real_t>::ny_;
  using DensityGrid<real_t>::nz_;
  using DensityGrid<real_t>::ox_;
  using DensityGrid<real_t>::oy_;
  using DensityGrid<real_t>::oz_;
  using DensityGrid<real_t>::data_;

  std::array<size_t, 3> pad_;

  // using DensityGrid<real_t>::fill_rand;
  using DensityGrid<real_t>::get_data_ptr;

public:
  PaddedDensitySubGrid(int ox, int oy, int oz, unsigned nx, unsigned ny, unsigned nz)
      : DensityGrid<real_t>(nx * 2, ny * 2, nz * 2, ox, oy, oz),
        pad_{{nx / 2, ny / 2, nz / 2}}
  {
  }

  PaddedDensitySubGrid(int ox, int oy, int oz, unsigned nx, unsigned ny, unsigned nz, unsigned padx, unsigned pady, unsigned padz)
      : DensityGrid<real_t>(nx + 2 * padx, ny + 2 * pady, nz + 2 * padz, ox, oy, oz),
        pad_{{padx, pady, padz}}
  {
  }

  PaddedDensitySubGrid(const PaddedDensitySubGrid<real_t> &o)
      : DensityGrid<real_t>(o)
  {
  }

  size_t margin(int i) const
  {
    return pad_[i];
  }

  template <class array3>
  void copy_unpad(array3 &v)
  {
#pragma omp parallel for
    for (size_t ix = pad_[0]; ix < nx_ - pad_[0]; ++ix)
    {
      const size_t ixu = ix - pad_[0];
      for (size_t iy = pad_[1], iyu = 0; iy < ny_ - pad_[1]; ++iy, ++iyu)
        for (size_t iz = pad_[2], izu = 0; iz < nz_ - pad_[2]; ++iz, ++izu)
          v(ixu, iyu, izu) = (*this)(ix, iy, iz);
    }
  }

  template <class array3>
  void copy_add_unpad(array3 &v)
  {
#pragma omp parallel for
    for (size_t ix = pad_[0]; ix < nx_ - pad_[0]; ++ix)
    {
      const size_t ixu = ix - pad_[0];
      for (size_t iy = pad_[1], iyu = 0; iy < ny_ - pad_[1]; ++iy, ++iyu)
        for (size_t iz = pad_[2], izu = 0; iz < nz_ - pad_[2]; ++iz, ++izu)
          v(ixu, iyu, izu) += (*this)(ix, iy, iz);
    }
  }

  template <class array3>
  void copy_subtract_unpad(array3 &v)
  {
#pragma omp parallel for
    for (size_t ix = pad_[0]; ix < nx_ - pad_[0]; ++ix)
    {
      const size_t ixu = ix - pad_[0];
      for (size_t iy = pad_[1], iyu = 0; iy < ny_ - pad_[1]; ++iy, ++iyu)
        for (size_t iz = pad_[2], izu = 0; iz < nz_ - pad_[2]; ++iz, ++izu)
          v(ixu, iyu, izu) -= (*this)(ix, iy, iz);
    }
  }
};