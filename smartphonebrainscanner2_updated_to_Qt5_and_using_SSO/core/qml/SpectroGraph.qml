//import QtQuick 1.1
import QtQuick 1.0


Rectangle {
    id: spect
    width: 200
    height: 100
    property int dwidth: 200
    property int dheight: 100
    property double maxV: 0
    property string name: ""

    color: "white"

    state: "minimized"

    states: [
        State {
            name: "maximized"

            ParentChange
            {
                target: spect
                parent: page
            }
            PropertyChanges {
                target: spect
                anchors.fill: page
            }
        },
        State {
            name: "minimized"
            ParentChange
            {
                target: spect
                parent: spectGrid
            }

            PropertyChanges {
                target: spect
                width: dwidth
                height: dheight



            }
        }
    ]

    MouseArea
    {
        anchors.fill: parent
        onClicked:
        {

            if (parent.state == "minimized")
                parent.state = "maximized"
            else if (parent.state == "maximized")
                parent.state = "minimized"


        }
    }

    property int freqs: 63
    function updateValues(values)
    {

        var vs = values.split(",");
        var max = 0;
        for (var i in vs)
        {
           // if (i < 8 || i > 13)
             //   continue;

            if (i < 3)
                continue

            if (i == freqs)
                break;


            vs[i] = parseFloat(vs[i]);

            if (vs[i] > max)
                max = vs[i]
        }
        //maxV = max;
        maxV = 10;
        for (var i in vs)
        {
            //if (i < 8 || i > 13)
              //  continue;
            if (i < 3)
                continue

            if (i == (freqs))
                break;

            var k = 1+parseFloat(i);

            //children[i].y = parent.height - (vs[i]/max * 0.9 * parent.height);
            children[k].height = (parseFloat(vs[k])/max * 0.9 * parent.height);

        }

    }



    Repeater
    {
        model: freqs
        //Rectangle{
         //   id: rect_index; color: "black"; height: 3; width: parent.width/freqs; x: width*index; y: index
        //}
        Rectangle{
            id: rect_index;
            color: "black";
            height: 3;
            smooth: true;
            width: parent.width/freqs * 0.9;
            x: 0.05* parent.width + width*index;
            y: 0.95 * parent.height - height
           // rotation: 180
            Text
            {
                text: index + 1
                visible: spect.state == "maximized"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.bottom
                color: "black"
            }
        }
    }

    Text
    {

        text: spect.maxV / 10000.0

        anchors.right: parent.children[1].left
        anchors.top: parent.children[1].top
        color: "black"
        visible: spect.state == "maximized"
    }
    Text
    {

        text: spect.name
        anchors.horizontalCenter: spect.horizontalCenter
        anchors.top: spect.top
        color: "black"
        //visible: spect.state == "maximized"
    }

}
