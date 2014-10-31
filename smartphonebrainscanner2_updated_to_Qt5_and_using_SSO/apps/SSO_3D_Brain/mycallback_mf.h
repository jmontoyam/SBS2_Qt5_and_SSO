#ifndef MYCALLBACK_MF_H
#define MYCALLBACK_MF_H

#include <sbs2callback.h>
#include <sbs2datahandler.h>
#include <glwidget.h>
#include<Eigen/Dense>
#include "sparsereconstruction_mf.h"

class MyCallback_MF : public Sbs2Callback
{
    Q_OBJECT
public:
    explicit MyCallback_MF(GLWidget* glwidget_ ,QObject *parent = 0);
    void getData(Sbs2Packet *packet);

private:
    GLWidget* glwidget;

    SparseReconstruction_MF *sparseReconstruction_mf;



    Eigen::MatrixXd B;
    Eigen::MatrixXd Bnew;
    Eigen::MatrixXd C;
    Eigen::MatrixXd Y;
    Eigen::MatrixXd copy_Y;
    Eigen::MatrixXd S;

    Eigen::MatrixXd colorData;

    int N;
    int M;
    int K;
    int T;

    int first_packet;
    int collectedSamples;
    int visualized;
    int is_NewC_ready;
    int is_NewB_ready;
    int counter_C_updates;
    int max_counter_C_updates;

    void createColorMatrix();




signals:

public slots:
    void new_C_ready();
    void new_B_ready();

};

#endif // MYCALLBACK_MF_H
