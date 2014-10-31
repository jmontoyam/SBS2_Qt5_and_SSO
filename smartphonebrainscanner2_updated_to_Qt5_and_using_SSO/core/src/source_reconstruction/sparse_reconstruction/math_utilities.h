#ifndef MATH_UTILITIES_H
#define MATH_UTILITIES_H

#include<dtu_array_2d.h>
#include<vector>
#include<QString>
#include<sbs2common.h>


void getMean(DTU::DtuArray2D<double> *matrix, DTU::DtuArray2D<double> *matrixMean);

void loadData(QString pathFile, DTU::DtuArray2D<double> *matrix);

void loadData(QString pathFile, double *scalar);

void matrixL21Norm(DTU::DtuArray2D<double> *matrix, double *out);

void matrixFrobNorm(DTU::DtuArray2D<double> *matrix, double *out);

void matrixL21NormEachRow(DTU::DtuArray2D<double> *matrix, vector<double> &out_vector);

void printVector(vector <double> &out_vector);

void printMatrix(DTU::DtuArray2D<double> *matrix);

void scalarDividedbyVectorComponentWise_insitu(double *scalar, vector<double> &x_vector);

void scalarMinusVector_insitu(double *scalar,vector<double> &x_vector);

void thresholding_insitu(vector<double> &x_vector);

void vectorOuterProduct(vector<double> &vector1 , vector<double> &vector2, DTU::DtuArray2D<double> *out);

void matrixMultiplicationComponentWise(DTU::DtuArray2D<double> *matrix_A, DTU::DtuArray2D<double> *matrix_B, DTU::DtuArray2D<double> *out);

void  copyMatrix(DTU::DtuArray2D<double> *matrix_source , DTU::DtuArray2D<double> *matrix_destiny);

#endif // MATH_UTILITIES_H
