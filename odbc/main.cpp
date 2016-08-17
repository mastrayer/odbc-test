#include <Windows.h>
#include <iostream>

#include <sql.h>
#include <sqlext.h>

#include <nana/gui.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/basic_types.hpp>

using namespace nana;

SQLHENV hEnv;
SQLHDBC hDbc;
SQLHSTMT hStmt;

SQLCHAR *ODBC_Name = (SQLCHAR*)"game_server";
SQLCHAR *ODBC_ID = (SQLCHAR*)"";
SQLCHAR *ODBC_PW = (SQLCHAR*)"";

char szID[128] = { 0, };
char szPW[128] = { 0, };

void UnicodeToAnsi(const wchar_t* pszText, const int destSize, char* pszDest)
{
	_snprintf_s(pszDest, destSize, _TRUNCATE, "%S", pszText);
}
bool DBConnect() {
	if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv) != SQL_SUCCESS)
		return false;
	if (SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER) != SQL_SUCCESS)
		return false;
	if (SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc) != SQL_SUCCESS)
		return false;

	if (SQLConnect(hDbc, ODBC_Name, SQL_NTS, ODBC_ID, SQL_NTS, ODBC_PW, SQL_NTS) != SQL_SUCCESS)
		return false;

	if (SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt) != SQL_SUCCESS)
		return false;

	return true;
}
void DBDisConnect() {
	if (hStmt)	SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
	if (hDbc)	SQLDisconnect(hDbc);
	if (hDbc)	SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
	if (hEnv)	SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
}

bool DBExcuteSQL(short type) {
	RETCODE retcode;
	// SQLBindParameter variables.
	char sParm1[128] = { 0, };
	char sParm2[128] = { 0, };
	BYTE sParm3 = 0;
	SQLLEN cbParm1 = SQL_NTS;
	SQLLEN cbParm2 = SQL_NTS;
	SQLLEN cbParm3 = SQL_NTS;

	strcpy(sParm1, szID);
	strcpy(sParm2, szPW);

	// Bind the return code to variable sParm1.
	retcode = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, &sParm1, 0, &cbParm1);
	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO)) {
		printf("SQLBindParameter(sParm1) Failed\n\n");
		DBDisConnect();
		return false;
	}

	if (type != 2) {
		retcode = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_VARCHAR, 0, 0, &sParm2, 0, &cbParm2);
		if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO)) {
			printf("SQLBindParameter(sParm2) Failed\n\n");
			DBDisConnect();
			return false;
		}
	}
	
	if (type == 1) {
		retcode = SQLBindParameter(hStmt, 3, SQL_PARAM_OUTPUT, SQL_C_TINYINT, SQL_TINYINT, 0, 0, &sParm3, 0, &cbParm3);
		if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO)) {
			printf("SQLBindParameter(sParm2) Failed\n\n");
			DBDisConnect();
			return false;
		}
	}

	UCHAR stmt[3][128] = {
		"{call CreateUser(?, ?)}",
		"{call CheckLoginData(?, ?, ?)}",
		"{call DeleteUser(?)}"
	};
	// Execute the command. 
	retcode = SQLExecDirect(hStmt, stmt[type], SQL_NTS);
	if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO)) {
		auto box = msgbox("SQLExecDirect").icon(msgbox::icon_error) << "Query Failed"; 
		box.show();
		//DBDisConnect();
		return false;
	}

	short TEST = -1;
	SQLBindCol(hStmt, 1, SQL_C_SSHORT, &TEST, sizeof(TEST), nullptr);

	/*
	// Show parameters are not filled.
	std::cout << "Before result sets cleared" << std::endl;
	std::cout << sParm1 << " / " << sParm2 << " / " << (int)sParm3 << std::endl;
	*/

	// Clear any result sets generated.
	SQLULEN num = 0;
	SQLUSMALLINT state;
	while ((retcode = SQLMoreResults(hStmt)) != SQL_NO_DATA) {
		retcode = SQLExtendedFetch(hStmt, SQL_FETCH_NEXT, 0, &num, &state);
		std::cout << TEST << std::endl;
	}

	// Show parameters are now filled.
	/*
	std::cout << "After result sets drained" << std::endl;
	std::cout << sParm1 << " / " << sParm2 << " / " << (int)sParm3 << std::endl;
	*/

	if (type != 1) {
		auto box = msgbox("Result").icon(msgbox::icon_information) << "Success";
		box.show();
	}
	else if(type == 1) {
		auto box = msgbox("Result");

		if (TEST == 1)
			box.icon(msgbox::icon_information) << "Equal";
		else
			box.icon(msgbox::icon_error) << "Not Equal";

		box.show();
	}

	return true;
}

void UpdateInputData(textbox &id, textbox &pw) {
	UnicodeToAnsi(id.caption_wstring().c_str(), 128, szID);
	UnicodeToAnsi(pw.caption_wstring().c_str(), 128, szPW);
}
int main() {
	if (DBConnect() == false) {
		auto msg = msgbox("DB Connect").icon(msgbox::icon_information) << "DB Connect failed";
		msg.show();

		exit(1);
	}

	form fm;

	textbox id{ fm };
	textbox pw{ fm };
	
	//Define a button and answer the click event.
	button sp1{ fm, "Create User" };
	sp1.events().click([&fm, &id, &pw] {
		msgbox(fm, "Call Stored Procedure").icon(msgbox::icon_information) << "Create User";
		UpdateInputData(id, pw);
		DBExcuteSQL(0);
	});
	button sp2{ fm, "Check Login Data" };
	sp2.events().click([&fm, &id, &pw] {
		msgbox(fm, "Call Stored Procedure").icon(msgbox::icon_information) << "Check Login Data";
		UpdateInputData(id, pw);
		DBExcuteSQL(1);
	});
	button sp3{ fm, "Delete User" };
	sp3.events().click([&fm, &id, &pw] {
		msgbox(fm, "Call Stored Procedure").icon(msgbox::icon_information) << "Delete User";
		UpdateInputData(id, pw);
		DBExcuteSQL(2);
	});

	
	id.caption("user id");
	pw.caption("password");

	auto a = id.caption_native();


	//Layout management
	fm.div("vert <><weight=24 <id><pw>><><sp1><sp2><sp3>");
	fm["sp1"] << sp1;
	fm["sp2"] << sp2;
	fm["sp3"] << sp3;
	fm["id"] << id;
	fm["pw"] << pw;
	fm.collocate();

	//Show the form
	fm.show();

	//Start to event loop process, it blocks until the form is closed.
	exec();

	DBDisConnect();
	return 0;
}
