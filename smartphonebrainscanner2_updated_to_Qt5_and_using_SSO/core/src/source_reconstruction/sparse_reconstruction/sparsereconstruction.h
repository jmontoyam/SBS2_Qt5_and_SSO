#ifndef SPARSERECONSTRUCTION_H
#define SPARSERECONSTRUCTION_H

#include <QObject>

#include<dtu_array_2d.h>
#include<vector>

#include<QString>
#include<source_reconstruction/sparse_reconstruction/math_utilities.h>
//#include <glwidget.h>


class SparseReconstruction : public QObject
{
    Q_OBJECT
public:
    explicit SparseReconstruction(QObject *parent = 0,
				  int channels_input=14,
				  int sources_input=1028,
				  int sourcesReconstructionDelta_input=8,
				  int numDatosTrain_input = 8,
				  int numDatosTest_input = 6);


//function out = f_objective_general_group_lasso(A_normalized,S,Y,lambda)
void f_objective_general_group_lasso(DTU::DtuArray2D<double> *A_normalized,
					   DTU::DtuArray2D<double> *S,
					   DTU::DtuArray2D<double> *Y,
					   double *lambda,
					   double *out);

//function out = derivative_square_loss_frobenius(A,Y,S)
void derivative_square_loss_frobenius(DTU::DtuArray2D<double> *A,
				 DTU::DtuArray2D<double> *Y,
				 DTU::DtuArray2D<double> *S,
				 DTU::DtuArray2D<double> *out);

//function out = proximal_operator_standard_group_lasso(X,regularizer_factor)
void proximal_operator_standard_group_lasso(DTU::DtuArray2D<double> *X,
					    double *regularizer_factor,
					    DTU::DtuArray2D<double> *out);

//function estimated_S = fista_method_group_lasso_v2(A_normalized,Y,error_tol,lambda,L)
void fista_method_group_lasso_v2(DTU::DtuArray2D<double> *A_normalized,
				 DTU::DtuArray2D<double> *Y,
				 double *error_tol,
				 double *lambda,
				 double *L,
				 DTU::DtuArray2D<double> *estimated_S);

void cross_validation_k_channel(DTU::DtuArray2D<double> *Y_mean_0,
				vector<double> &lambdas,
				double *error_tol,
				DTU::DtuArray2D<double> *estimated_S);

void rootMeanSquareError(DTU::DtuArray2D<double> *Y_mean_0_test,
			 DTU::DtuArray2D<double> *A_normalized_test,
			 DTU::DtuArray2D<double> *estimated_S,
			 double *rmse);

void doRec(DTU::DtuArray2D<double> *Y_input,
	   DTU::DtuArray2D<double> *S_output,
	   int *isSourceReconstructionReady,
	   vector<double> &lambdas,
	   double *error_tol);


void preprocessData();


/**Variables used for loading all the forward models for training and testing**/

DTU::DtuArray2D<double> *A_normalized;
double L;
/*
DTU::DtuArray2D<double> *A_normalized_train_1;
double L_train_1;

DTU::DtuArray2D<double> *A_normalized_train_2;
double L_train_2;

DTU::DtuArray2D<double> *A_normalized_train_3;
double L_train_3;

DTU::DtuArray2D<double> *A_normalized_test_1;
DTU::DtuArray2D<double> *A_normalized_test_2;
DTU::DtuArray2D<double> *A_normalized_test_3;
*/
private:



/****************Main Variables*******************/

DTU::DtuArray2D<double> *Y;

DTU::DtuArray2D<double> *Y_meanMatrix;

DTU::DtuArray2D<double> *S;
//DTU::DtuArray2D<double> *A;

int channels;
int sources;
int sourcesReconstructionDelta;
int sourcesReconstructionDeltaCollected;

int numDatosTrain;
int numDatosTest;

/**************************************************/

/*****************Variables used inside the functions****************************/

/*Variables used in the function f_objective_general_group_lasso*/
DTU::DtuArray2D<double> *A_S;//A_S = A*S
DTU::DtuArray2D<double> *A_S_minus_Y ;//A_S_minus_Y = A*S-Y

/*Variables used in the function derivative_square_loss_frobenius*/
DTU::DtuArray2D<double> *A_transpose;//A_transpose = A'

/*Variables used in the function proximal_operator_standard_group_lasso*/
vector<double> norm_2_each_row;
vector<double> ones_vector;
DTU::DtuArray2D<double> * thresholdingMatrix;

/**Variables used in the function fista_method_group_lasso_v2**/
DTU::DtuArray2D<double> *S_ant;
DTU::DtuArray2D<double> *V;
DTU::DtuArray2D<double> *grad_f_y;
DTU::DtuArray2D<double> *S_act;
DTU::DtuArray2D<double> *one_over_L_grad_f_y;
DTU::DtuArray2D<double> *V_minus_one_over_L_grad_f_y;
DTU::DtuArray2D<double> *S_act_minus_S_ant;
DTU::DtuArray2D<double> *scaled_S_act_minus_S_ant;








/**********************************************************************************/



signals:

void sourceReconstructionReady();

public slots:

};

#endif // SPARSERECONSTRUCTION_H
