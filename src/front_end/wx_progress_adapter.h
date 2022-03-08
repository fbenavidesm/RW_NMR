#ifndef WX_PROGRESS_BRIDGE_H
#define WX_PROGRESS_BRIDGE_H

#include <string>
#include "wx/wx.h"
#include "rw/binary_image/binary_image.h"
#include "wx/progdlg.h"

using std::string;

class WxProgressAdapter : public rw::BinaryImage::ProgressAdapter
{
private:
	wxGenericProgressDialog* _wxDlg;
public:
	WxProgressAdapter(wxGenericProgressDialog* wxDlg);
	void Update(uint step, const string& msg);
	void Update(const string& msg);
	void Set_Range(uint N);
};


#endif
