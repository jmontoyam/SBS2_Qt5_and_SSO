import Qt 4.7

Rectangle {

    color: "white"
    width: 80
    height: 10
    x: 0
    y: width/2
    scale: 1

    property string moving: "up"
    state: "idle"


    states: [
        State {
            name: "idle"
            PropertyChanges {
                target: cross
                moving: "idle"
            }
        },
        State {
            name: "movingUp"
            PropertyChanges {
                target: cross
                moving: "up"
                y: width/2
		x: x

            }
        },
        State {
            name: "movingDown"
            PropertyChanges {
                target: cross
                moving: "down"
                y: page.height - cross.width/2-cross.height
		x: x

            }
        },
        State {
            name: "movingRight"
            PropertyChanges {
                target: cross
                moving: "right"
                x: page.width - cross.width
                y: y

            }
        },
        State {
            name: "movingLeft"
            PropertyChanges {
                target: cross
                moving: "left"
                x: 0
                y: y

            }
        },
        State {
            name: "movingRightDown"
            PropertyChanges {
                target: cross
                moving: "rightDown"
                x: page.width - cross.width
                y: page.height - cross.width/2-cross.height

            }
        },
        State {
            name: "movingLeftDown"
            PropertyChanges {
                target: cross
                moving: "leftDown"
                x: 0
                y: page.height - cross.width/2-cross.height

            }
        },
        State {
            name: "movingRightUp"
            PropertyChanges {
                target: cross
                moving: "rightDown"
                x: page.width - cross.width
                y: width/2

            }
        },
        State {
            name: "movingLeftUp"
            PropertyChanges {
                target: cross
                moving: "leftDown"
                x: 0
                y: width/2

            }
        },
        State {
            name: "center"
            PropertyChanges {
                target: cross
                moving: "center"
                x: page.width/2 - cross.width/2
                y: page.height/2 - cross.width/2

            }
        },
        State {
            name: "blink"
            PropertyChanges {
                target: cross
                moving: "blink"
                x: x
                y: y


            }
        }



    ]

    transitions: [
       Transition {
            id: myTransition

           to: "blink"
           SequentialAnimation
           {
               loops: page.animationLoops
	    ScriptAction {script: page.event("started_"+cross.state+","+cross.x+","+cross.y);}

            PropertyAnimation { target: cross
                               properties: "scale"; to: 20; duration: 100; easing.type: Easing.InOutQuad }
            PropertyAnimation { target: cross
                               properties: "scale"; to: 1; duration: 100; easing.type: Easing.InOutQuad }
	    ScriptAction {script: page.event("finished_"+cross.state+","+cross.x+","+cross.y);}
            PropertyAnimation { target: cross; duration: page.animationInterval}

           }
       } ]

    Behavior on y
    {
	SequentialAnimation
	{
	    ScriptAction {script: page.event("started_"+cross.state+","+cross.x+","+cross.y);}
	    NumberAnimation {duration: 1000}
	    ScriptAction {script: page.event("finished_"+cross.state+","+cross.x+","+cross.y);}
	}
    }
    Behavior on x
    {
	NumberAnimation {duration: 1000}
    }



    Rectangle
    {

        color: parent.color
        anchors.centerIn: parent
        width: parent.height
        height: parent.width
    }








}
