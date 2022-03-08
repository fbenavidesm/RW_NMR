#ifndef WX_GL_PANEL_H
#define WX_GL_PANEL_H

#include <GL/glew.h>
#include <wx/wx.h>
#include "wx/glcanvas.h"
#include "wx/event.h"
#include "wx/time.h"


class WxGLPanel : public wxGLCanvas
{
private:
	bool _swap;
	wxGLContext* _context;
protected:
	void On_Paint(wxPaintEvent& evt);
	void On_Erase_Background(wxEraseEvent& evt);
	void On_Resize(wxSizeEvent& evt);
	virtual void On_Update(){};
	virtual void On_Size_Change(){};
public:
	bool AcceptsFocus() const;
	WxGLPanel(wxWindow* Parent, wxWindowID Id = -1, int* attrs = 0, const wxPoint& Position = wxDefaultPosition,
		const wxSize& Size = wxDefaultSize, long Style = 0);
	void Set_GL_Context();
	virtual ~WxGLPanel();
};

#endif