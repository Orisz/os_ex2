#ifndef OS_2_ATM_H
#define OS_2_ATM_H

#include <iostream>
#include <ostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include "Account.h"
#include "Bank.h"

using namespace std;

extern Bank* pBank;

class Atm{
public:
    Atm();
    Atm(int id, string commandsFileName);
    ~Atm();
    void DoOperation(string command , int args[]); // read operations lines from file, and execute each one
    void MainAtmLoop();
    // helper function for thread running
    static void* StartAtmHelper(void* args);

private:
    std::string _commandsFileName;
    int _id;

    void ParseCommand(const string line, string& command, int args[]);
    int GetAccountBalance(int accountId, int password);
    int WithdrawFromAccount(int accountId, int password, int amount);
    bool DepositToAccount(int accountId, int password, int amount);
    bool MakeAccountVip(int accountId, int password);
    bool TransferAmount(int srcAccountId, int srcAccountPass, int dstAccountId, int amount);
    bool OpenAccount(int accountId, int accountPassword, int initAmount);
};
#endif //OS_2_ATM_H
