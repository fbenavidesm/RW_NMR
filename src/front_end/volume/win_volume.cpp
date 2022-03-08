#include "win_volume.h"

WindowVolume::WindowVolume(wxWindow* parent, bool text) : wxWindow(parent,wxID_ANY)
{
	int attribs[] = { WX_GL_RGBA,
		WX_GL_DOUBLEBUFFER,
		WX_GL_SAMPLE_BUFFERS, GL_TRUE,
		WX_GL_SAMPLES, 8,
		WX_GL_DEPTH_SIZE, 24,
		0, 0 };
	this->_subtitle = 0;
	this->_volumePanel = new VolumePanel(this,wxID_ANY,attribs);
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	this->SetSizer(topSizer);
	topSizer->Add(this->_volumePanel,wxSizerFlags().Expand().Proportion(99));
	if (text)
	{
		wxStaticText* wtxt = new wxStaticText(this, 0, "");
		topSizer->Add(wtxt, 1, wxEXPAND);
		this->_subtitle = wtxt;
	}
	this->Bind(wxEVT_LEFT_DOWN, &VolumePanel::OnLeftMouseDown, this->_volumePanel);
	this->Bind(wxEVT_LEFT_UP, &VolumePanel::OnLeftMouseUp, this->_volumePanel);
	this->Bind(wxEVT_LEAVE_WINDOW, &VolumePanel::OnLeftMouseUp, this->_volumePanel);
	this->Bind(wxEVT_MOTION, &VolumePanel::OnLeftMouseMove, this->_volumePanel);
}


void WindowVolume::Set_Subtitle(const wxString& txt)
{
	if (this->_subtitle)
	{
		this->_subtitle->SetLabel(txt);
	}
}


void WindowVolume::Add_Image(const wxImage& img)
{
	this->_volumePanel->AddImage(img);
}

void WindowVolume::Set_Update_Event(VolumePanel::Observer* updevt)
{
	this->_volumePanel->Set_Update_Event(updevt);
}

float WindowVolume::Voxel_Factor() const
{
	return(this->_volumePanel->Voxel_Factor());
}

math_la::math_lac::space::Vec3 WindowVolume::Factor3D() const
{
	return(this->_volumePanel->Volume_Factor());
}

math_la::math_lac::space::Mtx3 WindowVolume::Rotation_Matrix() const
{
	return(this->_volumePanel->Volume_TrackBall().Rotation_Matrix());
}

