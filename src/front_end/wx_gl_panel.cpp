#include "gl/glew.h"
#include "wx_gl_panel.h"
#include "wx/glcanvas.h"


WxGLPanel::WxGLPanel(wxWindow* Parent, wxWindowID Id, int* attrs, const wxPoint& Position, const wxSize& Size, long Style) :
	wxGLCanvas(Parent,Id,attrs)
{	
	this->_swap = true;
	this->Enable(false);
	this->_context = 0;
	this->Bind(wxEVT_PAINT, &WxGLPanel::On_Paint, this);
	this->Bind(wxEVT_ERASE_BACKGROUND, &WxGLPanel::On_Erase_Background, this);
	this->Bind(wxEVT_SIZE, &WxGLPanel::On_Resize, this);
}

WxGLPanel::~WxGLPanel()
{
	this->Enable(false);
	this->Unbind(wxEVT_PAINT, &WxGLPanel::OnPaint, this);
	this->Unbind(wxEVT_ERASE_BACKGROUND, &WxGLPanel::On_Erase_Background, this);
	this->Unbind(wxEVT_SIZE, &WxGLPanel::On_Resize, this);
	if (this->_context)
	{
		delete this->_context;
	}	
}

bool WxGLPanel::AcceptsFocus() const
{
	return(false);
}

void WxGLPanel::On_Erase_Background(wxEraseEvent& evt)
{

}

void WxGLPanel::On_Resize(wxSizeEvent& evt)
{
	this->On_Size_Change();
	this->Refresh();
}

void WxGLPanel::Set_GL_Context()
{
	if (!this->_context)
	{
		this->_context = new wxGLContext(this);
		if (this->SetCurrent(*this->_context))
		{
			glewInit();
		}
		else
		{
			delete this->_context;
			this->_context = 0;
			Sleep(250);
			this->Set_GL_Context();
		}
	}
	else
	{
		if (!this->SetCurrent(*this->_context))
		{
			delete this->_context;
			this->_context = 0;
			Sleep(250);
			this->Set_GL_Context();
		}
	}
}

void WxGLPanel::On_Paint(wxPaintEvent& evt)
{
	if (this->IsShown())
	{
		wxPaintDC dc(this);
		this->Set_GL_Context();
		glClearColor((float)this->GetBackgroundColour().Red() / 255.0f,
			(float)this->GetBackgroundColour().Green() / 255.0f,
			(float)this->GetBackgroundColour().Blue() / 255.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glViewport(0, 0, this->GetSize().x, this->GetSize().y);
		this->On_Update();
		this->SwapBuffers();
	}
}

