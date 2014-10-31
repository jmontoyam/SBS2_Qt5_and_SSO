//import QtQuick 1.0
import QtQuick 1.0

Rectangle {
    id: page
    width: 1280
    height: 720


    function timeTick()
    {
        aliveTick = (aliveTick + 1)%100;
    }

    function batteryValue(value)
    {
        battery = value;
    }

    function spectrogramUpdated()
    {
        powerValues = callback.getPower();

        channel4.updateValues(powerValues.split(";")[4]);
        channel5.updateValues(powerValues.split(";")[5]);

    }

    function cqUpdated(name, value)
    {
        scalpmap.cqValue(name,value);
    }

    signal filterOn(int fbandLow, int fbandHigh,int order);
    signal filterOff();
    signal startRecording(string user, string description);
    signal stopRecording();
    signal setWindowType(int windowType);
    signal channelSpectrogramOn(int samples, int length, int delta);
    signal channelSpectrogramOff();
    signal sourceReconstructionOn(int samples, int delta, int modelUpdateLength, int modelUpdateDelta);
    signal sourceReconstructionOff();


    property double battery: 0
    property int aliveTick: 0
    property string powerValues: ""
    property int channelSpectrogramTurnedOn: 0
    property int sourceReconstructionTurnedOn: 0

    color: Qt.rgba(aliveTick/100.0,0,0,1);

    Text {
        text: "battery: " + page.battery
        anchors.centerIn: parent
        color: "white"
    }


    Grid
    {
        id: spectGrid
        anchors.left: parent.left
        anchors.top: buttonsGrid.bottom
        anchors.topMargin: 20
        SpectroGraph{id: channel4; name: "o2"}
        SpectroGraph{id: channel5; name: "o1"}
    }






    Grid
    {
        id: buttonsGrid
        spacing: 10
        Rectangle
        {
            color: "blue"
            width: 100
            height: 50
            property int fbandLow: 8
            property int fbandHigh: 30
            property int order: 32
            Text{
                text: ""+parent.fbandLow + "-" + parent.fbandHigh + ", "+parent.order
            }
            MouseArea
            {
                anchors.fill: parent
                onClicked:
                {
                    page.filterOn(parent.fbandLow,parent.fbandHigh,parent.order);
                }
            }
        }
        Rectangle
        {
            color: "blue"
            width: 100
            height: 50
            property int fbandLow: 8
            property int fbandHigh: 13
            property int order: 32
            Text{
                text: ""+parent.fbandLow + "-" + parent.fbandHigh + ", "+parent.order
            }
            MouseArea
            {
                anchors.fill: parent
                onClicked:
                {
                    page.filterOn(parent.fbandLow,parent.fbandHigh,parent.order);
                }
            }
        }
        Rectangle
        {
            color: "red"
            width: 100
            height: 50

            Text{
                text: "filter off"
            }
            MouseArea
            {
                anchors.fill: parent
                onClicked:
                {
                    page.filterOff();
                }
            }
        }

        Rectangle
        {
            color: "blue"
            width: 100
            height: 50
            Text{
                text: "RECT"
            }
            MouseArea
            {
                anchors.fill: parent
                onClicked:
                {
                    page.setWindowType(0);
                }
            }
        }
        Rectangle
        {
            color: "blue"
            width: 100
            height: 50
            Text{
                text: "HANN"
            }
            MouseArea
            {
                anchors.fill: parent
                onClicked:
                {
                    page.setWindowType(1);
                }
            }
        }
        Rectangle
        {
            color: "blue"
            width: 100
            height: 50
            Text{
                text: "HAMMING"
            }
            MouseArea
            {
                anchors.fill: parent
                onClicked:
                {
                    page.setWindowType(2);
                }
            }
        }
        Rectangle
        {
            color: "blue"
            width: 100
            height: 50
            Text{
                text: {if (page.channelSpectrogramTurnedOn) return "ch. spect."; return "ch. spect"}
            }
            MouseArea
            {
                anchors.fill: parent
                onClicked:
                {
                    if (page.channelSpectrogramTurnedOn)
                    {
                        page.channelSpectrogramOff();
                        page.channelSpectrogramTurnedOn = 0;
                    }
                    else
                    {
                        page.channelSpectrogramOn(128,128,16);
                        page.channelSpectrogramTurnedOn = 1;
                    }


                }
            }
        }

        Rectangle
        {
            color: "blue"
            width: 100
            height: 50
            Text{
                text: {if (page.sourceReconstructionTurnedOn) return "sr off"; return "sr on"}
            }
            MouseArea
            {
                anchors.fill: parent
                onClicked:
                {
                    if (page.sourceReconstructionTurnedOn)
                    {
                        page.sourceReconstructionOff();
                        page.sourceReconstructionTurnedOn = 0;
                    }
                    else
                    {
			//FIXME
			//page.sourceReconstructionOn(16,16,8*20,8*30);
			page.sourceReconstructionOn(16,16,8*1,8*1);
                        page.sourceReconstructionTurnedOn = 1;
                    }


                }
            }
        }



        Rectangle
        {
            color: Qt.rgba(recording,0,0);
            border.width: 1
            border.color: "white"
            width: 50
            height: width
            radius: width/2
            smooth: true
            property int recording: 0
            Text
            {
                anchors.centerIn: parent
                text: parent.recording
                color: "white"
            }

            MouseArea
            {
                anchors.fill: parent
                onClicked:
                {
                    if (parent.recording)
                    {
                        page.stopRecording();
                        parent.recording = 0;
                    }
                    else
                    {
                        parent.recording = 1;
                        page.startRecording("test_user","test_event");
                    }
                }
            }

        }

    }

    Scalpmap{id: scalpmap; anchors.top: spectGrid.bottom; anchors.left: parent.left; anchors.topMargin: 20}


}
