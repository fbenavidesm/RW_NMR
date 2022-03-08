#include <algorithm>
#include <istream>
#include <fstream>
#include <math.h>
#include "dlg_import.h"
#include "wx/progdlg.h"
#include "math_la/txt/separator.h"
#include "math_la/txt/converter.h"


DlgImport::DlgImport(wxWindow* parent, const wxString& Title) : wxDialog(parent, 0, Title)
{
	this->_samples = 16;
	this->_timeUnits = 2;
	this->SetSize(1000, 600);
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	wxGridSizer* ssizer = new wxGridSizer(2, 6, 2, 2);
	wxBoxSizer* asizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* bsizer = new wxBoxSizer(wxHORIZONTAL);
	wxGrid* grid = new wxGrid(this, 0);
	this->_grid = grid;
	this->_grid->SetTable(new wxGridStringTable(1, 1));
	wxButton* okbtn = new wxButton(this, wxID_OK, "OK");
	wxButton* cnbtn = new wxButton(this, wxID_CANCEL, "Cancel");

	wxImage image;
	wxBitmap bmp;
	image = wxImage(wxT("icons//check.png"), wxBITMAP_TYPE_PNG);
	image.Rescale(32, 32, wxImageResizeQuality::wxIMAGE_QUALITY_HIGH);
	bmp = wxBitmap(image);
	okbtn->SetBitmap(bmp);

	image = wxImage(wxT("icons//close.png"), wxBITMAP_TYPE_PNG);
	image.Rescale(32, 32, wxImageResizeQuality::wxIMAGE_QUALITY_HIGH);
	bmp = wxBitmap(image);
	cnbtn->SetBitmap(bmp);
	bsizer->Add(okbtn, wxSizerFlags().Center());
	bsizer->Add(cnbtn, wxSizerFlags().Center());
	wxPanel* pnl = new wxPanel(this);
	asizer->Add(pnl, wxSizerFlags().FixedMinSize().Expand().Proportion(1));
	asizer->Add(grid, wxSizerFlags().FixedMinSize().Expand().Proportion(10));
	this->_plotterDecay = new WxPlotter(this);
	asizer->Add(this->_plotterDecay, wxSizerFlags().Expand().Proportion(20));
	sizer->Add(ssizer, wxSizerFlags().Proportion(1).Center());
	sizer->Add(asizer, wxSizerFlags().Proportion(4).Expand());
	sizer->Add(bsizer, wxSizerFlags().Center().Proportion(1));

	wxStaticText* text = 0;
	text = new wxStaticText(this, 0, "Time column", wxDefaultPosition, wxSize(150,20));
	ssizer->Add(text,wxSizerFlags().Center());
	text = new wxStaticText(this, 0, "Magnetization column", wxDefaultPosition, wxSize(150, 25));
	ssizer->Add(text, wxSizerFlags().Center());
	text = new wxStaticText(this, 0, "Imaginary column", wxDefaultPosition, wxSize(150,25));
	ssizer->Add(text, wxSizerFlags().Center());
	text = new wxStaticText(this, 0, "Real column", wxDefaultPosition, wxSize(150,25));
	ssizer->Add(text, wxSizerFlags().Center());
	text = new wxStaticText(this, 0, "Samples (Ortho)", wxDefaultPosition, wxSize(150, 25));
	ssizer->Add(text, wxSizerFlags().Center());
	text = new wxStaticText(this, 0, "Rotate", wxDefaultPosition, wxSize(150, 25));
	ssizer->Add(text, wxSizerFlags().Center());


	wxListBox* timeCombo = new wxListBox(this,0, wxDefaultPosition, wxSize(150,25));
	wxListBox* decayCombo = new wxListBox(this, 0, wxDefaultPosition, wxSize(150,25));
	wxListBox* imgCombo = new wxListBox(this, 0, wxDefaultPosition,wxSize(150,25));
	wxListBox* realCombo = new wxListBox(this, 0, wxDefaultPosition,wxSize(150,25));
	wxListBox* orthoCombo = new wxListBox(this, 0, wxDefaultPosition, wxSize(150, 25));
	wxButton* applyButton = new wxButton(this, wxID_APPLY, "Apply rotation",wxDefaultPosition,wxSize(100,25));
	image.LoadFile("icons/check.png");
	image.Rescale(16,16);
	bmp = wxBitmap(image);
	applyButton->SetBitmap(bmp);
	this->_timeCombo = timeCombo;
	this->_decayCombo = decayCombo;
	this->_imgCombo = imgCombo;
	this->_realCombo = realCombo;
	this->_orthoCombo = orthoCombo;
	ssizer->Add(timeCombo);
	ssizer->Add(decayCombo);
	ssizer->Add(imgCombo);
	ssizer->Add(realCombo);
	ssizer->Add(orthoCombo);
	ssizer->Add(applyButton);
	this->_orthoCombo->Append("16 samples");
	this->_orthoCombo->Append("32 samples");
	this->_orthoCombo->Append("64 samples");
	this->_orthoCombo->Select(0);
	applyButton->Bind(wxEVT_BUTTON, &DlgImport::On_Rotate_Button, this);
	this->_timeCombo->Bind(wxEVT_LISTBOX, &DlgImport::OnChoose, this);
	this->_decayCombo->Bind(wxEVT_LISTBOX, &DlgImport::OnChoose, this);
	this->SetSizer(sizer);
}

void DlgImport::OnChoose(wxCommandEvent& evt)
{
	int time = this->_timeCombo->GetSelection();
	int decay = this->_decayCombo->GetSelection();
	this->Set_Time_Mgn(time, decay);
}

void DlgImport::On_Rotate_Button(wxCommandEvent& evt)
{
	this->Rotate(this->_timeCombo->GetSelection(), this->_imgCombo->GetSelection(),
		this->_realCombo->GetSelection());
	this->_decayCombo->Refresh();
}

void DlgImport::Rotate(int time, int img, int real)
{
	wxGenericProgressDialog pgdlg("Processing", "Processing data", 6);
	pgdlg.Show();
	this->_plotterDecay->Erase_All_Curves();
	this->_plotterDecay->Add_Curves(2);
	this->_plotterDecay->Set_Curve_Color(wxColor(255, 155, 155), 0);
	this->_plotterDecay->Set_Curve_Color(wxColor(155, 0, 0),1);

	pgdlg.Update(1, "Getting rotation angle");
	int mm = 16 * pow(2, this->_orthoCombo->GetSelection());
	float avg = 0;
	for (int k = 0; k < mm; ++k)
	{
		float v_r = this->_data[k][real];
		float v_i = this->_data[k][img];
		float v_n = sqrt(v_r*v_r + v_i*v_i);
		float angle = 0;
		if (v_n > 0)
		{
			angle = asin(v_i / v_n);
			if ((v_r < 0)&&(v_i > 0))
			{
				angle = M_PI - angle;
			}
			else if ((v_r < 0) && (v_i < 0))
			{
				angle = M_PI + angle;
			}
		}
		avg = avg + angle;
	}
	avg = avg / (float)mm;
	this->_mgn.clear();
	this->_time.clear();
	this->_noise.clear();
	float max_v = 0;
	pgdlg.Update(3, "Applying rotation angle");
	for (int k = 0; k < this->_data.size(); ++k)
	{
		float v_r = this->_data[k][real];
		float v_i = this->_data[k][img];
		float v_n = sqrt(v_r*v_r + v_i*v_i);
		float mgn = v_n*cos(avg);
		this->_mgn.push_back(mgn);
		float nn = v_n*sin(avg);
		this->_noise.push_back(nn);
		if (mgn > max_v)
		{
			max_v = mgn;
		}
		this->_time.push_back(this->_data[k][time]);
	}

	pgdlg.Update(5, "Updating");
	this->_plotterDecay->Set_Interval_Limits(0.000001f, 0.5f, 0.0f, 1.0f);
	for (int k = 0; k < this->_data.size(); ++k)
	{
		this->_mgn[k] = this->_mgn[k]/ max_v;
		this->_noise[k] = this->_noise[k] / max_v;
		scalar time_v = this->_time[k];
		scalar mgn_v = this->_mgn[k];
		wxString sttime = wxString::FromDouble(time_v, 4);
		wxString stmagn = wxString::FromDouble(mgn_v, 4);
		this->_grid->SetCellValue(k,this->_header.size(),sttime);
		this->_grid->SetCellValue(k, this->_header.size() + 1, stmagn);
		this->_plotterDecay->Add_Curve_Point(1, time_v, mgn_v);
		this->_plotterDecay->Add_Curve_Point(0, time_v, this->_noise[k]);
	}
	this->_plotterDecay->Refresh();
}

bool DlgImport::Select_File(wxString& filename)
{
	bool r = false;
	wxFileDialog
		openFileDialog(this, _("Open decay file"), "", "",
			"Text files (*.*)|*.*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	if (openFileDialog.ShowModal() == wxID_OK)
	{
		r = true;
		filename = openFileDialog.GetPath();
		openFileDialog.Close();
		if (r)
		{
			this->Analyze_File(filename);
		}
	}
	return(r);
}

void DlgImport::Find_Header(const wxString& filename)
{
	std::string test_sep;
	test_sep = "; \t,";	
	math_la::math_lac::txt::Separator sep;
	sep.Set_Datachars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVXYZ()!@#$%^&*<>[]{}");
	int colsize = 0;
	bool header = false;
	for (int k = 0; k < test_sep.size(); ++k)
	{
		std::string ss = "<>";
		ss = ss + test_sep[k];
		sep.Set_Separators(ss);
		std::ifstream file;
		file.open((std::string)filename.c_str());
		std::string  line;
		int i = 0;
		while ((!header)&&(i <= 15))
		{
			std::getline(file, line);
			std::vector<std::string> headers = sep.Separate(line);
			if (headers.size() >= 2)
			{
				if (headers.size() > colsize)
				{
					colsize = headers.size();
					this->_separators = ss;
				}
				int avg = 0;
				for (int k = 0; k < headers.size(); ++k)
				{
					avg = avg + headers[k].length();
				}
				avg = avg / headers.size();
				if (avg > 1)
				{
					header = true;
					this->_idc = i+1;
					k = test_sep.size();
					this->_separators = ss;
				}
			}
			++i;
			if ((header) && (i < 15))
			{
				for (int k = 0; k < headers.size(); ++k)
				{
					this->_header.push_back(wxString(headers[k].c_str()));
				}
			}
		}
	}
	if (!header)
	{
		for (int i = 0; i < colsize; ++i)
		{
			this->_header.push_back("_"+math_la::math_lac::txt::Converter::Convert_Int(i)+"-");
		}
	}
}

void DlgImport::Collect_Data(const wxString& filename)
{
	this->_colSize = this->_header.size();
	wxGenericProgressDialog dlg("Loading decay file ...", "Loading decay file ...");
	dlg.Center();
	dlg.Show();
	math_la::math_lac::txt::Separator sep;
	sep.Set_Datachars("0123456789Ee.,-+");
	sep.Set_Separators(this->_separators);
	std::ifstream file;
	file.open((std::string)filename.c_str());
	std::string  line;
	int ii = 0;
	dlg.Pulse(wxString("Loading file ... "));
	while (!file.eof())
	{
		for (int k = 0; k < this->_idc; ++k)
		{
			std::getline(file, line);
		}
		std::getline(file, line);
		std::vector<std::string> data = sep.Separate(line);
		vector<float> line; 
		if (data.size() > this->_colSize)
		{
			this->_colSize = data.size();
		}
		for (int i = 0; i < data.size(); ++i)
		{
			float v = math_la::math_lac::txt::Converter::Convert_To_Scalar(data[i]);
			line.push_back(v);			
		}
		if (line.size() > 0)
		{
			this->_data.push_back(line);
		}
		++ii;
	}
}

void DlgImport::Analyze_File(const wxString& filename)
{
	this->Find_Header(filename);
	this->Collect_Data(filename);
	this->_grid->Refresh();
	this->PopulateUI();
}

void DlgImport::Pick_Time_Mgn(int& time, int& magn)
{
	time = 0;
	magn = 1;
	for (int k = 0; k < this->_header.size(); ++k)
	{
		bool ltime = true;
		bool lmagn = true;
		for (int i = 1; i < 5; ++i)
		{
			if (this->_data[5*(i-1)][k] < this->_data[5*i][k])
			{
				lmagn = false;
			}
			if (this->_data[i-1][k] > this->_data[i][k])
			{
				ltime = false;
			}
			if (this->_data[i][k] < 0)
			{
				ltime = false;
			}
		}
		if (ltime)
		{
			time = k;
		}
		if (lmagn)
		{
			magn = k;
		}
	}
}

void DlgImport::PopulateUI()
{
	int rows = this->_data.size();
	this->_grid->AppendCols((int)this->_header.size()+1);

	this->_grid->AppendRows(rows-1);
	this->_grid->HideRowLabels();	
	this->_grid->SetColLabelAlignment(wxALIGN_CENTER,wxALIGN_CENTER);
	for (int k = 0; k < this->_header.size(); ++k)
	{
		this->_grid->SetColLabelValue(k, this->_header[k]);
		this->_realCombo->AppendString(this->_header[k]);
		this->_imgCombo->AppendString(this->_header[k]);
		this->_decayCombo->AppendString(this->_header[k]);
		this->_timeCombo->AppendString(this->_header[k]);
	}
	this->_grid->SetColLabelValue(this->_header.size(), "TD.");
	this->_grid->SetColLabelValue(this->_header.size() + 1, "MAGN.");
	wxGridCellAttr* a = new wxGridCellAttr();
	a->SetTextColour(wxColor(255, 0, 0));
	wxFont font = this->_grid->GetFont();
	font.MakeBold();
	a->SetFont(font);
	this->_grid->SetColAttr(this->_header.size(), a);
	this->_grid->SetColAttr(this->_header.size()+1, a);
	for (int i = 0; i < rows; ++i)
	{
		vector<float>& line = this->_data[i];
		for (int k = 0; k < line.size(); ++k)
		{
			wxString v = wxString::FromDouble(line[k], 4);
			this->_grid->SetCellValue(i,k,v);
		}
	}
	int time = 0;
	int mgn = 0;
	this->Pick_Time_Mgn(time,mgn);
	this->_timeCombo->Select(time);
	this->_decayCombo->Select(mgn);
	this->Set_Time_Mgn(time, mgn);
	this->_realCombo->Select(0);
	this->_imgCombo->Select(0);
}

void DlgImport::Set_Time_Mgn(int time, int magn)
{
	this->_plotterDecay->Erase_All_Curves();
	this->_plotterDecay->Add_Curves(1);
	this->_plotterDecay->Set_Curve_Color(wxColor(155, 0, 0), 0);
	this->_time.clear();
	this->_mgn.clear();
	this->_time.reserve(this->_data.size());
	this->_mgn.reserve(this->_data.size());
	int mm = std::min(100, (int)this->_data.size());
	float maxv = 0;
	for (int k = 0; k < mm; ++k)
	{
		if (this->_data[k][magn] > maxv)
		{
			maxv = this->_data[k][magn];
		}
	}
	int time_reg = pow(10, 3 * (2-this->_timeUnits));
	bool reject = false;
	for (int k = 0; k < this->_data.size(); ++k)
	{
		float mgn_v = this->_data[k][magn];
		float time_v = this->_data[k][time];
		mgn_v = mgn_v / maxv;
		time_v = time_v / time_reg;
		this->_time.push_back(time_v);
		this->_mgn.push_back(mgn_v);
		wxString sttime = wxString::FromDouble(time_v, 4);
		wxString stmagn = wxString::FromDouble(mgn_v, 4);
		this->_grid->SetCellValue(k, this->_header.size(), sttime);
		this->_grid->SetCellValue(k, this->_header.size()+1, stmagn);
		this->_plotterDecay->Add_Curve_Point(0, time_v, mgn_v);
		if ((mgn_v < 0.00001)&&(!reject))
		{
			reject = true;
			this->_idReject = k;			
		}
		if (reject)
		{
			wxColor green = wxColor(150, 255, 150);
			this->_grid->SetCellTextColour(k,this->_header.size(), green);
			this->_grid->SetCellTextColour(k,this->_header.size()+1,green);
		}
	}
	this->_plotterDecay->Refresh();
}

void DlgImport::Fill_Sim(rw::PlugPersistent& sim)
{
	this->_mgn = vector<scalar>(this->_mgn.begin(), this->_mgn.begin() + this->_idReject+1);
	this->_time = vector<scalar>(this->_time.begin(), this->_time.begin() + this->_idReject + 1);
	sim.Replace_Decay(this->_time, this->_mgn);
}

DlgImport::~DlgImport()
{

}
