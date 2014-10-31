#include "sparsereconstruction_mf.h"
#include"general_utilities.h"
#include<QDebug>

#include<iostream>


using std::cout;
using std::endl;

SparseReconstruction_MF::SparseReconstruction_MF(int eeg_channels, int timesamples,
                                                 int sources, const std::string &path_to_data,
                                                 double percentage_Lambda_Max, double error_Tol_Fista,
                                                 int max_Iter_Fista, QObject *parent) :
    M(eeg_channels), T(timesamples), N(sources), percentage_lambda_max(percentage_Lambda_Max),
    error_tol_fista(error_Tol_Fista), max_it_fista(max_Iter_Fista), QObject(parent)
{
    qDebug() << " **** Percentage **** : " << percentage_lambda_max;

    path_to_A   = path_to_data + "/" + "A.txt";
    path_to_L_a = path_to_data + "/" + "L_a.txt";

    K = T;

    A = Eigen::MatrixXd::Zero(M,N);
    D = Eigen::MatrixXd::Zero(K,K);
    E = Eigen::MatrixXd::Zero(K,T);
    tmp_duality_factor = Eigen::MatrixXd::Zero(N,K);
    I = Eigen::MatrixXd::Identity(K,K);

    SSO::loadMatrixFromFile( path_to_A, A );

    lambda = 0;
    lambda_max = 0;
    L_a = SSO::loadNumberFromFile( path_to_L_a );
    L = 0;
    final_duality_gap_fista = 0;
    final_iteration_fista = 0;


}


void SparseReconstruction_MF::receiveFirstPacket( Eigen::MatrixXd* pY,
                                                  Eigen::MatrixXd* pB,
                                                  Eigen::MatrixXd* pB_new,
                                                  Eigen::MatrixXd* pC
                                                )
{
    Eigen::MatrixXd& Y = *pY;
    Eigen::MatrixXd& B = *pB;
    Eigen::MatrixXd& B_new = *pB_new;
    Eigen::MatrixXd& C = *pC;

    Eigen::JacobiSVD<Eigen::MatrixXd> svd_Y( Y, Eigen::ComputeThinV );
    Eigen::MatrixXd V = svd_Y.matrixV();
    C = V.transpose();
    tmp_duality_factor = ( A.transpose() * Y * C.transpose() );
    lambda_max = reg_l21.evalDuality( tmp_duality_factor );
    lambda = lambda_max * percentage_lambda_max;
    L =  L_a * ( C * C.transpose() ).norm();

    SSO::SquaredLoss_MF squaredloss_mf( A, Y, B, C);



    SSO::fista( Y, B, squaredloss_mf, reg_l21, lambda, L, error_tol_fista,
                final_iteration_fista, final_duality_gap_fista, max_it_fista );


    B_new = B;

    emit new_C_Ready();

    cout << "Start." << endl;

}

void SparseReconstruction_MF::updateC( Eigen::MatrixXd* pY,
                                       Eigen::MatrixXd* pB,
                                       Eigen::MatrixXd* pC)
{
    Eigen::MatrixXd& Y = *pY;
    Eigen::MatrixXd& B = *pB;
    Eigen::MatrixXd& C = *pC;

    D = B.transpose()*A.transpose()*A*B + I;

    E = B.transpose()*A.transpose()*Y;

    C = D.ldlt().solve(E);

    emit new_C_Ready();

   cout << "C has been updated." << endl;

}


void SparseReconstruction_MF::updateB( Eigen::MatrixXd*  pY,
                                       Eigen::MatrixXd*  pC,
                                       Eigen::MatrixXd*  pBnew)
{
    Eigen::MatrixXd& Y = *pY;
    Eigen::MatrixXd& Bnew = *pBnew;
    Eigen::MatrixXd& C = *pC;

    SSO::SquaredLoss_MF squaredloss_mf( A, Y, Bnew, C);

    tmp_duality_factor = ( A.transpose() * Y * C.transpose() );
    lambda_max = reg_l21.evalDuality( tmp_duality_factor );
    lambda = lambda_max * percentage_lambda_max;
    L =  L_a * ( C*C.transpose()).norm();

    SSO::fista( Y, Bnew, squaredloss_mf, reg_l21, lambda, L, error_tol_fista,
                final_iteration_fista, final_duality_gap_fista, max_it_fista );

    emit new_B_Ready();

    cout << "B has been updated." << endl;
}
