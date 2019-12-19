#include "Bank.h"


//*************************************************************************
//* function name: SortByAccountId
//* Description  :helper function for the 'print status' function. printing needs to be in accending
//*					order of accounts ids
//* Parameters   : the two accounts we are currently comparing
//* Return value : bool: true if a's Id is bigger then b's Id. else false.NOTE the id should be different
//*************************************************************************
bool SortByAccountId(Account* a, Account* b)
{
	return (a->GetAccountId() < b->GetAccountId());
}

//*************************************************************************
//* function name: Bank
//* Description  : Init bank object with default self account (id=1, password=1234, initial amount=0)
//* Parameters   : none
//* Return value : none
//*************************************************************************
Bank::Bank():_bankAccount(1, 1234, 0){
	int retLogMutex = pthread_mutex_init(&logFileMutex, NULL);
	if (retLogMutex != 0)
	{
		cerr << "pthread_mutex_init failed: Log Mutex" << endl;
		exit(-1);
	}
	int retAccVecWriteMutex = pthread_mutex_init(&accountVecMutex, NULL);
	if (retAccVecWriteMutex != 0)
	{
		cerr << "pthread_mutex_init failed: Account Vector Write Mutex" << endl;
		exit(-1);
	}
	int retAccVecReadMutex = pthread_mutex_init(&accountVecReadersMutex, NULL);
	if (retAccVecReadMutex != 0)
	{
		cerr << "pthread_mutex_init failed: Account Vector Read Mutex" << endl;
		exit(-1);
	}
	_logFile.open("log.txt");
	if (_logFile.fail())
	{
		cerr << "Open Log Failed" << endl;
		exit(-1);
	}
	programIsRunning = true;
	bankReaderCounter = 0;
}

Bank::~Bank()
{
	_logFile.close();
	if (_logFile.fail())
	{
		cerr << "Close Log Failed" << endl;
		exit(-1);
	}
	int retLogMutex = pthread_mutex_destroy(&logFileMutex);
	if (retLogMutex != 0)
	{
		cerr << "pthread_mutex_destroy failed: Log Mutex" << endl;
		exit(-1);
	}
	int retAccVecWriteMutex = pthread_mutex_destroy(&accountVecMutex);
	if (retAccVecWriteMutex != 0)
	{
		cerr << "pthread_mutex_destroy failed: Account Vector Write Mutex" << endl;
		exit(-1);
	}
	int retAccVecReadMutex = pthread_mutex_destroy(&accountVecReadersMutex);
	if (retAccVecReadMutex != 0)
	{
		cerr << "pthread_mutex_destroy failed: Account Vector Read Mutex" << endl;
		exit(-1);
	}
}
//*************************************************************************
//* function name: AddAccountToBank
//* Description  : Add Account To the account vector of the Bank . the open account function has the lock!
//* Parameters   : &account
//* Return value : none
//*************************************************************************
void Bank::AddAccountToBank(Account* account) {
    _accountVec.push_back(account);//no need to lock the open acount function has it
}


//*************************************************************************
//* function name: GetAccountById
//* Description  : search for account by id. if not found return null
//* Parameters   : account id
//* Return value : &account (or null if not found)
//*************************************************************************
Account* Bank::GetAccountById(int id) {
    // return the first (and hopefully the only) account that has this id
	GetAccountVecReadLock();
    t_account_vec::iterator it = _accountVec.begin();
    while (it != _accountVec.end()){
        if ((*it)->GetAccountId() == id){
			ReleasetAccountVecReadLock();
            return (*it);
        }
		it++;
    }
    // Didn't find this id, return NULL
	ReleasetAccountVecReadLock();
    return NULL;
}

//*************************************************************************
//* function name: HelperFindAccountByIdForOpenAccount
//* Description  : helper function for the open account function
//*					search for account by id. if not found return null.
//*					dont use locks the "OpenAccount" func has it
//* Parameters   : account id
//* Return value : &account (or null if not found)
//*************************************************************************
Account* Bank::HelperFindAccountByIdForOpenAccount(int id) {
	// return the first (and hopefully the only) account that has this id
	t_account_vec::iterator it = _accountVec.begin();
	while (it != _accountVec.end()) {
		if ((*it)->GetAccountId() == id) {
			return (*it);
		}
		it++;
	}
	// Didn't find this id, return NULL
	return NULL;
}


//*************************************************************************
//* function name: TakeCommission
//* Description  : iterate over all the bank accounts and take commission from each one.
// will run in a different thread. will die when the main will tell him.
//* Parameters   : none
//* Return value : none
//*************************************************************************
void Bank::TakeCommission() {
    while (programIsRunning)
    {
        int moneyForTheBank = 0;
		GetAccountVecReadLock();//while itterating the accounts vec dont add accounts
        t_account_vec::iterator it = _accountVec.begin();
        while (it != _accountVec.end()){
			if (!(*it)->IsAccountVip())
			{
				int accountId = (*it)->GetAccountId();
				int precent = 0;// second output for GetCommission
				int currCommission = (*it)->GetCommission(precent);
				moneyForTheBank += currCommission;
				int args[3] = { precent, currCommission, accountId };
				log(COMMISSION, args);
			}
			(*it)->ReleaseAccountWriteLock();
			it++;
        }
		ReleasetAccountVecReadLock(); // Release the accounts vec.
		_bankAccount.DepositNoPassword(moneyForTheBank); // false because we don't need to sleep here
		_bankAccount.ReleaseAccountWriteLock();
        usleep(3000000); // 100 m sec = 100*1000 u sec
    }
}


//*************************************************************************
//* function name: TakeCommissionHelper
//* Description  : Static function that can be run using another thread
//* Parameters   : Bank object as a void* arg (need to be casted
//* Return value : none
//*************************************************************************
void* Bank::TakeCommissionHelper(void* arg){
	((Bank*)(arg))->TakeCommission();
	pthread_exit(NULL);
}


//*************************************************************************
//* function name: GetLogFileWriteLock
//* Description  : Get write lock on this object
//* Parameters   : none
//* Return value : none
//*************************************************************************
void Bank::GetLogFileWriteLock()
{
	int retLog = pthread_mutex_lock(&logFileMutex);
	if (retLog != 0)
	{
		cerr << "pthread_mutex_lock failed: Log Mutex" << endl;
		exit(-1);
	}
}

//*************************************************************************
//* function name: ReleaseLogFileWriteLock
//* Description  : unlock write lock on this object
//* Parameters   : none
//* Return value : none
//*************************************************************************
void Bank::ReleaseLogFileWriteLock()
{
	int retLog = pthread_mutex_unlock(&logFileMutex);
	if (retLog != 0)
	{
		cerr << "pthread_mutex_unlock failed: Log Mutex" << endl;
		exit(-1);
	}
}

//*************************************************************************
//* function name: GetAccountVecWriteLock
//* Description  : Get write lock on the account vector
//* Parameters   : none
//* Return value : none
//*************************************************************************
void Bank::GetAccountVecWriteLock() {
	int retAccVec = pthread_mutex_lock(&accountVecMutex);
	if (retAccVec != 0)
	{
		cerr << "pthread_mutex_lock failed: Account Vector Write Mutex" << endl;
		exit(-1);
	}
}


//*************************************************************************
//* function name: ReleasetAccountVecWriteLock
//* Description  : unlock write lock on the account vector
//* Parameters   : none
//* Return value : none
//*************************************************************************
void Bank::ReleasetAccountVecWriteLock() {
	int retAccVec = pthread_mutex_unlock(&accountVecMutex);
	if (retAccVec != 0)
	{
		cerr << "pthread_mutex_unlock failed: Account Vector Write Mutex" << endl;
		exit(-1);
	}
}

//*************************************************************************
//* function name: GetAccountVecReadLock
//* Description  : readrs lock for the account vector
//* Parameters   : none
//* Return value : none
//*************************************************************************
void Bank::GetAccountVecReadLock() {
	// update read counter
	int retVecRead = 0;
	int retVecWrite = 0;
	retVecRead = pthread_mutex_lock(&accountVecReadersMutex);
	if (retVecRead != 0)
	{
		cerr << "pthread_mutex_lock failed: Account Vector Read Mutex" << endl;
		exit(-1);
	}
	bankReaderCounter++;
	if (bankReaderCounter == 1) {
		// this is the first reader, lock the object
		retVecWrite = pthread_mutex_lock(&accountVecMutex);
		if (retVecWrite != 0)
		{
			cerr << "pthread_mutex_lock failed: Account Vector Write Mutex" << endl;
			exit(-1);
		}
	}
	retVecRead = pthread_mutex_unlock(&accountVecReadersMutex);
	if (retVecRead != 0)
	{
		cerr << "pthread_mutex_unlock failed: Account Vector Read Mutex" << endl;
		exit(-1);
	}
}

//*************************************************************************
//* function name: ReleasetAccountVecReadLock
//* Description  : unlock readrs lock for the account vector
//* Parameters   : none
//* Return value : none
//*************************************************************************
void Bank::ReleasetAccountVecReadLock() {
	// update read counter
	int retVecRead = 0;
	int retVecWrite = 0;
	retVecRead = pthread_mutex_lock(&accountVecReadersMutex);
	if (retVecRead != 0)
	{
		cerr << "pthread_mutex_lock failed: Account Vector Read Mutex" << endl;
		exit(-1);
	}
	bankReaderCounter--;
	if (bankReaderCounter == 0) {
		// this is the last reader, unlock the object
		retVecWrite = pthread_mutex_unlock(&accountVecMutex);
		if (retVecWrite != 0)
		{
			cerr << "pthread_mutex_unlock failed: Account Vector Write Mutex" << endl;
			exit(-1);
		}
	}
	retVecRead = pthread_mutex_unlock(&accountVecReadersMutex);
	if (retVecRead != 0)
	{
		cerr << "pthread_mutex_unlock failed: Account Vector Read Mutex" << endl;
		exit(-1);
	}
}

//*************************************************************************
//* function name: log
//* Description  : lock 'logFile' and writes to it, then realese it.
//* Parameters   : 'messageFlag' which message should we print , 'args' - array of arguments to print
//* Return value : none
//*************************************************************************
void Bank::log(int messageFlag, int args[])
{
	if (args == NULL || messageFlag < 0) { return; }
	switch (messageFlag)
	{
	case ACCOUNT_DOES_NOT_EXIST:
		GetLogFileWriteLock();
		_logFile << "Error " << args[0] << ": Your transaction failed - account id " << args[1] <<
			" does not exist" << endl;
		ReleaseLogFileWriteLock();
		break;
	case WRONG_PASSWORD:
		GetLogFileWriteLock();
		_logFile << "Error " << args[0] << ": Your transaction failed - password for account id " <<
			args[1] << " is incorrect" << endl;;
		ReleaseLogFileWriteLock();
		break;
	case GET_BALANCE:
		GetLogFileWriteLock();
		_logFile << args[0] << ": Account " << args[1] << " balance is " << args[2] << endl;
		ReleaseLogFileWriteLock();
		break;
	case WITHDRAW_FAIL_NOT_ENOUGH_MONEY:
		GetLogFileWriteLock();
		_logFile << "Error " << args[0] << ": Your transaction failed - account id " <<
			args[1] << " balance is lower than " << args[2] << endl;
		ReleaseLogFileWriteLock();
		break;
	case WITHDRAW:
		GetLogFileWriteLock();
		_logFile << args[0] << ": Account " << args[1] << " new balance is " << args[2] <<
			" after " << args[3] << " $ was withdrew" << endl;
		ReleaseLogFileWriteLock();
		break;
	case DEPOSIT:
		GetLogFileWriteLock();
		_logFile << args[0] << ": Account " << args[1] << " new balance is " << args[2] <<
			" after " << args[3] << " $ was deposited" << endl;
		ReleaseLogFileWriteLock();
		break;
	case TRANSFER:
		GetLogFileWriteLock();
		_logFile << args[0] << ": Transfer " << args[1] << " from account " << args[2] <<
			" to account " << args[3] << " new account balance is " << args[4] <<
			" new target account balance is " << args[5] << endl;
		ReleaseLogFileWriteLock();
		break;
	case OPEN_FAIL_ACCOUNT_ALREADY_EXIST:
		GetLogFileWriteLock();
		_logFile << "Error " << args[0] << ": Your transaction failed - account with the same id exists" << endl;
		ReleaseLogFileWriteLock();
		break;

	case OPEN_ACCOUNT:
		GetLogFileWriteLock();
		_logFile << args[0] << ": New account id is " << args[1] << " with password " <<
				 args[2] << " and initial balance " << args[3] << endl;
		ReleaseLogFileWriteLock();
		break;
	case COMMISSION:
		GetLogFileWriteLock();
		_logFile << "Bank: commissions of " << args[0] << " % were charged, the bank gained " << 
			args[1] << " $ from account " << args[2] << endl;
		ReleaseLogFileWriteLock();
	default:
		break;
	}
}

//*************************************************************************
//* function name: OpenAccount
//* Description  : open an account with the required parameters locks the accounts vector for the procces
//* Parameters   : 'atmId' the id of the atm who requested to open the account
//*					'accountId' (must be uniq), password and initial amount of money
//*					 (we assume password and amount are legal values)
//* Return value : bool true if succeeded else false
//*************************************************************************
bool Bank::OpenAccount(int atmId, int accountId, int accountPassword, int initAmount) {
	GetAccountVecWriteLock();/*lock the vector so there can be no race between two atm's trying to open 
	the same account*/
	// Search for account with the same id
	if (HelperFindAccountByIdForOpenAccount(accountId) != NULL) {
		int args[1] = { atmId };
		log(OPEN_FAIL_ACCOUNT_ALREADY_EXIST, args);
		ReleasetAccountVecWriteLock(); //release the lock before exiting
		return false;
	}
	Account* newAccount = new Account(accountId, accountPassword, initAmount);
	AddAccountToBank(newAccount);
	int args[4] = { atmId, accountId, accountPassword, initAmount };
	log(OPEN_ACCOUNT, args);
	ReleasetAccountVecWriteLock(); // Release the lock before exiting
	return true;
}

//*************************************************************************
//* function name: BankPrintStatus
//* Description  : prints the account and the bank statues every 0.5sec.
//* Parameters   : 'messageFlag' which message should we print , 'args' - array of arguments to print
//* Return value : none
//*************************************************************************
void Bank::BankPrintStatus()
{
	while (programIsRunning) {
		/*in order to give a true and updated statues we are going to lock
		the bank for new accounts to enter. to maintain lock oreder we will
		copy the accounts vector to a helper vector. we will reorder the helper vector
		and print it according to accending id oreder*/
		t_account_vec tmpPrintingVector;
		GetAccountVecReadLock();//do not add more accounts while status printing
		for (t_account_vec::iterator it = _accountVec.begin(); it != _accountVec.end(); it++) {
			tmpPrintingVector.push_back(*(it));//safe reading to new vector
		}
		//sort the printing vector
		std::sort(tmpPrintingVector.begin(), tmpPrintingVector.end(), SortByAccountId);
		// now lock accounts in ACCENDING order e.g. lock id 1 , lock id 3 , lock id 9 , etc.
		for (t_account_vec::iterator itLock = tmpPrintingVector.begin(); itLock != tmpPrintingVector.end(); itLock++) {
			(*itLock)->GetAccountReadLock();//all accounts read lock 
		}
		//print the status
		printf("\033[2J");
		printf("\033[1;1H");
		cout << "Current Bank Status" << endl;
		for (t_account_vec::iterator itTmp = tmpPrintingVector.begin(); itTmp != tmpPrintingVector.end(); itTmp++) {
			cout << "Account " << (*itTmp)->GetAccountId() << ": Balance - " <<
				 (*itTmp)->GetBalanceNoPassword() << "  $ , Account Password - " <<
				 setfill('0') << setw(4) << (*itTmp)->GetAccountPassword() << endl;
		}

		cout << "The Bank has " << _bankAccount.GetBalanceNoPassword() << " $" << endl;
		//release the account IN THE SAME ORDER AS LOCKING THEM
		for (t_account_vec::iterator itUnlock = tmpPrintingVector.begin(); itUnlock != tmpPrintingVector.end(); itUnlock++) {
			(*itUnlock)->ReleaseAccountReadLock();//release the original vector for changes if there are no more readers
		}
		ReleasetAccountVecReadLock(); // Release the accounts list so the atms can add accounts
		usleep(500000);
	}
}


//*************************************************************************
//* function name: BankPrintHelper
//* Description  : start the bank print from a different thread
//* Parameters   : bank object as a void* arg (need to be casted)
//* Return value : none
//*************************************************************************
void* Bank::BankPrintHelper(void *arg) {
	((Bank*)(arg))->BankPrintStatus();
	pthread_exit(NULL);
}
