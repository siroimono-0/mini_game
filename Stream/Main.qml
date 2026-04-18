import QtQuick
import QtQuick.Controls

Window {
    width: 1920
    height: 1080
    visible: true
    title: qsTr("Hello World")
    color: "#1e1e1e"

    GridView{
        id: view
        model: cpp_module.p_md
        width: 70 * 15; height: 70 * 13;
        anchors.centerIn: parent

        cellWidth: 70; cellHeight: 70;

        delegate: Rectangle{
            width: 70; height: 70;

            Image {
                id: ground
                source: set_ground(model.ground)
                anchors.fill: parent
                fillMode: Image.Stretch
                // fillMode: Image.PreserveAspectFit

                function set_ground(set)
                {
                    // console.log(set + " ground");
                    if(set === 65)
                    {
                        return "./img/ground_green.png";
                        // return
                    }
                    else if(set === 66)
                    {
                        return "./img/ground_gray.png";
                        // return
                    }
                    else if(set === 67)
                    {
                        return "./img/ground_green.png";
                        // return
                    }
                    // else
                    // {
                        // return "./img/block_orange.png";
                    // }
                }
            }
            Image {
                id: item
                source: set_item(model.item)
                anchors.fill: parent
                fillMode: Image.Stretch

                function set_item(set)
                {
                    // console.log(set + "  item");
                    if(set === 32)
                    {
                        return "./img/transparent.png";
                        // return
                    }
                    else if(set === 35)
                    {
                        return "./img/block_orange.png";
                    }
                    else if(set === 64)
                    {
                        return "./img/forest.png";
                    }
                    else if(set === 37)
                    {
                        return "./img/box.png";
                    }
                    else if(set === 94)
                    {
                        return "./img/home.png";
                    }
                    else if(set === 38)
                    {
                        return "./img/tree.png";
                    }
                    else if(set === 80)
                    {
                        return "./img/power.png";
                    }
                    else if(set === 43)
                    {
                        return "./img/boom_plus.png";
                    }
                    else if(set === 33)
                    {
                        return "./img/needle.png";
                    }
                    else if(set === 61)
                    {
                        return "./img/car.png";
                    }
                    else if(set === 88)
                    {
                        return "./img/RIP.png";
                    }
                    else if(set === 42)
                    {
                        return "./img/boom_1.png";
                    }
                    else if(set === 87)
                    {
                        return "./img/boom_2.png";
                    }
                    else if(set === 71)
                    {
                        return "./img/gost.png";
                    }
                    else if(set === 49)
                    {
                        return "./img/ch_1.png";
                    }
                    else if(set === 50)
                    {
                        return "./img/ch_2.png";
                    }
                    else if(set === 51)
                    {
                        return "./img/ch_1_0.png";
                    }
                    else if(set === 52)
                    {
                        return "./img/ch_2_0.png";
                    }
                    else if(set === 53)
                    {
                        return "./img/ch_1_0_0.png";
                    }
                    else if(set === 54)
                    {
                        return "./img/ch_2_0_0.png";
                    }
                }
            }
        }
    }
}
