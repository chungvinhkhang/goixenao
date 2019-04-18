//
//  main.cpp
//  goixenao-tx
//
//  Created by Administrator on 4/14/19.
//  Copyright © 2019 Lyd. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#pragma mark - Prototype: UI
void loginAndRun();
void displayMenu();
void run(int option, string userName);
void getBooking(string userName);
void updateLocation(string userName);
void displayPlaces(string places[], int length);

#pragma mark - Prototype: Data
void getPlaces(string places[], int& length);
void updateDriver(string userName, string placeCode, string status);

#pragma mark - Main
int main(int argc, const char * argv[]) {
    loginAndRun();
    return 0;
}

#pragma mark - Implementation
void loginAndRun() {
   
    string userName;
    cout << "Please log in: " << endl;
    cout << "User name: ";
    cin >> userName; cin.ignore();
    int option;
    do {
        displayMenu();
        cin >> option; cin.ignore();
        run(option , userName);
    } while(option != 0);
}


void displayMenu() {
    cout << "Menu: " << endl
    << "1. Get booking" << endl
    << "2. Update location" << endl
    << "0. Exit" << endl
    << "Choose: ";
}

void run(int option, string userName) {
    switch (option) {
        case 1:
            getBooking(userName);
            break;
        case 2:
            updateLocation(userName);
            break;
        default:
            break;
    }
}

void getBooking(string userName) {
    ifstream ifsBooking;
    ifsBooking.open(userName + "_booking.txt");
    if(ifsBooking.fail()) {
        cout << "There's no booking found" << endl;
    }else{
        string guestName, startPlaceCode, startPlace, endPlaceCode, endPlace, distance, rate, nextPrioDrivers;
        getline(ifsBooking, guestName);
        getline(ifsBooking, startPlaceCode);
        getline(ifsBooking, startPlace);
        getline(ifsBooking, endPlaceCode);
        getline(ifsBooking, endPlace);
        getline(ifsBooking, distance);
        getline(ifsBooking, rate);
        getline(ifsBooking, nextPrioDrivers);
        ifsBooking.close();
        cout << "Guest: " << guestName
            << " is booking a trip from " << startPlace
        << " to " << endPlace << "(" << distance << ") : " << rate << endl;
        cout << "Do you confirm to pick him/her? (1: Yes, 0: No)";
        int confirm;
        cin >> confirm; cin.ignore();
        if (confirm == 1) {
            rename((userName + "_booking.txt").c_str(), (userName + "_confirmedbooking.txt").c_str());
            updateDriver(userName, "", "1");
            cout << "Safe driving! Please press enter after finished this trip.";
            getchar();
            rename((userName + "_confirmedbooking.txt").c_str(), (userName + "_finishedbooking.txt").c_str());
            updateDriver(userName, endPlaceCode, "0");
        } else {
            string driverUserName = "ALL_REJECTED";
            if (nextPrioDrivers.length() > 1) {
                string delimiter = ",";
                size_t index = nextPrioDrivers.find(delimiter);
                driverUserName = nextPrioDrivers.substr(0, index);
                ofstream ofsBooking;
                ofsBooking.open(driverUserName + "_booking.txt");
                ofsBooking << guestName << endl;
                ofsBooking << startPlaceCode << endl;
                ofsBooking << startPlace << endl;
                ofsBooking << endPlaceCode << endl;
                ofsBooking << endPlace << endl;
                ofsBooking << distance << endl;
                ofsBooking << rate << endl;
                ofsBooking << nextPrioDrivers.substr(index + delimiter.length());
                ofsBooking.close();
            }
            
            remove((userName + "_booking.txt").c_str());
            
            ofstream ofsAssignedDriver;
            ofsAssignedDriver.open(guestName + "_driver.txt", ofstream::out | ofstream::trunc);
            ofsAssignedDriver << driverUserName;
            ofsAssignedDriver.close();
        }
    }
}

void updateLocation(string userName) {
    string places[1000];
    int placesLength = 0;
    cout << "Where are you now?" << endl;
    getPlaces(places, placesLength);
    displayPlaces(places, placesLength);
    cout << "Please select: ";
    int placeCode;
    cin >> placeCode; cin.ignore();
    updateDriver(userName, to_string(placeCode), "0");
}

void updateDriver(string userName, string placeCode, string status){
    ifstream ifsDriverPlace;
    ofstream ofsDriverPlaceTemp;
    const char* fileDriverPlaceName = "driver_place_status.txt";
    const char* tempFileDriverPlaceName = "~driver_place_status.tmp";
    
    ifsDriverPlace.open(fileDriverPlaceName);
    ofsDriverPlaceTemp.open(tempFileDriverPlaceName);
    string line;
    string delimiter = "\t";
    string currentStatus = "";
    string currentPlaceCode;
    while (getline(ifsDriverPlace, line))
    {
        size_t firstDelimiterIndex = line.find(delimiter);
        string driverUserName = line.substr(0, firstDelimiterIndex);
        if(userName.compare(driverUserName) != 0) {
            ofsDriverPlaceTemp << line << endl;
        }else{
            size_t secondDelimiterIndex = line.find(delimiter, firstDelimiterIndex + 1);
            currentStatus = line.substr(secondDelimiterIndex + 1);
            currentPlaceCode = line.substr(firstDelimiterIndex + delimiter.length(), secondDelimiterIndex - firstDelimiterIndex - 1);
        }
    }
    ifsDriverPlace.close();
    if(placeCode != "") {
        currentPlaceCode = placeCode;
    }
    if(status != "") {
        currentStatus = status;
    }
    ofsDriverPlaceTemp << userName << "\t" << currentPlaceCode << "\t" << currentStatus;
    ofsDriverPlaceTemp.close();
    
    remove(fileDriverPlaceName);
    rename(tempFileDriverPlaceName, fileDriverPlaceName);
}


void getPlaces(string places[], int& length){
    ifstream ifsPlace;
    ifsPlace.open("_place.txt");
    string line;
    length = 0;
    while (getline(ifsPlace, line))
        if(line.length() > 0)
            places[length++] = line.substr(line.find("\t") + 1);
    ifsPlace.close();
}

void displayPlaces(string places[], int length) {
    for (int i = 0; i < length; i++) {
        cout << i << "." << places[i] << endl;
    }
}
