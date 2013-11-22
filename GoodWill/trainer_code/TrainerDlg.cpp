
// TrainerDlg.cpp : implementation file
//

#include "afxdialogex.h"
#include "stdafx.h"
#include "station.h"
#include "commonut.h"
#include "CvvImage.h"
#include "CommonDef.h"
#include "commonFunctions.h"
#include <fstream>
#include "Trainer.h"
#include "TrainerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTrainerDlg dialog
CTrainerDlg::CTrainerDlg(CWnd* pParent /*=NULL*/)
: CDialogEx(CTrainerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pVehiclePic = NULL;
	m_pVehicleSeg = NULL;

	m_Vehtype = SMALL;
	m_VehtypeRadio = 0;

	m_VehIndex = -1;
}

void CTrainerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LOAD,        m_LoadBtn);
	DDX_Control(pDX, IDC_RESET,       m_ResetBtn);
	DDX_Control(pDX, IDC_VEHICLE_IDX, m_VehicleIdxEdt);
	DDX_Control(pDX, IDC_GOTO,        m_GotoBtn);
	DDX_Control(pDX, IDC_DECISION,    m_DecisionEdt);
	DDX_Control(pDX, IDC_VEHICLE_PIC, m_VehiclePic);
	DDX_Control(pDX, IDC_VEHICLE_SEG, m_VehicleSeg);
	DDX_Control(pDX, IDC_BACKWARD,    m_BackwardBtn);
	DDX_Control(pDX, IDC_FORWARD,     m_ForwardBtn);

	DDX_Radio(pDX, IDC_VEHTYPE_SMALL, m_VehtypeRadio);
}

BEGIN_MESSAGE_MAP(CTrainerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_BN_CLICKED(IDC_BROWSE_REPORT, &CTrainerDlg::OnBnClickedBrowseReport)
	ON_BN_CLICKED(IDC_LOAD,  &CTrainerDlg::OnBnClickedLoad)
	ON_BN_CLICKED(IDC_RESET, &CTrainerDlg::OnBnClickedReset)
	ON_BN_CLICKED(IDC_GOTO, &CTrainerDlg::OnBnClickedGoto)
	ON_BN_CLICKED(IDC_VEHTYPE_SMALL, &CTrainerDlg::OnBnClickedVehtypeSmall)
	ON_BN_CLICKED(IDC_VEHTYPE_MEDIUM, &CTrainerDlg::OnBnClickedVehtypeMedium)
	ON_BN_CLICKED(IDC_VEHTYPE_LARGE, &CTrainerDlg::OnBnClickedVehtypeLarge)
	ON_BN_CLICKED(IDC_VEHTYPE_DISGARD, &CTrainerDlg::OnBnClickedVehtypeDisgard)
	ON_BN_CLICKED(IDC_FORWARD, &CTrainerDlg::OnBnClickedForward)
	ON_BN_CLICKED(IDC_BACKWARD, &CTrainerDlg::OnBnClickedBackward)
	ON_BN_CLICKED(IDC_SAVE, &CTrainerDlg::OnBnClickedSave)
	ON_BN_CLICKED(IDC_TRAINING, &CTrainerDlg::OnBnClickedTraining)
END_MESSAGE_MAP()


// CTrainerDlg message handlers

BOOL CTrainerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	SetDlgItemText(IDC_REPORT_PATH, L"YYYY-MM-DD_CITY_STATION.txt");

	CRect rVehiclePic;
	m_VehiclePic.GetClientRect(&rVehiclePic);
	m_pVehiclePic = cvCreateImage(cvSize(rVehiclePic.Width(), rVehiclePic.Height()), IPL_DEPTH_8U, 3);
	cvZero(m_pVehiclePic);

	CRect rVehicleSeg;
	m_VehicleSeg.GetClientRect(&rVehicleSeg);
	m_pVehicleSeg = cvCreateImage(cvSize(rVehicleSeg.Width(), rVehicleSeg.Height()), IPL_DEPTH_8U, 3);
	cvZero(m_pVehicleSeg);

	m_Vehicles.clear();

	// read meta.dat
	if (loadMetaFile()) {
		showData(m_Vehicles[m_VehIndex]);
		SetVehicleIndex(m_VehIndex);
		if (m_Vehicles[m_VehIndex].type_gt == -1) {
			SetVehicleType(m_Vehicles[m_VehIndex].vehicle.type);
			SetDecision(false);
		}
		else {
			SetVehicleType(m_Vehicles[m_VehIndex].type_gt);
			SetDecision(true);
		}

		EnableItem(IDC_RESET, TRUE);
		EnableItem(IDC_VEHICLE_IDX, TRUE);
		EnableItem(IDC_GOTO, TRUE);
		EnableItem(IDC_VEHTYPE_SMALL, TRUE);
		EnableItem(IDC_VEHTYPE_MEDIUM, TRUE);
		EnableItem(IDC_VEHTYPE_LARGE, TRUE);
		EnableItem(IDC_VEHTYPE_DISGARD, TRUE);
		EnableItem(IDC_FORWARD, !(m_VehIndex == m_Vehicles.size()-1));
		EnableItem(IDC_BACKWARD, !(m_VehIndex == 0));
		EnableItem(IDC_SAVE, TRUE);
		EnableItem(IDC_TRAINING, TRUE);
	}
	else 
	{
		EnableItem(IDC_RESET, FALSE);
		EnableItem(IDC_VEHICLE_IDX, FALSE);
		EnableItem(IDC_GOTO, FALSE);
		EnableItem(IDC_VEHTYPE_SMALL, FALSE);
		EnableItem(IDC_VEHTYPE_MEDIUM, FALSE);
		EnableItem(IDC_VEHTYPE_LARGE, FALSE);
		EnableItem(IDC_VEHTYPE_DISGARD, FALSE);
		EnableItem(IDC_FORWARD, FALSE);
		EnableItem(IDC_BACKWARD, FALSE);
		EnableItem(IDC_SAVE, FALSE);
		EnableItem(IDC_TRAINING, FALSE);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}


bool CTrainerDlg::loadMetaFile()
{
	FILE* file = NULL;
	file = fopen("meta.dat", "r");
	if (!file) return false;

	int num_Veh;
	fscanf(file, "%d\n", &num_Veh);
	for (int i = 0; i < num_Veh; i++) {
		TrainData data;

		char link[MAX_PATH] = {0};
		fscanf(file, " %d\t%d\t%d\t%s\n", &data.vehicle.no, &data.vehicle.type, &data.type_gt, link);
		data.vehicle.link = link;
		int path_size;
		fscanf(file, " %d\n", &path_size);
		for (int p = 0; p < path_size; p++) {
			CvBox2D node; 
			int surf_area;
			fscanf(file, " %f %f %d\n", &node.center.x, &node.center.y, &surf_area);
			data.vehicle.path.push_back(node);
			data.surf_areas.push_back(surf_area);
		}

		m_Vehicles.push_back(data);
	}
	fclose(file);

	m_VehIndex = num_Veh - 1;
	return true;
}


bool CTrainerDlg::loadReportFile(char* path)
{
	FILE* file = NULL;
	
	file = fopen(path, "r");
	if (!file) return false;

m_VehIndex++;
	int num_Veh = 0;
	char city[30] = {0}, station[30] = {0}, date[20];	
	
	fscanf(file, "------------------------------------------------------------------------------------------------------------\n");
	fscanf(file, " No.\tID\ttime\t\t\ttype\taccum\tlocation\treference\n");
	fscanf(file, "------------------------------------------------------------------------------------------------------------\n");


	while (true) {
		TrainData data;
		char type;
		char link[MAX_PATH] = {0};
		string date="";
		fscanf(file, " %d\t%04d\t%s %02d:%02d\t%c\t%d\t%s\t%s\n", 
			   &data.vehicle.no, &data.vehicle.id, date,
			   &data.vehicle.hour, &data.vehicle.mins, 
			   &type, &data.vehicle.accum, 
			   data.station, 
			   link);
		data.vehicle.link = link;
		if(data.vehicle.link==""){
			break;
		}else{
			num_Veh++;
		}
		strcpy(data.city, city); 

		switch (type) {
		case 'S': data.vehicle.type = SMALL; break;
		case 'T': data.vehicle.type = MEDIUM; break;
		case 'U': data.vehicle.type = LARGE; break;
		case 'M':  
		default:  data.vehicle.type = DISGARD;  break;
		}

		m_Vehicles.push_back(data);
	}
	fprintf(file, "--------------------------------------------------------------------------------------------\n");
	fclose(file);

	return true;
}


void CTrainerDlg::showData(CTrainerDlg::TrainData& data)
{
	IplImage* buffer = NULL;
	string vehicle_pic, vehicle_seg;

	// retrieve any valid images.
	vector<string> validPaths = valid_paths(data);
	for each (string path in validPaths)
	{
		Mat car = imread(path, CV_LOAD_IMAGE_GRAYSCALE);
		CvBox2D newPath;
		newPath.center = center(car);
		int surf = surface(car);
		data.vehicle.path.push_back(newPath);
		data.surf_areas.push_back(surf);
	}
	//////////////////////////////////////////////////////

	if(validPaths.size()>0)
	{
		vehicle_seg = validPaths[0];
		vehicle_pic = vehicle_seg.substr(0,vehicle_seg.length()-8)+".bmp";
	}
	else
	{
		vehicle_seg = data.vehicle.link+"/000_seg.bmp";
		vehicle_pic = data.vehicle.link+"/000.bmp";
	}
	buffer = cvLoadImage(vehicle_pic.c_str());
	if (!buffer) { cvZero(m_pVehiclePic); }
	else { cvResize(buffer, m_pVehiclePic); cvReleaseImage(&buffer); }

	buffer = cvLoadImage(vehicle_seg.c_str());
	if (!buffer) { cvZero(m_pVehicleSeg); }
	else { cvResize(buffer, m_pVehicleSeg); cvReleaseImage(&buffer); }

	CRect rVehiclePic;
	m_VehiclePic.GetWindowRect(&rVehiclePic);
	ScreenToClient(rVehiclePic);
	InvalidateRect(rVehiclePic, !(validPaths.size()>0));

	CRect rVehicleSeg;
	m_VehicleSeg.GetWindowRect(&rVehicleSeg);
	ScreenToClient(rVehicleSeg);
	InvalidateRect(rVehicleSeg, !(validPaths.size()>0));
}


void CTrainerDlg::OnDeinitDialog()
{
	if (m_pVehiclePic) cvReleaseImage(&m_pVehiclePic);
	if (m_pVehicleSeg) cvReleaseImage(&m_pVehicleSeg);
}


void CTrainerDlg::SetVehicleIndex(int idx)
{
	TCHAR sztext[10] = {0};
	_itow(idx, sztext, 10);
	SetDlgItemText(IDC_VEHICLE_IDX, sztext);
}


void CTrainerDlg::SetDecision(bool decided)
{
	SetDlgItemText(IDC_DECISION, decided ? L"decided" : L"not decided");
}


void CTrainerDlg::SetVehicleType(int type)
{
	switch (type) {
	case SMALL: m_VehtypeRadio = 0; break;
	case MEDIUM: m_VehtypeRadio = 1; break;
	case LARGE: m_VehtypeRadio = 2; break;
	case DISGARD:  m_VehtypeRadio = 3; break;
	}
	if (m_Vehtype != type) {
		m_Vehtype = type;
		UpdateData(FALSE);
	}
}


int CTrainerDlg::GetVehicleIndex()
{
	CString cstext;
	GetDlgItemText(IDC_VEHICLE_IDX, cstext);
	return _wtoi(cstext);
}


void CTrainerDlg::DrawImageByID(IplImage* pImg, UINT ID)
{
	CDC* pcDC = GetDlgItem(ID)->GetDC();
	HDC hDC = pcDC->GetSafeHdc();				
	CRect CtrlRect;

	GetDlgItem(ID)->GetClientRect(&CtrlRect);

	int tx = (int)(CtrlRect.Width() - pImg->width)/2,
		ty = (int)(CtrlRect.Height() - pImg->height)/2;
	SetRect(CtrlRect, tx, ty, tx + pImg->width, ty + pImg->height);

	CvvImage cimg;
	cimg.CopyOf(pImg);
	cimg.DrawToHDC(hDC, &CtrlRect);

	ReleaseDC(pcDC);
}


// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTrainerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
		DrawImageByID(m_pVehiclePic, IDC_VEHICLE_PIC);
		DrawImageByID(m_pVehicleSeg, IDC_VEHICLE_SEG);
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTrainerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTrainerDlg::OnBnClickedBrowseReport()
{
	CString szFilter = _T("TXT (*.txt)|*.txt");
	CFileDialog fd(TRUE, NULL, NULL, 
		           OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER, 
				   szFilter);

	if (fd.DoModal() == IDOK) {
		TCHAR path[MAX_PATH] = {0};
		_tcscpy_s(path, fd.GetPathName());
		SetDlgItemText(IDC_REPORT_PATH, path);
	}
}


void CTrainerDlg::OnBnClickedLoad()
{	
	CString cstext;
	TCHAR sztext[MAX_PATH] = {0};
	char report_path[MAX_PATH] = {0};
	GetDlgItemText(IDC_REPORT_PATH, cstext);
	_tcscpy_s(sztext, MAX_PATH, cstext);
	cmTChar2Char(sztext, report_path);

	if (!loadReportFile(report_path)) {
		AfxMessageBox(L"cannot read report file.");
		return;
	}

	showData(m_Vehicles[m_VehIndex]);
	SetVehicleIndex(m_VehIndex);
	if (m_Vehicles[m_VehIndex].type_gt == -1) {
		SetVehicleType(m_Vehicles[m_VehIndex].vehicle.type);
		SetDecision(false);
	}
	else {
		SetVehicleType(m_Vehicles[m_VehIndex].type_gt);
		SetDecision(true);
	}

	EnableItem(IDC_REPORT_PATH, FALSE);
	EnableItem(IDC_BROWSE_REPORT, FALSE);
	EnableItem(IDC_LOAD, FALSE);
	
	EnableItem(IDC_RESET, TRUE);
	EnableItem(IDC_VEHICLE_IDX, TRUE);
	EnableItem(IDC_GOTO, TRUE);
	EnableItem(IDC_VEHTYPE_SMALL, TRUE);
	EnableItem(IDC_VEHTYPE_MEDIUM, TRUE);
	EnableItem(IDC_VEHTYPE_LARGE, TRUE);
	EnableItem(IDC_VEHTYPE_DISGARD, TRUE);
	EnableItem(IDC_FORWARD, TRUE);
	EnableItem(IDC_SAVE, TRUE);
	EnableItem(IDC_TRAINING, TRUE);
}


void CTrainerDlg::OnBnClickedReset()
{
	
}


void CTrainerDlg::OnBnClickedGoto()
{
	if (m_Vehicles[m_VehIndex].type_gt != m_Vehtype)
		m_Vehicles[m_VehIndex].type_gt = m_Vehtype;

	m_VehIndex = GetVehicleIndex();
	m_VehIndex = MIN(MAX(0, m_VehIndex), m_Vehicles.size()-1);

	EnableItem(IDC_FORWARD, !(m_VehIndex == m_Vehicles.size()-1));
	EnableItem(IDC_BACKWARD, !(m_VehIndex == 0));

	showData(m_Vehicles[m_VehIndex]);
	SetVehicleIndex(m_VehIndex);
	if (m_Vehicles[m_VehIndex].type_gt == -1) {
		SetVehicleType(m_Vehicles[m_VehIndex].vehicle.type);
		SetDecision(false);
	}
	else {
		SetVehicleType(m_Vehicles[m_VehIndex].type_gt);
		SetDecision(true);
	}
}


void CTrainerDlg::OnBnClickedVehtypeSmall()
{
	m_Vehtype = SMALL;
}


void CTrainerDlg::OnBnClickedVehtypeMedium()
{
	m_Vehtype = MEDIUM;
}


void CTrainerDlg::OnBnClickedVehtypeLarge()
{
	m_Vehtype = LARGE;
}


void CTrainerDlg::OnBnClickedVehtypeDisgard()
{
	m_Vehtype = DISGARD;
}

void CTrainerDlg::OnBnClickedForward()
{
	if (m_Vehicles[m_VehIndex].type_gt != m_Vehtype)
		m_Vehicles[m_VehIndex].type_gt = m_Vehtype;

	m_VehIndex++;
	EnableItem(IDC_FORWARD, !(m_VehIndex == m_Vehicles.size()-1));
	EnableItem(IDC_BACKWARD, !(m_VehIndex == 0));

	showData(m_Vehicles[m_VehIndex]);
	SetVehicleIndex(m_VehIndex);
	if (m_Vehicles[m_VehIndex].type_gt == -1) {
		SetVehicleType(m_Vehicles[m_VehIndex].vehicle.type);
		SetDecision(false);
	}
	else { 
		SetVehicleType(m_Vehicles[m_VehIndex].type_gt);
		SetDecision(true);
	}
}


void CTrainerDlg::OnBnClickedBackward()
{
	if (m_Vehicles[m_VehIndex].type_gt != m_Vehtype)
		m_Vehicles[m_VehIndex].type_gt = m_Vehtype;

	m_VehIndex--;
	EnableItem(IDC_FORWARD, !(m_VehIndex == m_Vehicles.size()-1));
	EnableItem(IDC_BACKWARD, !(m_VehIndex == 0));

	showData(m_Vehicles[m_VehIndex]);
	SetVehicleIndex(m_VehIndex);
	if (m_Vehicles[m_VehIndex].type_gt == -1) {
		SetVehicleType(m_Vehicles[m_VehIndex].vehicle.type);
		SetDecision(false);
	}
	else {
		SetVehicleType(m_Vehicles[m_VehIndex].type_gt);
		SetDecision(true);
	}
}


void CTrainerDlg::OnBnClickedSave()
{
	// calculate surface 


	FILE* file = NULL;
	file = fopen("meta.dat", "w");

	fprintf(file, "%d\n", m_Vehicles.size());
	for (int i = 0; i < m_Vehicles.size(); i++) {
		TrainData data = m_Vehicles[i];

		fprintf(file, " %d\t%d\t%d\t%s\n", 
			    data.vehicle.no, data.vehicle.type, data.type_gt, data.vehicle.link.c_str());
		fprintf(file, " %d\n", data.vehicle.path.size());
		for (int p = 0; p < data.vehicle.path.size(); p++)
			fprintf(file, " %.3f %.3f %d\n", data.vehicle.path[p].center.x, data.vehicle.path[p].center.y, data.surf_areas[p]);
	}
	fclose(file);
}


void CTrainerDlg::OnBnClickedTraining()
{
//Mat buf;
	//int vecSize;
	//vector<int> thresh1(vecSize), thresh2(vecSize);
	Mat buf = imread(m_Vehicles[0].vehicle.link+"/000.bmp", CV_LOAD_IMAGE_GRAYSCALE); 
	int HT = 240;
	int WD = 360;
	int vecSize = HT*WD;
	vector<int> small(vecSize), medium(vecSize), large(vecSize), thresh1(vecSize), thresh2(vecSize);
	vector<int> count1(vecSize), count2(vecSize), count3(vecSize);


	// train all data into threshold map. 
	for (int i = 0; i < m_Vehicles.size(); i++) 
	{
		TrainData data = m_Vehicles[i];
		for(int j=0 ; j< data.vehicle.path.size() ;j++)
		{
			int x = data.vehicle.path.at(j).center.x;
			int y = data.vehicle.path.at(j).center.y;
			int index = y*WD + x;
			for(int k=-12 ; k<13 ; k++)
			{
				int rIndex = (k>=0) ? index + (k+2)%5 + (k+2)/5*WD -2: index + (k-2)%5 + (k-2)/5*WD +2;
				if(rIndex >= 0 && rIndex < vecSize)
				{
					if(data.type_gt == 0)
					{
						small[rIndex] += data.surf_areas[j];
						count1[rIndex]++;
					}
					if(data.type_gt == 1)
					{
						medium[rIndex] += data.surf_areas[j];
						count2[rIndex]++;
					}
					if(data.type_gt == 2)
					{
						large[rIndex] += data.surf_areas[j];
						count3[rIndex]++;
					}
				}
			}
		}
	}
	int sizeLarge = 0;
	for(int i=0 ; i<vecSize ; i++)
	{
		small[i] = (count1[i]!=0) ? small[i]/count1[i] : 0;
		medium[i] = (count2[i]!=0) ? medium[i]/count2[i] : 0;
		sizeLarge = max(sizeLarge,medium[i]);
	}

	//using the enlargment of max medium size for size large.
	sizeLarge += 10;
	for(int i=0 ; i< vecSize ; i++){
		large[i] = sizeLarge;
	}

	small = thresh_interpolate(small, WD, HT);
	medium = thresh_interpolate(medium, WD, HT);
	large = thresh_interpolate(large, WD, HT);

	for(int i=0 ; i<vecSize ; i++)
	{
		thresh1[i] = (large[i] - medium[i])*0.7 + medium[i];
		thresh2[i] = (medium[i] - small[i])*0.7 + small[i];
	}
	ofstream outfile("thres1.txt");
	ofstream outfile2("thres2.txt");
	outfile << WD << " " << HT << endl;
	outfile2 << WD << " " << HT << endl;
	for(int i=0 ; i<vecSize ; i++)
	{		
		outfile << thresh1[i] << " ";
		outfile2 << thresh2[i]<< " ";
		if(i%WD == WD-1)
		{
			outfile << endl;
			outfile2 << endl;
		}
	}
	outfile.close();
	outfile2.close();
	//need extra error handler when there's no meta file.
}

std::string CTrainerDlg::int2str_index(int index)
{
	//convert an integer to our formatted index
	int zeros = 2 - (index/10);
	char iBuf[3];
	string zerosStr;
	for(int i=0 ; i<zeros ; i++)
	{
		zerosStr += "0";
	}
	_itoa(index,iBuf,10);
	std::string iStr(iBuf, iBuf + 1);

	return zerosStr + iStr;
}

//returns the valid segmented image paths contained inside a single vehicle folder.
vector<string> CTrainerDlg::valid_paths(CTrainerDlg::TrainData& data){
	vector<string> validPaths;
	string pic_path, seg_path, vehicle_pic, vehicle_seg, strStd;
	CString csSeg;
	//Mat car;
	pic_path = data.vehicle.link;
	seg_path = data.vehicle.link;
	csSeg = data.vehicle.link.c_str();
	csSeg += L"/*_seg.bmp";

	WIN32_FIND_DATA file;
	HANDLE handle = FindFirstFile(csSeg, &file);
	//////////////////////////////////////////////////////
	if (handle !=INVALID_HANDLE_VALUE) {
		do {
			csSeg = file.cFileName;


			// Convert a TCHAR string to a LPCSTR
			CT2CA pszConvertedAnsiString(csSeg);
			// construct a std::string using the LPCSTR input
			strStd = pszConvertedAnsiString;
			vehicle_seg = data.vehicle.link + "/" + strStd;

			//// rectification
			//char command[MAX_PATH] = {0};
			//sprintf(command, "RadialUndistort.exe intrinsic_matrix.txt distortion_matrix.txt undistort_list.txt");
			//system(command);

			Mat car = imread(vehicle_seg, CV_LOAD_IMAGE_GRAYSCALE);

			//
			int side1=0;
			int side2=0;
			int side3=0;
			int side4=0;
			int side_count=0;
			for(int i=5 ; i <=10 ; i++)
			{
				for (int j=5 ; j<car.cols ; j++)
				{
					if (car.data[car.step*i + car.channels()*j + 0]==255)
					{
						side1=1;
						side_count++;
					}
					if (car.data[car.step*(car.rows-i) + car.channels()*j + 0]==255)
					{
						side2=1;
						side_count++;
					}
				}
				for (int j=5 ; j<car.rows ; j++)
				{
					if (car.data[car.step*j + car.channels()*i + 0]==255)
					{
						side3=1;
						side_count++;
					}
					if (car.data[car.step*j + car.channels()*(car.cols-i) + 0]==255)
					{
						side4=1;
						side_count++;
					}
				}
			}
			//
			if (side1+side2+side3+side4>=1 && !(isPathValid(SEA_MONROE, data.vehicle))) //&& side_count > 50
			{
			}else{
				validPaths.push_back(vehicle_seg);
			}
			//
		} while (FindNextFile(handle, &file));
	}
	return validPaths;
}

vector<int> CTrainerDlg::thresh_interpolate(vector<int> thres, int WD, int HT)
{

	
	// column direction
	int start=0;
	int end=0;
	int count=0;
	
	for (int j=0 ; j <=WD-1 ; j++)
	{
		int ini_start=10000;
		int ini_end=0;
		for(int i=0 ; i <=HT-1 ; i++)
		{
			if (thres[ j + i*WD]!=0)
			{
				count=count+1;
				if (count%2 ==1)
				{
					start=i;
					ini_start=start;
				}
				
				else 
				{
					end=i;
				
					int count1=1;
					for (int k=start+1 ; k < end ; k++)
					{
				
						thres[j+(k*WD)]= floor(double((thres[j+(end*WD)]-thres[j+(start*WD)])/(end-start)*count1))+thres[j+(start*WD)];
						count1=count1+1;
					}
					
					start=end;
					count=count+1;
					if(ini_end<end)
						ini_end=end;
				}
			}	
		}
		if (ini_start!=10000 && ini_end!=0)
		{
			int count2=1;
			for (int k=ini_start-1 ; k >=0 ; k--)
			{
				thres[j+(k*WD)]=  thres[j+(ini_start*WD)] - (thres[j+((ini_start+1)*WD)]-thres[j+(ini_start*WD)])*count2;				
				count2=count2+1;
			}
			count2=1;
			for (int k=ini_end+1 ; k < HT ; k++)
			{
				thres[j+(k*WD)]=  thres[j+(ini_end*WD)] + (thres[j+(ini_end*WD)]-thres[j+((ini_end-1)*WD)])*count2;
				count2=count2+1;
			}
		}
		start=0;
		end=0;
		count=0;
	}
	
	// row direction
	start=0;
	end=0;
	count=0;
	for (int i=0 ; i <=HT-1 ; i++)
	{
		int ini_start=10000;
		int ini_end=0;
		for(int j=0 ; j <=WD-1 ; j++)
		{
			if (thres[ j + i*WD]!=0)
			{
				count=count+1;
				if (count%2 ==1)
				{
					start=j;
					ini_start=start;
				}
				
				else 
				{
					end=j;
				
					int count1=1;
					for (int k=start+1 ; k < end ; k++)
					{
				
						thres[k+(i*WD)]= floor(double((thres[end+(i*WD)]-thres[start+(i*WD)])/(end-start)*count1))+thres[start+(i*WD)];
						count1=count1+1;
					}
					
					start=end;
					count=count+1;
					if(ini_end<end)
						ini_end=end;
				}
			}	
		}
		if (ini_start!=10000 && ini_end!=0)
		{
			int count2=1;
			for (int k=ini_start-1 ; k >=0 ; k--)
			{
				thres[k+(i*WD)]=  thres[ini_start+(i*WD)] - (thres[(ini_start+1)+(i*WD)]-thres[(ini_start)+(i*WD)])*count2;				
				count2=count2+1;
			}
			count2=1;
			for (int k=ini_end+1 ; k < WD ; k++)
			{
				thres[k+(i*WD)]=  thres[ini_end+(i*WD)] + (thres[ini_end+(i*WD)]-thres[(ini_end-1)+(i*WD)])*count2;
				count2=count2+1;
			}
		}
		start=0;
		end=0;
		count=0;
	}
	return thres;
}




