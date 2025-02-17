#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
using namespace std;
using std::string;
using std::ifstream;
using std::vector;
using std::map;
using std::istringstream;
template <typename T>

#pragma warning(disable: 4244)  // stop warning: "conversion from 'int' to 'float', possible loss of data"

string NumberToString ( T Number )
{
	ostringstream ss;
	ss << Number;
	return ss.str();
}


template <typename T>
T StringToNumber ( const string &Text )
{
	istringstream ss(Text);
	T result;
	return ss >> result ? result : 0;
}
class CCSVParser
{
public:
	char Delimiter;
	bool IsFirstLineHeader;
	ifstream inFile;
	vector<string> LineFieldsValue;
	vector<string> Headers;

	map<string,int> FieldsIndices;

	map<string, int> FieldsIndices_read;

	vector<string> ParseLine(string line);
	vector<int> LineIntegerVector;

public:
	string GetUnreadHeaders()
	{
		string str_UnreadHeaders;
		
		for (unsigned i = 0; i < Headers.size(); i++)
		{
			if (FieldsIndices_read.find(Headers[i]) == FieldsIndices_read.end())  // unread
			{

				str_UnreadHeaders += Headers[i];
				str_UnreadHeaders += ",";
			}
		}

		return str_UnreadHeaders;

	}

	string GetUnreadLineFieldsValues()
	{
		string str_LineFieldsValues;

		for (unsigned i = 0; i < Headers.size(); i++)
		{
			if (FieldsIndices_read.find(Headers[i]) == FieldsIndices_read.end())  // unread
			{

				if(i < LineFieldsValue.size() &&  LineFieldsValue[i].size() > 0)
				str_LineFieldsValues += LineFieldsValue[i];

				str_LineFieldsValues += ",";

			}
		}
		return str_LineFieldsValues;

	}
	void  ConvertLineStringValueToIntegers()
	{
		LineIntegerVector.clear();
		for(unsigned i = 0; i < LineFieldsValue.size(); i++)
		{
			std::string si = LineFieldsValue[i];
			int value = atoi(si.c_str ());

			if(value>=1)
				LineIntegerVector.push_back(value);

		}
	}
	vector<string> GetHeaderVector()
	{
		return Headers;
	}

	int m_EmptyLineCount ;
	bool m_bSynchroSingleCSVFile;
	string m_SynchroSectionName;
	string SectionName;
	bool m_bLastSectionRead;

	bool ReadSectionHeader(string s);

	bool m_bSkipFirstLine;  // for synchro CSV files
	CCSVParser(void);
	bool OpenCSVFile(string fileName, bool bIsFirstLineHeader = true);
	void CloseCSVFile(void);
	bool ReadRecord();
	bool ReadRecord_Section();
	vector<string> GetLineRecord(void);
	vector<string> GetHeaderList();


	template <class T> bool GetValueBySectionKeyFieldName(string file_name, string section_name, string key_name, string field_name, T& value)
	{
		OpenCSVFile(file_name);
		while(ReadRecord())
		{
			if(LineFieldsValue[0]!=section_name || LineFieldsValue[1] != key_name)
			continue;
	
		if (FieldsIndices.find(field_name) == FieldsIndices.end())
		{
			CloseCSVFile();
			return false;
		}
		else
		{
			if (LineFieldsValue.size() == 0)
			{
				CloseCSVFile();
				return false;
			}

			int size = (int)(LineFieldsValue.size());
			if(FieldsIndices[field_name]>= size)
			{
				CloseCSVFile();
				return false;
			}

			string str_value = LineFieldsValue[FieldsIndices[field_name]];

			if (str_value.length() <= 0)
			{
			CloseCSVFile();
				return false;
			}

			istringstream ss(str_value);

			T converted_value;
			ss >> converted_value;

			if (/*!ss.eof() || */ ss.fail())
			{

			CloseCSVFile();
			return false;
			}

			value = converted_value;
			CloseCSVFile();
			return true;
		}
		}
			CloseCSVFile();

			return false;
	}
	template <class T> int GetValueByFieldName(string field_name, T& value, bool NonnegativeFlag  = true)
	{
		if (FieldsIndices.find(field_name) == FieldsIndices.end())  // field not defined
		{
			return -1;
		}
		else
		{	// field is defined 

			if (LineFieldsValue.size() == 0)
			{
				return 0;
			}

			int size = (int)(LineFieldsValue.size());
			if(FieldsIndices[field_name]>= size)
			{
				return 0;
			}

			string str_value = LineFieldsValue[FieldsIndices[field_name]];

			FieldsIndices_read[field_name] = 1;  // the data item has been read


			if (str_value.length() <= 0)
			{
				return 0;
			}


			istringstream ss(str_value);

			T converted_value;
			ss >> converted_value;

			if (/*!ss.eof() || */ ss.fail())
			{
				return 0;
			}

			if(NonnegativeFlag && converted_value<0)
				converted_value = 0;

			value = converted_value;
			return 1;
		}
	}

	bool GetValueByFieldName(string field_name, string& value)
	{
		if (FieldsIndices.find(field_name) == FieldsIndices.end())
		{
			return false;
		}
		else
		{
			if (LineFieldsValue.size() == 0)
			{
				return false;
			}

			unsigned int index  = FieldsIndices[field_name];
			if(index >=LineFieldsValue.size())
			{
				return false;
			}
			string str_value = LineFieldsValue[index];

			FieldsIndices_read[field_name] = 1;  // the data item has been read

			if (str_value.length() <= 0)
			{
				return false;
			}

			value = str_value;
			return true;
		}
	}

	~CCSVParser(void);
};

class CCSVWriter
{
public : 
	ofstream outFile;
	char Delimiter;
	int FieldIndex;
	bool IsFirstLineHeader;
	map<int,string> LineFieldsValue;
	vector<string> LineFieldsName;
	vector<string> LineFieldsCategoryName;
	map<string,int> FieldsIndices;  

	bool row_title;

public:
	void SetRowTitle(bool flag)
	{
		row_title = flag;
	}

	bool OpenCSVFile(string fileName, bool b_required=true);
	void CloseCSVFile(void)
	{
	outFile.close();
	}
	template <class T> bool SetValueByFieldName(string field_name, T& value)  // by doing so, we do not need to exactly follow the sequence of field names
	{
		if (FieldsIndices.find(field_name) == FieldsIndices.end())
		{
			return false;
		}
		else
		{

			LineFieldsValue[FieldsIndices[field_name]] = NumberToString(value);

			return true;
		}
	}

	void Reset()
	{

		LineFieldsValue.clear();
		LineFieldsName.clear();
		LineFieldsCategoryName.clear();
		FieldsIndices.clear();

	}
	void SetFieldName(string field_name)
	{ 
		FieldsIndices[field_name] = (int)(LineFieldsName.size());
		LineFieldsName.push_back (field_name);
		LineFieldsCategoryName.push_back(" ");

	}

	template <class T>  void SetFieldNameAndValue(string field_name, T& value)
	{ 
		FieldsIndices[field_name] = (int)(LineFieldsName.size());
		LineFieldsName.push_back (field_name);
		LineFieldsCategoryName.push_back(" ");
		SetValueByFieldName(field_name,value);
	}

	void SetFieldNameWithCategoryName(string field_name,string category_name)
	{ 
		FieldsIndices[field_name] = (int)(LineFieldsName.size());
		LineFieldsName.push_back (field_name);
		LineFieldsCategoryName.push_back(category_name);

	}


	void WriteTextString(CString textString)
	{
		if (!outFile.is_open()) 
			return;
		outFile << textString << endl;

	}

	void WriteTextLabel(CString textString)
	{
		if (!outFile.is_open()) 
			return;
		outFile << textString;

	}

	template <class T>  void WriteNumber(T value)
	{
		if (!outFile.is_open()) 
			return;
		outFile << NumberToString(value) << endl;
	}

	template <class T>  void WriteParameterValue(CString textString, T value)
	{
		if (!outFile.is_open()) 
			return;

		outFile << textString <<"=,"<< NumberToString(value) << endl;
	}

	void WriteNewEndofLine()
	{
		if (!outFile.is_open()) 
			return;
		outFile << endl;
	}


	void WriteHeader()
	{
		if (!outFile.is_open()) 
			return;


		//for(unsigned int i = 0; i< FieldsIndices.size(); i++)
		//{
		//outFile << LineFieldsCategoryName[i] << ",";
		//}
		//outFile << endl;

		//if(row_title == true)
		//	outFile << ",";

		for(unsigned int i = 0; i< FieldsIndices.size(); i++)
		{
			outFile << LineFieldsName[i] << ",";
		}

		outFile << endl;
	}
	void WriteRecord()
	{
		if (!outFile.is_open()) 
			return;

		for(unsigned int i = 0; i< FieldsIndices.size(); i++)
		{
			string str ;
			if(LineFieldsValue.find(i) != LineFieldsValue.end() && LineFieldsValue[i].size() >=1) // has been initialized
				outFile << LineFieldsValue[i].c_str () << ",";
			else
				outFile << ' ' << ",";
		}

		LineFieldsValue.clear();

		outFile << endl;
	}

	CCSVWriter::CCSVWriter()
	{
		row_title = false;
		FieldIndex = 0;
		Delimiter = ',';
		IsFirstLineHeader = true;
	}

	CCSVWriter::~CCSVWriter(void)
	{
		if (outFile.is_open()) outFile.close();
	}


	CCSVWriter::CCSVWriter(string fileName)
	{
		Open(fileName);

	};

	bool CCSVWriter::Open(string fileName)
	{
		outFile.open(fileName.c_str());

		if (outFile.is_open()==false)
		{

			CString str;
			str.Format("File %s cannot be opened.", fileName.c_str());
			AfxMessageBox(str);
			return false;
		}

		return true;
	};

	void CCSVWriter::OpenAppend(string fileName)
	{
		outFile.open(fileName.c_str(), fstream::app);

		if (outFile.is_open()==false)
		{
			cout << "File " << fileName.c_str() << " cannot be opened." << endl;
			getchar();
			exit(0);
		}

	};
};


