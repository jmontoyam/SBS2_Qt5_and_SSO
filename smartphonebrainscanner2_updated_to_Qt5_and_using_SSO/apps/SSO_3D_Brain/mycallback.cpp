#include "mycallback.h"
#include <QtConcurrent/QtConcurrent>
#include<cmath>

MyCallback::MyCallback(GLWidget *glwidget_, QObject *parent) :
    Sbs2Callback(parent), glwidget(glwidget_)
{



    verticesData = new DTU::DtuArray2D<double>(64,1028);

    colorData = new DTU::DtuArray2D<double>(1028,4); //rgba values
    (*colorData) = 0;


    Sn = Eigen::MatrixXd::Zero(1028,8);

    //QObject::connect(sbs2DataHandler,SIGNAL(sourceReconstructionPowerReady()),this,SLOT(soureReconstructionPowerReady()));


    QObject::connect(glwidget,SIGNAL(turnSourceReconstructionPowerOn(int,int,int,int, QString)),this,SLOT(turnSourceReconstructionPowerOn(int,int,int,int, QString)));
    QObject::connect(glwidget,SIGNAL(changeBand(QString)),this,SLOT(changeBand(QString)));


    QObject::connect(this,SIGNAL(deviceFoundSignal(QMap<QString,QVariant>)),glwidget,SLOT(deviceFound(QMap<QString,QVariant>)));
    qsrand(QDateTime::currentMSecsSinceEpoch());

    updateColorMap(3);


    /* This is optional. Remove to work on all brain or add additional regions. */
    //dtuEmotivRegion = new DtuEmotivRegion();
    //dtuEmotivRegion->addRegion("Frontal_Lobe");


#ifdef Q_WS_MAEMO_5
    meanWindowLength = 16;
#else
    meanWindowLength = 32;
#endif
    maxValues = new QVector<double>();
    minValues = new QVector<double>();

    changeBand("alpha");

    int channels2 = 14;
    int sources2 = 1028;
    int sourcesReconstructionDelta2 = 8;

    int numDatosTrain2 = 8;
    int numDatosTest2 = 6;

    sparseReconstruction2 = new SparseReconstruction(
        0,channels2,sources2,sourcesReconstructionDelta2,numDatosTrain2, numDatosTest2);

    QObject::connect(sparseReconstruction2, SIGNAL(sourceReconstructionReady()),this,SLOT(sourceReconstructionReady()));


    estimated_S = new DTU::DtuArray2D<double>(sources2,sourcesReconstructionDelta2);
    Y = new DTU::DtuArray2D<double>(channels2,sourcesReconstructionDelta2);

    error_tol = 0.0001;
    lambda = 0.1;

    A = new DTU::DtuArray2D<double>(channels2,sources2);
    copyMatrix(sparseReconstruction2->A_normalized,A);
    L = sparseReconstruction2->L;

    //lambdas = vector<double>(5);
    lambdas = vector<double>(1);


    lambdas[0]=0.01;

    //lambdas[1]=0.1;
    //lambdas[2]=1;
    //lambdas[3]=10;
    //lambdas[4]=100;

    isSourceReconstructionReady = 1;

    /*
    for(int i=0; i<Y->dim1() ; i++)
    {
	for(int j=0 ; j<Y->dim2() ; j++)
	{

	    (*Y)[i][j] = (double)(i+j)/100;
	}

    }
*/
    collectedSamples = 0; //fixme

    //QString pathTrainTestData("/Users/arks/Desktop/SparseReconstruction_SmartPhoneBrainScannerFiles/EmotivIntegration/");

    //loadData(pathTrainTestData+"Y.txt",Y);

    visualized = 1;

}

void MyCallback::changeBand(QString name)
{
    minValues->clear();
    maxValues->clear();
    if (name.compare("delta"))
    {
	lowFreq = 1;
	highFreq = 4;
    }
    if (name.compare("theta"))
    {
	lowFreq = 4;
	highFreq = 8;
    }
    if (name.compare("alpha"))
    {
	lowFreq = 8;
	highFreq = 12;
    }
    if (name.compare("lowBeta"))
    {
	lowFreq = 12;
	highFreq = 16;
    }
    if (name.compare("beta"))
    {
	lowFreq = 16;
	highFreq = 20;
    }
}


void MyCallback::getData(Sbs2Packet *packet)
{
    thisPacket = packet;
    currentPacketCounter = packet->counter;
    currentPacket += 1;

    if (isSourceReconstructionReady && collectedSamples >= 8 && visualized)
    {
	//Y->print();
	//std::cout << "***************" <<std::endl;
	//estimated_S->print();
	qDebug() << "HERE!!!";
	visualized = 0;

	QtConcurrent::run(sparseReconstruction2,&SparseReconstruction::doRec,Y,estimated_S,&isSourceReconstructionReady, lambdas, &error_tol);

	//sparseReconstruction2->doRec(Y,estimated_S,&sourceReconstructionReady,lambdas,&error_tol);
    estimated_S->print();
    }





    for (int row = 0; row<Sbs2Common::channelsNo(); ++row)
    {
	for (int column = (Y->dim2()-1); column > 0; --column)
	    (*Y)[row][column] = (*Y)[row][column-1];
	(*Y)[row][0] = thisPacket->filteredValues[Sbs2Common::getChannelNames()->at(row)];

    }

    ++collectedSamples;



    glwidget->updateGyroX(packet->gyroX);
    glwidget->updateGyroY(packet->gyroY);

    sbs2DataHandler->setThisPacket(thisPacket);
    //sbs2DataHandler->sourceReconstructionPower();



}


void MyCallback::soureReconstructionPowerReady()
{
    createColorMatrix(sbs2DataHandler->getSourceReconstructionPowerValues());
    updateModel();

}

void MyCallback::sourceReconstructionReady()
{
    /**********************************/
    /********* Código Original********/
    /*********************************/
    //createColorMatrix2(estimated_S);

    //updateModel();

    /********************************/


    /*****************************************/
    /************* Nuevo código **************/
    /*****************************************/

    // El codigo de las anteriores dos
    // lineas era todo el codigo de esta funcion.
    // Pero dado que llama a muchas funciones una detras
    // de otra, aparecia un error muy extraño de corrupcion
    // de memoria. He reescrito el codigo haciendo todas las
    // llamadas en una sola linea, evitando así tanto salto del
    // program counter, y el codigo funciona, y parece más rápido
    // que antes!

    createColorMatrix2(estimated_S);


    for (int vertex = 0; vertex < colorData->dim1(); ++vertex)
    {

        glwidget->model->updateColorForVertex(
                            vertex,
                            (*colorData)[vertex][0],
                            (*colorData)[vertex][1],
                            (*colorData)[vertex][2],
                            (*colorData)[vertex][3]);

    }


    /*
    for (int vertex = 0; vertex < colorData->dim1(); ++vertex)
    {
        int low = 0;
        int high = 255;
        double rand_r = (qrand() % ((high + 1) - low) + low)/255.0;
        double rand_g = (qrand() % ((high + 1) - low) + low)/255.0;
        double rand_b = (qrand() % ((high + 1) - low) + low)/255.0;

        glwidget->model->updateColorForVertex(
                            vertex,
                            rand_r,
                            rand_g,
                            rand_b,
                            (*colorData)[vertex][3]);

    }
    */

    visualized = 1;

}

void MyCallback::createColorMatrix2(DTU::DtuArray2D<double> *verticesData_)
{
    /*
    for (int vertex = 0; vertex<verticesData_->dim1(); ++vertex)
    {
        double v = 0.0;
        double sum = 0.0;

        for (int timesample = 0; timesample < verticesData_->dim2(); ++timesample)
        {
            //sum += (*verticesData_)[vertex][timesample];

            //Prueba std::abs
            sum += std::abs( (*verticesData_)[vertex][timesample] );
        }

        if (sum > 0)
        {
            v = 0.5;
            //qDebug()<<"vertex: "<<vertex<<", sum: "<<sum;
        }


        (*colorData)[vertex][0] = 1.0-v;
        (*colorData)[vertex][1] = 1.0-v;
        (*colorData)[vertex][2] = 1.0;
        (*colorData)[vertex][3] = 1.0;
    }*/



    for (int vertex = 0; vertex<verticesData_->dim1(); ++vertex)
    {
        for (int timesample = 0; timesample < verticesData_->dim2(); ++timesample)
        {
            Sn(vertex, timesample) = (*verticesData_)[vertex][timesample];
        }
    }

    Eigen::VectorXd energy = Sn.array().square().rowwise().sum();
    double max_energy = energy.maxCoeff();
    Eigen::VectorXd normalized_energy = energy.array() / max_energy;

    double red, green, blue, source_energy;

    for(int i=0;i<Sn.rows();i++)
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

        (*colorData)[i][0] = red;
        (*colorData)[i][1] = green;
        (*colorData)[i][2] = blue;
        (*colorData)[i][3] = 1.0;
    }


    //qDebug()<<"**************************************************************";

}


/**
  This function sees 64x1028 input matrix and should produce 1028x3 color matrix ready to pass to visualization.
  */


void MyCallback::createColorMatrix(DTU::DtuArray2D<double> *verticesData_)
{
    double meanMax = 0;
    double meanMin = 0;


    for (int t=0; t<minValues->size(); ++t)
    {
	meanMin += minValues->at(t);
    }
    if (minValues->size())
	meanMin /= (double)minValues->size();
    for (int t=0; t<maxValues->size(); ++t)
    {
	meanMax += maxValues->at(t);
    }
    if (maxValues->size())
	meanMax /= (double)maxValues->size();

    double currentMax = -999999999;
    double currentMin = 9999999999;

    double scaling = meanMax - meanMin;

    for (int vertex = 0; vertex<verticesData_->dim2(); ++vertex)
    {
	double this_vertex_power = 0.0;
	for (int freq = lowFreq; freq < highFreq; ++freq)
	{
	    this_vertex_power += (*verticesData_)[freq][vertex];

	}
	this_vertex_power = 20 * qLn(this_vertex_power + 1)/qLn(10);
	if (this_vertex_power > currentMax) currentMax = this_vertex_power;
	if (this_vertex_power < currentMin) currentMin = this_vertex_power;

	double v = 0.0;


	v += (this_vertex_power - meanMin)/scaling * 1.0;
	if (v < 0.5) v = 0;
	if (v > 1.0) v = 1.0;


	(*colorData)[vertex][0] = 1.0 - v;
	(*colorData)[vertex][1] = 1.0 - v;
	(*colorData)[vertex][2] = 1.0;
	(*colorData)[vertex][3] = 1.0;



    }


    minValues->append(currentMin);
    if (minValues->size() == meanWindowLength) minValues->erase(minValues->begin());
    maxValues->append(currentMax);
    if (maxValues->size() == meanWindowLength) maxValues->erase(maxValues->begin());



}


//void MyEmotivCallback::createColorMatrix(DTU::DtuArray2D<double>* verticesData_)
//{



//    /*
//    This uses moving average over meanWindowLength time bins.
//    Standard frequency of 3D reconstruction we use is 8Hz, so setting meanWindowLength = 32 equals to 4 second average.
//    For Maemo we set frequency of 3D reconstruction to 4Hz due to insufficient processing power and set meanWindowLength = 16
//    The mean is calculated over sum of powers in the band for all vertices.
//    Each vertex is colored red if it is above average (taken from the last time bin to do everything in one iteration.
//    If vertex power is below average, it will stay gray. If it is 2 times the average, it will be fully red. Linearly red in between.
//      */

//    meanPower = 0.0;
//    for (int t=0; t<meanValues->size(); ++t)
//    {
//	meanPower += meanValues->at(t);
//    }
//    if (meanValues->size())
//	meanPower /= (double)meanValues->size();


//    double sum_power = 0;
//    for (int vertex = 0; vertex < verticesData_->dim2(); ++vertex)
//    {
//	double this_vertex_power = 0.0;
//	for (int freq = lowFreq; freq < highFreq; ++freq)
//	{
//	    this_vertex_power += (*verticesData_)[freq][vertex];

//	}

//	//Color lookup
//	/*
//	double v = int(( (std::min(std::max(this_vertex_power/((double)meanPower + 0.0001),1.0),2.0)-1.0)) * 255);
//	(*colorData)[vertex][0] = cmap.at( v * 3);
//	(*colorData)[vertex][1] = cmap.at( v * 3 + 1);
//	(*colorData)[vertex][2] = cmap.at( v * 3 + 2);
//	*/
//	/*
//	(*colorData)[vertex][0] = 0.5 + (std::min(std::max(this_vertex_power/((double)meanPower + 0.0001),1.0),2.0)-1.0) * 0.5;
//	(*colorData)[vertex][1] = 0.5;
//	(*colorData)[vertex][2] = 0.5;
//	*/

//	/* rainbow brain
//	int chooser = qrand()%3;
//	(*colorData)[vertex][0] = 0.5;
//	(*colorData)[vertex][1] = 0.5;
//	(*colorData)[vertex][2] = 0.5;

//	(*colorData)[vertex][chooser] = 0.5 + (std::min(std::max(this_vertex_power/((double)meanPower + 0.0001),1.0),2.0)-1.0) * 0.5;
//	*/

//	(*colorData)[vertex][0] = 0.5;
//	(*colorData)[vertex][1] = 0.5;
//	(*colorData)[vertex][2] = 0.5 + (std::min(std::max(this_vertex_power/((double)meanPower + 0.0001),1.0),2.0)-1.0) * 0.5;



//	(*colorData)[vertex][3] = 1.0;
//	sum_power += this_vertex_power;

//    }


//    if (dtuEmotivRegion != 0)
//	meanValues->append(sum_power/dtuEmotivRegion->getVerticesToExtract()->size());
//    else
//	meanValues->append(sum_power/colorData->dim1());
//    if (meanValues->size() == meanWindowLength) meanValues->erase(meanValues->begin());



//}

void MyCallback::updateModel()
{


    for (int vertex = 0; vertex < colorData->dim1(); ++vertex)
    {

    glwidget->updateColorForVertex(vertex,(*colorData)[vertex][0],(*colorData)[vertex][1],(*colorData)[vertex][2], (*colorData)[vertex][3]);
    }

    visualized = 1;

}

void MyCallback::updateColorMap(int colorMap)
{

    qDebug() << "updating color map "<<colorMap;

    QString filename(":/colortable");
    filename.append(QString::number(colorMap));

    QFile file2(filename);

    if (!file2.open(QIODevice::ReadOnly | QIODevice::Text))
	qDebug() <<"file problem";

    int i=0;
    cmap.clear();
    while (!file2.atEnd())
    {
	QByteArray line = file2.readLine();
	QString str = line.data();
	QStringList list1 = str.split(",");
	for (int j = 0; j < list1.size(); j++)
	{
	    cmap.push_back(list1.at(j).toDouble());
	}
	i++;

    }

}

