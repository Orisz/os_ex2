#include "Atm.h"


using namespace std;

	//*************************************************************************
	//* function name: Atm() empty C'tor
	//* Description  : init the class with empty command and invalid id
	//* Parameters   : void
	//* Return value : none
	//*************************************************************************
	Atm::Atm()
	{
		_id = -1;
		_commandsFileName = "";
	}
	
	//*************************************************************************
	//* function name: Atm  C'tor
	//* Description  : init the class with given id and command line
	//* Parameters   : int id , string command
	//* Return value : none
	//*************************************************************************
	Atm::Atm(int id, string commandsFileName)
	{
		_id = id;
		_commandsFileName = commandsFileName;
	}
	
	//*************************************************************************
	//* function name: ~Atm  D'tor
	//* Description  : destroy class
	//* Parameters   : void
	//* Return value : none
	//*************************************************************************
	Atm::~Atm()
	{
	}
	
	//*************************************************************************
	//* function name: MainAtmLoop
	//* Description  : run with a loop on the command file, reading lines and calling 
	//*					'ParseCommand' to parse them
	//* Parameters   : void
	//* Return value : void
	//*************************************************************************
	void Atm::MainAtmLoop()
	{
		ifstream file(_commandsFileName);
		if (!file)
		{
			cerr << "Can't open file " << _commandsFileName << endl;
			exit(-1);
		}
		string line;
		while (getline(file, line))
		{
			string command = "";
			int args[4] = { 0 };
			ParseCommand(line , command , args);
			DoOperation(command , args);
			usleep(100000); // 100 m sec = 100*1000 u sec
		}
	}


	//*************************************************************************
	//* function name: StartAtmHelper
	//* Description  : Static function that can be started as a thread
	//* Parameters   : Atm object as a void* (need to be casted)
	//* Return value : void
	//*************************************************************************
	void* Atm::StartAtmHelper(void *args) {
		((Atm*)(args))->MainAtmLoop();
		pthread_exit(NULL);
	}


	//*************************************************************************
	//* function name: ParseCommand
	//* Description  : parsing a line from the command file
	//* Parameters   : raw data command strin 'line' , type ofcommand output 'command' , output args 'args'
	//* Return value : bool if command was legal. here we assume input is always legal
	//*************************************************************************
	void Atm::ParseCommand(const string line, string& command, int args[])
	{
		istringstream linestream(line);
		linestream >> command; // wanted operation
		int i = 0;
		int tmp = 0;

		while (linestream >> tmp)
		{
			args[i] = tmp;
			i++;
		}
	}

	//*************************************************************************
	//* function name: DoOperation
	//* Description  : based on the command type input from the user calls the right execute function
	//* Parameters   : command type 'command' args for the command 'args'
	//* Return value : void
	//*************************************************************************
	void Atm::DoOperation(string command, int args[])
	{
		int accountId = args[0];
		int accountPassword = args[1];
		char cmd = command[0];
		switch (cmd)
		{
			//open account
			case 'O':
				OpenAccount(accountId, accountPassword, args[2]);
				break;
			//make account vip
			case 'L':
				MakeAccountVip(accountId, accountPassword);
				break;
			//deposit
			case 'D':
				DepositToAccount(accountId, accountPassword, args[2]);
				break;
			//withdraw
			case 'W':
				WithdrawFromAccount(accountId, accountPassword, args[2]);
				break;
			// get account ballance
			case 'B':
				GetAccountBalance(accountId, accountPassword);
				break;
			//transfer money to other account
			case 'T':
				TransferAmount(accountId, accountPassword, args[2], args[3]);
				break;
			default:
				break;
		}
	}

	//*************************************************************************
	//* function name: GetAccountBalance
	//* Description  : prints the ballance of an account with id 'accountId'
	//* Parameters   : account id - 'accountId' , account password - 'password'
	//* Return value : void
	//*************************************************************************
	int Atm::GetAccountBalance(int accountId, int password)
	{
		Account* account = pBank->GetAccountById(accountId);
		if (account == NULL)
		{
			int args[2] = { _id, accountId };
			pBank->log(ACCOUNT_DOES_NOT_EXIST, args);
			return -1;
		}
		int balance = account->GetBalance(password);
		if (balance == -1)//meaninig password was wrong
		{
			int args[2] = { _id , accountId };
			pBank->log(WRONG_PASSWORD, args);
			sleep(1);
			account->ReleaseAccountReadLock();
			return -1;
		}
		else
		{
			int args[3] = { _id , accountId , balance };
			pBank->log(GET_BALANCE, args);
			sleep(1);
			account->ReleaseAccountReadLock();
			return balance;
		}
	}


	//*************************************************************************
	//* function name: WithdrawFromAccount
	//* Description  : if the password is correct takes amount of money 'amount' from
	//*					with id 'accountId'
	//* Parameters   : account id - 'accountId' , account password - 'password' , amount - 'amount'
	//* Return value : int -1 if fail
	//*************************************************************************
	int Atm::WithdrawFromAccount(int accountId, int password, int amount)
	{
		Account* account = pBank->GetAccountById(accountId);
		if (account == NULL)
		{
			int args[2] = { _id, accountId };
			pBank->log(ACCOUNT_DOES_NOT_EXIST, args);
			return -1;
		}
		int retWithdraw = account->Withdraw(password, amount);
		if (retWithdraw == -1) //incorrect password
		{
			int args[2] = { _id , accountId };
			pBank->log(WRONG_PASSWORD, args);
			sleep(1);
			account->ReleaseAccountWriteLock();
			return -1;
		}
		else if (retWithdraw == -2) //withdraw amount is bigger then balance AKA electrical engineer student...
		{
			int args[3] = { _id, accountId, amount };
			pBank->log(WITHDRAW_FAIL_NOT_ENOUGH_MONEY, args);
			sleep(1);
			account->ReleaseAccountWriteLock();
			return -1;
		}
		else
		{
			int args[4] = { _id , accountId, retWithdraw, amount };
			pBank->log(WITHDRAW, args);
			sleep(1);
			account->ReleaseAccountWriteLock();
			return amount;
		}
	}


	//*************************************************************************
	//* function name: DepositToAccount
	//* Description  : Safe Deposit an amount of money to account.
	//* Parameters   : Account password and amount to deposit
	//* Return value : bool false if password was wrong else true
	//*************************************************************************
	bool Atm::DepositToAccount(int accountId, int password, int amount) {
	    Account* account = pBank->GetAccountById(accountId);
		if (account == NULL)
		{ 
			int args[2] = { _id, accountId };
			pBank->log(ACCOUNT_DOES_NOT_EXIST, args);
			return false;
		}
	    int newBalance;
	    if ((newBalance = account->Deposit(password, amount)) == -1){
			int args[2] = { _id , accountId };
			pBank->log(WRONG_PASSWORD, args);
			sleep(1);
			account->ReleaseAccountWriteLock();
	        return false;
	    }
	    else{
	        int args[4] = {_id, accountId, newBalance, amount};
			pBank->log(DEPOSIT, args);
			sleep(1);
			account->ReleaseAccountWriteLock();
	        return true;
	    }
	}


	//*************************************************************************
	//* function name: MakeAccountVip
	//* Description  : change account with id 'accountId' to vip account
	//* Parameters   : account id - 'accountId' , account password - 'password'
	//* Return value : bool true if succeeded else false
	//*************************************************************************
	bool Atm::MakeAccountVip(int accountId, int password)
	{
		Account* account = pBank->GetAccountById(accountId);
		if (account == NULL)
		{ 
			int args[2] = { _id, accountId };
			pBank->log(ACCOUNT_DOES_NOT_EXIST, args);
			return false;
		}
		bool retMakeVip = account->MakeVip(password);
		if (retMakeVip == false)
		{
			int args[2] = { _id , accountId };
			pBank->log(WRONG_PASSWORD, args);
			sleep(1);
			account->ReleaseAccountWriteLock();
			return false;
		}
		sleep(1);
		account->ReleaseAccountWriteLock();
		return true;
	}


	//*************************************************************************
	//* function name: TransferAmount
	//* Description  : transfer amount 'amount' from account with id 'srcAccountId'
	//*					to account with id 'dstAccountId'
	//* Parameters   : source account id - 'accountId' ,source account password - 'srcAccountPass'
	//*					destination account id - 'dstAccountId' , amount to transfer - 'amount'
	//* Return value : bool true if succeeded else false
	//*************************************************************************
	bool Atm::TransferAmount(int srcAccountId, int srcAccountPass, int dstAccountId, int amount)
	{
		// can't transfer to the src account
		if (srcAccountId == dstAccountId){
			return false;
		}
		// check if null
		Account* srcAccount = pBank->GetAccountById(srcAccountId);
		if (srcAccount == NULL)
		{ 
			int args[2] = { _id, srcAccountId };
			pBank->log(ACCOUNT_DOES_NOT_EXIST, args);
			return false;
		} //separated 'if' because we need only 1 error print
		Account* dstAccount = pBank->GetAccountById(dstAccountId);
		if (dstAccount == NULL)
		{ 
			int args[2] = { _id, dstAccountId };
			pBank->log(ACCOUNT_DOES_NOT_EXIST, args);
			return false;
		}
		//check validity and decrease amount from first account
		// lock both locks in ascending order
		Account* maxAccount = (srcAccountId > dstAccountId) ? srcAccount : dstAccount;
		Account* minAccount = (srcAccountId < dstAccountId) ? srcAccount : dstAccount;

		minAccount->GetAccountWriteLock();
		maxAccount->GetAccountWriteLock();
		int retDstBalance = 0;
		retDstBalance = srcAccount->TransferToAccount(srcAccountPass, dstAccount, amount);
		if (retDstBalance == -1)//password incorrect
		{
			int args[2] = { _id , srcAccountId };
			pBank->log(WRONG_PASSWORD, args);
		}
		else if (retDstBalance == -2)//dons't have enough money in the account
		{
			int args[3] = { _id, srcAccountId, amount };
			pBank->log(WITHDRAW_FAIL_NOT_ENOUGH_MONEY, args);
		}
		else
		{
			int retSrcBalance = srcAccount->GetBalanceNoPasswordNoLock();
			int args[6] = { _id , amount,  srcAccountId, dstAccountId, retSrcBalance, retDstBalance };
			pBank->log(TRANSFER, args);
		}
		sleep(1);
		minAccount->ReleaseAccountWriteLock();
		maxAccount->ReleaseAccountWriteLock();
		return true;
	}

	//*************************************************************************
	//* function name: OpenAccount
	//* Description  : ask from the bank to open an account with the required parameters
	//* Parameters   : account id (must be uniq), password and initial amount of money
	//*				   (we assume password and amount are legal values)
	//* Return value : bool true if succeeded else false
	//*************************************************************************
	bool Atm::OpenAccount(int accountId, int accountPassword, int initAmount)
	{
		bool ret = pBank->OpenAccount(_id, accountId, accountPassword, initAmount);
		sleep(1);
		return ret;
	}


