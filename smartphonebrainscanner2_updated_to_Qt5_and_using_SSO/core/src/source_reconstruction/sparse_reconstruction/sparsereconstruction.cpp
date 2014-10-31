#include "sparsereconstruction.h"

#include<cassert>
#include<iostream>

#include<sbs2common.h>

SparseReconstruction::SparseReconstruction(QObject *parent,
					   int channels_input,
					   int sources_input,
					   int sourcesReconstructionDelta_input,
					   int numDatosTrain_input,
					   int numDatosTest_input) :
    QObject(parent),channels(channels_input),
    sources(sources_input), sourcesReconstructionDelta(sourcesReconstructionDelta_input),
    numDatosTrain(numDatosTrain_input), numDatosTest(numDatosTest_input)
{
    /************************************ Main Variables ************************************/

    sourcesReconstructionDeltaCollected = 0;

    Y = new DTU::DtuArray2D<double>(channels, sourcesReconstructionDelta);//channel matrix
    S = new DTU::DtuArray2D<double>(sources, sourcesReconstructionDelta);//sources matrix
    //A = new DTU::DtuArray2D<double>(channels, sources);//lead-field matrix (forward model)

    Y_meanMatrix = new DTU::DtuArray2D<double>(channels, sourcesReconstructionDelta);//channel matrix


    /****************************************************************************************/


    /************************ Variables used inside the functions *******************************/

    /*Variables used in the function f_objective_general_group_lasso*/

    A_S = new DTU::DtuArray2D<double>(channels, sourcesReconstructionDelta);
    A_S_minus_Y = new DTU::DtuArray2D<double>(channels, sourcesReconstructionDelta);

    /*Variables used in the function derivative_square_loss_frobenius*/

    A_transpose = new DTU::DtuArray2D<double>(sources, channels);

    /*Variables used in the function proximal_operator_standard_group_lasso*/

    norm_2_each_row.resize(sources);//contains the L21 of each row
    ones_vector.resize(sourcesReconstructionDelta,1);
    thresholdingMatrix = new DTU::DtuArray2D<double>(sources, sourcesReconstructionDelta);

    /**Variables used in the function fista_method_group_lasso_v2**/

    S_ant = new DTU::DtuArray2D<double>(sources, sourcesReconstructionDelta);
    V = new DTU::DtuArray2D<double>(sources, sourcesReconstructionDelta);
    grad_f_y = new DTU::DtuArray2D<double>(sources, sourcesReconstructionDelta);
    S_act = new DTU::DtuArray2D<double>(sources, sourcesReconstructionDelta);
    one_over_L_grad_f_y = new DTU::DtuArray2D<double>(sources, sourcesReconstructionDelta);
    V_minus_one_over_L_grad_f_y = new DTU::DtuArray2D<double>(sources, sourcesReconstructionDelta);
    S_act_minus_S_ant = new DTU::DtuArray2D<double>(sources, sourcesReconstructionDelta);
    scaled_S_act_minus_S_ant = new DTU::DtuArray2D<double>(sources, sourcesReconstructionDelta);

    /**********************************************************************************************/


    /*** Load all the forward models for training and testing ***/

    A_normalized = new DTU::DtuArray2D<double>(channels,sources);
/*
    A_normalized_train_1 = new DTU::DtuArray2D<double>(numDatosTrain, sources);
    A_normalized_train_2 = new DTU::DtuArray2D<double>(numDatosTrain, sources);
    A_normalized_train_3 = new DTU::DtuArray2D<double>(numDatosTrain, sources);

    A_normalized_test_1 = new DTU::DtuArray2D<double>(numDatosTest, sources);
    A_normalized_test_2 = new DTU::DtuArray2D<double>(numDatosTest, sources);
    A_normalized_test_3 = new DTU::DtuArray2D<double>(numDatosTest, sources);
*/

    //#ifndef Q_WS_ANDROID
    #ifndef Q_OS_ANDROID
    QString pathTrainTestData("/home/user/smartphonebrainscanner2_data/hardware/emotiv/train_test_forward_models_3_sets_4_4_6/");
    qDebug()<<"-------- Math Files loaded from Desktop -----------";
    #else
    QString pathTrainTestData("/sdcard/smartphonebrainscanner2_data/hardware/emotiv/train_test_forward_models_3_sets_4_4_6/");
    qDebug()<<"-------- Math Files loaded from Android -----------";
    #endif

    //loadData(pathTrainTestData+"A_normalized.txt",A_normalized);

    std::cout<<"p0"<<std::endl;

    //QPainter painter(glwidget);
    //painter.fillRect(0, 100, 128, 128, Qt::white);

    loadData(pathTrainTestData+"newforwardmodel_spheres_reduced_1028.txt",A_normalized);

    std::cout<<"p1"<<std::endl;

    loadData(pathTrainTestData+"L_forwardmodel_sphere_reduced_1028.txt",&L);

    std::cout<<"p2"<<std::endl;

    /*
    loadData(pathTrainTestData+"A_normalized_train_1.txt",A_normalized_train_1);
    loadData(pathTrainTestData+"L_train_1.txt",&L_train_1);

    loadData(pathTrainTestData+"A_normalized_train_2.txt",A_normalized_train_2);
    loadData(pathTrainTestData+"L_train_2.txt",&L_train_2);

    loadData(pathTrainTestData+"A_normalized_train_3.txt",A_normalized_train_3);
    loadData(pathTrainTestData+"L_train_3.txt",&L_train_3);

    loadData(pathTrainTestData+"A_normalized_test_1.txt",A_normalized_test_1);
    loadData(pathTrainTestData+"A_normalized_test_2.txt",A_normalized_test_2);
    loadData(pathTrainTestData+"A_normalized_test_3.txt",A_normalized_test_3);

*/

    /**********************************************************************************************/

}

void SparseReconstruction::f_objective_general_group_lasso(DTU::DtuArray2D<double> *A_normalized,
							   DTU::DtuArray2D<double> *S,
							   DTU::DtuArray2D<double> *Y,
							   double *lambda,
							   double *out)
{
    //group_lasso_S = sum(sqrt(sum(S.^2,2)));
    //out = 1/2*(norm(A_normalized*S - Y,'fro'))^2 + lambda*group_lasso_S;

    //group_lasso_S = || ||_{21}
    double group_lasso_S;
    matrixL21Norm(S,&group_lasso_S);

    //A_S = A*S
    A_normalized->multiply(S,A_S);

    //A_S_minus_Y = A*S-Y
    A_S->subtract(Y,A_S_minus_Y);

    //frobenius = || ||_{F}
    double frobenius;
    matrixFrobNorm(A_S_minus_Y,&frobenius);

    (*out) = 0.5*pow(frobenius,2) + (*lambda)*group_lasso_S;



}



void SparseReconstruction::derivative_square_loss_frobenius(DTU::DtuArray2D<double> *A,
							    DTU::DtuArray2D<double> *Y,
							    DTU::DtuArray2D<double> *S,
							    DTU::DtuArray2D<double> *out)
{
    //out = (A')*( A*S - Y );

    //A_transpose = A'
    A->transpose(A_transpose);

    //A_S = A*S
    A->multiply(S,A_S);

    //A_S_minus_Y = A*S-Y
    A_S->subtract(Y,A_S_minus_Y);

    //out = (A')*( A*S - Y )
    A_transpose->multiply(A_S_minus_Y,out);


}




void SparseReconstruction::proximal_operator_standard_group_lasso(DTU::DtuArray2D<double> *X,
								  double *regularizer_factor,
								  DTU::DtuArray2D<double> *out)
{
    /*
    %norm 2 of each row
    norm_2_each_row = sqrt(sum(x.^2,2));

    %Group lasso proximal operator
    temp = 1 - (regularizer_factor./ norm_2_each_row);
    temp(temp<0) = 0;
    temp2 = temp*ones(1,size(x,2));
    out = x.*temp2;
    */

    //norm_2_each_row  = || ||_{21}
    matrixL21NormEachRow(X , norm_2_each_row );

    //regularizer_factor./ norm_2_each_row
    scalarDividedbyVectorComponentWise_insitu(regularizer_factor , norm_2_each_row);

    // temp = 1 - (regularizer_factor./ norm_2_each_row);
    double temp = 1;
    scalarMinusVector_insitu(&temp , norm_2_each_row);

    //temp(temp<0) = 0;
    thresholding_insitu(norm_2_each_row);

    //vectorOuterProduct -> thresholdingMatrix

    //thresholdingMatrix = temp*ones(1,size(x,2))
    vectorOuterProduct(norm_2_each_row, ones_vector, thresholdingMatrix);

    //matrixMultiplicationComponentWise -> out = x.*thresholdingMatrix;
    matrixMultiplicationComponentWise(X,thresholdingMatrix,out);

}


void SparseReconstruction::fista_method_group_lasso_v2(DTU::DtuArray2D<double> *A_normalized,
						       DTU::DtuArray2D<double> *Y,
						       double *error_tol,
						       double *lambda,
						       double *L,
						       DTU::DtuArray2D<double> *estimated_S)
{
    /*
    S_ant = zeros(size(A_normalized,2),size(Y,2));
    t_ant = 1;

    flag = 1;

    iteration_counter = 0;

    f_obj_ant = f_objective_general_group_lasso(A_normalized,S_ant,Y,lambda);

    V = S_ant;


    regularizer_factor = lambda/L;

    while (flag==1)

	grad_f_y = derivative_square_loss_frobenius(A_normalized,Y,V);

	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	S_act = proximal_operator_standard_group_lasso(V - 1/L*grad_f_y,regularizer_factor);

	t_act = (1 + sqrt(1 + 4*(t_ant^2))) / 2;

	V = S_act + ((t_ant - 1)/t_act) * (S_act - S_ant);

	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	f_obj_act = f_objective_general_group_lasso(A_normalized,S_act,Y,lambda);

	error_rel = abs(f_obj_act - f_obj_ant);

	if error_rel<=error_tol

	    flag=0;

	end

	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	t_ant = t_act;
	S_ant = S_act;
	f_obj_ant = f_obj_act;
	iteration_counter = iteration_counter + 1;
	%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    end

    estimated_S = S_act;

    */

    (*S_ant)=0;

    double t_ant = 1;

    int flag = 1;

    double iteration_counter = 0;

    double f_obj_ant;
    f_objective_general_group_lasso(A_normalized, S_ant, Y, lambda, &f_obj_ant);

    (*V) = 0;

    double regularizer_factor = (*lambda) / (*L);

    /****************************************************************************************************/
    /*Variable to be used inside the while loop*/
    /****************************************************************************************************/

    double t_act;

    double f_obj_act;

    double error_rel;

    /****************************************************************************************************/

    //qDebug() << (*lambda);
    while(flag==1)
    {

	derivative_square_loss_frobenius(A_normalized,Y,V,grad_f_y);

	grad_f_y->multiply( ( (double)1.0 )/(*L) , one_over_L_grad_f_y);

	V->subtract( one_over_L_grad_f_y , V_minus_one_over_L_grad_f_y);

	proximal_operator_standard_group_lasso(V_minus_one_over_L_grad_f_y , &regularizer_factor , S_act);

	t_act = ( 1.0 + sqrt( 1.0 + 4.0*pow(t_ant,2) ) ) / 2.0;

	S_act->subtract(S_ant,S_act_minus_S_ant);
	S_act_minus_S_ant->multiply( ( (t_ant-1.0)/t_act ) , scaled_S_act_minus_S_ant);
	S_act->add(scaled_S_act_minus_S_ant,V);

	f_objective_general_group_lasso(A_normalized,S_act,Y,lambda,&f_obj_act);

	error_rel = abs( f_obj_act - f_obj_ant );

	//std::cout<<"error_rel: "<<error_rel<<std::endl;
	//std::cout<<"f_obj_act: "<<f_obj_act<<" , f_obj_ant: "<<f_obj_ant<<" , L: "<<(*L)<<std::endl;


    if (iteration_counter==50)
    {
        flag=0;
    }

	if(error_rel <= (*error_tol))
	{
	    flag=0;
	}


	t_ant = t_act;
	copyMatrix(S_act,S_ant);//S_ant = S_act
	f_obj_ant = f_obj_act;
	++iteration_counter;
	/******************************************************************************************/

    }

    copyMatrix(S_act,estimated_S);//estimated_S = S_act;

    /************************************/
    // Print some variable for debugging
    /************************************/

    /*
    std::cout<<"\n*****************************************************\n";

    std::cout<<"\nf_obj_ant: "<<f_obj_ant<<std::endl<<std::endl;

    std::cout<<"\nV: "<<std::endl;
    V->print();

    std::cout<<"\nregularizer_factor: "<<regularizer_factor<<std::endl<<std::endl;

    std::cout<<"\ngrad_f_y:"<<std::endl;
    grad_f_y->print();

    std::cout<<"\nS_act:"<<std::endl;
    S_act->print();

    std::cout<<"\nt_act: "<<t_act<<std::endl<<std::endl;

    std::cout<<"\nV: "<<std::endl;
    V->print();

    std::cout<<"\nf_obj_act: "<<f_obj_act<<std::endl;

    std::cout<<"\nerror_rel: "<<error_rel<<std::endl;

    /*
    std::cout<<"\nS_act (before copyMatrix):"<<std::endl;
    S_act->print();
    std::cout<<"\nS_ant (before copyMatrix):"<<std::endl;
    S_ant->print();

    std::cout<<"\nS_act (after copyMatrix):"<<std::endl;
    S_act->print();
    std::cout<<"\nS_ant (after copyMatrix):"<<std::endl;
    S_ant->print();
    */


    /************************************/

}

void SparseReconstruction::cross_validation_k_channel(DTU::DtuArray2D<double> *Y_mean_0,
						      vector<double> &lambdas,
						      double *error_tol,
						      DTU::DtuArray2D<double> *estimated_S)
{
    double lambda;
    double rmse_temp;
    vector<double> rmse(lambdas.size());

    int pos_best_lambda = 0;


    for(int i=0 ; i<lambdas.size() ; i++)
    {
	lambda = lambdas[i];

	fista_method_group_lasso_v2(A_normalized, Y_mean_0, error_tol,
				    &lambda, &L, estimated_S);


	rootMeanSquareError(Y_mean_0, A_normalized, estimated_S, &rmse_temp);

	rmse[i] = rmse_temp;

	//std::cout<<"i: "<<i<<" , rmse[i]: "<<rmse[i]<<std::endl;

    /*
	if (i!=0)
	{
	    if(rmse[i]<rmse[0])
	    {
		rmse[0]=rmse[i];
		pos_best_lambda=i;
	    }
	}
    */

	//std::cout<<"pos_best_lambda: "<<pos_best_lambda<<std::endl;


    }


    double best_lambda = lambdas[pos_best_lambda];

    //qDebug() << "best lambda: " <<best_lambda << pos_best_lambda;

    fista_method_group_lasso_v2(A_normalized, Y_mean_0, error_tol,
				&best_lambda, &L, estimated_S);


}

void SparseReconstruction::rootMeanSquareError(DTU::DtuArray2D<double> *Y,
					       DTU::DtuArray2D<double> *A,
					       DTU::DtuArray2D<double> *estimated_S,
					       double *rmse)
{
    double N = (Y->dim1())*(Y->dim2());

    //A_S = A*S
    A->multiply(estimated_S,A_S);

    //A_S_minus_Y = A*S-Y
    A_S->subtract(Y,A_S_minus_Y);

    double frobenius;
    matrixFrobNorm(A_S_minus_Y,&frobenius);

    (*rmse) = ( ((double)1.0)/sqrt(N) )*frobenius;

}


void SparseReconstruction::doRec(DTU::DtuArray2D<double> *Y_input,
				 DTU::DtuArray2D<double> *S_output,
				 int *isSourceReconstructionReady,
				 vector<double> &lambdas,
				 double *error_tol)
{
    (*isSourceReconstructionReady) = 0;

    S = S_output;


    for (int row=0; row<Y_input->dim1(); ++row)
    {
	for (int column=0; column<Y_input->dim2(); ++column)
	{
	    (*Y)[row][column] = (*Y_input)[row][column];
	}
    }


    preprocessData();

    //Y->print();
    //std::cout<<"\n******************************************"<<std::endl;

    //call cross-validation method
    cross_validation_k_channel(Y,lambdas,error_tol,S);



    (*isSourceReconstructionReady) = 1;
    emit sourceReconstructionReady();

}

void SparseReconstruction::preprocessData()
{
    getMean(Y,Y_meanMatrix);
    Y->subtract(Y_meanMatrix, Y);
}
