#ifndef MYECALLBACK_H
#define MYECALLBACK_H

#include <sbs2callback.h>
#include <sbs2datahandler.h>
#include <glwidget.h>
#include <QDateTime>
#include <source_reconstruction/sparse_reconstruction/sparsereconstruction.h>

#include<Eigen/Dense>

class MyCallback : public Sbs2Callback
{
    Q_OBJECT
public:
    explicit MyCallback(GLWidget* glwidget_, QObject *parent = 0);
    void getData(Sbs2Packet *packet);


private:
    DTU::DtuArray2D<double>* verticesData;
    GLWidget* glwidget;

    DTU::DtuArray2D<double>* colorData;
    Eigen::MatrixXd Sn;

    int lowFreq; //included
    int highFreq; //excluded

    QVector<double>* maxValues;
    QVector<double>* minValues;
    double meanPower;
    int meanWindowLength;
    QVector<double> cmap;

    SparseReconstruction *sparseReconstruction2;
    DTU::DtuArray2D<double> *estimated_S;
    DTU::DtuArray2D<double> *Y;
    double error_tol;
    double lambda;
    DTU::DtuArray2D<double> *A;
    double L;
    vector<double> lambdas;
    int isSourceReconstructionReady;

    int collectedSamples;
    int visualized;

private:
    void createColorMatrix(DTU::DtuArray2D<double>* verticesData_);
    void updateModel();

    void createColorMatrix2(DTU::DtuArray2D<double>* verticesData_);


signals:
    
public slots:
    void soureReconstructionPowerReady();
    void changeBand(QString name);
    void updateColorMap(int colorMap);
    void sourceReconstructionReady();
    
};

#endif // MYCALLBACK_H
