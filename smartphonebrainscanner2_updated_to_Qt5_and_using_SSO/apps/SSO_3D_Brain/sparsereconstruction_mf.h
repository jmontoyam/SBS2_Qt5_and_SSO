#ifndef SPARSERECONSTRUCTION_MF_H
#define SPARSERECONSTRUCTION_MF_H

#include <QObject>
#include<string>
#include<Eigen/Dense>
#include"regularizer_l21.h"
#include"fista_method.h"
#include"squaredloss_mf.h"


class SparseReconstruction_MF : public QObject
{
    Q_OBJECT
public:
    explicit SparseReconstruction_MF(int eeg_channels, int timesamples,
                                     int sources, const std::string& path_To_Data,
                                     double percentage_Lambda_Max = 0.2,
                                     double error_Tol_Fista = 1e-1,
                                     int max_Iter_Fista = 500,
                                     QObject *parent = 0);


    void receiveFirstPacket( Eigen::MatrixXd* pY,
                             Eigen::MatrixXd* pB,
                             Eigen::MatrixXd* pB_new,
                             Eigen::MatrixXd* pC
                           );

    void updateC( Eigen::MatrixXd* pY,
                  Eigen::MatrixXd* pB,
                  Eigen::MatrixXd* pC);


    void updateB( Eigen::MatrixXd*  pY,
                  Eigen::MatrixXd*  pC,
                  Eigen::MatrixXd*  pBnew);

private:
    int N;//number of sources
    int M;//number of eeg channels
    int T;//number of time samples
    int K;

    double percentage_lambda_max;
    double error_tol_fista;
    int max_it_fista;

    std::string path_to_A;
    std::string path_to_L_a;

    Eigen::MatrixXd A;
    Eigen::MatrixXd D;
    Eigen::MatrixXd E;
    Eigen::MatrixXd I;
    Eigen::MatrixXd tmp_duality_factor;

    SSO::Regularizer_L21 reg_l21;

    double lambda;
    double lambda_max;
    double L_a;
    double L;
    double final_duality_gap_fista;

    int final_iteration_fista;



signals:
    void new_C_Ready();
    void new_B_Ready();

public slots:

};

#endif // SPARSERECONSTRUCTION_MF_H
