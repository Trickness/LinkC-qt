#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent){
    json object =
        {
            {"the good", "il buono"},
            {"the bad", "il cativo"},
            {"the ugly", "il brutto"}
        };

        // output element with key "the ugly"
        std::cout << object.at("the ugly") << '\n';

        // try to read from a nonexisting key
        try
        {
            std::cout << object.at("the fast") << '\n';
        }
        catch (std::out_of_range)
        {
            std::cout << "out of range" << '\n';
        }
}

MainWindow::~MainWindow(){

}