//
//  main.cpp
//  goixenao-tx
//
//  Created by Administrator on 4/14/19.
//  Copyright © 2019 Lyd. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

struct DriverInfo{
    string driverName;
    int placeCode;
    string status;
    double distance;
};

#pragma mark - Prototype: UI
void loginAndRun();
void displayMenu();
void run(int option, string userName);
void bookCar(string userName);
void displayPlaces(string places[], int length);

#pragma mark - Prototype: Business
void selectionSortByDistanceThenTakeTop(DriverInfo* arr, int length, int takeTop);

#pragma mark - Prototype: Data
//----data:driver
string getCurrentAssignedDriver(string userName);
void getTop10NearestDrivers(int placeCode, DriverInfo* driversInfo, int& driversInfoLength);

//----data:booking
void saveBooking(string assignedDriverName, string userName, int pickPlaceCode, string pickPlace, int destinationCode, string destination, double distance, double rate, string nextPrioDrivers);
bool isBookingConfirmed(string userName, string driverName);
bool isBookingFinished(string userName, string driverName);

//----data:place
void getPlaces(string places[], int& length);

//----data:distance
double* calculateDistances(int* fromCodes, int destinationCode, int fromCodeLength);
double calculateDistance(int fromCode, int destinationCode);

//----data:rate
double getRatePerKm();

#pragma mark - Prototype: Common
//----common:array
void addIfNotExist(int arr[], int& length, int value);
bool isExistedInArray(int* arr, int length, int value);

#pragma mark - Main
//----main
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
    cout << "Choose menu: " << endl
    << "1. Book car" << endl
    << "0. Exit" << endl
    << "Choose: ";
}

void run(int option, string userName) {
    switch (option) {
        case 1:
            bookCar(userName);
            break;
        default:
            break;
    }
}

void bookCar(string userName) {
    int pickPlaceCode, destinationCode;
    string places[1000];
    int placeLength;
    getPlaces(places, placeLength);
    displayPlaces(places, placeLength);
    cout << "Enter your location: ";
    cin >> pickPlaceCode; cin.ignore();
    cout << "Enter destination: ";
    cin >> destinationCode; cin.ignore();
    
    double distance = calculateDistance(pickPlaceCode, destinationCode);
    double ratePerKm = getRatePerKm();
    double rate = distance * ratePerKm;
    int confirm;
    cout << "The estimated price is: " << rate << "vnd (distance: " << distance << "km)" << endl << "Do you want to book (1: yes, 0: no)?  ";
    cin >> confirm; cin.ignore();
    if(confirm == 0) {
        return;
    }
    
    DriverInfo drivers[1000];
    int driversLength;
    getTop10NearestDrivers(pickPlaceCode, drivers, driversLength);
   
    string currentAssignedDriver = "";
    if(driversLength > 0) {
        string nextPrioDrivers = "";
        string assignedDriverName = drivers[0].driverName;
        for (int i = 1; i < driversLength; i++) {
            nextPrioDrivers += drivers[i].driverName + ",";
        }
        string pickPlace = places[pickPlaceCode];
        string destination = places[destinationCode];
        
        saveBooking(assignedDriverName, userName, pickPlaceCode, pickPlace, destinationCode, destination, distance, rate, nextPrioDrivers);
        
        do {
            cout << "Waiting for driver's confirm...(press enter to refresh the confirmation)" << endl;
            getchar();
            currentAssignedDriver = getCurrentAssignedDriver(userName);
            if(currentAssignedDriver == "ALL_REJECTED") {
                currentAssignedDriver = "";
                break;
            }
            if(currentAssignedDriver != "" && isBookingConfirmed(userName, currentAssignedDriver)){
                break;
            }
        } while(1);
        
    }
    
    if(currentAssignedDriver == ""){
        cout << "There's no available driver now." << endl;
    }else{
        do {
            cout <<  "Driver " << currentAssignedDriver << " is accepted the booking and process your request soon. Have a nice trip. (press enter when arrived)";
            getchar();
            if(currentAssignedDriver != "" && isBookingFinished(userName, currentAssignedDriver)){
                cout << "Thanks for booking." << endl;
                break;
            }
        } while(1);
    }
}

void displayPlaces(string places[], int length) {
    for (int i = 0; i < length; i++) {
         cout << i << "." << places[i] << endl;
    }
}

double getRatePerKm() {
    double rate = 0.0;
    ifstream ifsRate;
    ifsRate.open("_rate.txt");
    string line;
    getline(ifsRate, line);
    rate = atof(line.c_str());
    ifsRate.close();
    return rate;
}

void getTop10NearestDrivers(int pickUpPlaceCode, DriverInfo* driversInfo, int& driversInfoLength) {
    ifstream ifsDriverPlaceStatus;
    ifsDriverPlaceStatus.open("driver_place_status.txt");
    
    string delimiter = "\t";
    
    string lineDrivePlaceStatus;
    int driverPlaceCodes[1000];
    driversInfoLength = 0;
    int numOfDistinctPlaceCodes = 0;
    while(getline(ifsDriverPlaceStatus, lineDrivePlaceStatus)){
        size_t firstDelimIndex = lineDrivePlaceStatus.find(delimiter);
        size_t secondDelimIndex = lineDrivePlaceStatus.find(delimiter, firstDelimIndex + 1);
        
        string status = lineDrivePlaceStatus.substr(secondDelimIndex + 1);
        if(status == "0") {
            DriverInfo& driverInfo = driversInfo[driversInfoLength];
            driverInfo.driverName = lineDrivePlaceStatus.substr(0, firstDelimIndex);
            driverInfo.placeCode = stoi(lineDrivePlaceStatus.substr(firstDelimIndex + 1, secondDelimIndex - firstDelimIndex - 1));
            driverInfo.status = status;
            
            driversInfoLength++;
            
            addIfNotExist(driverPlaceCodes, numOfDistinctPlaceCodes, driverInfo.placeCode);
        }
    }
    
    double* distancesByPlaceCode = calculateDistances(driverPlaceCodes, pickUpPlaceCode, numOfDistinctPlaceCodes);
    for (int i = 0; i < driversInfoLength; i++) {
        driversInfo[i].distance = distancesByPlaceCode[driversInfo[i].placeCode];
    }
    delete[] distancesByPlaceCode;
    
    ifsDriverPlaceStatus.close();
    selectionSortByDistanceThenTakeTop(driversInfo, driversInfoLength, 10);
}


void addIfNotExist(int arr[], int& length, int value) {
    for (int i = 0; i < length ; i++) {
        if(arr[i] == value)
            return;
    }
    arr[length] = value;
    length++;
}


double* calculateDistances(int* fromCodes, int destinationCode, int fromCodeLength) {
    double* distancesByFromCode = new double[1000];
    ifstream ifPlaceDistance;
    ifPlaceDistance.open("_place_distance.txt");
    
    string delimiter = "\t";
    
    string lineDistance;
    
    while(getline(ifPlaceDistance, lineDistance)){
        size_t firstDelimIndex = lineDistance.find(delimiter);
        size_t secondDelimIndex = lineDistance.find(delimiter, firstDelimIndex + 1);
        string from = lineDistance.substr(0, firstDelimIndex);
        string to = lineDistance.substr(firstDelimIndex + 1, secondDelimIndex - firstDelimIndex - 1);
        if(destinationCode == stoi(to)){
            int fromCode = stoi(from);
            if(isExistedInArray(fromCodes, fromCodeLength, fromCode)) {
                string distanceValue = lineDistance.substr(secondDelimIndex + 1);
                distancesByFromCode[fromCode] = stod(distanceValue);
            }
         }
    }
    ifPlaceDistance.close();
    
    distancesByFromCode[destinationCode] = 0;
    return distancesByFromCode;
}


double calculateDistance(int fromCode, int destinationCode){
    int fromCodes[1];
    fromCodes[0] = fromCode;
    double* d = calculateDistances(fromCodes, destinationCode, 1);
    double result = d[0];
    delete d;
    return result;
}

void selectionSortByDistanceThenTakeTop(DriverInfo* arr, int length, int takeTop) {
    if(takeTop > length)
        takeTop = length;
    for(int i = 0; i < takeTop ; i++) {
        int minDistanceIndex = i;
        for(int j = i + 1; j < length; j++){
            if(arr[j].distance < arr[minDistanceIndex].distance){
                minDistanceIndex = j;
            }
        }
        swap(arr[i], arr[minDistanceIndex]);
    }
}


bool isExistedInArray(int* arr, int length, int value) {
    for (int i = 0; i < length; i++)
        if(arr[i] == value)
            return true;
    return false;
}

void saveBooking(string assignedDriverName, string userName, int pickPlaceCode, string pickPlace, int destinationCode, string destination, double distance, double rate, string nextPrioDrivers) {
    ofstream ofDriverBooking;
    ofDriverBooking.open(assignedDriverName + "_booking.txt");

    ofDriverBooking << std::fixed;
    ofDriverBooking << std::setprecision(2);

    ofDriverBooking << userName << endl
        << pickPlaceCode << endl
        << pickPlace << endl
        << destinationCode << endl
        << destination << endl
        << distance << "km" << endl;
    
    ofDriverBooking << std::setprecision(0);
    
    ofDriverBooking << rate << "vnd" << endl
        << nextPrioDrivers << endl;
    
    ofDriverBooking.close();
    
    ofstream ofAssignedDriver;
    ofAssignedDriver.open(userName + "_driver.txt", ofstream::out | ofstream::trunc);
    ofAssignedDriver << assignedDriverName << endl;
    ofAssignedDriver.close();
}

string getCurrentAssignedDriver(string userName) {
    ifstream ifAssignedDriver;
    ifAssignedDriver.open(userName + "_driver.txt");
    string driverName;
    getline(ifAssignedDriver, driverName);
    ifAssignedDriver.close();
    return driverName;
}


bool isBookingConfirmed(string userName, string driverName){
    ifstream ifConfirmedBooking;
    ifConfirmedBooking.open(driverName + "_confirmedbooking.txt");
    bool failedToOpenConfirmFile = ifConfirmedBooking.fail();
    if (!failedToOpenConfirmFile) {
        string guestName;
        getline(ifConfirmedBooking, guestName);
        ifConfirmedBooking.close();
        return guestName == userName;
    }
    ifConfirmedBooking.close();
    return false;
}

bool isBookingFinished(string userName, string driverName){
    ifstream ifConfirmedBooking;
    ifConfirmedBooking.open(driverName + "_finishedbooking.txt");
    bool failedToOpenConfirmFile = ifConfirmedBooking.fail();
    if (!failedToOpenConfirmFile) {
        string guestName;
        getline(ifConfirmedBooking, guestName);
        ifConfirmedBooking.close();
        return guestName == userName;
    }
    ifConfirmedBooking.close();
    return false;
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
