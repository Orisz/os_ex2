#include "Account.h"
#include <ctime>
#include <cstdlib>


using namespace std;
//!*************************************************************************
//* function name: Account
//* Description  : Account constructor, gets only valid values (all validity checks are made in the ATM)
//* Parameters   : Account id, account password and initial amount (all int)
//* Return value : None
//!*************************************************************************
Account::Account(int id, int password, int initialAmount):
        _id(id), _password(password), _balance(initialAmount), _isVip(false) {
    int retReadMutex = pthread_mutex_init(&readLock, NULL);
	if (retReadMutex != 0)
	{
		cerr << "pthread_mutex_init failed: Account Read Mutex" << endl;
		exit(-1);
	}
    int retWriteMutex = pthread_mutex_init(&writeLock, NULL);
	if (retWriteMutex != 0)
	{
		cerr << "pthread_mutex_init failed: Account Write Mutex" << endl;
		exit(-1);
	}
    readerCounter = 0;
}

//!*************************************************************************
//* function name: ~Account
//* Description  : Account destructor
//* Parameters   : None
//* Return value : None
//!*************************************************************************
Account::~Account(){
    int retWriteMutex = pthread_mutex_destroy(&writeLock);
	if (retWriteMutex != 0)
	{
		cerr << "pthread_mutex_destroy failed: Account Write Mutex" << endl;
		exit(-1);
	}
    int retReadMutex = pthread_mutex_destroy(&readLock);
	if (retReadMutex != 0)
	{
		cerr << "pthread_mutex_destroy failed: Account Read Mutex" << endl;
		exit(-1);
	}
}


//*************************************************************************
//* function name: Withdraw
//* Description  : Safe withdraw an amount of money. withdraw only if balance is enough and password is correct.
//* Parameters   : Account password and amount to withdraw
//* Return value : incorrect password = -1, withdraw > account balance =-2, else return new balance
//*************************************************************************
int Account::Withdraw(int password, int withdraw) {
    int retBalance = 0;
	GetAccountWriteLock();
    // Check for password
    if (!IsPasswordValid(password)){
        // incorrect password
        return -1;
    }
    //check if balance is valid to withdraw, and update the balance if so

    if (_balance < withdraw){
        // Balance is to low. you need more money
        return -2;
    }

    _balance -= withdraw;
    retBalance = _balance;
    return retBalance;
}

//*************************************************************************
//* function name: IsAccountVip
//* Description  : Safe vip check for the bank
//* Parameters   : void
//* Return value : if vip return true else false
//*************************************************************************
bool Account::IsAccountVip()
{
	GetAccountWriteLock();//uses for commision of the bank so the write lock is needed 
	bool ret = _isVip;
	return ret;
}
//*************************************************************************
//* function name: GetBalance
//* Description  : Safe balance check
//* Parameters   : Account password
//* Return value : incorrect password = -1, else return new balance
//*************************************************************************
int Account::GetBalance(int password) {
    int retBalance = 0;
    // Check for password
	GetAccountReadLock();
    if (!IsPasswordValid(password)){
        // incorrect password
        return -1;
    }
    // Password is valid, return the balance
    retBalance = _balance;
    return retBalance;
}

//*************************************************************************
//* function name: GetBalanceNoPassword
//* Description  : Safe balance check
//* Parameters   : void
//*************************************************************************
int Account::GetBalanceNoPassword() {
	int retBalance = 0;
	GetAccountReadLock();
	retBalance = _balance;
	ReleaseAccountReadLock();
	return retBalance;
}


//*************************************************************************
//* function name: GetBalanceNoPasswordNoLock
//* Description  : Unsafe balance check
//* Parameters   : void
//*************************************************************************
int Account::GetBalanceNoPasswordNoLock() {
    return _balance;
}

//*************************************************************************
//* function name: Deposit
//* Description  : Safe Deposit an amount of money.
//* Parameters   : Account password and amount to deposit
//* Return value : incorrect password = -1, else return new balance
//*************************************************************************

int Account::Deposit(int password, int amount){
    int retBalance = 0;
    // Check for password
	GetAccountWriteLock();
    if (!IsPasswordValid(password)){
        // incorrect password
        return -1;
    }
    _balance += amount;
    retBalance = _balance;
    return retBalance;
}

//*************************************************************************
//* function name: DepositNoPassword
//* Description  : Safe Deposit an amount of money, with no password
//* Parameters   : amount to deposit
//* Return value : new balance
//*************************************************************************
int Account::DepositNoPassword(int amount){
    int retBalance = 0;

    GetAccountWriteLock();
    _balance += amount;
    retBalance = _balance;
    return retBalance;
}




//*************************************************************************
//* function name: IsPasswordValid
//* Description  : Check if the given password is the account password
//* Parameters   : Account password
//* Return value : incorrect password = false, correct = true
//*************************************************************************
bool Account::IsPasswordValid(int password) {
    return password == _password;
}


//*************************************************************************
//* function name: MakeVip
//* Description  : Make account VIP
//* Parameters   : Account password
//* Return value : incorrect password = false, correct = true
//*************************************************************************
bool Account::MakeVip(int password) {
	GetAccountWriteLock();
    if (!IsPasswordValid(password)){
        // incorrect password
        return false;
    }
    _isVip = true;
    return true;
}


//*************************************************************************
//* function name: GetCommission
//* Description  : Get random commission from account. the bank has the write lock!
//* Parameters   : output variable for theprecent taken
//* Return value : commission amount
//*************************************************************************
int Account::GetCommission(int& precent) {
    int MAX = 4;
    int MIN = 2;
	// like in: http://www.math.uaa.alaska.edu/~afkjm/csce211/fall2018/handouts/RandomFunctions.pdf
	int random_num = ((rand() % (MAX - MIN + 1)) + MIN);
	precent = random_num;
    int commission;
    commission = (int) ((double)random_num * (double)_balance / 100);
	_balance -= commission;
	return commission;
}


//*************************************************************************
//* function name: GetAccountWriteLock
//* Description  : Get write lock on this object
//* Parameters   : none
//* Return value : none
//*************************************************************************
void Account::GetAccountWriteLock() {
    int retLock = pthread_mutex_lock(&writeLock);
	if (retLock != 0)
	{
		cerr << "pthread_mutex_lock failed: Account Write Mutex" << endl;
		exit(-1);
	}
}


//*************************************************************************
//* function name: ReleaseAccountWriteLock
//* Description  : unlock write lock on this object
//* Parameters   : none
//* Return value : none
//*************************************************************************
void Account::ReleaseAccountWriteLock(){
    int retUnlock = pthread_mutex_unlock(&writeLock);
	if (retUnlock != 0)
	{
		cerr << "pthread_mutex_unlock failed: Account Write Mutex" << endl;
		exit(-1);
	}
}


//*************************************************************************
//* function name: GetAccountReadLock
//* Description  : lock read lock on this object
//* Parameters   : none
//* Return value : none
//*************************************************************************
void Account::GetAccountReadLock(){
    // update read counter
	int retReadLock = 0;
	int retWriteLock = 0;
    retReadLock = pthread_mutex_lock(&readLock);
	if (retReadLock != 0)
	{
		cerr << "pthread_mutex_lock failed: Account Read Mutex" << endl;
		exit(-1);
	}
    readerCounter ++;
    if(readerCounter == 1){
        // this is the first reader, lock the object
		retWriteLock = pthread_mutex_lock(&writeLock);
		if (retWriteLock != 0)
		{
			cerr << "pthread_mutex_lock failed: Account Write Mutex" << endl;
			exit(-1);
		}

    }
	retReadLock = pthread_mutex_unlock(&readLock);
	if (retReadLock != 0)
	{
		cerr << "pthread_mutex_unlock failed: Account Read Mutex" << endl;
		exit(-1);
	}
}


//*************************************************************************
//* function name: ReleaseAccountReadLock
//* Description  : unlock read lock on this object
//* Parameters   : none
//* Return value : none
//*************************************************************************
void Account::ReleaseAccountReadLock(){
    // update read counter
	int retReadLUnlock = 0;
	int retWriteUnlock = 0;
	retReadLUnlock = pthread_mutex_lock(&readLock);
	if (retReadLUnlock != 0)
	{
		cerr << "pthread_mutex_lock failed: Account Read Mutex" << endl;
		exit(-1);
	}
    readerCounter --;
    if(readerCounter == 0){
        // this is the last reader, unlock the object
		retWriteUnlock = pthread_mutex_unlock(&writeLock);
		if (retWriteUnlock != 0)
		{
			cerr << "pthread_mutex_unlock failed: Account Write Mutex" << endl;
			exit(-1);
		}

    }
	retReadLUnlock = pthread_mutex_unlock(&readLock);
	if (retReadLUnlock != 0)
	{
		cerr << "pthread_mutex_unlock failed: Account Read Mutex" << endl;
		exit(-1);
	}
}


//*************************************************************************
//* function name: TransferToAccount
//* Description  : transfer money to account. no locks! the atm has the locks fo both of them
//* Parameters   : dst account and amount to transfer
//* Return value : incorrect password = -1, not enough money = -2, else return the dst account new balance
//*************************************************************************
int Account::TransferToAccount(int srcAccountPassword, Account* dstAccount, int amount) {
    if (!IsPasswordValid(srcAccountPassword)){
        // incorrect password
        return -1;
    }
    // no need to lock the account, it is already locked
    if (amount > _balance){
        return -2;
    }
    // we have enough money, take it from the account
    _balance -= amount;
    return dstAccount->ReceiveTransfer(amount);
}


//*************************************************************************
//* function name: ReceiveTransfer
//* Description  : get a transfer from other account, no need to lock, the atm has the lock
//* Parameters   : amount to transfer
//* Return value : balance after transfer
//*************************************************************************
int Account::ReceiveTransfer(int amount) {
    _balance += amount;
    return _balance;
}



//*************************************************************************
//* function name: GetAccountId
//* Description  : getter of account id
//* Parameters   : none
//* Return value : account id
//************************************************************************
int Account::GetAccountId() {
    return _id;
}

//*************************************************************************
//* function name: GetAccountPassword
//* Description  : get account password for the bank printings
//* Parameters   : none
//* Return value : account password
//************************************************************************
int Account::GetAccountPassword()
{
	return _password;
}