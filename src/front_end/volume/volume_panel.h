#ifndef VOLUME_PANEL_H
#define VOLUME_PANEL_H

#include <vector>
#include <string>
#include "wx/wx.h"
#include "front_end/wx_gl_panel.h"
#include "gl/shader.h"
#include "gl/graphics_buffer.h"
#include "gl/trackball.h"
#include "math_la/math_lac/space/mtx4.h"
#include "math_la/math_lac/space/vec4.h"
#include "math_la/math_lac/space/vec3.h"

using math_la::math_lac::space::Mtx4;
using math_la::math_lac::space::Vec4;
using math_la::math_lac::space::Vec3;

using gl::TrackBall;

using std::vector;
using std::string;

/**
* A VolumePanel is the class that displays a volume in a WxWidgets control.
* It uses SFML functions and reners according to WxWidgets rules.
**/

class VolumePanel : public WxGLPanel
{
public:
	/**
	* The Observer event is executed every time the volume is rendered, exactly after displaying the volume
	**/
	class Observer
	{
	private:
		friend class VolumePanel;
		/**
		* This value is TRUE when the volume is being rendered
		*/
		volatile bool _painting;
		/**
		* This variable allows to block this event after the volume is rendered
		*/
		volatile bool _blocked;
	public:
		/**
		* Executes just after displaying the volume
		*/
		virtual void On_Update() = 0;
		/**
		* A bar can be displayed at the left side of the volume to control maximal and minimal values.
		* This bar can be rendered and updated by the user. This is executed whenever the user changes
		* these values. They should be associated to a section of the rendered volume.
		* @param barmin Minimal value of the bar
		* @[aram barmax Maximal value of the bar
		*/
		virtual void On_Update_Bar(float barmin, float barmax) {};
		/**
		* Blocks the update event
		*/
		void Block_Event();
		/**
		* Unblocks the event
		*/
		void Unblock_Event();
		/**
		* @return TRUE if the event is blocked
		*/
		bool Blocked() const;
	};
private:

	float _zcompression;
	/**
	* A simple graphic buffer associated to the rendering cube
	*/
	gl::GraphicsBuffer _graphicBuffer;

	/**
	* The rendering volume shader
	*/
	gl::Shader _shader;

	/**
	* List of images that define the 3D texture
	*/
	vector<wxImage*> _imageTextureList;

	/**
	* 3D Object projection matrix
	*/
	Mtx4 _projectionMatrix;

	/**
	* Camera position to visualize the object
	*/
	Vec4 _cameraPosition;

	/**
	* Defines the 3D object dominant color
	*/
	Vec3 _objectColor;

	/**
	* Minimal value of color recognized as solid
	*/
	Vec3 _solidBoxMin;

	/**
	* Maximal value of color recognized as solid
	*/
	Vec3 _solidBoxMax;

	/**
	* Coordinates of the displayed minimal extreme of the solid
	*/
	Vec3 _solidMinVol;

	/**
	* Coordinates of the displayed maximal extreme of the solid
	*/
	Vec3 _solidMaxVol;

	/**
	* Coordinates of the minimal extreme of the displayed cube
	*/
	Vec3 _cubeMin;

	/**
	* Coordinates of the maximal extreme of the displayed cube
	*/
	Vec3 _cubeMax;

	/**
	* Vector describning the volumetric space
	*/
	Vec3 _cubeVol;

	/**
	* OpenGL id of the displayed volume
	*/
	GLint _idxCubeVol;

	/**
	* Normalizer of the 3D image texture
	*/
	scalar _nn;

	/**
	* Background coltron color
	*/
	wxColor _controlColor;

	/**
	* Control light color
	*/
	wxColor _lightControlColor;

	/**
	* The color of the borders of the volume control
	*/
	wxColor _borderColor;

	/**
	* This color defines the borders of the region of interest
	*/
	wxColor _revMarker;
	/**
	* OpenGL id of the camera position vector
	*/
	GLint _idxCameraPositionGL;

	/**
	* OpenGL id of the model matrix
	*/
	GLint _idxModelMatrixGL;

	/**
	* OpenGL id of the model matrix (transposed)
	*/
	GLint _idxModelMatrixTransposedGL;

	/**
	* OpenGL id of the background color
	*/
	GLint _idxBackgroundColorGL;

	/**
	* OpenGL id of the object color
	*/
	GLint _object3DColorGL;

	/**
	* OpenGL id of the lower extreme of the displayed object
	*/
	GLint _idxMinVolumeGL;

	/**
	* OpenGL id of the upper extreme color of the object considered as solid
	*/
	GLint _idxMaxVolumeGL;

	/**
	* OpenGL id of the lightest color of the object recongized as solid
	*/
	GLint _idxMaxVolumeColorGL;

	/**
	* OpenGL id of the darkest color of the object recongized as solid
	*/
	GLint _idxMinVolumeColorGL;

	/**
	* The object handling trackball
	*/
	TrackBall _trackBall;

	/**
	* TRUE if mouse has been clicked
	*/
	bool _mouseDown;

	/**
	* Update event when rendering
	*/
	VolumePanel::Observer* _updateEvent;

	/**
	* 3D correction factor
	*/
	Vec3 _correctionFactor;

	/**
	* Voxel global correction factor
	*/
	float _voxelFactor;

	/**
	* Minimal moving bar value
	*/
	float _barMin;

	/**
	* Maximal moving bar value
	*/
	float _barMax;

	/**
	* Moving bar width
	*/
	int _barWidth;

	/**
	* Space between the control border and the bar
	*/
	int _barBorder;

	/**
	* Sensibility of the moving bar
	*/
	int _controlFactor;

	/**
	* Current position of the bar
	*/
	int _currentBarPosition;

	/**
	* TRUE if the minimal bar marker is being moved
	*/
	bool _barMinMoving;

	/**
	* TRUE if the maximal bar marker is being moved
	*/
	bool _barMaxMoving;

	/**
	* TRUE if the main bar marker is being moved
	*/
	bool _barMoving;

	/**
	* Precision of the moving bar
	*/
	int _depthPrecision;

	/**
	* Precision of the moving bar stored in the OpenGL volume shader
	*/
	int _idxDepthPrecisionGL;

	/**
	* This mutex is necessary to coordinate several actions that can be performed on the volume renderer
	* from other threads
	*/
	wxMutex _locker;

	/**
	* Stops or resume the displaying process of the volume renderer
	*/
	bool _lockRefresh;

	/**
	* Set of minimal values of the marked box
	*/
	Vec3 _boxMin;

	/**
	* Set of maximal values of the marked box
	*/
	Vec3 _boxMax;

	/**
	* Box color
	*/
	Vec3 _boxColor;

	/**
	* Identifier of the minimal box values
	*/
	int _idxBoxMinGL;

	/**
	* Identifier of the maximal box values
	*/
	int _idxBoxMaxGL;

	/**
	* Identifier of the marked box color
	*/
	int _idxBoxColorGL;

protected:
	/**
	* Draws the control bar at the right side of the rendered volume
	*/
	void Draw_Bar();

	/**
	* Loads the 3D texture to the graphics device
	*/
	void Load_Texture_To_Graphic_Device();

	/**
	* Draws a circle
	* @param radius Radius
	* @param x X coordinate of the center
	* @param y Y coordinate of the center
	* @param scx Scale factor for the axis X
	* @param scy Scale factor for the axis Y
	* @param color Color of the circle
	*/
	void Draw_Circle(float radius, float x, float y, float scx, float scy,
		const wxColor& border_color, const wxColor& color, int res = 32);
public:
	VolumePanel(wxWindow* Parent, wxWindowID Id = -1, int* attrs = 0, const wxPoint& Position = wxDefaultPosition,
		const wxSize& Size = wxDefaultSize, long Style = 0);
	/**
	* Defines the 3D volume of the volume
	*/
	void Setup_Cube();
	~VolumePanel();

	/**
	* Handles display event
	*/
	void On_Update();

	/**
	* Handles MouseDown event
	*/
	void OnLeftMouseDown(wxMouseEvent& evt);

	/**
	* Handles MouseUp event. It stops updating the rotation matrix
	*/
	void OnLeftMouseUp(wxMouseEvent& evt);

	/**
	* Handles MouseMove event. The object is moved according to the TrackBall rotation matrix
	*/
	void OnLeftMouseMove(wxMouseEvent& evt);

	/**
	* OnResize event, updating trackball
	*/
	void On_Resize(wxSizeEvent& evt);

	/**
	* Adds a SFML image to the 3D texture
	*/
	void AddImage(const wxImage& img);


	/**
	* Loads the ray-casting shader that handles the volume rendering
	*/
	void Load_Volume_Shader();

	/**
	* Defines the view-port and the projection matrix of the volume. This is automatic, based on the window size
	*/
	void Define_Projection_Matrix();

	/**
	* @return The volume trackball
	*/
	const TrackBall& Volume_TrackBall() const;

	/**
	* Defines the update event that is executed after rendering the volume
	*/
	void Set_Update_Event(VolumePanel::Observer* evt);

	/**
	* The correction factor applied to each voxel size, according to how the volume is rendered
	*/
	Vec3 Volume_Factor() const;

	/**
	* The correction factor of one voxel, according to how the volume is rendered
	*/
	float Voxel_Factor() const;

	/**
	* Releases the memory associated to the 3D texture
	*/
	void Release_3D_Texture();

	/**
	* Define the solid color of the object
	*/
	void Set_Solid_Color(wxColor Sim_Color);

	/**
	* Lower extreme of the volume (in each isolated coordinate)
	* @param vmin Vector of minimal positions
	*/
	void Set_Min_Vol(const Vec3& vmin);

	/**
	* Upper extreme of the volume (in each isolated coordinate)
	* @param vmax Vector of maximal positions
	*/

	void Set_Max_Vol(const Vec3& vmax);

	/**
	* Stops OpenGL rendering
	*/
	void Lock_Refresh();

	/**
	* Resumes OpenGL rendering
	*/
	void Unlock_Refresh();

	/**
	* Loads the 3D texture to the Graphics Device.
	*/
	void Load_3D_Texture();

	/**
	* Sets the marked box color (for example, to mark a subimage)
	*/
	void Set_Box_Color(const wxColor& color);

	/**
	* Defines the borders of the marker box
	* @param xmin Minimal X
	* @param ymin Minimal Y
	* @param zmin Minimal Z
	* @param xmax Maximal X
	* @param ymax Maximal Y
	* @param zmax Maximal Z
	*/
	void Set_Mark_Box(float xmin, float ymin, float zmin,
		float xmax, float ymax, float zmax);

	/**
	* Sets the borders of the bar that defines the observed volume
	*/
	void Set_Bar_Border(float bordermin, float bordermax);

	const Vec3& Box() const;
	void Set_Z_Compression(float zcompression);
	float ZCompression() const;
};

inline math_la::math_lac::space::Vec3 VolumePanel::Volume_Factor() const
{
	return(this->_correctionFactor);
}

inline const TrackBall& VolumePanel::Volume_TrackBall() const
{
	return(this->_trackBall);
}

inline float VolumePanel::Voxel_Factor() const
{
	return(this->_voxelFactor);
}

inline bool VolumePanel::Observer::Blocked() const
{
	return(this->_blocked);
}

inline const Vec3& VolumePanel::Box() const
{
	return(this->_cubeVol);
}

inline float VolumePanel::ZCompression() const
{
	return(this->_zcompression);
}




#endif