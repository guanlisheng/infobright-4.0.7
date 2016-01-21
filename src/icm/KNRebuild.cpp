#include <string>
#include "KNRebuild.h"
#include "core/Filter.h"
#include "core/RCAttr.h"
#include "common/CommonDefinitions.h"
#include "edition/local.h"

#define KN_REBUILDER "KN Rebuilder"

using namespace std;

KNRebuild::KNRebuild(void)
{	
	table_test = 0;
	test_name = (string(KN_REBUILDER));
}

KNRebuild::KNRebuild(string& tp, TableTest* table_test, int col_number, string& knf_path)
{	
	this->table_test = (table_test);
	test_name = (string(KN_REBUILDER));
	vector<string> tp_dec = TableTest::ParseTablePath(tp);
	data_dir = tp_dec[0];
	database_name = tp_dec[1];
	table_name = tp_dec[2].substr(0, tp_dec[2].find('.'));	
	column_number = col_number;
	knfolder_path = knf_path;
}

KNRebuild::~KNRebuild(void)
{
}

std::pair<int, std::string> KNRebuild::RebuildHistogram() throw(DatabaseRCException)
{
	pair<int,string> res;
	return res;
}

std::pair<int, std::string> KNRebuild::RebuildCMap() throw(DatabaseRCException)
{
	pair<int,string> res;
	return res;
}

std::pair<int, std::string> KNRebuild::Run() throw(DatabaseRCException)
{	
	pair<int, string> res;	
	if(table_test->IsEmptyRCTable()) {
		res.first = 0;
		res.second = string("RC Table could not be read.");
		return res;
	}
	RCTable& rct = table_test->GetRCTable();
	RCAttr* rca = rct.GetAttr(column_number);
	char s[1024];
	try {		
		IBFile ib_f;
		if (rca->PackType() == PackN) {			
			sprintf(s, "%s.%d.%d.rsi", (knfolder_path + DIR_SEPARATOR_STRING + string("HIST")).c_str(), rct.GetID(), column_number);
			RSIndex_Hist kn;
			if(DoesFileExist(string(s)))
				ib_f.OpenReadOnly(string(s));
			else {
				res.first = 0;
				res.second = string("Histogram for column " + boost::lexical_cast<string>(column_number) + string(" not found."));
				return res;
			}
			kn.Load(&ib_f, rca->GetCurReadLocation());
			if(kn.NoObj()!=0 && (kn.GetFixed() != ATI::IsFixedNumericType(rca->TypeName()))) {
				res.first = 2;
				res.second = string("Value of RCAttr::fixed parameter in histogram is inconsistent with column type.");
				return res;
			} else {
				res.first = 1;
				res.second = string("");
				return res;
			}
		} else if(rca->PackType() == PackS) {
			res.first = 0;
			res.second = string("");
			return res;
		} else {
			res.first = 0;
			res.second = string("No KNs for column ") + string(rct.AttrName(column_number)) + string(" found.");
			return res;
		}
	} catch(DatabaseRCException& e) {
		res.first = 2;
		res.second = string("Error") + e.what();
		return res;
	}
	return res;		
}
