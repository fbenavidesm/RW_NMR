#include "wx/wx.h"
#include "mkl.h"
#include "front_end/rwui/win_main.h"

class App : public wxApp
{
private:
	WindowMain* _mwin;
public:
	App() : wxApp()
	{
	};
	~App()
	{
	}
	bool OnInit()
	{		
		this->_mwin = new WindowMain();

		this->_mwin->Show();	

		return(true);

	};
};


DECLARE_APP(App);

IMPLEMENT_APP(App);
