
// TrainerDlg.h : header file
//

#pragma once

#include "cvut.h"
#include "CommonDef.h"

using namespace cv;


// CTrainerDlg dialog
class CTrainerDlg : public CDialogEx
{
// Construction
public:
	CTrainerDlg(CWnd* pParent = NULL);	// standard constructor

	struct TrainData {
		TrainData() { type_gt = -1; }
		VehicleInfo vehicle;
		int type_gt;
		char city[30];
		char station[30];
		vector<int> surf_areas;
	};

// Dialog Data
	enum { IDD = IDD_TRAINER_DIALOG };

protected:
	void EnableItem(UINT ID, bool enable) { GetDlgItem(ID)->EnableWindow(enable); }
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	IplImage* m_pVehiclePic;
	IplImage* m_pVehicleSeg;

	int m_Vehtype;
	int m_VehtypeRadio;

	CButton m_LoadBtn;
	CButton m_ResetBtn;
	CButton m_GotoBtn;
	CButton m_VehiclePic;
	CButton m_VehicleSeg;
	CButton m_ForwardBtn;
	CButton m_BackwardBtn;

	CEdit m_VehicleIdxEdt;
	CEdit m_DecisionEdt;

	int m_VehIndex;
	vector<TrainData> m_Vehicles;

	// Generated message map functions
	virtual BOOL OnInitDialog();

	bool loadMetaFile();
	bool loadReportFile(char* path);
	void showData(TrainData& vehicle);

public:
	void OnDeinitDialog();
	void SetVehicleIndex(int idx);
	void SetDecision(bool decided);
	void SetVehicleType(int type);
	int GetVehicleIndex();
	std::string int2str_index(int index);
	vector<int> thresh_interpolate(vector<int> thres, int WD, int HT);
	vector<string> valid_paths(CTrainerDlg::TrainData& data);

protected:
	void DrawImageByID(IplImage* pImg, UINT ID);

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	afx_msg void OnBnClickedBrowseReport();
	afx_msg void OnBnClickedLoad();
	afx_msg void OnBnClickedReset();
	afx_msg void OnBnClickedGoto();
	afx_msg void OnBnClickedVehtypeSmall();
	afx_msg void OnBnClickedVehtypeMedium();
	afx_msg void OnBnClickedVehtypeLarge();
	afx_msg void OnBnClickedVehtypeDisgard();
	afx_msg void OnBnClickedForward();
	afx_msg void OnBnClickedBackward();
	afx_msg void OnBnClickedSave();
	afx_msg void OnBnClickedTraining();
	DECLARE_MESSAGE_MAP()
};
