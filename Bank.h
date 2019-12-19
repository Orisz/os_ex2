#include <iostream>
#include <ostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include "Account.h"
#ifndef OS_2_BANK_H
#define OS_2_BANK_H

typedef std::vector<Account*> t_account_vec;
using namespace std;


//log codes for log file printing
enum
{
	ACCOUNT_DOES_NOT_EXIST,
	WRONG_PASSWORD,
	GET_BALANCE,
	WITHDRAW_FAIL_NOT_ENOUGH_MONEY,
	WITHDRAW,
	DEPOSIT,
	TRANSFER,
	OPEN_FAIL_ACCOUNT_ALREADY_EXIST,
	OPEN_ACCOUNT,
	COMMISSION
};

bool SortByAccountId(Account* a, Account* b);

class Bank {
public:
    Bank();
    ~Bank();
	void BankPrintStatus();
    void log(int messageFlag, int args[]);
    Account* GetAccountById(int id);
    void AddAccountToBank(Account* account);
	bool programIsRunning; // when main notice that all ATMs are done, this flag will be false
	bool OpenAccount(int atmId, int accountId, int accountPassword, int initAmount);
	static void* TakeCommissionHelper(void* arg);
	static void* BankPrintHelper(void* arg);
private:
	Account _bankAccount;
    void TakeCommission();
	t_account_vec _accountVec;
	pthread_mutex_t logFileMutex;
	pthread_mutex_t accountVecMutex;
	pthread_mutex_t accountVecReadersMutex;
	void GetLogFileWriteLock();
	void ReleaseLogFileWriteLock();
	void GetAccountVecWriteLock();
	void ReleasetAccountVecWriteLock();
	void GetAccountVecReadLock();
	void ReleasetAccountVecReadLock();
	Account* HelperFindAccountByIdForOpenAccount(int id);
	int bankReaderCounter;
	std::ofstream _logFile;
};


#endif //OS_2_BANK_H
