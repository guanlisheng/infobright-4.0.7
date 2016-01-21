#include <string>
#include "TestOfKNsConsistencyLight.h"
#include "boost/filesystem.hpp"
#include "core/Filter.h"
#include "core/RCAttr.h"
#include "common/CommonDefinitions.h"
#include "edition/local.h"

#define TEST_OF_KNS_CONSISTENCY_LIGHT "Test of knowledge grid consistency"

using namespace std;
using namespace boost::filesystem;

TestOfKNsConsistencyLight::TestOfKNsConsistencyLight(void)
{	
	table_test = 0;
	test_name = (string(TEST_OF_KNS_CONSISTENCY_LIGHT));
}

TestOfKNsConsistencyLight::TestOfKNsConsistencyLight(const string& tp, TableTest* table_test, int col_number, const string& knf_path)
{	
	this->table_test = (table_test);
	test_name = (string(TEST_OF_KNS_CONSISTENCY_LIGHT));
	vector<string> tp_dec = TableTest::ParseTablePath(tp);
	data_dir = tp_dec[0];
	database_name = tp_dec[1];
	table_name = tp_dec[2].substr(0, tp_dec[2].find('.'));	
	column_number = col_number;
	knfolder_path = knf_path;
}

TestOfKNsConsistencyLight::~TestOfKNsConsistencyLight(void)
{
}

std::pair<int, std::string> TestOfKNsConsistencyLight::Run() throw(DatabaseRCException)
{	
	pair<int, string> res;	
	if(table_test->IsEmptyRCTable()) {
		res.first = 0;
		res.second = string("RC Table could not be read.");
		return res;
	}
	RCTable& rct = table_test->GetRCTable();
	RCAttr* rca = rct.GetAttr(column_number);
	string kn_type, kn_name;
	if (rca->PackType() == PackN){
		kn_type = string("HIST");
		kn_name = string("Histogram");
	} else {
		kn_type = string("CMAP");
		kn_name = string("CMap");
	}
	char s[1024];
	try {		
		IBFile ib_f;
		sprintf(s, (knfolder_path + DIR_SEPARATOR_CHAR + string("%s.%d.%d.rsi")).c_str(), kn_type.c_str(), rca->table_number, column_number);
		if(DoesFileExist(string(s))) {
			ib_f.OpenReadOnly(string(s));
			if(ib_f.Seek(0,SEEK_END)==0){
				res.first = 2;
				res.second = string(kn_name + string(" file is zero-length for this column."));
				return res;
			}
		}
		else {
			bool should_exist = false;
			if(rca->PackType() == PackN) {
				for(int p = 0; !should_exist && p < rca->NoPack(); p++)
					if (rca->GetNoNulls(p) < rca->GetNoValues(p) && rca->GetMinInt64(p) != rca->GetMaxInt64(p))
						should_exist = true;
			} else if (rca->PackType() == PackS && !RequiresUTFConversions(rca->Type().GetCollation())) {
				for(int p = 0; !should_exist && p < rca->NoPack(); p++)
					if (rca->GetNoNulls(p) < rca->GetNoValues(p))
						should_exist = true;
			}

			if (should_exist) {
				res.first = 0;
				res.second = kn_name + string(" for column " + boost::lexical_cast<string>(column_number) + string(" not found."));
				return res;
			} else {
				res.first = 1;
				res.second = string("");
				return res;
			}
		}

		if(rca->PackType() == PackN){			
			RSIndex_Hist kn;
			kn.Load(&ib_f, rca->GetCurReadLocation());
			// check consistency of number of objects in dpn vs kn 	
			// deprecated
			//if((kn->NoObj()!=0) && (rca->NoObj() < kn->NoObj())) {
			//	res.first = 3;
			//	res.second = string("Number of objects in ") + kn_name + string(" exceed number of objects in DPN.");
			//	return res;
			//}		
			if(kn.NoObj()!=0 &&
				( (!rct.IsLookup(column_number) && (ATI::IsFixedNumericType(rca->TypeName()) || ATI::IsDateTimeType(rca->TypeName())) != kn.GetFixed()) ||
					(rct.IsLookup(column_number) && kn.GetFixed() != 1) )) {
				res.first = 2;
				res.second = string("Value of RCAttr::fixed parameter in histogram is inconsistent with column type.");
				return res;
			}

		} else if (rca->PackType() == PackS) {
			RSIndex_CMap kn;
			kn.Load(&ib_f, rca->GetCurReadLocation());
			// check consistency of number of objects in dpn vs kn
			// deprecated
			//if((kn->NoObj()!=0) && (rca->NoObj() < kn->NoObj())) {
			//	res.first = 3;
			//	res.second = string("Number of objects in ") + kn_name + string(" exceed number of objects in DPN.");
			//	return res;
			//}		
		} else {
			res.first = 0;
			res.second = string("No KNs for column ") + string(rct.AttrName(column_number)) + string(" found.");
			return res;
		}

		res.first = 1;
		res.second = string("");
		return res;	

	} catch(DatabaseRCException&) {
		res.first = 2;
		res.second = kn_name + string(" file for column ") + string(rct.AttrName(column_number)) + string(" could not be read.");
		return res;
	}
	return res;		
}
