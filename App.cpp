#include "Bank.h"
#include "Atm.h"
#include <vector>
#include "pthread_includes.h"

using namespace std;
typedef std::vector<Atm*> t_atm_vec;
Bank* pBank = new Bank();

//*************************************************************************
//* function name: InitAtmVector
//* Description  : init N Atms and start their main loop
//* Parameters   : number of atms, inputFilesNames for each Atm, array of threads
//* Return value : none
//*************************************************************************
void InitAtmVector(int numOfAtms, char* inputFilesNames[], pthread_t atmThreadArray[]) {
	t_atm_vec* atmVector = new t_atm_vec;

	for (int i = 0; i < numOfAtms; ++i) {
		Atm* pNewAtm = new Atm(i + 1, inputFilesNames[i]);
		atmVector->push_back(pNewAtm);

		void* pAtmObject = static_cast<void*>(pNewAtm);
		int retThread = pthread_create(&atmThreadArray[i], NULL, Atm::StartAtmHelper, pAtmObject);
		if (retThread != 0)
		{
			cerr << "Error from pthread_create of Atm number: " << i << ". error code:" << retThread << endl;
			exit(-1);
		}

	}
}


//*************************************************************************
//* function name: RunInThreads
//* Description  : call all the init functions and then wait for them to finish
//* Parameters   : args of main program
//* Return value : none
//*************************************************************************
void RunInThreads(int numOfAtms, char* argv[]) {

	// Create a N Atms and a thread for every Atm
	pthread_t atmThreadArray[numOfAtms];
	InitAtmVector(numOfAtms, argv, atmThreadArray);

	// Create a bank commission thread
	pthread_t bankCommissionThread;
	void* pBankObject = static_cast<void*>(pBank);
	int retCommThread = pthread_create(&bankCommissionThread, NULL, Bank::TakeCommissionHelper, pBankObject);
	if (retCommThread != 0)
	{
		cerr << "Error from pthread_create of Bank Commision thread. error code:" << retCommThread << endl;
		exit(-1);
	}
	// Create a bank print thread
	pthread_t bankPrintThread;
	int retPrintThread = pthread_create(&bankPrintThread, NULL, Bank::BankPrintHelper, pBankObject);
	if (retPrintThread != 0)
	{
		cerr << "Error from pthread_create of Status Printing thread. error code:" << retPrintThread << endl;
		exit(-1);
	}

	// Wait for all the ATM threads to end
	for (int i = 0; i < numOfAtms; i++) {
		int retJoin = pthread_join(atmThreadArray[i], NULL);
		if (retJoin != 0)
		{
			cerr << "Error from pthread_join of Atm number: " << i << ". error code:" << retJoin << endl;
			exit(-1);
		}
	}
	// tell the bank its all over
	pBank->programIsRunning = false;
	int retCommJoin = pthread_join(bankCommissionThread, NULL);
	if (retCommJoin != 0)
	{
		cerr << "Error from pthread_join of Commission thread. error code:" << retCommJoin << endl;
		exit(-1);
	}
	int retPrintJoin = pthread_join(bankPrintThread, NULL);
	if (retPrintJoin != 0)
	{
		cerr << "Error from pthread_join of Status Printing thread. error code:" << retPrintJoin << endl;
		exit(-1);
	}
}
// Main entry point of the application
// Gets the arguments as following
// Bank numOfAtm's	Atm(i)InputFileName
int main(int argc, char* argv[]) {
    // Check for minimum number of arguments
    if (argc < 2) {
        cout << "illegal arguments" << endl;
        return 1;
    }

    // Get the number of Atm's
    int numOfAtms = atoi(argv[1]);

    // Validate the number of input file names
    if (numOfAtms != argc - 2) {
        cout << "illegal arguments" << endl;
        return 1;
    }
    RunInThreads(numOfAtms, argv + 2);
    return 0;
}
