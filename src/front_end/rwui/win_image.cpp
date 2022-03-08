#include "tbb/parallel_for.h"
#include "tbb/spin_mutex.h"
#include "win_image.h"
#include "wx/progdlg.h"
#include "wx/choicdlg.h"
#include "wx/ribbon/toolbar.h"
#include "win_main.h"
#include "front_end/wx_image_adapter.h"
#include "front_end/wx_progress_adapter.h"


WindowImage::WindowImage(wxWindow* parent, WindowMain* windowMain) :
	wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, "Images")
{
	this->Freeze();
	this->_showProcessedImages = false;
	this->_paintPores = true;
	this->_dispImage = 0;
	this->_windowMain = windowMain;
	this->_mgr = new wxAuiManager(this, wxAUI_MGR_ALLOW_FLOATING | wxAUI_MGR_ALLOW_ACTIVE_PANE | wxAUI_MGR_TRANSPARENT_HINT);
	this->_mgr->SetDockSizeConstraint(0.3, 0.36);

	/** Slider to change displayed image **/

	wxPanel* spnl = new wxPanel(this);
	wxBoxSizer* pnlsizer = new wxBoxSizer(wxVERTICAL);
	wxStaticText* sttext = new wxStaticText(spnl, wxID_ANY,"  File: ");
	wxStaticText* sttext_depth = new wxStaticText(spnl, wxID_ANY, "  Texture depth: ");
	wxSlider* slider = new wxSlider(spnl, wxID_ANY, 0, 0, 100);
	spnl->SetSizer(pnlsizer);
	spnl->SetBackgroundColour(wxColor(255, 255, 255));	
	pnlsizer->Add(sttext,wxSizerFlags().Expand().Proportion(1));
	pnlsizer->Add(sttext_depth, wxSizerFlags().Expand().Proportion(1));
	pnlsizer->Add(slider, wxSizerFlags().Expand().Proportion(1));

	this->_sliderImg = slider;
	/**
	Image panel to display texture layers
	*/
	this->_leftImg = new WxImagePanel(this);
	this->_leftImg->SetBackgroundColour(wxColor(235, 235, 235));	

	this->_rightImg = new WxImagePanel(this);
	this->_rightImg->SetBackgroundColour(wxColor(235, 235, 235));

	this->_fileText = sttext;
	this->_depthText = sttext_depth;

	/**Segmentation panel to binarize images**/
	wxPanel* npnl = new wxPanel(spnl);
	npnl->SetBackgroundColour(wxColor(255, 255, 255));
	npnl->SetWindowStyle(wxBORDER_RAISED);

	wxImage img;
	wxBitmap bmp;
	wxPropertyGrid* pg = new wxPropertyGrid(
		npnl,
		0,
		wxDefaultPosition,
		wxDefaultSize,
		wxPG_SPLITTER_AUTO_CENTER |
		wxPG_DEFAULT_STYLE);
	this->_propertyGrid = pg;
	pg->Append(new wxPropertyCategory("Binarization"));
	wxPGProperty* pp = 0;
	pp = pg->Append(new wxIntProperty("Pore-solid level", "LMIN", 1));
	pp = pg->Append(new wxColourProperty("Pore-solid border", "MIN", wxColor(1, 1, 1)));
	pp->ChangeFlag(wxPG_PROP_READONLY, true);
	pp->ChangeFlag(wxPG_PROP_NOEDITOR, true);
	pp->ChangeFlag(wxPG_PROP_DISABLED, true);
	wxBoxSizer* apnlsizer = new wxBoxSizer(wxVERTICAL);
	npnl->SetSizer(apnlsizer);
	wxStaticText* txt = new wxStaticText(npnl, wxID_ANY, "  Solid-border");
	wxSlider* sliderSolid = new wxSlider(npnl, wxID_ANY, 255, 0, 255);
	apnlsizer->Add(pg, wxSizerFlags().Expand().Proportion(16));
	apnlsizer->Add(txt, wxSizerFlags().Expand().Proportion(2));
	apnlsizer->Add(sliderSolid, wxSizerFlags().Expand().Proportion(2));

	this->_sliderSolid = sliderSolid;
	pnlsizer->Add(npnl, wxSizerFlags().Expand().Proportion(1));
	
	this->_sliderImg->Bind(wxEVT_SCROLL_CHANGED, &WindowImage::On_Scroll_Img, this);
	this->_sliderImg->Bind(wxEVT_SCROLL_THUMBTRACK, &WindowImage::On_Scroll_Img, this);
	sliderSolid->Bind(wxEVT_SCROLL_CHANGED, &WindowImage::On_Scroll_Cut, this);
	sliderSolid->Bind(wxEVT_SCROLL_THUMBTRACK, &WindowImage::On_Scroll_Cut, this);

	pg->Bind(wxEVT_PG_CHANGED, &WindowImage::Change_Grid_Value, this);
	pg->Bind(wxEVT_PG_CHANGING, &WindowImage::Change_Grid_Value, this);
	this->_sliderSolid->SetValue(1);
	int s_width;
	int s_height;
	WindowMain* wm = static_cast<WindowMain*>(parent);
	int diff = wm->Ribbon_Size() + wm->Bar_Size();
	wxDisplaySize(&s_width, &s_height);
	s_width = s_width / 3;
	s_height = s_height - diff;

	this->_pnlSliderFile = spnl;
	this->Thaw();
	this->_mgr->AddPane(spnl, wxAuiPaneInfo().Bottom().Caption("3D Texture processing").BestSize(s_width, 36 * s_height / 100));
	this->_mgr->AddPane(this->_leftImg, wxAuiPaneInfo().Center().Caption("Original image").BestSize(s_width, 64 * s_height / 100));
	this->_mgr->AddPane(this->_rightImg, wxAuiPaneInfo().Center().Caption("Binary image").BestSize(s_width, 64 * s_height / 100));
	this->_mgr->GetPane(this->_leftImg).Hide();
	this->_ribbonPanel = 0;
}

void WindowImage::Hide_Button_Panel()
{
	if (this->_ribbonPanel)
	{
		this->_ribbonPanel->Refresh();
	}
}

void WindowImage::Show_Button_Panel()
{
	if (this->_ribbonPanel)
	{
		this->_ribbonPanel->Refresh();
	}
}


void WindowImage::Update_Layout()
{
	this->_mgr->Update();
}

void WindowImage::Add_Button_Tools(wxRibbonPage* ribbonPage, wxMenuBar* menubar)
{
	wxMenu* menu = new wxMenu();
	wxRibbonPanel* ribbonPanel = new wxRibbonPanel(ribbonPage, wxID_ANY, wxT("Image processing tools"), wxNullBitmap,
		wxDefaultPosition, wxDefaultSize, wxRIBBON_PANEL_NO_AUTO_MINIMISE);
	this->_ribbonPanel = ribbonPanel;
	wxRibbonToolBar* btnBar = new wxRibbonToolBar(ribbonPanel);	
	this->_btnBar = btnBar;
	wxImage img;
	wxBitmap bmp;
	int bmpsize = 32;
	img.LoadFile("icons/openrocks.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);	
	btnBar->AddTool(wxID_ADD, bmp, "Load image list");
	menu->Append(wxID_ADD, "Load image list")->SetBitmap(bmp);
	img.LoadFile("icons/green_paint.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddSeparator();
	menu->AppendSeparator();
	btnBar->AddTool(wxID_FILE2,  bmp, "Paint pore space, as it has been selected");
	menu->Append(wxID_FILE2, "Paint pore space, as it has been selected")->SetBitmap(bmp);
	img.LoadFile("icons/binary.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_EXECUTE, bmp, "Transform all images to a 3D binary image");
	menu->Append(wxID_EXECUTE, "Transform all images to a 3D binary image")->SetBitmap(bmp);
	img.LoadFile("icons/denoise.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddSeparator();
	menu->AppendSeparator();
	btnBar->AddTool(wxID_BOLD,  bmp, "Apply an erosion and a dilation to denoise the image");
	menu->Append(wxID_BOLD, "Apply an erosion and a dilation to denoise the image")->SetBitmap(bmp);

	img.LoadFile("icons/balloons-gray.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_FILE1,  bmp, "Apply morphological opening to estimate pore size distribution");
	menu->Append(wxID_FILE1, "Apply morphological opening to estimate pore size distribution")->SetBitmap(bmp);
	img.LoadFile("icons/flip.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddSeparator();
	menu->AppendSeparator();
	btnBar->AddTool(wxID_BACKWARD, bmp, "Flip image colors, transforming solid phase into pore phase");
	menu->Append(wxID_BACKWARD, "Flip image colors, transforming solid phase into pore phase")->SetBitmap(bmp);
	img.LoadFile("icons/watershed.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddTool(wxID_FILE3,  bmp, "Cluster pores by overlapping maximal spheres");
	menu->Append(wxID_FILE3, "Cluster pores by overlapping maximal spheres")->SetBitmap(bmp);
	img.LoadFile("icons/brick-add.png");
	img.Rescale(bmpsize, bmpsize);
	bmp = wxBitmap(img);
	btnBar->AddSeparator();
	menu->AppendSeparator();
	btnBar->AddTool(wxID_SAVE,  bmp, "Save binary image (with its transformations) to a file");
	menu->Append(wxID_SAVE, "Save binary image (with its transformations) to a file")->SetBitmap(bmp);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowImage::Binarize_All_Samples, this, wxID_EXECUTE);
	menu->Bind(wxEVT_MENU, &WindowImage::Binarize_All_Samples, this, wxID_EXECUTE);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowImage::On_Load, this, wxID_ADD);
	menu->Bind(wxEVT_MENU, &WindowImage::On_Load, this, wxID_ADD);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowImage::Morpho_Open, this, wxID_FILE1);
	menu->Bind(wxEVT_MENU, &WindowImage::Morpho_Open, this, wxID_FILE1);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowImage::Paint_Pore, this, wxID_FILE2);
	menu->Bind(wxEVT_MENU, &WindowImage::Paint_Pore, this, wxID_FILE2);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowImage::Save_Image, this, wxID_SAVE);
	menu->Bind(wxEVT_MENU, &WindowImage::Save_Image, this, wxID_SAVE);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowImage::Flip, this, wxID_BACKWARD);
	menu->Bind(wxEVT_MENU, &WindowImage::Flip, this, wxID_BACKWARD);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowImage::Cluster_Pores, this, wxID_FILE3);
	menu->Bind(wxEVT_MENU, &WindowImage::Cluster_Pores, this, wxID_FILE3);
	btnBar->Bind(wxEVT_RIBBONTOOLBAR_CLICKED, &WindowImage::Denoise, this, wxID_BOLD);
	menu->Bind(wxEVT_MENU, &WindowImage::Denoise, this, wxID_BOLD);
	menubar->Append(menu, "Image processing tools");
}

void WindowImage::Cluster_Pores(wxCommandEvent& evt)
{
	if (this->_binImg.Opened())
	{
		this->_windowMain->Create_Cluster_PSD(this->_binImg);
		int depth = this->_sliderImg->GetValue();
		this->Show_Images(depth);
	}
	else
	{
		wxMessageDialog mgdlg((wxWindow*)this, wxString("There is no image to binarize or it hast not been opened"), wxString("No image to watershed"), wxOK);
		mgdlg.ShowModal();
	}
}

void WindowImage::Flip(wxCommandEvent& evt)
{
	if (this->_binImg.Depth() > 0)
	{
		this->_binImg = this->_binImg.Invert();
		int depth = this->_sliderImg->GetValue();
		this->Show_Images(depth);
	}
	else
	{
		wxMessageDialog mgdlg((wxWindow*)this, wxString("There is no image to invert"), wxString("No image"), wxOK);
		mgdlg.ShowModal();
	}
}

WindowImage::~WindowImage()
{
	this->_mgr->UnInit();
}


void WindowImage::Save_Image(wxCommandEvent& evt)
{
	wxCommandEvent evt_main;
	this->_windowMain->Save_Image(evt_main);
}

void WindowImage::Change_Grid_Value(wxPropertyGridEvent& evt)
{
	wxPGProperty* pp = evt.GetProperty();
	if (pp->GetName() == wxString("LMIN"))
	{
		int solid_border = pp->GetValue().GetInteger();
		this->_sliderSolid->SetValue(solid_border);
		int depth = this->_sliderImg->GetValue();
		this->Show_Images(depth);
	}
}

void WindowImage::Paint_Pore(wxCommandEvent& evt)
{
	wxImage img;
	wxBitmap bmp;
	if (this->_paintPores)
	{
		img.LoadFile("icons/gray_paint.png");
		img.Rescale(32, 32);
		bmp = wxBitmap(img);

		this->_paintPores = false;
		this->_btnBar->SetToolNormalBitmap(wxID_FILE2,bmp);
	}
	else
	{
		img.LoadFile("icons/green_paint.png");
		img.Rescale(32, 32);
		bmp = wxBitmap(img);

		this->_paintPores = true;
		this->_btnBar->SetToolNormalBitmap(wxID_FILE2,bmp);
	}
	int depth = this->_sliderImg->GetValue();
	this->Show_Images(depth);
	this->_btnBar->Refresh();
	this->_btnBar->Update();
}

void WindowImage::On_Scroll_Cut(wxScrollEvent& evt)
{
	this->Show_Images(this->_sliderImg->GetValue());
	this->_propertyGrid->GetProperty("LMIN")->SetValue(evt.GetPosition());
	int depth = this->_sliderImg->GetValue();
	this->Show_Images(depth);
}

void WindowImage::On_Scroll_Img(wxScrollEvent& evt)
{
	int depth = evt.GetPosition();
	this->Show_Images(depth);
}

void WindowImage::Binarize(wxImage& img)
{
	int border = this->_propertyGrid->GetProperty("LMIN")->GetValue().GetInteger();
	tbb::parallel_for(tbb::blocked_range<int>(0, (int)img.GetSize().y, SCHUNK_SIZE), [&img,border](const tbb::blocked_range<int>& b)
	{
		uchar wh = 255;
		for (int y = b.begin(); y < b.end(); ++y)
		{
			for (int x = 0; x < img.GetSize().x; ++x)
			{
				int color = (int)img.GetRed(x, y) + (int)img.GetGreen(x, y) + (int)img.GetBlue(x, y);
				color = color / 3;
				if (color < border)
				{
					img.SetRGB(x, y, 0, 0, 0);
				}
				else
				{
					img.SetRGB(x, y, wh, wh, wh);
				}
			}
		}
	});
}

void WindowImage::Paint_Pore_Space(wxImage& img)
{
	int border = this->_propertyGrid->GetProperty("LMIN")->GetValue().GetInteger();
	tbb::parallel_for(tbb::blocked_range<int>(0, (int)img.GetSize().y, SCHUNK_SIZE), [&img,border](const tbb::blocked_range<int>& b)
	{
		uchar wh = 255;
		for (int y = b.begin(); y < b.end(); ++y)
		{
			for (int x = 0; x < (int)img.GetSize().x; ++x)
			{
				int wy = 80 * y / img.GetSize().y;
				int color = (int)img.GetRed(x, y) + (int)img.GetGreen(x, y) + (int)img.GetBlue(x, y);
				color = color / 3;
				if (color < border)
				{
					img.SetRGB(x, y, 10,175+wy,10);
				}
			}
		}
	});
}

void WindowImage::Binarize_All_Samples(wxCommandEvent& evt)
{
	if ((int)this->_imgs.size() > 0)
	{
		wxMessageDialog rmvImgs(this->GetParent(),"Remove all images from memory?","Free image memory",wxYES_NO);
		int rmvImg = rmvImgs.ShowModal();
		wxGenericProgressDialog pgdlg("Executing process ..... ", "Loading image set", (int)this->_imgNames.size(), this, wxPD_APP_MODAL | wxPD_AUTO_HIDE);
		pgdlg.Show();
		wxImage img;
		img.LoadFile(this->_imgNames[0]);
		this->_binImg.Create(img.GetSize().x, img.GetSize().y, (int)this->_imgs.size());
		for (int k = 0; k < this->_imgs.size(); ++k)
		{
			wxImage img;
			img.LoadFile(this->_imgNames[k]);
			this->Binarize(img);
			this->_binImg.Add_Layer(WxImageAdapter(img), k);
			pgdlg.Update(k, wxString("Registering: ") << this->_imgNames[k]);
			pgdlg.Refresh();
		}
		if (rmvImg == wxID_YES)
		{
			this->_mgr->GetPane(this->_leftImg).Hide();
			for (uint i = 0; i < this->_imgs.size(); ++i)
			{
				delete this->_imgs[i];
			}
			this->_imgs.clear();
			this->_imgNames.clear();
		}
		pgdlg.Close();
		this->_dispImage = &this->_binImg;
		this->_mgr->GetPane(this->_rightImg).Show();
		this->_mgr->Update();
		this->Show_Images(this->_sliderImg->GetValue());
	}
	else
	{
		wxMessageDialog mgdlg((wxWindow*)this, wxString("There is no image to binarize"), wxString("No image"), wxOK);
		mgdlg.ShowModal();
	}
}

void WindowImage::Load_Binary_Image(const wxString& file_name,wxGenericProgressDialog* pgdlg)
{
	pgdlg->Pulse(wxString("Loading ") + file_name);
	this->_binImg = rw::BinaryImage();
	this->_binImg.Load_File(std::string(file_name.c_str()),&WxProgressAdapter(pgdlg));
	this->_fileText->SetLabelText(" File path: " + file_name);
	this->_mgr->GetPane(this->_pnlSliderFile).Show();
	this->_sliderImg->SetMax(this->_binImg.Depth()-1);
	this->_sliderImg->SetMin(0);
	this->_mgr->GetPane(this->_rightImg).Show();
	this->_mgr->GetPane(this->_leftImg).Hide();
	this->_mgr->Update();
	this->_dispImage = &this->_binImg;
	this->Show_Images(this->_sliderImg->GetValue());
	wxCommandEvent evt;
	this->_windowMain->Build_3D_Model(evt);
}

const rw::BinaryImage& WindowImage::Binary_Image() const
{
	return(this->_binImg);
}

void WindowImage::Show_Images(int depth)
{
	if (depth < (int)this->_imgs.size())
	{
		if (this->_imgs.size() > 0)
		{
			if (this->_leftImg->IsShown())
			{
				wxImage img = this->_imgs[depth]->Copy();
				if (this->_paintPores)
				{
					this->Paint_Pore_Space(img);
				}
				this->_leftImg->Set_Image(img);
				this->_leftImg->Refresh();
			}
			if (this->_rightImg->IsShown())
			{
				if (this->_dispImage)
				{
					if (!this->_showProcessedImages)
					{
						WxImageAdapter b_img;
						this->_dispImage->Layer(b_img,depth, rw::RGBColor(55,40,0), 
							rw::RGBColor(255, 225, 105));						
						this->_rightImg->Set_Image(b_img.Image());
						this->_rightImg->Refresh();
					}
					else
					{
						if (this->_dispImage->Opened())
						{
							WxImageAdapter b_img;							
							WxColorMap cmp;
							this->_binImg.Populate_Color_Map(cmp);
							this->_rightImg->Set_Color_Map(cmp);
							this->_dispImage->Pore_Segmentation_Layer(b_img, depth, rw::RGBColor(255, 225, 105));
							this->_rightImg->Set_Image(b_img.Image());
							this->_rightImg->Refresh();
						}
					}
				}
			}
			this->_fileText->SetLabelText(" File: " + this->_imgNames[depth]);
			this->_depthText->SetLabelText(" Texture depth: " + wxString::Format("%i", depth));
		}
	}
	else
	{
		if (this->_binImg.Depth() > 0)
		{
			this->_depthText->SetLabelText(" Texture depth: " + wxString::Format("%i", depth));
			if (this->_rightImg->IsShown())
			{
				if (this->_dispImage)
				{
					if (!this->_showProcessedImages)
					{
						WxImageAdapter b_img;
						this->_dispImage->Layer(b_img,depth, rw::RGBColor(55, 40, 0), 
							rw::RGBColor(255, 225, 105));
						this->_rightImg->Set_Image(b_img.Image());
						this->_rightImg->Refresh();
					}
					else
					{
						if (this->_dispImage->Opened())
						{
							WxImageAdapter b_img;
							this->_dispImage->Pore_Size_Distribution_Layer(b_img,depth,true,
								this->_dispImage->Build_Diameter_Color_Map(), 
								rw::RGBColor(255, 225, 105));
							WxColorMap cmp;
							this->_binImg.Populate_Color_Map(cmp);
							this->_rightImg->Set_Color_Map(cmp);
							this->_rightImg->Set_Image(b_img.Image());
							this->_rightImg->Refresh();
						}
					}
				}
			}
		}
	}
}

void WindowImage::On_Load(wxCommandEvent& evt)
{
	this->Load_Image_List();
}

void WindowImage::Morpho_Open(wxCommandEvent& evt)
{
	if (this->_binImg.Depth() > 0)
	{
		wxImage img;
		wxBitmap bmp;
		if (!this->_showProcessedImages)
		{
			img.LoadFile("icons/balloons.png");
			img.Rescale(32, 32);
			bmp = wxBitmap(img);
			this->_showProcessedImages = true;
			this->_dispImage = &this->_binImg;
		}
		else
		{
			img.LoadFile("icons/balloons-gray.png");
			img.Rescale(32, 32);
			bmp = wxBitmap(img);
			this->_showProcessedImages = false;
		}
		if (!this->_binImg.Opened())
		{
			wxProgressDialog* pgdlg
				= new wxProgressDialog("Opening image. This may take several minutes", "Opening ....");
			this->_pgdlg = pgdlg;
			pgdlg->Show();

			this->_gpuFinished = false;
			std::thread t(&WindowImage::Open_Spheres, this);
			t.detach();
		}
		else
		{
			wxMessageDialog askdlg(this, "Execute morphology opening again?", "Image already opened", wxYES | wxNO);
			if (askdlg.ShowModal() == wxID_YES)
			{
				askdlg.Close();
				Sleep(250);
				wxProgressDialog* pgdlg
					= new wxProgressDialog("Opening image. This may take several minutes", "Opening ....");
				this->_pgdlg = pgdlg;
				pgdlg->Show();

				this->_gpuFinished = false;
				std::thread t(&WindowImage::Open_Spheres, this);
				t.detach();
			}
			else
			{
				this->_gpuFinished = true;
			}
		}
		while (!this->_gpuFinished)
		{
			Sleep(1000);
			if (this->_pgdlg)
			{
				this->_pgdlg->Refresh();
			}
		}
		//this->_windowMain->Save_Sphere_PSD(this->_binImg);
		this->_btnBar->SetToolNormalBitmap(wxID_FILE1,bmp);
		this->Show_Images(this->_sliderImg->GetValue());
		this->_btnBar->Refresh();
		this->_btnBar->Update();
		if (this->_pgdlg)
		{
			delete this->_pgdlg;
		}
		this->_pgdlg = 0;
	}
	else
	{
		wxMessageDialog mgdlg((wxWindow*)this,wxString("There is no image to open"),wxString("No image"),wxOK);
		mgdlg.ShowModal();
	}
}

void WindowImage::Denoise(wxCommandEvent& evt)
{
	if (this->_binImg.Depth() > 0)
	{
		wxArrayString arrstr;
		for (int k = 1; k <= 7; ++k)
		{
			arrstr.Add(wxString("Diameter ") << 2*k+1 << " vx");
		}
		wxSingleChoiceDialog schdlg(0,wxString("Select the structuring element radius"), wxString("Structuring element size"), arrstr,(void**)0, wxOK | wxCANCEL);
		if (schdlg.ShowModal() == wxID_OK)
		{
			int diam = schdlg.GetSelection();
			++diam;
			diam = 2 * diam + 1;
			wxGenericProgressDialog prgdlg("Denoising image", "Denoising image by applying erosion and dilation operators");
			prgdlg.Show();
			prgdlg.Pulse("Denoising image by applying erosion and dilation operators");
			this->_binImg.Denoise(diam);
			this->Show_Images(this->_sliderImg->GetValue());
		}
	}
	else
	{
		wxMessageDialog mgdlg((wxWindow*)this, wxString("There is no image to denoise"), wxString("No image"), wxOK);
		mgdlg.ShowModal();
	}
}


void WindowImage::Open_Spheres()
{
	this->_binImg.Open(&WxProgressAdapter(this->_pgdlg));
	this->_gpuFinished = true;
}

void WindowImage::Load_Image_List()
{
	wxFileDialog open(this, _("Open Image file"), "", "",
		"Image files (*.png;*.gif;*.bmp;*.jpg;*.tif;*.tiff)|*.png;*.gif;*.bmp;*.jpg;*.tif;*.tiff", wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);
	if (open.ShowModal() == wxID_OK)
	{
		this->_imgs.clear();
		this->_imgNames.clear();
		wxArrayString arr;
		open.GetPaths(arr);
		open.Close();		
		wxGenericProgressDialog pgdlg("Executing process ..... ", "Loading image set", arr.GetCount()+1);
		pgdlg.Show();
		pgdlg.Update(1, wxString("Clearing"));
		if (this->_imgs.size() > 0)
		{
			for (int i = 0; i < this->_imgs.size(); ++i)
			{
				delete this->_imgs[i];
			}
			this->_imgs.clear();
			this->_imgNames.Clear();
		}
		for (int i = 0; i < arr.size(); ++i)
		{
			wxImage* img = new wxImage();
			img->LoadFile(arr[i]);
			this->_imgs.push_back(img);
			this->_imgNames.Add(arr[i]);
			pgdlg.Update(i+1,wxString("Registering: ") + arr[i]);			
		}
		this->_sliderImg->SetRange(0, (int)arr.size()-1);		
		this->_mgr->GetPane(this->_leftImg).Show();
		this->_mgr->GetPane(this->_rightImg).Hide();
		this->_mgr->Update();		
		this->Show_Images(0);
		pgdlg.Close();
	}
}

