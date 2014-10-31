#include "mycallback_mf.h"
#include <QtConcurrent/QtConcurrent>
#include<string>
#include<sstream>
#include<iostream>
#include<functional>
#include"squaredloss_mf.h"
#include<cmath>


using std::cout;
using std::endl;

MyCallback_MF::MyCallback_MF(GLWidget *glwidget_, QObject *parent) :
    Sbs2Callback(parent), glwidget(glwidget_)
{
    N = 1028;//number of sources
    M = 14;//number of eeg channels
    T = 4;//number of time samples to collect
    K = T;


    B = Eigen::MatrixXd::Zero(N,K);
    Bnew = Eigen::MatrixXd::Zero(N,K);;
    C = Eigen::MatrixXd::Zero(K,T);
    Y = Eigen::MatrixXd::Zero(M,T);;
    copy_Y = Eigen::MatrixXd::Zero(M,T);;
    S = Eigen::MatrixXd::Zero(N,T);;

    colorData = Eigen::MatrixXd::Zero(N,4);;

    first_packet = 1;
    collectedSamples = 0;
    visualized = 1;
    is_NewC_ready = 1;
    is_NewB_ready = 0;
    counter_C_updates = 0;
    max_counter_C_updates = 10;

#ifdef Q_OS_ANDROID

    //use stringstream to convert int to string because
    //ndk des not support c++11 function std::to_string yet
    std::stringstream converter_N;
    std::stringstream converte_K;

    converter_N << N;
    converte_K << K;

    std::string path_to_data =
         std::string("/sdcard/Measuring_Time/Simulation_Compute_Only_B/Varying_K") +
         std::string("/N") + converter_N.str() +
         std::string("/K") +
         converte_K.str() + "/Data";
#else
    std::string path_to_data =
         std::string("/home/user/Desktop/Pruebas_en_Matlab/PhD_Matlab_Files") +
         std::string("/Measuring_Time/Simulation_Compute_Only_B/Varying_K") +
         std::string("/N") + std::to_string(N) +
         std::string("/K") +
         std::to_string(K) + "/Data";
#endif



    sparseReconstruction_mf = new SparseReconstruction_MF( M, T, N, path_to_data );

    QObject::connect(sparseReconstruction_mf, SIGNAL( new_C_Ready()) ,
                     this,SLOT( new_C_ready() ) );

    QObject::connect(sparseReconstruction_mf, SIGNAL( new_B_Ready()) ,
                     this,SLOT( new_B_ready() ) );

    //Turn the filter on
    sbs2DataHandler->turnFilterOn( 8, 13, 32 );

}

void MyCallback_MF::getData(Sbs2Packet *packet)
{


    thisPacket = packet;
    currentPacketCounter = packet->counter;
    currentPacket += 1;
    sbs2DataHandler->setThisPacket( thisPacket );

    //Filter the EEG packet
    sbs2DataHandler->filter();

    if( (is_NewC_ready==1) && (collectedSamples>=T) && (visualized==1) )
    {
        visualized = 0;
        is_NewC_ready = 0;

        copy_Y = Y;

        if(first_packet==1)
        {

            first_packet = 0;

            QtConcurrent::run( sparseReconstruction_mf,
                               &SparseReconstruction_MF::receiveFirstPacket,
                               &copy_Y, &B, &Bnew, &C
                               );
        }
        else
        {

            QtConcurrent::run( sparseReconstruction_mf,
                               &SparseReconstruction_MF::updateC,
                               &copy_Y, &B, &C);
        }

    }

    for(int row=0; row<Sbs2Common::channelsNo(); ++row)
    {
        for(int column=(T-1); column>0; --column)
        {
            Y(row,column) = Y(row,column-1);
        }

        Y(row,0) = thisPacket->filteredValues[Sbs2Common::getChannelNames()->at(row)];
    }

    collectedSamples++;

    glwidget->updateGyroX( packet->gyroX );
    glwidget->updateGyroY( packet->gyroY );


}

void MyCallback_MF::createColorMatrix()
{
    const double EPSILON = 1e-8;

    Eigen::VectorXd energy = S.array().square().rowwise().sum();

    //cout<<"Energy before threshold: " << energy.transpose() << endl << endl;

    //set to zero spirious energy (less than EPSILON)
    energy = ((energy.array() <  EPSILON )&&(energy.array()>0)).select(0,energy);

    //cout<<"Energy after threshold: " << energy.transpose() << endl << endl;

    double max_energy = energy.maxCoeff();
    Eigen::VectorXd normalized_energy = energy.array() / max_energy;

    double red, green, blue, source_energy;

    for(int i=0; i<N; i++)
    {
        source_energy = normalized_energy(i);

        if( (source_energy>0) && (source_energy<=0.3) )
        {
            blue = 1-source_energy;
            red = 0;
            green = 0;
        }
        else if( (source_energy>0.3) && (source_energy<=0.6) )
        {
            green = source_energy + 0.3;
            red = 0;
            blue = 0;
        }
        else if( (source_energy>0.6) && (source_energy<=1) )
        {
            red = source_energy;
            green = 0;
            blue = 0;
        }
        else
        {
            red   = 1;
            green = 1;
            blue  = 1;
        }

        colorData(i,0) = red;
        colorData(i,1) = green;
        colorData(i,2) = blue;
        colorData(i,3) = 1.0;
    }



}

void MyCallback_MF::new_C_ready()
{
    S.noalias() = B*C;

    double te = S.array().square().rowwise().sum().sum();

    if ( (te==0)||( std::isnan(te) ) )
    {
        counter_C_updates = 0;
        collectedSamples = 0;
        first_packet = 1;
        B = Eigen::MatrixXd::Zero(N,K);

    }
    else
    {

        createColorMatrix();

        for (int vertex = 0; vertex < colorData.rows(); ++vertex)
        {

            glwidget->model->updateColorForVertex(
                        vertex,
                        colorData(vertex,0),
                        colorData(vertex,1),
                        colorData(vertex,2),
                        colorData(vertex,3)
                        );

        }

        ++counter_C_updates;

        if( (counter_C_updates==max_counter_C_updates) || (is_NewB_ready==1) )
        {

            is_NewB_ready = 0;

            B = Bnew;

            QtConcurrent::run( sparseReconstruction_mf,
                               &SparseReconstruction_MF::updateB,
                               &copy_Y, &C, &Bnew);

        }

     }

    visualized = 1;
    is_NewC_ready = 1;

}


void MyCallback_MF::new_B_ready()
{
    is_NewB_ready = 1;
}
