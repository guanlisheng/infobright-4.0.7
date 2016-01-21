#include <string>
#include "DIMParameters.h"
#include "DataIntegrityTest.h"
#include "TableTest.h"
#include "system/RCException.h"
#include "system/Channel.h"
#include "system/ChannelOut.h"
#include "system/StdOut.h"
#include "system/FileOut.h"


using namespace std;
using namespace boost;

DataIntegrityTest::DataIntegrityTest() : table_test(NULL)
{
	column_number = -1;
}

DataIntegrityTest::~DataIntegrityTest(void)
{
}

bool DataIntegrityTest::IsTableTestCorrupted() {
	return table_test->IsCorrupted(); 
}

int DataIntegrityTest::RunDataIntegrityTest() throw(DatabaseRCException)
{
	pair<int, string> DIT_res;
	string col_comm("");
	if(column_number>-1)
		col_comm = string(" for col: ") + boost::lexical_cast<string>(column_number) + string(" ");
//	DIMParameters::LogOutput() << "For db: " << database_name << "  tab: " << table_name << " " << col_comm << " test: " << GetTestName() <<".....";
	DIMParameters::LogOutput() << "  " << GetTestName() << col_comm <<".....";
	if(!IsTableTestCorrupted()) {
		DIT_res = Run();
		if(DIT_res.first > 1) {
			FailedTest ftest(this->GetDatabaseName(), this->GetTableName(), this->GetTestName());
			DIMParameters::failed_tests.push_back(ftest);
			DIMParameters::LogOutput() << "[ CORRUPT ] " << DIT_res.second;
		}
		else if(DIT_res.first == 1)
			DIMParameters::LogOutput() << "[ PASS ]";
		else
			DIMParameters::LogOutput() << "[ ABANDONED ] - " << DIT_res.second;

		DIMParameters::LogOutput() << endl;
		return DIT_res.first;
	} else {
		FailedTest ftest(this->GetDatabaseName(), this->GetTableName(), this->GetTestName());
		DIMParameters::failed_tests.push_back(ftest);
		DIMParameters::LogOutput() << "[ CORRUPT ] " << "RC_Table could not be loaded (file error)." << endl;
		return 2;
	}
}
