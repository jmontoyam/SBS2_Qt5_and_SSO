#include "sbs2datahandler.h"
#include <QtConcurrent/QtConcurrent>



Sbs2DataHandler::Sbs2DataHandler(QObject *parent) :
    QObject(parent)
{
    samplesCollected = 0;

    //filtering
    filterOn = 0;
    filterOrder = 0;
    sbs2Filter = 0;
    toFilterValues = 0;
    filterResultValues = 0;

    //recording
    recording = 0;
    sbs2FileHandler = 0;

    //spectral analysis
    spectrogramChannelOn = 0;
    spectrogramChannelSamples = 0;
    spectrogramChannelLength = 0;
    spectrogramChannelDelta = 0;
    spectrogramChannelDeltaCollected = 0;
    toSpectrogramValues = 0;
    spectrogramValues = 0;
    powerValues = 0;
    sbs2Spectrogram = 0;

    //source reconstruction

    isSourceReconstructionReady = 0;
    sourceReconstructionDelta = 0;
    sourceReconstructionDeltaCollected = 0;
    sourceReconstructionSamples = 0;
    sourceReconstructionOn = 0;
    sourceReconstructionPowerOn = 0;
    sourceReconstructionModelUpdateLength = 0;
    sourceReconstructionModelUpdateDelta = 0;
    sbs2SourceReconstruction = 0;
    toSourceReconstructionValues = 0;
    sourceReconstructionValues = 0;
    sourceReconstructionPowerValues = 0;

    //network
    sbs2NetworkHandler = new Sbs2NetworkHandler();
    networkSendRawDataOn = 0;


    QThreadPool::globalInstance()->setMaxThreadCount(6); //3 is minimal right now, annyoing that it needs to be set manually
    qDebug() <<  QThreadPool::globalInstance()->activeThreadCount() << QThreadPool::globalInstance()->maxThreadCount() << QThread::idealThreadCount();


}

void Sbs2DataHandler::setThisPacket(Sbs2Packet *thisPacket_)
{
    thisPacket = thisPacket_;
}

void Sbs2DataHandler::sendRawData()
{
    if (!networkSendRawDataOn)
	return;
    sbs2NetworkHandler->sendRawData(thisPacket->rawData);
}

void Sbs2DataHandler::filter()
{

    if (!filterOn)
	return;

    for (int row=0; row<Sbs2Common::channelsNo(); ++row)
    {
	for (int column = filterOrder; column > 0; --column)
	    (*toFilterValues)[row][column] = (*toFilterValues)[row][column-1];

	(*toFilterValues)[row][0] = thisPacket->values[Sbs2Common::getChannelNames()->at(row)];
    }

    sbs2Filter->doFilter(toFilterValues,filterResultValues);

    for (int row = 0; row<Sbs2Common::channelsNo(); ++row)
	thisPacket->filteredValues[Sbs2Common::getChannelNames()->at(row)] = (*filterResultValues)[row][0];

}

void Sbs2DataHandler::spectrogramChannel()
{
    if (!spectrogramChannelOn)
	return;


    for (int row = 0; row<Sbs2Common::channelsNo(); ++row)
    {
	for (int column = (toSpectrogramValues->dim2() - 1); column > 0; --column)
	    (*toSpectrogramValues)[row][column] = (*toSpectrogramValues)[row][column-1];

	(*toSpectrogramValues)[row][0] = thisPacket->filteredValues[Sbs2Common::getChannelNames()->at(row)];
    }
    ++spectrogramChannelDeltaCollected;

    if (spectrogramChannelDeltaCollected < spectrogramChannelDelta)
	return;

    spectrogramChannelDeltaCollected = 0;

    sbs2Spectrogram->doSpectrogram(toSpectrogramValues,spectrogramValues);


    for (int row = 0; row < Sbs2Common::channelsNo(); ++row)
    {
	for (int column = 0; column < Sbs2Common::samplingRate()/2; ++column)
	{

	    if (column == 0)
		(*powerValues)[row][column] = sqrt(pow((*spectrogramValues)[row][column],2.0));
	    else
		(*powerValues)[row][column] = sqrt(pow((*spectrogramValues)[row][column],2.0) + pow((*spectrogramValues)[row][column+Sbs2Common::samplingRate()/2],2.0));
	}
    }


    emit spectrogramUpdated();

}

void Sbs2DataHandler::sourceReconstruction()
{
    if (!sourceReconstructionOn)
	return;

    if (isSourceReconstructionReady)
    {
	isSourceReconstructionReady = 0;
	emit sourceReconstructionReady();
    }



    for (int row = 0; row<Sbs2Common::channelsNo(); ++row)
    {
	for (int column = (toSourceReconstructionValues->dim2()-1); column > 0; --column)
	    (*toSourceReconstructionValues)[row][column] = (*toSourceReconstructionValues)[row][column-1];
	(*toSourceReconstructionValues)[row][0] = thisPacket->filteredValues[Sbs2Common::getChannelNames()->at(row)];

    }

    ++sourceReconstructionDeltaCollected;
    if (sourceReconstructionDeltaCollected < sourceReconstructionDelta)
	return;

    sourceReconstructionDeltaCollected = 0;

    QtConcurrent::run(sbs2SourceReconstruction,&Sbs2SourceReconstrucion::doRec,toSourceReconstructionValues,sourceReconstructionValues,&isSourceReconstructionReady);

}

void Sbs2DataHandler::sourceReconstructionPower()
{
    if (!sourceReconstructionPowerOn)
	return;

    if (isSourceReconstructionReady)
    {
	isSourceReconstructionReady = 0;
	emit sourceReconstructionPowerReady();
    }



    for (int row = 0; row<Sbs2Common::channelsNo(); ++row)
    {
	for (int column = (toSourceReconstructionValues->dim2()-1); column > 0; --column)
	    (*toSourceReconstructionValues)[row][column] = (*toSourceReconstructionValues)[row][column-1];
	(*toSourceReconstructionValues)[row][0] = thisPacket->filteredValues[Sbs2Common::getChannelNames()->at(row)];

    }

    ++sourceReconstructionDeltaCollected;
    if (sourceReconstructionDeltaCollected < sourceReconstructionDelta)
	return;

    sourceReconstructionDeltaCollected = 0;

    QtConcurrent::run(sbs2SourceReconstruction,&Sbs2SourceReconstrucion::doRecPow,toSourceReconstructionValues,sourceReconstructionPowerValues,&isSourceReconstructionReady);

}



void Sbs2DataHandler::turnFilterOff()
{
    filterOn = 0;
    if (!(toFilterValues == 0))
    {
	delete toFilterValues;
	toFilterValues = 0;
    }
    if (!(filterResultValues == 0))
    {
	delete filterResultValues;
	filterResultValues = 0;
    }
    if (!(sbs2Filter == 0))
    {
	delete sbs2Filter;
	sbs2Filter = 0;
    }

}

void Sbs2DataHandler::turnFilterOn(int fbandLow_, int fbandHigh_, int filterOrder_)
{
    fbandLow = fbandLow_;
    fbandHigh = fbandHigh_;
    filterOrder = filterOrder_;

    if (!(toFilterValues == 0))
    {
	delete toFilterValues;
	toFilterValues = 0;
    }
    if (!(filterResultValues == 0))
    {
	delete filterResultValues;
	filterResultValues = 0;
    }

    sbs2Filter = Sbs2Filter::New(fbandLow,fbandHigh,filterOrder,this);


    toFilterValues = new DTU::DtuArray2D<double> (Sbs2Common::channelsNo(),filterOrder+1);
    filterResultValues = new DTU::DtuArray2D<double> (Sbs2Common::channelsNo(),1);
    reset();
    sbs2Filter->updateFilter(filterOrder,fbandLow,fbandHigh);

    filterOn = 1;
}

void Sbs2DataHandler::record()
{
    if (recording)
	sbs2FileHandler->dumpRawData(thisPacket->rawData);
}

void Sbs2DataHandler::startRecording(QString user, QString description)
{
    user = user.trimmed();
    description = description.trimmed();
    sbs2FileHandler = Sbs2FileHandler::New(this);
    sbs2FileHandler->createMetaFile(user,description);
    recording = 1;

}

void Sbs2DataHandler::stopRecording()
{
    recording = 0;
    sbs2FileHandler->close();
}

void Sbs2DataHandler::turnChannelSpectrogramOff()
{
    spectrogramChannelOn = 0;
    if (!(toSpectrogramValues == 0))
    {
	delete toSpectrogramValues;
	toSpectrogramValues = 0;
    }
    if (!(spectrogramValues == 0))
    {
	delete spectrogramValues;
	spectrogramValues = 0;
    }
    if (!(powerValues == 0))
    {
	delete powerValues;
	powerValues = 0;
    }
    if (!(sbs2Spectrogram == 0))
    {
	delete sbs2Spectrogram;
	sbs2Spectrogram = 0;
    }

}

void Sbs2DataHandler::turnChannelSpectrogramOn(int spectrogramChannelSamples_, int spectrogramChannelLength_, int spectrogramChannelDelta_)
{
    spectrogramChannelSamples = spectrogramChannelSamples_;
    spectrogramChannelLength = spectrogramChannelLength_;
    spectrogramChannelDelta = spectrogramChannelDelta_;
    spectrogramChannelDeltaCollected = 0;

    if (!(toSpectrogramValues == 0))
    {
	delete toSpectrogramValues;
	toSpectrogramValues = 0;
    }
    if (!(spectrogramValues == 0))
    {
	delete spectrogramValues;
	spectrogramValues = 0;
    }
    if (!(powerValues == 0))
    {
	delete powerValues;
	powerValues = 0;
    }
    if (!(sbs2Spectrogram == 0))
    {
	delete sbs2Spectrogram;
	sbs2Spectrogram = 0;
    }

    toSpectrogramValues = new DTU::DtuArray2D<double>(Sbs2Common::channelsNo(),spectrogramChannelSamples);
    spectrogramValues = new DTU::DtuArray2D<double>(Sbs2Common::channelsNo(),spectrogramChannelLength);
    powerValues = new DTU::DtuArray2D<double>(Sbs2Common::channelsNo(),spectrogramChannelLength/2);
    sbs2Spectrogram = new Sbs2Spectrogram(spectrogramChannelLength,this);


    connect(this,SIGNAL(setWindowTypeSignal(Sbs2Spectrogram::WindowType)),sbs2Spectrogram,SLOT(setWindowType(Sbs2Spectrogram::WindowType)));

    (*toSpectrogramValues) = 0;
    (*spectrogramValues) = 0;

    spectrogramChannelOn = 1;

}

void Sbs2DataHandler::setWindowType(Sbs2Spectrogram::WindowType windowType)
{
    emit setWindowTypeSignal(windowType);
}

void Sbs2DataHandler::turnSourceReconstructionOn(int sourceReconstructionSamples_, int sourceReconstructionDelta_, int sourceReconstructionModelUpdateLength_, int sourceReconstructionModelUpdateDelta_, QString hardware_)
{

    hardware = hardware_;
    turnSourceReconstructionPowerOff();

    sourceReconstructionDelta = sourceReconstructionDelta_;
    sourceReconstructionDeltaCollected = 0;
    sourceReconstructionSamples = sourceReconstructionSamples_;
    sourceReconstructionModelUpdateLength = sourceReconstructionModelUpdateLength_; //128 / sourceReconstructionSamples * 20; //20 seconds
    sourceReconstructionModelUpdateDelta = sourceReconstructionModelUpdateDelta_; //128 / sourceReconstructionSamples * 10; //every 10 seconds



    if (!(toSourceReconstructionValues == 0))
    {
	delete toSourceReconstructionValues;
	toSourceReconstructionValues = 0;
    }

    if (!(sourceReconstructionValues == 0))
    {
	delete sourceReconstructionValues;
	sourceReconstructionValues = 0;
    }
    if (!(sbs2SourceReconstruction == 0))
    {
	delete sbs2SourceReconstruction;
	sbs2SourceReconstruction = 0;
    }

    sbs2SourceReconstruction = new Sbs2SourceReconstrucion(Sbs2Common::channelsNo(),sourceReconstructionSamples,sourceReconstructionDelta, Sbs2Common::verticesNo(),hardware, this,sourceReconstructionModelUpdateLength,sourceReconstructionModelUpdateDelta);
    toSourceReconstructionValues = new DTU::DtuArray2D<double>(Sbs2Common::channelsNo(),sourceReconstructionSamples);
    sourceReconstructionValues = new DTU::DtuArray2D<double>(1,Sbs2Common::verticesNo());

    (*toSourceReconstructionValues) = 0;
    (*sourceReconstructionValues) = 0;


    sbs2SourceReconstruction->setMeanExtraction(1);

    sourceReconstructionOn = 1;
}

void Sbs2DataHandler::turnSourceReconstructionOff()
{
    sourceReconstructionOn = 0;
    sourceReconstructionDelta = 0;
    sourceReconstructionDeltaCollected = 0;
    sourceReconstructionSamples = 0;
    sourceReconstructionModelUpdateLength = 0;
    sourceReconstructionModelUpdateDelta = 0;

    if (!(toSourceReconstructionValues == 0))
    {
	delete toSourceReconstructionValues;
	toSourceReconstructionValues = 0;
    }

    if (!(sourceReconstructionValues == 0))
    {
	delete sourceReconstructionValues;
	sourceReconstructionValues = 0;
    }
    if (!(sbs2SourceReconstruction == 0))
    {
	delete sbs2SourceReconstruction;
	sbs2SourceReconstruction = 0;
    }
}

void Sbs2DataHandler::turnSourceReconstructionPowerOn(int sourceReconstructionSamples_, int sourceReconstructionDelta_, int sourceReconstructionModelUpdateLength_, int sourceReconstructionModelUpdateDelta_, QString hardware_)
{

    hardware = hardware_;
    turnSourceReconstructionOff();

    sourceReconstructionDelta = sourceReconstructionDelta_;
    sourceReconstructionDeltaCollected = 0;
    sourceReconstructionSamples = sourceReconstructionSamples_;
    sourceReconstructionModelUpdateLength = sourceReconstructionModelUpdateLength_; //128 / sourceReconstructionSamples * 20; //20 seconds
    sourceReconstructionModelUpdateDelta = sourceReconstructionModelUpdateDelta_; //128 / sourceReconstructionSamples * 10; //every 10 seconds

    if (!(toSourceReconstructionValues == 0))
    {
	delete toSourceReconstructionValues;
	toSourceReconstructionValues = 0;
    }

    if (!(sourceReconstructionPowerValues == 0))
    {
	delete sourceReconstructionPowerValues;
	sourceReconstructionPowerValues = 0;
    }
    if (!(sbs2SourceReconstruction == 0))
    {
	delete sbs2SourceReconstruction;
	sbs2SourceReconstruction = 0;
    }


    sbs2SourceReconstruction = new Sbs2SourceReconstrucion(Sbs2Common::channelsNo(),sourceReconstructionSamples,sourceReconstructionDelta, Sbs2Common::verticesNo(),hardware, this,sourceReconstructionModelUpdateLength,sourceReconstructionModelUpdateDelta);
    toSourceReconstructionValues = new DTU::DtuArray2D<double>(Sbs2Common::channelsNo(),sourceReconstructionSamples);
    sourceReconstructionPowerValues = new DTU::DtuArray2D<double>(Sbs2Common::samplingRate()/2,Sbs2Common::verticesNo());

    (*toSourceReconstructionValues) = 0;
    (*sourceReconstructionPowerValues) = 0;
    sbs2SourceReconstruction->setMeanExtraction(1);
    sourceReconstructionPowerOn = 1;
}

void Sbs2DataHandler::turnSourceReconstructionPowerOff()
{
    sourceReconstructionPowerOn = 0;
    sourceReconstructionDelta = 0;
    sourceReconstructionDeltaCollected = 0;
    sourceReconstructionSamples = 0;
    sourceReconstructionModelUpdateLength = 0;
    sourceReconstructionModelUpdateDelta = 0;

    if (!(toSourceReconstructionValues == 0))
    {
	delete toSourceReconstructionValues;
	toSourceReconstructionValues = 0;
    }

    if (!(sourceReconstructionPowerValues == 0))
    {
	delete sourceReconstructionPowerValues;
	sourceReconstructionPowerValues = 0;
    }
    if (!(sbs2SourceReconstruction == 0))
    {
	delete sbs2SourceReconstruction;
	sbs2SourceReconstruction = 0;
    }
}

void Sbs2DataHandler::turnSendRawDataOn(QString rawDataServerAddress_, int rawDataPort_, int rawDataSize_, int rawDataQueueLength_)
{
    networkSendRawDataOn = 0;
    sbs2NetworkHandler->turnSendRawDataOn(rawDataServerAddress_,rawDataPort_,rawDataSize_,rawDataQueueLength_);
    networkSendRawDataOn = 1;
}

void Sbs2DataHandler::turnSendRawDataOff()
{
    networkSendRawDataOn = 0;
    if (sbs2NetworkHandler != 0)
    {
	sbs2NetworkHandler->turnSendRawDataOff();
    }
}

void Sbs2DataHandler::addRawDataHost(QString address, int port)
{
    if (sbs2NetworkHandler != 0)
    {
	sbs2NetworkHandler->addRawDataHost(address,port);
    }
}

void Sbs2DataHandler::removeRawDataHost(QString address, int port)
{
    if (sbs2NetworkHandler != 0)
    {
	sbs2NetworkHandler->removeRawDataHost(address,port);
    }
}

void Sbs2DataHandler::sendMessage(QString message)
{
    if (sbs2NetworkHandler != 0)
    {
	sbs2NetworkHandler->sendMessage(message);
    }
}

void Sbs2DataHandler::sendMessage(QString message, QString address, int port)
{
    if (sbs2NetworkHandler != 0)
    {
	sbs2NetworkHandler->sendMessage(message,address,port);
    }
}

void Sbs2DataHandler::addMessageUdpOutputHost(QString address, int port)
{
    if (sbs2NetworkHandler != 0)
    {
	sbs2NetworkHandler->addMessageUdpOutputHost(address,port);
    }

}

void Sbs2DataHandler::removeMessageUdpOutputHost(QString address)
{
    if (sbs2NetworkHandler != 0)
    {
	sbs2NetworkHandler->removeMessageUdpOutputHost(address);
    }
}

void Sbs2DataHandler::clearMessageUdpOutputHosts()
{
    if (sbs2NetworkHandler != 0)
    {
	sbs2NetworkHandler->clearMessageUdpOutputHosts();
    }
}

void Sbs2DataHandler::turnReceiveMessageOn(QString address, int port)
{
    if (sbs2NetworkHandler != 0)
    {
	sbs2NetworkHandler->turnReceiveMessageOn(address,port);
	QObject::connect(sbs2NetworkHandler,SIGNAL(messageReceived(QString,QString,int)),this,SLOT(readMessage(QString,QString,int)));
    }

}

void Sbs2DataHandler::turnReceiveMessageOff()
{
    if (sbs2NetworkHandler != 0)
    {
	sbs2NetworkHandler->turnReceiveMessageOff();
	QObject::disconnect(sbs2NetworkHandler,SIGNAL(messageReceived(QString,QString,int)),this,SLOT(readMessage(QString,QString,int)));
    }
}

void Sbs2DataHandler::readMessage(QString data, QString sender, int senderPort)
{
    emit udpMessageReceived(data,sender,senderPort);
}


void Sbs2DataHandler::reset()
{
    if (!(toFilterValues == 0))
    {
	(*toFilterValues) = 0;
    }
    if (!(filterResultValues ==0))
    {
	(*filterResultValues) = 0;
    }
    if (!(toSpectrogramValues ==0))
    {
	(*toSpectrogramValues) = 0;
    }
    if (!(spectrogramValues ==0))
    {
	(*spectrogramValues) = 0;
    }
    if (!(powerValues ==0))
    {
	(*powerValues) = 0;
    }
    if (!(toSourceReconstructionValues == 0))
    {
	(*toSourceReconstructionValues) = 0;
    }

    if (!(sourceReconstructionValues == 0))
    {
	(*sourceReconstructionValues) = 0;
    }
    sourceReconstructionDeltaCollected = 0;
    spectrogramChannelDeltaCollected = 0;
}

void Sbs2DataHandler::insertIntoMetaFile(QString event)
{
    if (sbs2FileHandler == 0)
	return;
    sbs2FileHandler->insertIntoMetaFile(event);
}

DTU::DtuArray2D<double>* Sbs2DataHandler::getPowerValues()
{
    return powerValues;
}

DTU::DtuArray2D<double>* Sbs2DataHandler::getSourceReconstructionPowerValues()
{
    return sourceReconstructionPowerValues;
}

Sbs2DataHandler::~Sbs2DataHandler()
{

}

void Sbs2DataHandler::setVerticesToExtract(QVector<int> *verticesToExtract)
{
    sbs2SourceReconstruction->setVerticesToExtract(verticesToExtract);
}

QString Sbs2DataHandler::getRawFilename()
{
    if (sbs2FileHandler == 0)
	return "";
    return sbs2FileHandler->getRawFilename();
}

int Sbs2DataHandler::getPacketZero()
{
    return sbs2FileHandler->getPacketZero();
}
