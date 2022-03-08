#include "wx_progress_adapter.h"

WxProgressAdapter::WxProgressAdapter(wxGenericProgressDialog* wxDlg)
{
	this->_wxDlg = wxDlg;
}

void WxProgressAdapter::Update(uint step, const string& msg)
{
	this->_wxDlg->Update(step, wxString(msg.c_str()));
	this->_wxDlg->Refresh();
}

void WxProgressAdapter::Update(const string& msg)
{
	this->_wxDlg->Pulse(wxString(msg.c_str()));
	this->_wxDlg->Refresh();
}

void WxProgressAdapter::Set_Range(uint N)
{
	this->_wxDlg->SetRange(N);
}

