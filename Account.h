#ifndef OS_2_ACCOUNT_H
#define OS_2_ACCOUNT_H

#include <pthread.h>
#include <stdio.h>
#include <iostream>

#include "pthread_includes.h"

class Account{
public:
    pthread_mutex_t writeLock;
    pthread_mutex_t readLock;
    int readerCounter;

    Account();
    Account(int id, int password, int initialAmount);
    ~Account();
    int GetBalance(int password);
	int GetBalanceNoPassword();
	int GetBalanceNoPasswordNoLock();
    int Withdraw(int password, int withdraw);
	bool IsAccountVip();
    int Deposit(int password, int amount);
    bool MakeVip(int password);
    int GetCommission(int& precent); //internal random commission
	void GetAccountWriteLock();
	void ReleaseAccountWriteLock();
	void GetAccountReadLock();
	void ReleaseAccountReadLock();
	int TransferToAccount(int srcAccountPassword, Account* dstAccount, int amount);
	int ReceiveTransfer(int amount);
    int DepositNoPassword(int amount);
    int GetAccountId();
	int GetAccountPassword();

private:
    int _id;
    int _password;
    int _balance;
    bool _isVip;

    bool IsPasswordValid(int password);
};

#endif //OS_2_ACCOUNT_H
