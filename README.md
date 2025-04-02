# HolePuncherUI Project

## What this is about
This is a cross-platform native Qt GUI application that goes with the [Automatic Hole Puncher](https://makerepo.com/Tayyeb/2530.automated-punching-system). The software is deployed on a computer (Raspberry Pi) that is connected to a modified 3D printer with a hole punch in the place of the 3D printer head. Just simply upload an SVG image as a drawing of the hole pattern, this application will send the appropriate G-code command to the 3D printer to punch holes in the film.

## Building instruction
Simply clone this project and open and build it in Qt Creator. You need to install the [Qt Serial Port submodule](https://doc.qt.io/qt-6/qtserialport-index.html) and Qt 6.8.2.

## Usage
After opening the application, click on `Browse...` to select a hole pattern SVG file. Find the port name of your 3D printer and click on `Serial Configuration...` to configure serial connection to the 3D printer. Please consult your 3D printer manual for specific connection settings.
