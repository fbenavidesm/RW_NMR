#ifndef WINVOL_H
#define WINVOL_H

#include "wx/wx.h"
#include "volume_panel.h"
#include "math_la/math_lac/space/mtx3.h"

using math_la::math_lac::space::Mtx3;

/**
* This window displays a volume texture (a 3D texture). It executes user actions, sending its messages to the subordinate
* VolumePanel, which takes charge of displaying the volume and execute the user events. 
* 
**/
class WindowVolume : public wxWindow
{
private:
	/**
	* The volume panel that renders the volume and execute user commands
	**/
	VolumePanel* _volumePanel;
	/**
	* A subtitle of the volume renderer
	**/
	wxStaticText* _subtitle;
public:
	WindowVolume(wxWindow* parent, bool text = true);

	/**
	* Adds an image to the texture renderer. These textures are not inmediatelly displayed, but they are stored
	* in order to build a 3D texture
	* @param img A SFML Image to accumulate in the texture
	**/
	void Add_Image(const wxImage& img);

	/**
	* @return The rotation matrix that is currently being applied to the volume
	**/
	math_la::math_lac::space::Mtx3 Rotation_Matrix() const;
	/**
	* This event is executed whenever the VolumePanel is rendering the volume, complementing the information
	* that is displayed to the user.
	* @param updevt An inherited object from VolumePanel::UpdateEvent. 
	**/
	void Set_Update_Event(VolumePanel::Observer* updevt);
	/**
	* @return The reduction factor that is being applied to the texture, according to the image sizes and the 
	* number of images
	**/
	math_la::math_lac::space::Vec3 Factor3D() const;
	/**
	* @return The reduction factor that is being applied to the image texture, according to the Panel size
	**/
	float Voxel_Factor() const;

	/**
	* @return The VolumePanel is the class that takes charge of displaying the volume and makes interface
	* to OpenGL functions, and compiles the volume shader. 
	**/
	VolumePanel& Volume_Panel();

	/**
	* A simple subtitle that describes the volume
	* @param txt The subtitle
	**/
	void Set_Subtitle(const wxString& txt);
};

inline VolumePanel& WindowVolume::Volume_Panel()
{
	return(*this->_volumePanel);
}


#endif