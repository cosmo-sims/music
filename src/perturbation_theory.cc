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

#include <perturbation_theory.hh>
#include <mesh.hh>
#include <mg_operators.hh>
#include <general.hh>

#define ACC(i, j, k) ((*u.get_grid((ilevel)))((i), (j), (k)))
#define SQR(x) ((x) * (x))

void compute_LLA_density(const grid_hierarchy &u, grid_hierarchy &fnew, unsigned order)
{
	fnew = u;

	for (unsigned ilevel = u.levelmin(); ilevel <= u.levelmax(); ++ilevel)
	{
		double h = pow(2.0, ilevel), h2 = h * h, h2_4 = 0.25 * h2;
		meshvar_bnd *pvar = fnew.get_grid(ilevel);

		if (order == 2)
		{
#pragma omp parallel for // reduction(+:sum_corr,sum,sum2)
			for (int ix = 0; ix < (int)(*u.get_grid(ilevel)).size(0); ++ix)
				for (int iy = 0; iy < (int)(*u.get_grid(ilevel)).size(1); ++iy)
					for (int iz = 0; iz < (int)(*u.get_grid(ilevel)).size(2); ++iz)
					{
						double D[3][3];

						D[0][0] = (ACC(ix - 1, iy, iz) - 2.0 * ACC(ix, iy, iz) + ACC(ix + 1, iy, iz)) * h2;
						D[1][1] = (ACC(ix, iy - 1, iz) - 2.0 * ACC(ix, iy, iz) + ACC(ix, iy + 1, iz)) * h2;
						D[2][2] = (ACC(ix, iy, iz - 1) - 2.0 * ACC(ix, iy, iz) + ACC(ix, iy, iz + 1)) * h2;

						D[0][1] = D[1][0] = (ACC(ix - 1, iy - 1, iz) - ACC(ix - 1, iy + 1, iz) - ACC(ix + 1, iy - 1, iz) + ACC(ix + 1, iy + 1, iz)) * h2_4;
						D[0][2] = D[2][0] = (ACC(ix - 1, iy, iz - 1) - ACC(ix - 1, iy, iz + 1) - ACC(ix + 1, iy, iz - 1) + ACC(ix + 1, iy, iz + 1)) * h2_4;
						D[1][2] = D[2][1] = (ACC(ix, iy - 1, iz - 1) - ACC(ix, iy - 1, iz + 1) - ACC(ix, iy + 1, iz - 1) + ACC(ix, iy + 1, iz + 1)) * h2_4;

						D[0][0] += 1.0;
						D[1][1] += 1.0;
						D[2][2] += 1.0;

						double det = D[0][0] * D[1][1] * D[2][2] - D[0][0] * D[1][2] * D[2][1] - D[1][0] * D[0][1] * D[2][2] + D[1][0] * D[0][2] * D[1][2] + D[2][0] * D[0][1] * D[1][2] - D[2][0] * D[0][2] * D[1][1];

						(*pvar)(ix, iy, iz) = 1.0 / det - 1.0;
					}
		}
		else if (order == 4)
		{
#pragma omp parallel for
			for (int ix = 0; ix < (int)(*u.get_grid(ilevel)).size(0); ++ix)
				for (int iy = 0; iy < (int)(*u.get_grid(ilevel)).size(1); ++iy)
					for (int iz = 0; iz < (int)(*u.get_grid(ilevel)).size(2); ++iz)
					{
						double D[3][3];

						D[0][0] = (-ACC(ix - 2, iy, iz) + 16. * ACC(ix - 1, iy, iz) - 30.0 * ACC(ix, iy, iz) + 16. * ACC(ix + 1, iy, iz) - ACC(ix + 2, iy, iz)) * h2 / 12.0;
						D[1][1] = (-ACC(ix, iy - 2, iz) + 16. * ACC(ix, iy - 1, iz) - 30.0 * ACC(ix, iy, iz) + 16. * ACC(ix, iy + 1, iz) - ACC(ix, iy + 2, iz)) * h2 / 12.0;
						D[2][2] = (-ACC(ix, iy, iz - 2) + 16. * ACC(ix, iy, iz - 1) - 30.0 * ACC(ix, iy, iz) + 16. * ACC(ix, iy, iz + 1) - ACC(ix, iy, iz + 2)) * h2 / 12.0;

						D[0][1] = D[1][0] = (ACC(ix - 1, iy - 1, iz) - ACC(ix - 1, iy + 1, iz) - ACC(ix + 1, iy - 1, iz) + ACC(ix + 1, iy + 1, iz)) * h2_4;
						D[0][2] = D[2][0] = (ACC(ix - 1, iy, iz - 1) - ACC(ix - 1, iy, iz + 1) - ACC(ix + 1, iy, iz - 1) + ACC(ix + 1, iy, iz + 1)) * h2_4;
						D[1][2] = D[2][1] = (ACC(ix, iy - 1, iz - 1) - ACC(ix, iy - 1, iz + 1) - ACC(ix, iy + 1, iz - 1) + ACC(ix, iy + 1, iz + 1)) * h2_4;

						D[0][0] += 1.0;
						D[1][1] += 1.0;
						D[2][2] += 1.0;

						double det = D[0][0] * D[1][1] * D[2][2] - D[0][0] * D[1][2] * D[2][1] - D[1][0] * D[0][1] * D[2][2] + D[1][0] * D[0][2] * D[1][2] + D[2][0] * D[0][1] * D[1][2] - D[2][0] * D[0][2] * D[1][1];

						(*pvar)(ix, iy, iz) = 1.0 / det - 1.0;
					}
		}
		else if (order == 6)
		{
			h2_4 /= 36.;
			h2 /= 180.;
#pragma omp parallel for
			for (int ix = 0; ix < (int)(*u.get_grid(ilevel)).size(0); ++ix)
				for (int iy = 0; iy < (int)(*u.get_grid(ilevel)).size(1); ++iy)
					for (int iz = 0; iz < (int)(*u.get_grid(ilevel)).size(2); ++iz)
					{
						double D[3][3];

						D[0][0] = (2. * ACC(ix - 3, iy, iz) - 27. * ACC(ix - 2, iy, iz) + 270. * ACC(ix - 1, iy, iz) - 490.0 * ACC(ix, iy, iz) + 270. * ACC(ix + 1, iy, iz) - 27. * ACC(ix + 2, iy, iz) + 2. * ACC(ix + 3, iy, iz)) * h2;
						D[1][1] = (2. * ACC(ix, iy - 3, iz) - 27. * ACC(ix, iy - 2, iz) + 270. * ACC(ix, iy - 1, iz) - 490.0 * ACC(ix, iy, iz) + 270. * ACC(ix, iy + 1, iz) - 27. * ACC(ix, iy + 2, iz) + 2. * ACC(ix, iy + 3, iz)) * h2;
						D[2][2] = (2. * ACC(ix, iy, iz - 3) - 27. * ACC(ix, iy, iz - 2) + 270. * ACC(ix, iy, iz - 1) - 490.0 * ACC(ix, iy, iz) + 270. * ACC(ix, iy, iz + 1) - 27. * ACC(ix, iy, iz + 2) + 2. * ACC(ix, iy, iz + 3)) * h2;

						//.. this is actually 8th order accurate
						D[0][1] = D[1][0] = (64. * (ACC(ix - 1, iy - 1, iz) - ACC(ix - 1, iy + 1, iz) - ACC(ix + 1, iy - 1, iz) + ACC(ix + 1, iy + 1, iz)) - 8. * (ACC(ix - 2, iy - 1, iz) - ACC(ix + 2, iy - 1, iz) - ACC(ix - 2, iy + 1, iz) + ACC(ix + 2, iy + 1, iz) + ACC(ix - 1, iy - 2, iz) - ACC(ix - 1, iy + 2, iz) - ACC(ix + 1, iy - 2, iz) + ACC(ix + 1, iy + 2, iz)) + 1. * (ACC(ix - 2, iy - 2, iz) - ACC(ix - 2, iy + 2, iz) - ACC(ix + 2, iy - 2, iz) + ACC(ix + 2, iy + 2, iz))) * h2_4;
						D[0][2] = D[2][0] = (64. * (ACC(ix - 1, iy, iz - 1) - ACC(ix - 1, iy, iz + 1) - ACC(ix + 1, iy, iz - 1) + ACC(ix + 1, iy, iz + 1)) - 8. * (ACC(ix - 2, iy, iz - 1) - ACC(ix + 2, iy, iz - 1) - ACC(ix - 2, iy, iz + 1) + ACC(ix + 2, iy, iz + 1) + ACC(ix - 1, iy, iz - 2) - ACC(ix - 1, iy, iz + 2) - ACC(ix + 1, iy, iz - 2) + ACC(ix + 1, iy, iz + 2)) + 1. * (ACC(ix - 2, iy, iz - 2) - ACC(ix - 2, iy, iz + 2) - ACC(ix + 2, iy, iz - 2) + ACC(ix + 2, iy, iz + 2))) * h2_4;
						D[1][2] = D[2][1] = (64. * (ACC(ix, iy - 1, iz - 1) - ACC(ix, iy - 1, iz + 1) - ACC(ix, iy + 1, iz - 1) + ACC(ix, iy + 1, iz + 1)) - 8. * (ACC(ix, iy - 2, iz - 1) - ACC(ix, iy + 2, iz - 1) - ACC(ix, iy - 2, iz + 1) + ACC(ix, iy + 2, iz + 1) + ACC(ix, iy - 1, iz - 2) - ACC(ix, iy - 1, iz + 2) - ACC(ix, iy + 1, iz - 2) + ACC(ix, iy + 1, iz + 2)) + 1. * (ACC(ix, iy - 2, iz - 2) - ACC(ix, iy - 2, iz + 2) - ACC(ix, iy + 2, iz - 2) + ACC(ix, iy + 2, iz + 2))) * h2_4;

						D[0][0] += 1.0;
						D[1][1] += 1.0;
						D[2][2] += 1.0;

						double det = D[0][0] * D[1][1] * D[2][2] - D[0][0] * D[1][2] * D[2][1] - D[1][0] * D[0][1] * D[2][2] + D[1][0] * D[0][2] * D[1][2] + D[2][0] * D[0][1] * D[1][2] - D[2][0] * D[0][2] * D[1][1];

						(*pvar)(ix, iy, iz) = 1.0 / det - 1.0;
					}
		}
		else
			throw std::runtime_error("compute_LLA_density : invalid operator order specified");
	}
}

void compute_Lu_density(const grid_hierarchy &u, grid_hierarchy &fnew, unsigned order)
{
	fnew = u;

	for (unsigned ilevel = u.levelmin(); ilevel <= u.levelmax(); ++ilevel)
	{
		double h = pow(2.0, ilevel), h2 = h * h;
		meshvar_bnd *pvar = fnew.get_grid(ilevel);

#pragma omp parallel for
		for (int ix = 0; ix < (int)(*u.get_grid(ilevel)).size(0); ++ix)
			for (int iy = 0; iy < (int)(*u.get_grid(ilevel)).size(1); ++iy)
				for (int iz = 0; iz < (int)(*u.get_grid(ilevel)).size(2); ++iz)
				{
					double D[3][3];

					D[0][0] = 1.0 + (ACC(ix - 1, iy, iz) - 2.0 * ACC(ix, iy, iz) + ACC(ix + 1, iy, iz)) * h2;
					D[1][1] = 1.0 + (ACC(ix, iy - 1, iz) - 2.0 * ACC(ix, iy, iz) + ACC(ix, iy + 1, iz)) * h2;
					D[2][2] = 1.0 + (ACC(ix, iy, iz - 1) - 2.0 * ACC(ix, iy, iz) + ACC(ix, iy, iz + 1)) * h2;

					(*pvar)(ix, iy, iz) = -(D[0][0] + D[1][1] + D[2][2] - 3.0);
				}
	}
}

void compute_2LPT_source_FFT(config_file &cf_, const grid_hierarchy &u, grid_hierarchy &fnew)
{
	if (u.levelmin() != u.levelmax())
		throw std::runtime_error("FFT 2LPT can only be run in Unigrid mode!");

	fnew = u;
	size_t nx, ny, nz, nzp;
	nx = u.get_grid(u.levelmax())->size(0);
	ny = u.get_grid(u.levelmax())->size(1);
	nz = u.get_grid(u.levelmax())->size(2);
	nzp = 2 * (nz / 2 + 1);

	//... copy data ..................................................
	real_t *data = new real_t[nx * ny * nzp];
	complex_t *cdata = reinterpret_cast<complex_t *>(data);

	complex_t *cdata_11, *cdata_12, *cdata_13, *cdata_22, *cdata_23, *cdata_33;
	real_t *data_11, *data_12, *data_13, *data_22, *data_23, *data_33;

	data_11 = new real_t[nx * ny * nzp];
	cdata_11 = reinterpret_cast<complex_t *>(data_11);
	data_12 = new real_t[nx * ny * nzp];
	cdata_12 = reinterpret_cast<complex_t *>(data_12);
	data_13 = new real_t[nx * ny * nzp];
	cdata_13 = reinterpret_cast<complex_t *>(data_13);
	data_22 = new real_t[nx * ny * nzp];
	cdata_22 = reinterpret_cast<complex_t *>(data_22);
	data_23 = new real_t[nx * ny * nzp];
	cdata_23 = reinterpret_cast<complex_t *>(data_23);
	data_33 = new real_t[nx * ny * nzp];
	cdata_33 = reinterpret_cast<complex_t *>(data_33);

#pragma omp parallel for
	for (int i = 0; i < (int)nx; ++i)
		for (size_t j = 0; j < ny; ++j)
			for (size_t k = 0; k < nz; ++k)
			{
				size_t idx = ((size_t)i * ny + j) * nzp + k;
				data[idx] = (*u.get_grid(u.levelmax()))(i, j, k);
			}

	//... perform FFT and Poisson solve................................

	fftw_plan_t
			plan = FFTW_API(plan_dft_r2c_3d)(nx, ny, nz, data, cdata, FFTW_ESTIMATE),
			iplan = FFTW_API(plan_dft_c2r_3d)(nx, ny, nz, cdata, data, FFTW_ESTIMATE),
			ip11 = FFTW_API(plan_dft_c2r_3d)(nx, ny, nz, cdata_11, data_11, FFTW_ESTIMATE),
			ip12 = FFTW_API(plan_dft_c2r_3d)(nx, ny, nz, cdata_12, data_12, FFTW_ESTIMATE),
			ip13 = FFTW_API(plan_dft_c2r_3d)(nx, ny, nz, cdata_13, data_13, FFTW_ESTIMATE),
			ip22 = FFTW_API(plan_dft_c2r_3d)(nx, ny, nz, cdata_22, data_22, FFTW_ESTIMATE),
			ip23 = FFTW_API(plan_dft_c2r_3d)(nx, ny, nz, cdata_23, data_23, FFTW_ESTIMATE),
			ip33 = FFTW_API(plan_dft_c2r_3d)(nx, ny, nz, cdata_33, data_33, FFTW_ESTIMATE);

	FFTW_API(execute)
	(plan);

	double kfac = 2.0 * M_PI;
	double norm = 1.0 / ((double)(nx * ny * nz));

#pragma omp parallel for
	for (int i = 0; i < (int)nx; ++i)
		for (size_t j = 0; j < ny; ++j)
			for (size_t l = 0; l < nz / 2 + 1; ++l)
			{
				int ii = i;
				if (ii > (int)nx / 2)
					ii -= nx;
				int jj = (int)j;
				if (jj > (int)ny / 2)
					jj -= ny;
				double ki = (double)ii;
				double kj = (double)jj;
				double kk = (double)l;

				double k[3];
				k[0] = (double)ki * kfac;
				k[1] = (double)kj * kfac;
				k[2] = (double)kk * kfac;

				size_t idx = ((size_t)i * ny + j) * nzp / 2 + l;
				// double re = cdata[idx][0];
				// double im = cdata[idx][1];

				cdata_11[idx][0] = -k[0] * k[0] * cdata[idx][0] * norm;
				cdata_11[idx][1] = -k[0] * k[0] * cdata[idx][1] * norm;

				cdata_12[idx][0] = -k[0] * k[1] * cdata[idx][0] * norm;
				cdata_12[idx][1] = -k[0] * k[1] * cdata[idx][1] * norm;

				cdata_13[idx][0] = -k[0] * k[2] * cdata[idx][0] * norm;
				cdata_13[idx][1] = -k[0] * k[2] * cdata[idx][1] * norm;

				cdata_22[idx][0] = -k[1] * k[1] * cdata[idx][0] * norm;
				cdata_22[idx][1] = -k[1] * k[1] * cdata[idx][1] * norm;

				cdata_23[idx][0] = -k[1] * k[2] * cdata[idx][0] * norm;
				cdata_23[idx][1] = -k[1] * k[2] * cdata[idx][1] * norm;

				cdata_33[idx][0] = -k[2] * k[2] * cdata[idx][0] * norm;
				cdata_33[idx][1] = -k[2] * k[2] * cdata[idx][1] * norm;

				if (i == (int)nx / 2 || j == ny / 2 || l == nz / 2)
				{
					cdata_11[idx][0] = 0.0;
					cdata_11[idx][1] = 0.0;

					cdata_12[idx][0] = 0.0;
					cdata_12[idx][1] = 0.0;

					cdata_13[idx][0] = 0.0;
					cdata_13[idx][1] = 0.0;

					cdata_22[idx][0] = 0.0;
					cdata_22[idx][1] = 0.0;

					cdata_23[idx][0] = 0.0;
					cdata_23[idx][1] = 0.0;

					cdata_33[idx][0] = 0.0;
					cdata_33[idx][1] = 0.0;
				}
			}

	delete[] data;
	/*cdata_11[0][0]	= 0.0; cdata_11[0][1]	= 0.0;
	 cdata_12[0][0]	= 0.0; cdata_12[0][1]	= 0.0;
	 cdata_13[0][0]	= 0.0; cdata_13[0][1]	= 0.0;
	 cdata_22[0][0]	= 0.0; cdata_22[0][1]	= 0.0;
	 cdata_23[0][0]	= 0.0; cdata_23[0][1]	= 0.0;
	 cdata_33[0][0]	= 0.0; cdata_33[0][1]	= 0.0;*/

	FFTW_API(execute)(ip11);
	FFTW_API(execute)(ip12);
	FFTW_API(execute)(ip13);
	FFTW_API(execute)(ip22);
	FFTW_API(execute)(ip23);
	FFTW_API(execute)(ip33);

	FFTW_API(destroy_plan)(plan);
	FFTW_API(destroy_plan)(iplan);
	FFTW_API(destroy_plan)(ip11);
	FFTW_API(destroy_plan)(ip12);
	FFTW_API(destroy_plan)(ip13);
	FFTW_API(destroy_plan)(ip22);
	FFTW_API(destroy_plan)(ip23);
	FFTW_API(destroy_plan)(ip33);

//... copy data ..........................................
#pragma omp parallel for
	for (int i = 0; i < (int)nx; ++i)
		for (size_t j = 0; j < ny; ++j)
			for (size_t k = 0; k < nz; ++k)
			{
				size_t ii = ((size_t)i * ny + j) * nzp + k;
				(*fnew.get_grid(u.levelmax()))(i, j, k) = ((data_11[ii] * data_22[ii] - data_12[ii] * data_12[ii]) +
																									 (data_11[ii] * data_33[ii] - data_13[ii] * data_13[ii]) +
																									 (data_22[ii] * data_33[ii] - data_23[ii] * data_23[ii]));

				//(*fnew.get_grid(u.levelmax()))(i,j,k) =
			}

	// delete[] data;
	delete[] data_11;
	delete[] data_12;
	delete[] data_13;
	delete[] data_23;
	delete[] data_22;
	delete[] data_33;
}

void compute_2LPT_source(const grid_hierarchy &u, grid_hierarchy &fnew, unsigned order)
{
	fnew = u;
	fnew.zero();

	for (unsigned ilevel = u.levelmin(); ilevel <= u.levelmax(); ++ilevel)
	{
		double h = pow(2.0, ilevel), h2 = h * h, h2_4 = 0.25 * h2;
		meshvar_bnd *pvar = fnew.get_grid(ilevel);

		if (order == 2)
		{

#pragma omp parallel for
			for (int ix = 0; ix < (int)(*u.get_grid(ilevel)).size(0); ++ix)
				for (int iy = 0; iy < (int)(*u.get_grid(ilevel)).size(1); ++iy)
					for (int iz = 0; iz < (int)(*u.get_grid(ilevel)).size(2); ++iz)
					{
						double D[3][3];

						D[0][0] = (ACC(ix - 2, iy, iz) - 2.0 * ACC(ix, iy, iz) + ACC(ix + 2, iy, iz)) * h2_4;
						D[1][1] = (ACC(ix, iy - 2, iz) - 2.0 * ACC(ix, iy, iz) + ACC(ix, iy + 2, iz)) * h2_4;
						D[2][2] = (ACC(ix, iy, iz - 2) - 2.0 * ACC(ix, iy, iz) + ACC(ix, iy, iz + 2)) * h2_4;

						D[0][1] = D[1][0] = (ACC(ix - 1, iy - 1, iz) - ACC(ix - 1, iy + 1, iz) - ACC(ix + 1, iy - 1, iz) + ACC(ix + 1, iy + 1, iz)) * h2_4;
						D[0][2] = D[2][0] = (ACC(ix - 1, iy, iz - 1) - ACC(ix - 1, iy, iz + 1) - ACC(ix + 1, iy, iz - 1) + ACC(ix + 1, iy, iz + 1)) * h2_4;
						D[1][2] = D[2][1] = (ACC(ix, iy - 1, iz - 1) - ACC(ix, iy - 1, iz + 1) - ACC(ix, iy + 1, iz - 1) + ACC(ix, iy + 1, iz + 1)) * h2_4;

						(*pvar)(ix, iy, iz) = (D[0][0] * D[1][1] - D[0][1] * D[0][1] + D[0][0] * D[2][2] - D[0][2] * D[0][2] + D[1][1] * D[2][2] - D[1][2] * D[1][2]);
					}
		}
		else if (order == 4 || order == 6)
		{
			double h2_144 = h2 / 144.;
#pragma omp parallel for
			for (int ix = 0; ix < (int)(*u.get_grid(ilevel)).size(0); ++ix)
				for (int iy = 0; iy < (int)(*u.get_grid(ilevel)).size(1); ++iy)
					for (int iz = 0; iz < (int)(*u.get_grid(ilevel)).size(2); ++iz)
					{
						//.. this is actually 8th order accurate

						double D[3][3];

						D[0][0] = ((ACC(ix - 4, iy, iz) + ACC(ix + 4, iy, iz)) - 16. * (ACC(ix - 3, iy, iz) + ACC(ix + 3, iy, iz)) + 64. * (ACC(ix - 2, iy, iz) + ACC(ix + 2, iy, iz)) + 16. * (ACC(ix - 1, iy, iz) + ACC(ix + 1, iy, iz)) - 130. * ACC(ix, iy, iz)) * h2_144;

						D[1][1] = ((ACC(ix, iy - 4, iz) + ACC(ix, iy + 4, iz)) - 16. * (ACC(ix, iy - 3, iz) + ACC(ix, iy + 3, iz)) + 64. * (ACC(ix, iy - 2, iz) + ACC(ix, iy + 2, iz)) + 16. * (ACC(ix, iy - 1, iz) + ACC(ix, iy + 1, iz)) - 130. * ACC(ix, iy, iz)) * h2_144;

						D[2][2] = ((ACC(ix, iy, iz - 4) + ACC(ix, iy, iz + 4)) - 16. * (ACC(ix, iy, iz - 3) + ACC(ix, iy, iz + 3)) + 64. * (ACC(ix, iy, iz - 2) + ACC(ix, iy, iz + 2)) + 16. * (ACC(ix, iy, iz - 1) + ACC(ix, iy, iz + 1)) - 130. * ACC(ix, iy, iz)) * h2_144;

						D[0][1] = D[1][0] = (64. * (ACC(ix - 1, iy - 1, iz) - ACC(ix - 1, iy + 1, iz) - ACC(ix + 1, iy - 1, iz) + ACC(ix + 1, iy + 1, iz)) - 8. * (ACC(ix - 2, iy - 1, iz) - ACC(ix + 2, iy - 1, iz) - ACC(ix - 2, iy + 1, iz) + ACC(ix + 2, iy + 1, iz) + ACC(ix - 1, iy - 2, iz) - ACC(ix - 1, iy + 2, iz) - ACC(ix + 1, iy - 2, iz) + ACC(ix + 1, iy + 2, iz)) + 1. * (ACC(ix - 2, iy - 2, iz) - ACC(ix - 2, iy + 2, iz) - ACC(ix + 2, iy - 2, iz) + ACC(ix + 2, iy + 2, iz))) * h2_144;
						D[0][2] = D[2][0] = (64. * (ACC(ix - 1, iy, iz - 1) - ACC(ix - 1, iy, iz + 1) - ACC(ix + 1, iy, iz - 1) + ACC(ix + 1, iy, iz + 1)) - 8. * (ACC(ix - 2, iy, iz - 1) - ACC(ix + 2, iy, iz - 1) - ACC(ix - 2, iy, iz + 1) + ACC(ix + 2, iy, iz + 1) + ACC(ix - 1, iy, iz - 2) - ACC(ix - 1, iy, iz + 2) - ACC(ix + 1, iy, iz - 2) + ACC(ix + 1, iy, iz + 2)) + 1. * (ACC(ix - 2, iy, iz - 2) - ACC(ix - 2, iy, iz + 2) - ACC(ix + 2, iy, iz - 2) + ACC(ix + 2, iy, iz + 2))) * h2_144;
						D[1][2] = D[2][1] = (64. * (ACC(ix, iy - 1, iz - 1) - ACC(ix, iy - 1, iz + 1) - ACC(ix, iy + 1, iz - 1) + ACC(ix, iy + 1, iz + 1)) - 8. * (ACC(ix, iy - 2, iz - 1) - ACC(ix, iy + 2, iz - 1) - ACC(ix, iy - 2, iz + 1) + ACC(ix, iy + 2, iz + 1) + ACC(ix, iy - 1, iz - 2) - ACC(ix, iy - 1, iz + 2) - ACC(ix, iy + 1, iz - 2) + ACC(ix, iy + 1, iz + 2)) + 1. * (ACC(ix, iy - 2, iz - 2) - ACC(ix, iy - 2, iz + 2) - ACC(ix, iy + 2, iz - 2) + ACC(ix, iy + 2, iz + 2))) * h2_144;

						(*pvar)(ix, iy, iz) = (D[0][0] * D[1][1] - SQR(D[0][1]) + D[0][0] * D[2][2] - SQR(D[0][2]) + D[1][1] * D[2][2] - SQR(D[1][2]));
					}
		}
		else
			throw std::runtime_error("compute_2LPT_source : invalid operator order specified");
	}

	//.. subtract global mean so the multi-grid poisson solver behaves well

	for (int i = fnew.levelmax(); i > (int)fnew.levelmin(); --i)
		mg_straight().restrict((*fnew.get_grid(i)), (*fnew.get_grid(i - 1)));

	long double sum = 0.0;
	int nx, ny, nz;

	nx = fnew.get_grid(fnew.levelmin())->size(0);
	ny = fnew.get_grid(fnew.levelmin())->size(1);
	nz = fnew.get_grid(fnew.levelmin())->size(2);

	for (int ix = 0; ix < nx; ++ix)
		for (int iy = 0; iy < ny; ++iy)
			for (int iz = 0; iz < nz; ++iz)
				sum += (*fnew.get_grid(fnew.levelmin()))(ix, iy, iz);

	sum /= (double)((size_t)nx * (size_t)ny * (size_t)nz);

	for (unsigned i = fnew.levelmin(); i <= fnew.levelmax(); ++i)
	{
		nx = fnew.get_grid(i)->size(0);
		ny = fnew.get_grid(i)->size(1);
		nz = fnew.get_grid(i)->size(2);

		for (int ix = 0; ix < nx; ++ix)
			for (int iy = 0; iy < ny; ++iy)
				for (int iz = 0; iz < nz; ++iz)
					(*fnew.get_grid(i))(ix, iy, iz) -= sum;
	}
}
#undef SQR
#undef ACC
