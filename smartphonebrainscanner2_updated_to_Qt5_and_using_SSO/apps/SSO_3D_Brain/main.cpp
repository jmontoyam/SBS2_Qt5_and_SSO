#include<iostream>
#include<QApplication>

#include"glwidget.h"

#include"mycallback.h"
#include"mycallback_mf.h"

#include <hardware/emotiv/sbs2emotivdatareader.h>

int main( int argc, char* argv[] )
{

    QApplication app( argc, argv );

    qDebug() << "rootAppPath: "<<Sbs2Common::setDefaultRootAppPath();

    QSurfaceFormat format;
    format.setDepthBufferSize( 24 );
    //format.setSamples(4);

    GLWidget window;
    window.setFormat(format);
    window.resize(640, 480);
    window.show();

    MyCallback_MF* myCallback = new MyCallback_MF(&window);
    Sbs2EmotivDataReader* sbs2DataReader = Sbs2EmotivDataReader::New(myCallback);


    return app.exec();
}
